/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */
#ifndef MODBUS_RINGBUFFER_H__
#define MODBUS_RINGBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>

/* ring buffer */
struct modbus_ringbuffer
{
    uint8_t *buffer_ptr;
    /* use the msb of the {read,write}_index as mirror bit. You can see this as
     * if the buffer adds a virtual mirror and the pointers point either to the
     * normal or to the mirrored buffer. If the write_index has the same value
     * with the read_index, but in a different mirror, the buffer is full.
     * While if the write_index and the read_index are the same and within the
     * same mirror, the buffer is empty. The ASCII art of the ringbuffer is:
     *
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     *
     * The tradeoff is we could only use 32KiB of buffer for 16 bit of index.
     * But it should be enough for most of the cases.
     *
     * Ref: http://en.wikipedia.org/wiki/Circular_buffer#Mirroring */
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    int16_t buffer_size;
};

enum modbus_ringbuffer_state
{
    MODBUS_RINGBUFFER_EMPTY,
    MODBUS_RINGBUFFER_FULL,
    /* half full is neither full nor empty */
    MODBUS_RINGBUFFER_HALFFULL,
};

void modbus_ringbuffer_init(struct modbus_ringbuffer *rb, uint8_t *pool, int16_t size);
void modbus_ringbuffer_reset(struct modbus_ringbuffer *rb);
int modbus_ringbuffer_put(struct modbus_ringbuffer *rb, const uint8_t *ptr, uint16_t length);
int modbus_ringbuffer_put_force(struct modbus_ringbuffer *rb, const uint8_t *ptr, uint16_t length);
int modbus_ringbuffer_putchar(struct modbus_ringbuffer *rb, const uint8_t ch);
int modbus_ringbuffer_putchar_force(struct modbus_ringbuffer *rb, const uint8_t ch);
int modbus_ringbuffer_get(struct modbus_ringbuffer *rb, uint8_t *ptr, uint16_t length);
int modbus_ringbuffer_getchar(struct modbus_ringbuffer *rb, uint8_t *ch);
int modbus_ringbuffer_data_len(struct modbus_ringbuffer *rb);

uint16_t modbus_ringbuffer_get_size(struct modbus_ringbuffer *rb)
{
    return rb->buffer_size;
}

/** return the size of empty space in rb */
#define modbus_ringbuffer_space_len(rb) ((rb)->buffer_size - modbus_ringbuffer_data_len(rb))

#define MODBUS_ASSERT(EX)

#define MODBUS_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

#ifdef __cplusplus
}
#endif

#endif
