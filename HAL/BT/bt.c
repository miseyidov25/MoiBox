#include "bt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for external functions
extern void puzzle_dispatch(const char *minor, int8_t rssi);
extern void leds_set_distance_color(int8_t tx_power);
extern bool leds_get_sw3_mode(void);
extern void set_ibeacon_seen(bool seen);
extern void set_last_tx_power(int8_t power);

// Local variables
static uint8_t ibeacon_buffer[IBEACON_FRAME_SIZE];
static uint32_t ibeacon_count = 0U;
static char ibeacon_line_buffer[IBEACON_LINE_MAX];
static uint32_t ibeacon_line_count = 0U;
static beacon_t beacons[MAX_BEACONS];
static int beacon_count = 0;

// Initialize BT module
void bt_init(void)
{
    ibeacon_count = 0U;
    ibeacon_line_count = 0U;
    beacon_count = 0;
}

// Parse OK+DISC: line for beacon data
static void parse_at_disc_line(const char *line, uint32_t length)
{
    const char prefix[] = "OK+DISC:";
    const uint32_t prefix_len = sizeof(prefix) - 1U;
    if (length <= prefix_len || strncmp(line, prefix, prefix_len) != 0)
    {
        return;
    }

    const char *field_start = line + prefix_len;
    const char *field_end;
    const char *major_minor_field = NULL;
    uint32_t major_minor_len = 0U;
    const char *mac_field = NULL;
    uint32_t mac_len = 0U;
    const char *rssi_field = NULL;
    uint32_t field_index = 0U;

    while (field_start < line + length && field_index < 5U)
    {
        field_end = memchr(field_start, ':', (line + length) - field_start);
        if (field_end == NULL)
        {
            field_end = line + length;
        }

        if (field_index == 2U)
        {
            major_minor_field = field_start;
            major_minor_len = (uint32_t)(field_end - field_start);
        }
        else if (field_index == 3U)
        {
            mac_field = field_start;
            mac_len = (uint32_t)(field_end - field_start);
        }
        else if (field_index == 4U)
        {
            rssi_field = field_start;
            break;
        }

        if (field_end >= line + length)
        {
            break;
        }

        field_start = field_end + 1U;
        field_index++;
    }

    // Check for special Puzzle 1 beacon (MAJOR 0B01)
    if (major_minor_field && major_minor_len >= 8 && strncmp(major_minor_field, PUZZLE1_SPECIAL_MAJOR, 4) == 0 &&
        mac_field && rssi_field)
    {
        char rssi_text[8];
        uint32_t text_len = (uint32_t)((line + length) - rssi_field);
        if (text_len >= sizeof(rssi_text))
        {
            return;
        }

        memcpy(rssi_text, rssi_field, text_len);
        rssi_text[text_len] = '\0';

        char *endptr = NULL;
        long rssi_value = strtol(rssi_text, &endptr, 10);
        if (endptr == rssi_text || *endptr != '\0')
        {
            return;
        }

        if (rssi_value < -128 || rssi_value > 127)
        {
            return;
        }

        int8_t rssi = (int8_t)rssi_value;
        puzzle_dispatch(PUZZLE1_MINOR, rssi);
        return;
    }

    if (major_minor_field && major_minor_len >= 8 && strncmp(major_minor_field, "0AEA", 4) == 0 &&
        mac_field && rssi_field)
    {
        char minor[5];
        memcpy(minor, major_minor_field + 4, 4);
        minor[4] = '\0';

        char rssi_text[8];
        uint32_t text_len = (uint32_t)((line + length) - rssi_field);
        if (text_len >= sizeof(rssi_text))
        {
            return;
        }

        memcpy(rssi_text, rssi_field, text_len);
        rssi_text[text_len] = '\0';

        char *endptr = NULL;
        long rssi_value = strtol(rssi_text, &endptr, 10);
        if (endptr == rssi_text || *endptr != '\0')
        {
            return;
        }

        if (rssi_value < -128 || rssi_value > 127)
        {
            return;
        }

        int8_t rssi = (int8_t)rssi_value;

        if (beacon_count < MAX_BEACONS)
        {
            memcpy(beacons[beacon_count].minor, minor, 5);
            memcpy(beacons[beacon_count].mac, mac_field, mac_len);
            beacons[beacon_count].mac[mac_len] = '\0';
            beacons[beacon_count].rssi = rssi;
            beacon_count++;
        }
    }
}

