#include <MCXA153.h>
#include "fifo.h"

void f_init(fifo_t *f, void *buffer, const uint32_t items, const uint32_t item_size)
{
#ifdef DEBUG
    if ((items & (items - 1u)) != 0u)
    {
        while (1)
        {
        }
    }
#endif

    f->data = buffer;
    f->head = 0u;
    f->tail = 0u;
    f->cnt = 0u;
    f->items = items;
    f->item_size = item_size;
    f->mask = items - 1u;
}

bool f_push(fifo_t *f, void *data)
{
    if (f->cnt < f->items)
    {
        uint32_t m = __get_PRIMASK();
        __disable_irq();

        f->head = (f->head + 1u) & f->mask;

        memcpy(
            &(((uint8_t *)(f->data))[f->head * f->item_size]),
            data,
            f->item_size
        );

        f->cnt++;

        __set_PRIMASK(m);

        return true;
    }

    return false;
}

bool f_pop(fifo_t *f, void *data)
{
    if (f->cnt > 0u)
    {
        uint32_t m = __get_PRIMASK();
        __disable_irq();

        f->tail = (f->tail + 1u) & f->mask;

        memcpy(
            data,
            &(((uint8_t *)(f->data))[f->tail * f->item_size]),
            f->item_size
        );

        f->cnt--;

        __set_PRIMASK(m);

        return true;
    }

    return false;
}

uint32_t f_cnt(fifo_t *f)
{
    return f->cnt;
}