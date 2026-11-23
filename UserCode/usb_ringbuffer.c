#include "usb_ringbuffer.h"
#include "main.h"

ring_buf_t usb_rx_ring;

void ring_init(ring_buf_t *rb)
{
    if (rb != 0) {
        rb->write_idx = 0U;
        rb->read_idx = 0U;
    }
}

uint32_t ring_write(ring_buf_t *rb, const uint8_t *data, uint32_t len)
{
    uint32_t write_idx;
    uint32_t read_idx;
    uint32_t used;
    uint32_t writable;
    uint32_t count;
    uint32_t i;

    if ((rb == 0) || (data == 0) || (len == 0U)) {
        return 0U;
    }

    write_idx = rb->write_idx;
    read_idx = rb->read_idx;
    __DMB();

    used = write_idx - read_idx;
    if (used >= RING_BUF_SIZE) {
        return 0U;
    }

    writable = RING_BUF_SIZE - used;
    count = (len < writable) ? len : writable;
    for (i = 0U; i < count; i++) {
        rb->buffer[(write_idx + i) & RING_BUF_MASK] = data[i];
    }

    __DMB();
    rb->write_idx = write_idx + count;
    return count;
}

const uint8_t *ring_peek(ring_buf_t *rb, uint32_t *len)
{
    uint32_t write_idx;
    uint32_t read_idx;
    uint32_t available;
    uint32_t contiguous;

    if (len == 0) {
        return 0;
    }
    *len = 0U;

    if (rb == 0) {
        return 0;
    }

    write_idx = rb->write_idx;
    __DMB();
    read_idx = rb->read_idx;
    available = write_idx - read_idx;
    if (available == 0U) {
        return 0;
    }

    contiguous = RING_BUF_SIZE - (read_idx & RING_BUF_MASK);
    *len = (available < contiguous) ? available : contiguous;
    return &rb->buffer[read_idx & RING_BUF_MASK];
}

void ring_advance(ring_buf_t *rb, uint32_t len)
{
    uint32_t read_idx;
    uint32_t available;

    if ((rb == 0) || (len == 0U)) {
        return;
    }

    read_idx = rb->read_idx;
    available = rb->write_idx - read_idx;
    if (len > available) {
        len = available;
    }

    __DMB();
    rb->read_idx = read_idx + len;
}