// Handle discovery end (OK+DISCE message)
void bt_handle_discovery_end(void)
{
    printf("\r\n");
    bool target_found = false;
    int8_t last_detected_rssi = 0;
    
    for (int i = 0; i < beacon_count; i++)
    {
        if (strcmp(beacons[i].minor, "0037") == 0)
        {
            const char *range_label;
            int8_t r = beacons[i].rssi;
            if (r >= -55)
            {
                range_label = "close";
            }
            else if (r >= -70)
            {
                range_label = "medium";
            }
            else
            {
                range_label = "far";
            }
            printf("Beacon 0AEA%sC5, RSSI %d %s", beacons[i].minor, r, range_label);
            puzzle_dispatch(beacons[i].minor, r);
            target_found = true;
            last_detected_rssi = r;
            break;
        }
    }
    
    if (!target_found)
    {
        printf("\r\n========== PUZZLE 1 ACTIVATED ==========\r\n");
        printf("Math Challenge!\r\n");
        puzzle_dispatch(PUZZLE1_MINOR, 0);
    }
    
    for (int i = 0; i < beacon_count; i++)
    {
        if (strcmp(beacons[i].minor, "0037") != 0)
        {
            const char *range_label;
            int8_t r = beacons[i].rssi;
            if (r >= -55)
            {
                range_label = "close";
            }
            else if (r >= -70)
            {
                range_label = "medium";
            }
            else
            {
                range_label = "far";
            }
            printf("  Beacon 0AEA%sC5 RSSI %d %s", beacons[i].minor, r, range_label);
            puzzle_dispatch(beacons[i].minor, r);
            if (!target_found)
            {
                last_detected_rssi = r;
            }
        }
    }
    printf("\r\n");
    
    if (beacon_count > 0 && !leds_get_sw3_mode())
    {
        leds_set_distance_color(last_detected_rssi);
    }
    
    beacon_count = 0;
}

// Process incoming byte from BLE module (LPUART2)
// Handles both binary iBeacon frames and AT command responses
void bt_process_byte(uint8_t data)
{
    // Binary iBeacon frame detection
    if (ibeacon_count < IBEACON_FRAME_SIZE)
    {
        ibeacon_buffer[ibeacon_count++] = data;
    }
    else
    {
        memmove(ibeacon_buffer, ibeacon_buffer + 1, IBEACON_FRAME_SIZE - 1);
        ibeacon_buffer[IBEACON_FRAME_SIZE - 1] = data;
    }

    if (ibeacon_count >= IBEACON_FRAME_SIZE)
    {
        if (ibeacon_buffer[0] == 0x4CU && ibeacon_buffer[1] == 0x00U &&
            ibeacon_buffer[2] == 0x02U && ibeacon_buffer[3] == 0x15U)
        {
            uint16_t major = (uint16_t)((ibeacon_buffer[20] << 8) | ibeacon_buffer[21]);
            uint16_t minor = (uint16_t)((ibeacon_buffer[22] << 8) | ibeacon_buffer[23]);
            int8_t tx_power = (int8_t)ibeacon_buffer[24];
            if (major == IBEACON_MAJOR_H0_37 && minor == IBEACON_MINOR_H0_37)
            {
                set_ibeacon_seen(true);
                set_last_tx_power(tx_power);
                if (!leds_get_sw3_mode())
                {
                    leds_set_distance_color(tx_power);
                }
            }
        }
    }

    // AT command response line buffering
    if (data == '\r' || data == '\n')
    {
        if (ibeacon_line_count > 0)
        {
            ibeacon_line_buffer[ibeacon_line_count] = '\0';
            parse_at_disc_line(ibeacon_line_buffer, ibeacon_line_count);
            
            if (strcmp(ibeacon_line_buffer, "OK+DISCE") == 0)
            {
                bt_handle_discovery_end();
            }
            
            ibeacon_line_count = 0U;
        }
    }
    else if (ibeacon_line_count + 1 < IBEACON_LINE_MAX)
    {
        ibeacon_line_buffer[ibeacon_line_count++] = (char)data;
    }
}
