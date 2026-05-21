#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct fifo
{
    void *data;
    uint32_t head;
    uint32_t tail;
    uint32_t cnt;
    uint32_t items;
    uint32_t item_size;
    uint32_t mask;
} fifo_t;

void f_init(fifo_t *f, void *buffer, const uint32_t items, const uint32_t item_size_bytes);
bool f_push(fifo_t *f, void *data);
bool f_pop(fifo_t *f, void *data);
uint32_t f_cnt(fifo_t *f);

#endif