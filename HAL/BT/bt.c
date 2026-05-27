#include "HAL/BT/bt.h"

#include <stdint.h>
#include <stdbool.h>

#include "serial.h"
#include "HAL/BT/lpuart2.h"

/*
 * HM-10 wiring:
 *
 * HM-10 TXD -> P1_4 / LPUART2_RXD
 * HM-10 RXD -> P1_5 / LPUART2_TXD
 * HM-10 VCC -> 3.3V
 * HM-10 GND -> GND
 *
 * This file uses the same LPUART2 style as the working serial2lpuart project.
 */

#define BT_UART_BAUD              9600u

/*
 * Your working detected iBeacon:
 *
 * OK+DISC:4C000215:74278BDAB64445208F0C720EAF059935:0AEA0026C5:685E1C1A68F3:-064
 *
 * major = 0x0AEA
 * minor = 0x0026
 */
// #define BT_EXPECTED_MAJOR         0x0AEAu //HAN
#define BT_EXPECTED_MAJOR         0x0B01u    //Brokhorst

#define BT_RSSI_MIN              (-90)
#define BT_DEBOUNCE_TICKS        20u

#define BT_SCAN_INTERVAL_MS       8000u
#define BT_LINE_IDLE_TIMEOUT_MS   200u

#define BT_LINE_BUFFER_SIZE       220u

static uint16_t expected_minor[5] =
{
    0x0001u,  /* Location 1 */
    0x0003u,  /* Location 2 */
    0x0006u,  /* Location 3 */
    0x0008u,  /* Location 4 */
    0x000Au   /* Location 5 */
};

static bool scanning_enabled = false;

static bool pending_event_ready = false;
static app_event_t pending_event;

static uint16_t last_minor = 0u;
static uint32_t debounce_counter = 0u;

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
        return -50;
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
        return -50;
    }

    return sign * value;
}

static app_event_type_t minor_to_event(uint16_t minor)
{
    if (minor == expected_minor[0])
    {
        return EVENT_BEACON_1_DETECTED;
    }

    if (minor == expected_minor[1])
    {
        return EVENT_BEACON_2_DETECTED;
    }

    if (minor == expected_minor[2])
    {
        return EVENT_BEACON_3_DETECTED;
    }

    if (minor == expected_minor[3])
    {
        return EVENT_BEACON_4_DETECTED;
    }

    if (minor == expected_minor[4])
    {
        return EVENT_BEACON_5_DETECTED;
    }

    return EVENT_NONE;
}

/*
 * Parses:
 *
 * OK+DISC:4C000215:UUID32:MAJOR4MINOR4TX2:MAC12:RSSI
 *
 * Example:
 * OK+DISC:4C000215:74278BDAB64445208F0C720EAF059935:0AEA0026C5:685E1C1A68F3:-064
 */
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

    /*
     * iBeacon prefix.
     * Ignore normal BLE devices with 00000000 or other prefixes.
     */
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
        lpuart2_putchar((uint8_t)*cmd);
        cmd++;
    }
}

static void process_bt_line(const char *line)
{
    uint16_t major;
    uint16_t minor;
    uint8_t tx_power;
    const char *uuid_start;
    const char *mac_start;
    int rssi;

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
        print_serial("\r\n");

        bt_detect_beacon(major, minor, rssi);
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

    last_scan_ms = 0u;
    last_rx_ms = 0u;
    line_index = 0u;

    lpuart2_init(BT_UART_BAUD);

    print_serial("BT: active HM-10 scanner initialized\r\n");
    print_serial("BT: expected major ");
    print_hex4(BT_EXPECTED_MAJOR);
    print_serial("\r\n");

    bt_send_init_commands();

    /*
     * Important:
     * Do NOT start scanning here.
     * The FSM enables scanning only while waiting for a location.
     */
}

void bt_update(void)
{
    uint32_t now = ms;

    if (debounce_counter > 0u)
    {
        debounce_counter--;
    }

    /*
     * Always drain the LPUART2 FIFO.
     *
     * If scanning is enabled, parse received HM-10 data.
     * If scanning is disabled, discard it so old scan results do not
     * start puzzles while a puzzle is active.
     */
    while (lpuart2_rxcnt() > 0u)
    {
        char c = (char)lpuart2_getchar();

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
    app_event_type_t event_type;

    if (!scanning_enabled)
    {
        return;
    }

    if (major != BT_EXPECTED_MAJOR)
    {
        print_serial("BT: iBeacon ignored, wrong major\r\n");
        return;
    }

    if (rssi < BT_RSSI_MIN)
    {
        print_serial("BT: iBeacon ignored, weak RSSI\r\n");
        return;
    }

    if ((minor == last_minor) && (debounce_counter > 0u))
    {
        return;
    }

    event_type = minor_to_event(minor);

    if (event_type == EVENT_NONE)
    {
        print_serial("BT: iBeacon ignored, unknown minor\r\n");
        return;
    }

    pending_event.type = event_type;
    pending_event.keypad_key = '\0';
    pending_event.rssi = (int8_t)rssi;

    pending_event_ready = true;

    last_minor = minor;
    debounce_counter = BT_DEBOUNCE_TICKS;

    print_serial("BT: location event queued\r\n");
}

void bt_set_expected_minor_for_puzzle(uint8_t puzzle_number, uint16_t minor)
{
    if ((puzzle_number < 1u) || (puzzle_number > 5u))
    {
        return;
    }

    expected_minor[puzzle_number - 1u] = minor;
}

uint16_t bt_get_expected_minor_for_puzzle(uint8_t puzzle_number)
{
    if ((puzzle_number < 1u) || (puzzle_number > 5u))
    {
        return 0u;
    }

    return expected_minor[puzzle_number - 1u];
}

void bt_debug_simulate_minor(uint16_t minor)
{
    /*
     * Debug simulate ignores HM-10 data but still requires scanning enabled,
     * because it represents a location detection.
     */
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