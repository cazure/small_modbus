/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       chenbin
 * 2020-08-21     Administrator       the first version
 */
#ifndef _SMALL_MODBUS_H_
#define _SMALL_MODBUS_H_

#include "stdint.h"
#include "stdio.h"

#define MODBUS_BROADCAST_ADDRESS 0xFF

typedef struct _small_modbus        small_modbus_t;
typedef struct _small_modbus_core   small_modbus_core_t;
typedef struct _small_modbus_port   small_modbus_port_t;
typedef struct _small_modbus_mapping   small_modbus_mapping_t;


struct _small_modbus_core{
    uint8_t type;
    uint8_t len_header;
    uint8_t len_checksum;
    uint8_t len_max;
    int (*build_req_header)(small_modbus_t *smb,uint8_t *buff,int slave,int fun,int reg,int num);
    int (*build_res_header)(small_modbus_t *smb,uint8_t *buff,int slave,int fun);
    int (*check_send_pre)(small_modbus_t *smb,uint8_t *buff,int length);
    int (*check_wait_poll)(small_modbus_t *smb,uint8_t *buff,int length);
    int (*check_wait_confirm)(small_modbus_t *smb,uint8_t *buff,int length);
//    unsigned int backend_type;
//    unsigned int header_length;
//    unsigned int checksum_length;
//    unsigned int max_adu_length;
//    int (*build_request_basis) (modbus_t *ctx, int function,int regaddr, int nb,uint8_t *req);
//    int (*build_response_basis) (sft_t *sft, uint8_t *rsp);
//    int (*prepare_response_tid) (const uint8_t *req, int *req_length);
//    int (*send_msg_pre) (uint8_t *req, int req_length);
//    int (*check_integrity)(modbus_t *ctx, uint8_t *msg,const int msg_length);
//    int (*pre_check_confirmation)(modbus_t *ctx, const uint8_t *req,const uint8_t *rsp, int rsp_length);
};

struct _small_modbus_port {
    int  read_timeout;
    int  write_timeout;
    int (*open) (small_modbus_t *smb);
    int (*close)(small_modbus_t *smb);
    int (*read) (small_modbus_t *smb,uint8_t *data,uint16_t length);
    int (*write)(small_modbus_t *smb,uint8_t *data,uint16_t length);
    int (*flush)(small_modbus_t *smb);
    int (*select)(small_modbus_t *smb,int timeout);
    void (*debug)(small_modbus_t *smb,int level,const char *fmt, ...);
};

struct _small_modbus_mapping {
    struct bit{int start;int num;uint8_t *array;}bit;
    struct input_bit{int start;int num;uint8_t *array;}input_bit;
    struct registers{int start;int num;uint16_t *array;}registers;
    struct input_registers{int start;int num;uint16_t *array;}input_registers;
};

struct _small_modbus{
    int         fd;
    uint8_t     addr;
    uint8_t     debug_level;
    uint16_t    tid;

    small_modbus_core_t *core;
    small_modbus_port_t *port;
    small_modbus_mapping_t *mapping;
    void *port_data;
};

enum returnCode {
    MODBUS_DIS = -3,
    MODBUS_FAIL = -2,
    MODBUS_TIMEOUT = -1,
    MODBUS_OK = 0
};

int modbus_write(small_modbus_t *smb,uint8_t *data,uint16_t length);
int modbus_read(small_modbus_t *smb,uint8_t *data,uint16_t length);
int modbus_open(small_modbus_t *smb);
int modbus_close(small_modbus_t *smb);
int modbus_flush(small_modbus_t *smb);
int modbus_select(small_modbus_t *smb,int timeout);
#define modbus_debug(smb,level,...)     if(smb->port->debug)smb->port->debug(smb,level,__VA_ARGS__)


/* Waiting for reply confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response);
/* Waiting for host to query data */
int modbus_wait_poll(small_modbus_t *smb,uint8_t *request);
int modbus_handle_poll(small_modbus_t *smb,small_modbus_mapping_t * mapping_tab,uint8_t *request,int length);

int modbus_set_slave(small_modbus_t *smb, int slave);
int modbus_set_debug(small_modbus_t *smb, int level);

int modbus_read_bits(small_modbus_t *smb, int addr, int nb, uint8_t *dest);
int modbus_read_input_bits(small_modbus_t *smb, int addr, int nb, uint8_t *dest);
int modbus_read_registers(small_modbus_t *smb, int addr, int nb, uint16_t *dest);
int modbus_read_input_registers(small_modbus_t *smb, int addr, int nb, uint16_t *dest);

int modbus_write_bit(small_modbus_t *smb, int addr, int status);
int modbus_write_register(small_modbus_t *smb, int addr, int value);
int modbus_write_bits(small_modbus_t *smb, int addr, int nb, const uint8_t *data);
int modbus_write_registers(small_modbus_t *smb, int addr, int nb, const uint16_t *data);

int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask);
int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb,const uint16_t *src, int read_addr, int read_nb,uint16_t *dest);



#endif /* _SMALL_MODBUS_H_ */
