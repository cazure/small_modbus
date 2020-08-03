/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "modbus.h"

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
 */
#define MODBUS_RTU_MAX_ADU_LENGTH  256

#define _MODBUS_RTU_HEADER_LENGTH      1
#define _MODBUS_RTU_PRESET_REQ_LENGTH  6
#define _MODBUS_RTU_PRESET_RSP_LENGTH  2

#define _MODBUS_RTU_CHECKSUM_LENGTH    2


/* It's not really the minimal length (the real one is report slave ID
 * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
 * communications to read many values or write a single one.
 * Maximum between :
 * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
 * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
 */
#define _MIN_REQ_LENGTH 12

#define _REPORT_SLAVE_ID 180

#define _MODBUS_EXCEPTION_RSP_LENGTH 5


int modbus_rtu_init(modbus_t *ctx,modbus_backend_t *backend,void *config);

int modbus_rtu_set_open_ops(modbus_t *ctx, int (*open)(modbus_t *ctx));
int modbus_rtu_set_close_ops(modbus_t *ctx, int (*close)(modbus_t *ctx));
int modbus_rtu_set_read_ops(modbus_t *ctx, int (*read)(modbus_t *ctx,uint8_t *data,uint16_t length));
int modbus_rtu_set_write_ops(modbus_t *ctx, int (*write)(modbus_t *ctx,uint8_t *data,uint16_t length));
int modbus_rtu_set_flush_ops(modbus_t *ctx, int (*flush)(modbus_t *ctx));
int modbus_rtu_set_select_ops(modbus_t *ctx, int (*select)(modbus_t *ctx,int timeout_ms));
int modbus_rtu_set_read_timeout(modbus_t *ctx,int timeout_ms);
int modbus_rtu_set_write_timeout(modbus_t *ctx,int timeout_ms);


#ifdef MODBUS_USED_RTOS

#include <rtthread.h>
#include <dfs_posix.h>
#include <termios.h>

typedef struct _modbus_rtu_config {
    char *device;
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;

    /* Save old termios settings */
    struct termios old_tios;

    int (*rts_set)(modbus_t *ctx, int on);
} modbus_rtu_config_t;

int modbus_rtu_config(modbus_t *ctx,char *device,int baud,uint8_t data_bit, uint8_t stop_bit,char parity);
int modbus_rtu_set_rts_ops(modbus_t *ctx,int (*rts_set)(modbus_t *ctx, int on));

#endif

extern const modbus_core_t modbus_rtu_core;

#endif

