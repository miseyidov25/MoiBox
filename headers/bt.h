#ifndef BT_H
#define BT_H

#include <stdint.h>


void bt_init(void);                // set up UART
void bt_send(const char *data);    // send null-terminated string
char bt_receive(void);             // receive single character
uint8_t bt_available(void);        // non-zero if data pending

#endif // BT_H
