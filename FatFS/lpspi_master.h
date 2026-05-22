#ifndef LPSPI_MASTER_H
#define LPSPI_MASTER_H

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

void lpspi_master_init(void);

bool lpspi_busy(void);

void lpspi_transmit(uint8_t *tx_buffer, const uint32_t n);
void lpspi_receive(uint8_t *rx_buffer, const uint32_t n);
void lpspi_transceive(uint8_t *tx_buffer, uint8_t *rx_buffer, const uint32_t n);

void lpspi_set_dummy(uint8_t dummy);

#endif