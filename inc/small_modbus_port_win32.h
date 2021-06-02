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

