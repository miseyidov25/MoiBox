#ifndef BT_H
#define BT_H

#include <stdint.h>
#include <stdbool.h>

// iBeacon and Bluetooth constants
#define IBEACON_MAJOR_H0_37    0x0AEAU
#define IBEACON_MINOR_H0_37    0x0037U
#define IBEACON_FRAME_SIZE     25U
#define IBEACON_LINE_MAX       128U
#define MAX_BEACONS            10

// Puzzle beacon definitions
#define PUZZLE1_MINOR          "0037"
#define PUZZLE1_SPECIAL_MAJOR  "0B01"
#define PUZZLE1_SPECIAL_MINOR  "0001"
#define PUZZLE2_MINOR          "0032"
#define PUZZLE3_MINOR          "0026"
#define PUZZLE4_MINOR          "0029"
#define PUZZLE5_MINOR          "0031"

// Beacon structure
typedef struct {
    char minor[9];
    char mac[13];
    int8_t rssi;
} beacon_t;

// Function declarations
void bt_init(void);
void bt_process_byte(uint8_t data);
void bt_handle_discovery_end(void);

#endif // BT_H
