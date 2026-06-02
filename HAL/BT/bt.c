#include "HAL/BT/bt.h"

#include <stdint.h>
#include <stdbool.h>

#include "serial.h"
#include "HAL/BT/lpuart1.h"
#include "HAL/Display/oled.h"
#include "HAL/Display/leds.h"
#include "App/fsm.h"

/*
 * HM-10 wiring:
 *
 * HM-10 TXD -> P3_8 / LPUART1_RXD
 * HM-10 RXD -> P3_9 / LPUART1_TXD
 * HM-10 VCC -> 3.3V
 * HM-10 GND -> GND
 */

#define BT_UART_BAUD              9600u

/*
 * ============================================================
 * BEACON SET SELECT
 * ============================================================
 *
 * For now this is set to HAN test mode.
 *
 * HAN test mode:
 * Major: 0x0AEA
 * Puzzle 1 -> 0x0037
 * Puzzle 2 -> 0x0032
 * Puzzle 3 -> 0x0026
 * Puzzle 4 -> 0x0032
 * Puzzle 5 -> 0x0037
 *
 * For the final Bronkhorst route, change:
 *
 * #define BT_USE_HAN_TEST_BEACONS 1
 *
 * to:
 *
 * #define BT_USE_HAN_TEST_BEACONS 0
 */

#define BT_USE_HAN_TEST_BEACONS   1

#if BT_USE_HAN_TEST_BEACONS

#define BT_EXPECTED_MAJOR         0x0AEAu

static uint16_t puzzle_minor[5] =
{
    0x0037u,  /* Puzzle 1 */
    0x0032u,  /* Puzzle 2 */
    0x0026u,  /* Puzzle 3 */
    0x0032u,  /* Puzzle 4 */
    0x0037u   /* Puzzle 5 */
};

#else

#define BT_EXPECTED_MAJOR         0x0B01u

/*
 * Final Bronkhorst route:
 *
 * 0x0001 no puzzle
 * 0x0002
 * 0x0003 no puzzle
 * 0x0004
 * 0x0005
 * 0x0006 puzzle 1
 * 0x0005
 * 0x0004
 * 0x0003 puzzle 2
 * 0x0004 or straight to 0x0007
 * 0x0007
 * 0x0008 no puzzle
 * 0x0009
 * 0x000A puzzle 3
 * 0x0009
 * 0x0008 puzzle 4
 * 0x0007
 * 0x0004
 * 0x0003
 * 0x0002
 * 0x0001 puzzle 5 / box open
 */

static uint16_t puzzle_minor[5] =
{
    0x0006u,  /* Puzzle 1 */
    0x0003u,  /* Puzzle 2 */
    0x000Au,  /* Puzzle 3 */
    0x0008u,  /* Puzzle 4 */
    0x0001u   /* Puzzle 5 */
};

#endif

#define BT_THERE_RSSI             (-70)
#define BT_CLOSER_RSSI            (-85)
#define BT_FAR_RSSI               (-95)

#define BT_DEBOUNCE_TICKS         20u

#define BT_SCAN_INTERVAL_MS       8000u
#define BT_LINE_IDLE_TIMEOUT_MS   200u

#define BT_LINE_BUFFER_SIZE       220u

typedef enum
{
    BT_ZONE_OUT = 0,
    BT_ZONE_FAR,
    BT_ZONE_CLOSER,
    BT_ZONE_THERE
} bt_zone_t;

static bool scanning_enabled = false;

static bool pending_event_ready = false;
static app_event_t pending_event;

static uint16_t last_minor = 0u;
static uint32_t debounce_counter = 0u;

static uint16_t last_route_minor = 0u;
static uint32_t route_message_cooldown = 0u;

static uint32_t last_scan_ms = 0u;
static uint32_t last_rx_ms = 0u;

static char line_buffer[BT_LINE_BUFFER_SIZE];
static uint32_t line_index = 0u;

