/*
#include <modbus_ringbuffer.h>
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-09-30     Bernard      first version.
 * 2013-05-08     Grissiom     reimplement
 * 2016-08-18     heyuanjie    add interface
 */
#include <string.h>
#include "modbus_ringbuffer.h"


enum modbus_ringbuffer_state modbus_ringbuffer_status(struct modbus_ringbuffer *rb)
{
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
            return MODBUS_RINGBUFFER_EMPTY;
        else
            return MODBUS_RINGBUFFER_FULL;
    }
    return MODBUS_RINGBUFFER_HALFFULL;
}

void modbus_ringbuffer_init(struct modbus_ringbuffer *rb,uint8_t *pool,int16_t size)
{
    MODBUS_ASSERT(rb != RT_NULL);
    RT_ASSERT(size > 0);

    /* initialize read and write index */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* set buffer pool and size */
    rb->buffer_ptr = pool;
    rb->buffer_size = MODBUS_ALIGN_DOWN(size, 4);
}

/**
 * put a block of data into ring buffer
 */
int modbus_ringbuffer_put(struct modbus_ringbuffer *rb,const uint8_t *ptr,uint16_t length)
{
    uint16_t size;

    MODBUS_ASSERT(rb != RT_NULL);

    /* whether has enough space */
    size = modbus_ringbuffer_space_len(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;
        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    return length;
}

/**
 * put a block of data into ring buffer
 *
 * When the buffer is full, it will discard the old data.
 */
int modbus_ringbuffer_put_force(struct modbus_ringbuffer *rb,const uint8_t *ptr,uint16_t length)
{
    uint16_t space_length;

    MODBUS_ASSERT(rb != RT_NULL);

    space_length = modbus_ringbuffer_space_len(rb);

    if (length > rb->buffer_size)
    {
        ptr = &ptr[length - rb->buffer_size];
        length = rb->buffer_size;
    }

    if (rb->buffer_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;

        if (length > space_length)
            rb->read_index = rb->write_index;

        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    if (length > space_length)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = rb->write_index;
    }

    return length;
}

/**
 *  get data from ring buffer
 */
int modbus_ringbuffer_get(struct modbus_ringbuffer *rb,uint8_t *ptr,uint16_t length)
{
    int size;

    MODBUS_ASSERT(rb != RT_NULL);

    /* whether has enough data  */
    size = modbus_ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &rb->buffer_ptr[rb->read_index],
           rb->buffer_size - rb->read_index);
    memcpy(&ptr[rb->buffer_size - rb->read_index],
           &rb->buffer_ptr[0],
           length - (rb->buffer_size - rb->read_index));

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buffer_size - rb->read_index);

    return length;
}

/**
 * put a character into ring buffer
 */
int modbus_ringbuffer_putchar(struct modbus_ringbuffer *rb, const uint8_t ch)
{
    MODBUS_ASSERT(rb != RT_NULL);

    /* whether has enough space */
    if (!modbus_ringbuffer_space_len(rb))
        return 0;

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * put a character into ring buffer
 *
 * When the buffer is full, it will discard one old data.
 */
int modbus_ringbuffer_putchar_force(struct modbus_ringbuffer *rb, const uint8_t ch)
{
    enum modbus_ringbuffer_state old_state;

    MODBUS_ASSERT(rb != RT_NULL);

    old_state = modbus_ringbuffer_status(rb);

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == MODBUS_RINGBUFFER_FULL)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        if (old_state == MODBUS_RINGBUFFER_FULL)
            rb->read_index = rb->write_index;
    }

    return 1;
}

/**
 * get a character from a ringbuffer
 */
int modbus_ringbuffer_getchar(struct modbus_ringbuffer *rb, uint8_t *ch)
{
    MODBUS_ASSERT(rb != RT_NULL);

    /* ringbuffer is empty */
    if (!modbus_ringbuffer_data_len(rb))
        return 0;

    /* put character */
    *ch = rb->buffer_ptr[rb->read_index];

    if (rb->read_index == rb->buffer_size-1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}

/** 
 * get the size of data in rb 
 */
int modbus_ringbuffer_data_len(struct modbus_ringbuffer *rb)
{
    switch (modbus_ringbuffer_status(rb))
    {
    case MODBUS_RINGBUFFER_EMPTY:
        return 0;
    case MODBUS_RINGBUFFER_FULL:
        return rb->buffer_size;
    case MODBUS_RINGBUFFER_HALFFULL:
    default:
        if (rb->write_index > rb->read_index)
            return rb->write_index - rb->read_index;
        else
            return rb->buffer_size - (rb->read_index - rb->write_index);
    };
}

/** 
 * empty the rb 
 */
void modbus_ringbuffer_reset(struct modbus_ringbuffer *rb)
{
    MODBUS_ASSERT(rb != RT_NULL);

    rb->read_mirror = 0;
    rb->read_index = 0;
    rb->write_mirror = 0;
    rb->write_index = 0;
}
