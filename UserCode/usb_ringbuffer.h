#ifndef USB_RINGBUFFER_H
#define USB_RINGBUFFER_H

#include <stdint.h>

#define RING_BUF_SIZE 512U
#define RING_BUF_MASK (RING_BUF_SIZE - 1U)

typedef struct {
    uint8_t buffer[RING_BUF_SIZE];
    volatile uint32_t write_idx;
    volatile uint32_t read_idx;
} ring_buf_t;

extern ring_buf_t usb_rx_ring;

void ring_init(ring_buf_t *rb);
uint32_t ring_write(ring_buf_t *rb, const uint8_t *data, uint32_t len);
const uint8_t *ring_peek(ring_buf_t *rb, uint32_t *len);
void ring_advance(ring_buf_t *rb, uint32_t len);

#endif // USB_RINGBUFFER_H
