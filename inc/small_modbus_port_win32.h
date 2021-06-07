#ifndef _SMALL_MODBUS_PORT_WIN32_H_
#define _SMALL_MODBUS_PORT_WIN32_H_

#include "stdint.h"
#include "string.h"
#include "small_modbus_base.h"
#include "small_modbus_port.h"

int modbus_init(small_modbus_t* smb, uint8_t core_type, void* port);
small_modbus_t* modbus_create(uint8_t core_type, void* port);

/*
* modbus on win32
*/
#if SMALL_MODBUS_WIN32

#define BAUD_RATE_2400                  2400
#define BAUD_RATE_4800                  4800
#define BAUD_RATE_9600                  9600
#define BAUD_RATE_19200                 19200
#define BAUD_RATE_38400                 38400
#define BAUD_RATE_57600                 57600
#define BAUD_RATE_115200                115200
#define BAUD_RATE_230400                230400
#define BAUD_RATE_460800                460800
#define BAUD_RATE_921600                921600
#define BAUD_RATE_2000000               2000000
#define BAUD_RATE_3000000               3000000

#define DATA_BITS_5                     5
#define DATA_BITS_6                     6
#define DATA_BITS_7                     7
#define DATA_BITS_8                     8
#define DATA_BITS_9                     9

#define STOP_BITS_1                     0
#define STOP_BITS_2                     1
#define STOP_BITS_3                     2
#define STOP_BITS_4                     3

#ifdef _WIN32
#include <windows.h>
#else
#define PARITY_NONE                     0
#define PARITY_ODD                      1
#define PARITY_EVEN                     2
#endif

#define BIT_ORDER_LSB                   0
#define BIT_ORDER_MSB                   1

#define NRZ_NORMAL                      0       /* Non Return to Zero : normal mode */
#define NRZ_INVERTED                    1       /* Non Return to Zero : inverted mode */

#define SERIAL_RB_BUFSZ              64


/* Default config for serial_configure structure */
#define SERIAL_CONFIG_DEFAULT           \
{                                          \
    BAUD_RATE_115200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    SERIAL_RB_BUFSZ, /* Buffer size */  \
    0                                      \
}

struct serial_configure
{
    uint32_t baud_rate;

    uint32_t data_bits : 4;
    uint32_t stop_bits : 2;
    uint32_t parity : 2;
    uint32_t bit_order : 1;
    uint32_t invert : 1;
    uint32_t bufsz : 16;
    uint32_t reserved : 6;
};



/* ring buffer */
struct win32device_ringbuffer
{
    uint8_t* buffer_ptr;
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


enum win32device_ringbuffer_state
{
    WIN32DEVICE_RINGBUFFER_EMPTY,
    WIN32DEVICE_RINGBUFFER_FULL,
    /* half full is neither full nor empty */
    WIN32DEVICE_RINGBUFFER_HALFFULL,
};

enum win32device_ringbuffer_state win32device_ringbuffer_status(struct win32device_ringbuffer* rb);

void win32device_ringbuffer_init(struct win32device_ringbuffer* rb, uint8_t* pool, int16_t size);
/**
 * put a block of data into ring buffer
 */
size_t win32device_ringbuffer_put(struct win32device_ringbuffer* rb,const uint8_t* ptr,uint16_t length);
/**
 *  get data from ring buffer
 */
size_t win32device_ringbuffer_get(struct win32device_ringbuffer* rb,uint8_t* ptr,uint16_t  length);
/**
 * get the size of data in rb
 */
size_t win32device_ringbuffer_data_len(struct win32device_ringbuffer* rb);
/**
 * empty the rb
 */
void win32device_ringbuffer_reset(struct win32device_ringbuffer* rb);

uint16_t win32device_ringbuffer_get_size(struct win32device_ringbuffer* rb);

/** return the size of empty space in rb */
#define win32device_ringbuffer_space_len(rb) ((rb)->buffer_size - win32device_ringbuffer_data_len(rb))


#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif

#if defined(_WIN32)
#include <windows.h>
#else
#include <termios.h>
#endif

#if defined(_WIN32)
#if !defined(ENOTSUP)
#define ENOTSUP WSAEOPNOTSUPP
#endif

/* WIN32: struct containing serial handle and a receive buffer */
#define PY_BUF_SIZE 512
//struct win32_ser {
//    /* File handle */
//    HANDLE fd;
//    /* Receive buffer */
//    uint8_t buf[PY_BUF_SIZE];
//    /* Received chars */
//    DWORD n_bytes;
//};
#endif /* _WIN32 */

typedef struct _small_modbus_port_win32device   small_modbus_port_win32device_t;

struct _small_modbus_port_win32device
{
    struct _small_modbus_port 	base;
    struct serial_configure     serial_config;
    const char*                 device_name;
    int                         (*rts_set)(int on);

    struct win32device_ringbuffer rx_ringbuff;

    uint8_t __rx_ringbuff_data[256];

#if defined(_WIN32)
    //struct win32_ser w_ser;
    /* File handle */
    HANDLE fd;
    /* Receive buffer */
    uint8_t read_buff[PY_BUF_SIZE];
    /* Received chars */
    uint16_t read_buff_pos;
    uint16_t read_buff_len;

    DCB old_dcb;
#else
    /* Save old termios settings */
    struct termios old_tios;
#endif

};

int modbus_port_win32device_init(small_modbus_port_win32device_t* port, const char* device_name);
small_modbus_port_win32device_t* modbus_port_win32device_create(const char* device_name);
small_modbus_port_win32device_t* modbus_port_win32device_get(small_modbus_t* smb);

int modbus_set_rts(small_modbus_t* smb, int (*rts_set)(int on));
int modbus_set_serial_config(small_modbus_t* smb, struct serial_configure* serial_config);
int modbus_set_oflag(small_modbus_t* smb, int oflag);

#endif

#endif /* _SMALL_MODBUS_PORT_WIN32_H_ */