extern volatile uint32_t ms;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void print_int(int value)
{
    char buffer[12];
    int index = 0;

    if (value < 0)
    {
        serial_putchar('-');
        value = -value;
    }

    if (value == 0)
    {
        serial_putchar('0');
        return;
    }

    while ((value > 0) && (index < 11))
    {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
    {
        serial_putchar(buffer[--index]);
    }
}

static void print_uint(uint32_t value)
{
    char buffer[12];
    int index = 0;

    if (value == 0u)
    {
        serial_putchar('0');
        return;
    }

    while ((value > 0u) && (index < 11))
    {
        buffer[index++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (index > 0)
    {
        serial_putchar(buffer[--index]);
    }
}

static void print_hex2(uint8_t value)
{
    const char hex[] = "0123456789ABCDEF";

    serial_putchar(hex[(value >> 4) & 0x0F]);
    serial_putchar(hex[value & 0x0F]);
}

static void print_hex4(uint16_t value)
{
    print_serial("0x");
    print_hex2((uint8_t)(value >> 8));
    print_hex2((uint8_t)value);
}

static int is_hex_char(char c)
{
    return ((c >= '0') && (c <= '9')) ||
           ((c >= 'A') && (c <= 'F')) ||
           ((c >= 'a') && (c <= 'f'));
}

static uint8_t hex_value(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return (uint8_t)(c - '0');
    }

    if ((c >= 'A') && (c <= 'F'))
    {
        return (uint8_t)(c - 'A' + 10);
    }

    if ((c >= 'a') && (c <= 'f'))
    {
        return (uint8_t)(c - 'a' + 10);
    }

    return 0u;
}

static int parse_hex4_at(const char *s, uint16_t *value)
{
    uint16_t result = 0u;

    if ((s == 0) || (value == 0))
    {
        return 0;
    }

    for (uint32_t i = 0u; i < 4u; i++)
    {
        if (!is_hex_char(s[i]))
        {
            return 0;
        }

        result = (uint16_t)((result << 4) | hex_value(s[i]));
    }

    *value = result;
    return 1;
}

static int parse_hex2_at(const char *s, uint8_t *value)
{
    if ((s == 0) || (value == 0))
    {
        return 0;
    }

    if (!is_hex_char(s[0]) || !is_hex_char(s[1]))
    {
        return 0;
    }

    *value = (uint8_t)((hex_value(s[0]) << 4) | hex_value(s[1]));
    return 1;
}

static int chars_equal_ignore_case(char a, char b)
{
    if ((a >= 'a') && (a <= 'z'))
    {
        a = (char)(a - 'a' + 'A');
    }

    if ((b >= 'a') && (b <= 'z'))
    {
        b = (char)(b - 'a' + 'A');
    }

    return a == b;
}

static const char *find_text_ignore_case(const char *text, const char *pattern)
{
    if ((text == 0) || (pattern == 0))
    {
        return 0;
    }

    while (*text != '\0')
    {
        const char *t = text;
        const char *p = pattern;

        while ((*t != '\0') &&
               (*p != '\0') &&
               chars_equal_ignore_case(*t, *p))
        {
            t++;
            p++;
        }

        if (*p == '\0')
        {
            return text;
        }

        text++;
    }

    return 0;
}

static const char *find_next_colon(const char *s)
{
    if (s == 0)
    {
        return 0;
    }

    while (*s != '\0')
    {
        if (*s == ':')
        {
            return s;
        }

        s++;
    }

    return 0;
}

static int parse_rssi(const char *s)
{
    int sign = 1;
    int value = 0;
    int found_digit = 0;

    if (s == 0)
    {
        return -100;
    }

    if (*s == '-')
    {
        sign = -1;
        s++;
    }

    while ((*s >= '0') && (*s <= '9'))
    {
        found_digit = 1;
        value = (value * 10) + (*s - '0');
        s++;
    }

    if (!found_digit)
    {
        return -100;
    }

    return sign * value;
}

static bt_zone_t rssi_to_zone(int rssi)
{
    if (rssi >= BT_THERE_RSSI)
    {
        return BT_ZONE_THERE;
    }

    if (rssi >= BT_CLOSER_RSSI)
    {
        return BT_ZONE_CLOSER;
    }

    if (rssi >= BT_FAR_RSSI)
    {
        return BT_ZONE_FAR;
    }

    return BT_ZONE_OUT;
}

static const char *zone_to_text(bt_zone_t zone)
{
    switch (zone)
    {
        case BT_ZONE_THERE:
            return "THERE 0-3m";

        case BT_ZONE_CLOSER:
            return "CLOSE 3-9m";

        case BT_ZONE_FAR:
            return "FAR 9m+";

        default:
            return "OUT OF RANGE";
    }
}

static uint16_t expected_minor_for_current_target(void)
{
    uint8_t current_target = fsm_get_current_location();

    if ((current_target < 1u) || (current_target > 5u))
    {
        return 0u;
    }

    return puzzle_minor[current_target - 1u];
}

static app_event_type_t current_target_to_event(void)
{
    uint8_t current_target = fsm_get_current_location();

    switch (current_target)
    {
        case 1u:
            return EVENT_BEACON_1_DETECTED;

        case 2u:
            return EVENT_BEACON_2_DETECTED;

        case 3u:
            return EVENT_BEACON_3_DETECTED;

        case 4u:
            return EVENT_BEACON_4_DETECTED;

        case 5u:
            return EVENT_BEACON_5_DETECTED;

        default:
            return EVENT_NONE;
    }
}

static int is_route_minor_for_current_target(uint16_t minor)
{
#if BT_USE_HAN_TEST_BEACONS

    /*
     * HAN test mode:
     * Any known HAN minor that is not the current target minor
     * becomes a route hint. This avoids the unused target variable
     * warning when HAN mode is enabled.
     */
    if (minor == expected_minor_for_current_target())
    {
        return 0;
    }

    if ((minor == 0x0037u) || (minor == 0x0032u) || (minor == 0x0026u))
    {
        return 1;
    }

    return 0;

#else

    uint8_t target = fsm_get_current_location();

    if (target == 1u)
    {
        return (minor == 0x0001u) ||
               (minor == 0x0002u) ||
               (minor == 0x0003u) ||
               (minor == 0x0004u) ||
               (minor == 0x0005u);
    }

    if (target == 2u)
    {
        return (minor == 0x0005u) ||
               (minor == 0x0004u);
    }

    if (target == 3u)
    {
        return (minor == 0x0004u) ||
               (minor == 0x0007u) ||
               (minor == 0x0008u) ||
               (minor == 0x0009u);
    }

    if (target == 4u)
    {
        return (minor == 0x0009u);
    }

    if (target == 5u)
    {
        return (minor == 0x0007u) ||
               (minor == 0x0004u) ||
               (minor == 0x0003u) ||
               (minor == 0x0002u);
    }

    return 0;

#endif
}

static void oled_show_target_beacon(uint8_t target_location, uint16_t minor, bt_zone_t zone)
{
    (void)minor;
    
    oled_clear();

    oled_display_string(0, 0, "TARGET FOUND");
    oled_display_string(1, 0, "PUZZLE");
    oled_display_value(1, 7, target_location);

    if (zone == BT_ZONE_THERE)
    {
        oled_display_string(2, 0, "SIGNAL: THERE");
        oled_display_string(3, 0, "STARTING...");
    }
    else if (zone == BT_ZONE_CLOSER)
    {
        oled_display_string(2, 0, "SIGNAL: CLOSE");
        oled_display_string(3, 0, "MOVE CLOSER");
    }
    else if (zone == BT_ZONE_FAR)
    {
        oled_display_string(2, 0, "SIGNAL: FAR");
        oled_display_string(3, 0, "KEEP LOOKING");
    }
    else
    {
        oled_display_string(2, 0, "SIGNAL: OUT");
        oled_display_string(3, 0, "KEEP LOOKING");
    }
}

static void oled_show_right_direction(uint8_t target_location, uint16_t minor, bt_zone_t zone)
{
    oled_clear();

    oled_display_string(0, 0, "RIGHT DIRECTION");
    oled_display_string(1, 0, "GO TO LOC");
    oled_display_value(1, 10, target_location);

    if (zone == BT_ZONE_THERE)
    {
        oled_display_string(3, 0, "VERY CLOSE");
    }
    else if (zone == BT_ZONE_CLOSER)
    {
        oled_display_string(3, 0, "CLOSE");
    }
    else if (zone == BT_ZONE_FAR)
    {
        oled_display_string(3, 0, "FAR");
    }
    else
    {
        oled_display_string(3, 0, "WEAK");
    }
}

static bool parse_hm10_ibeacon_line(
    const char *line,
    uint16_t *major,
    uint16_t *minor,
    uint8_t *tx_power,
    const char **uuid_start,
    const char **mac_start,
    int *rssi
)
{
    const char *p;
    const char *field_company;
    const char *field_uuid;
    const char *field_data;
    const char *field_mac;
    const char *field_rssi;
    const char *colon;

    if ((line == 0) || (major == 0) || (minor == 0) ||
        (tx_power == 0) || (uuid_start == 0) ||
        (mac_start == 0) || (rssi == 0))
    {
        return false;
    }

    p = find_text_ignore_case(line, "OK+DISC:");

    if (p == 0)
    {
        return false;
    }

    field_company = p + 8;

    if (find_text_ignore_case(field_company, "4C000215") != field_company)
    {
        return false;
    }

    colon = find_next_colon(field_company);
    if (colon == 0)
    {
        return false;
    }

    field_uuid = colon + 1;

    for (uint32_t i = 0u; i < 32u; i++)
    {
        if (!is_hex_char(field_uuid[i]))
        {
            return false;
        }
    }

    colon = find_next_colon(field_uuid);
    if (colon == 0)
    {
        return false;
    }

    field_data = colon + 1;

    if (!parse_hex4_at(field_data, major))
    {
        return false;
    }

    if (!parse_hex4_at(field_data + 4, minor))
    {
        return false;
    }

    if (!parse_hex2_at(field_data + 8, tx_power))
    {
        return false;
    }

    colon = find_next_colon(field_data);
    if (colon == 0)
    {
        return false;
    }

    field_mac = colon + 1;

    colon = find_next_colon(field_mac);
    if (colon == 0)
    {
        return false;
    }

    field_rssi = colon + 1;

    *uuid_start = field_uuid;
    *mac_start = field_mac;
    *rssi = parse_rssi(field_rssi);

    return true;
}

static void bt_print_fixed_field(const char *start, uint32_t length)
{
    for (uint32_t i = 0u; i < length; i++)
    {
        serial_putchar(start[i]);
    }
}

static void send_at_command(const char *cmd)
{
    if (cmd == 0)
    {
        return;
    }

    while (*cmd != '\0')
    {
        lpuart1_putchar((uint8_t)*cmd);
        cmd++;
    }
}

static void show_route_message(uint16_t minor, int rssi)
{
    uint8_t target;
    bt_zone_t zone;

    target = fsm_get_current_location();
    zone = rssi_to_zone(rssi);

    if (zone == BT_ZONE_OUT)
    {
        print_serial("BT: route beacon ignored, too weak\r\n");
        return;
    }

    if ((last_route_minor == minor) && (route_message_cooldown > 0u))
    {
        return;
    }

    last_route_minor = minor;
    route_message_cooldown = 3u;

    print_serial("BT: route beacon ");
    print_hex4(minor);
    print_serial(" -> right direction for puzzle ");
    print_uint(target);
    print_serial(" | ");
    print_serial(zone_to_text(zone));
    print_serial("\r\n");

    oled_show_right_direction(target, minor, zone);
}

static void process_expected_puzzle_beacon(uint16_t minor, int rssi)
{
    uint8_t target;
    uint16_t expected_minor;
    bt_zone_t zone;
    app_event_type_t event_type;

    target = fsm_get_current_location();
    expected_minor = expected_minor_for_current_target();
    zone = rssi_to_zone(rssi);

    if (expected_minor == 0u)
    {
        return;
    }

    if (minor != expected_minor)
    {
        return;
    }

    oled_show_target_beacon(target, minor, zone);

    if ((zone == BT_ZONE_CLOSER) || (zone == BT_ZONE_THERE))
    {
        leds_set_current_location_blink(target);
    }

    if (zone == BT_ZONE_OUT)
    {
        print_serial("BT: target beacon ignored, out of range\r\n");
        return;
    }

    if (zone != BT_ZONE_THERE)
    {
        print_serial("BT: target beacon seen, not close enough to start puzzle\r\n");
        return;
    }

    if ((minor == last_minor) && (debounce_counter > 0u))
    {
        return;
    }

    event_type = current_target_to_event();

    if (event_type == EVENT_NONE)
    {
        return;
    }

    pending_event.type = event_type;
    pending_event.keypad_key = '\0';
    pending_event.rssi = (int8_t)rssi;

    pending_event_ready = true;

    last_minor = minor;
    debounce_counter = BT_DEBOUNCE_TICKS;

    print_serial("BT: target puzzle event queued\r\n");
}

static void process_bt_line(const char *line)
{
    uint16_t major;
    uint16_t minor;
    uint8_t tx_power;
    const char *uuid_start;
    const char *mac_start;
    int rssi;
    bt_zone_t zone;
    uint16_t expected_minor;

    if ((line == 0) || (line[0] == '\0'))
    {
        return;
    }

    if (parse_hm10_ibeacon_line(
            line,
            &major,
            &minor,
            &tx_power,
            &uuid_start,
            &mac_start,
            &rssi))
    {
        zone = rssi_to_zone(rssi);
        expected_minor = expected_minor_for_current_target();

        print_serial("\r\nBT: iBeacon detected\r\n");

        print_serial("UUID: ");
        bt_print_fixed_field(uuid_start, 32u);
        print_serial("\r\n");

        print_serial("Major: ");
        print_hex4(major);
        print_serial("  Minor: ");
        print_hex4(minor);
        print_serial("  TX: 0x");
        print_hex2(tx_power);
        print_serial("\r\n");

        print_serial("MAC: ");
        bt_print_fixed_field(mac_start, 12u);
        print_serial("  RSSI: ");
        print_int(rssi);
        print_serial("  Zone: ");
        print_serial(zone_to_text(zone));
        print_serial("\r\n");

        if (major != BT_EXPECTED_MAJOR)
        {
            print_serial("BT: ignored, wrong major\r\n");
            return;
        }

        if (minor == expected_minor)
        {
            process_expected_puzzle_beacon(minor, rssi);
            return;
        }

        if (is_route_minor_for_current_target(minor))
        {
            show_route_message(minor, rssi);
            return;
        }

        print_serial("BT: beacon ignored, not on current route\r\n");
    }
}

static void bt_finish_current_line(void)
{
    if (line_index == 0u)
    {
        return;
    }

    line_buffer[line_index] = '\0';
    process_bt_line(line_buffer);
    line_index = 0u;
}

static void bt_process_received_char(char c)
{
    last_rx_ms = ms;

    if ((c == '\r') || (c == '\n'))
    {
        bt_finish_current_line();
        return;
    }

    if (line_index < (BT_LINE_BUFFER_SIZE - 1u))
    {
        line_buffer[line_index++] = c;
    }
    else
    {
        line_index = 0u;
    }
}

static void bt_send_scan_command(void)
{
    print_serial("BT TX: AT+DISI?\r\n");
    send_at_command("AT+DISI?\r\n");
}

static void bt_send_init_commands(void)
{
    print_serial("BT: sending AT+ROLE1\r\n");
    send_at_command("AT+ROLE1\r\n");

    for (volatile int i = 0; i < 50000; i++)
    {
    }

    print_serial("BT: sending AT+IMME1\r\n");
    send_at_command("AT+IMME1\r\n");

    for (volatile int i = 0; i < 50000; i++)
    {
    }

    print_serial("BT: sending AT+RESET\r\n");
    send_at_command("AT+RESET\r\n");

    for (volatile int i = 0; i < 50000; i++)
    {
    }
}

void bt_init(void)
{
    scanning_enabled = false;

    pending_event_ready = false;

    pending_event.type = EVENT_NONE;
    pending_event.keypad_key = '\0';
    pending_event.rssi = 0;

    last_minor = 0u;
    debounce_counter = 0u;

    last_route_minor = 0u;
    route_message_cooldown = 0u;

    last_scan_ms = 0u;
    last_rx_ms = 0u;
    line_index = 0u;

    lpuart1_init(BT_UART_BAUD);

    print_serial("BT: active HM-10 scanner initialized on LPUART1\r\n");
    print_serial("BT: expected major ");
    print_hex4(BT_EXPECTED_MAJOR);
    print_serial("\r\n");

#if BT_USE_HAN_TEST_BEACONS
    print_serial("BT: using HAN test beacon map\r\n");
#else
    print_serial("BT: using Bronkhorst route beacon map\r\n");
#endif

    bt_send_init_commands();
}

void bt_update(void)
{
    uint32_t now = ms;

    if (debounce_counter > 0u)
    {
        debounce_counter--;
    }

    if (route_message_cooldown > 0u)
    {
        route_message_cooldown--;
    }

    while (lpuart1_rxcnt() > 0u)
    {
        char c = (char)lpuart1_getchar();

        if (scanning_enabled)
        {
            bt_process_received_char(c);
        }
    }

    if (!scanning_enabled)
    {
        line_index = 0u;
        return;
    }

    if ((line_index > 0u) &&
        ((now - last_rx_ms) >= BT_LINE_IDLE_TIMEOUT_MS))
    {
        bt_finish_current_line();
    }

    if ((now - last_scan_ms) >= BT_SCAN_INTERVAL_MS)
    {
        last_scan_ms = now;
        bt_send_scan_command();
    }
}

bool bt_get_event(app_event_t *event)
{
    if (event == 0)
    {
        return false;
    }

    if (!pending_event_ready)
    {
        return false;
    }

    *event = pending_event;

    pending_event_ready = false;
    pending_event.type = EVENT_NONE;
    pending_event.keypad_key = '\0';
    pending_event.rssi = 0;

    return true;
}

void bt_detect_beacon(uint16_t major, uint16_t minor, int rssi)
{
    if (!scanning_enabled)
    {
        return;
    }

    if (major != BT_EXPECTED_MAJOR)
    {
        print_serial("BT: iBeacon ignored, wrong major\r\n");
        return;
    }

    if (minor == expected_minor_for_current_target())
    {
        process_expected_puzzle_beacon(minor, rssi);
        return;
    }

    if (is_route_minor_for_current_target(minor))
    {
        show_route_message(minor, rssi);
        return;
    }

    print_serial("BT: iBeacon ignored, not on current route\r\n");
}

void bt_set_expected_minor_for_puzzle(uint8_t puzzle_number, uint16_t minor)
{
    if ((puzzle_number < 1u) || (puzzle_number > 5u))
    {
        return;
    }

    puzzle_minor[puzzle_number - 1u] = minor;
}

uint16_t bt_get_expected_minor_for_puzzle(uint8_t puzzle_number)
{
    if ((puzzle_number < 1u) || (puzzle_number > 5u))
    {
        return 0u;
    }

    return puzzle_minor[puzzle_number - 1u];
}

void bt_debug_simulate_minor(uint16_t minor)
{
    bt_detect_beacon(BT_EXPECTED_MAJOR, minor, -50);
}

void bt_set_scanning_enabled(bool enabled)
{
    if (scanning_enabled == enabled)
    {
        return;
    }

    scanning_enabled = enabled;

    line_index = 0u;
    last_route_minor = 0u;
    route_message_cooldown = 0u;

    if (enabled)
    {
        last_scan_ms = ms;

        print_serial("BT: scanning enabled\r\n");
        bt_send_scan_command();
    }
    else
    {
        print_serial("BT: scanning disabled\r\n");
    }
}

bool bt_is_scanning_enabled(void)
{
    return scanning_enabled;
}