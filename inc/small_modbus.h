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

/* function codes */
enum functionCode {
    MODBUS_FC_READ_HOLDING_COILS        =0x01,
    MODBUS_FC_READ_INPUTS_COILS         =0x02,
    MODBUS_FC_READ_HOLDING_REGISTERS    =0x03,
    MODBUS_FC_READ_INPUT_REGISTERS      =0x04,
    MODBUS_FC_WRITE_SINGLE_COIL         =0x05,
    MODBUS_FC_WRITE_SINGLE_REGISTER     =0x06,
    MODBUS_FC_READ_EXCEPTION_STATUS     =0x07,
    MODBUS_FC_WRITE_MULTIPLE_COILS      =0x0F,
    MODBUS_FC_WRITE_MULTIPLE_REGISTERS  =0x10,
    MODBUS_FC_REPORT_SLAVE_ID           =0x11,
    MODBUS_FC_MASK_WRITE_REGISTER       =0x16,
    MODBUS_FC_WRITE_AND_READ_REGISTERS  =0x17,
};

#define MODBUS_BROADCAST_ADDRESS    0

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* The size of the MODBUS PDU is limited by the size constraint inherited from
 * the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
 * bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
 * address (1 byte) - CRC (2 bytes) = 253 bytes.
 */
#define MODBUS_MAX_PDU_LENGTH              253

/* Consequently:
 * - RTU MODBUS ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256
 *   bytes.
 * - TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes.
 * so the maximum of both backend in 260 bytes. This size can used to allocate
 * an array of bytes to store responses and it will be compatible with the two
 * backends.
 */
#define MODBUS_MAX_ADU_LENGTH              260

//#define MODBUS_BROADCAST_ADDRESS 0xFF

typedef struct _small_modbus        small_modbus_t;
typedef struct _small_modbus_core   small_modbus_core_t;
typedef struct _small_modbus_port   small_modbus_port_t;
typedef struct _small_modbus_mapping   small_modbus_mapping_t;


struct _small_modbus_core{
    uint16_t type;
    uint16_t len_header;
    uint16_t len_checksum;
    uint16_t len_adu_max;
    int (*build_request_header)(small_modbus_t *smb,uint8_t *buff,int slave,int fun,int reg,int num);
    int (*build_response_header)(small_modbus_t *smb,uint8_t *buff,int slave,int fun);
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
    int (*open) (small_modbus_t *smb);
    int (*close)(small_modbus_t *smb);
    int (*read) (small_modbus_t *smb,uint8_t *data,uint16_t length);
    int (*write)(small_modbus_t *smb,uint8_t *data,uint16_t length);
    int (*flush)(small_modbus_t *smb);
    int (*select)(small_modbus_t *smb,int timeout);
    void (*debug)(small_modbus_t *smb,int level,const char *fmt, ...);
};

struct _small_modbus_mapping {
    int (*status_callback) (small_modbus_mapping_t *mapping,int read_write,int data_type,int start,int num);
    struct bit{int start;int num;uint8_t *array;}bit;
    struct input_bit{int start;int num;uint8_t *array;}input_bit;
    struct registers{int start;int num;uint16_t *array;}registers;
    struct input_registers{int start;int num;uint16_t *array;}input_registers;
};

#define modbus_mapping_init(map,callback,bit_start,bit_num,input_bit_start,input_bit_num,reg_start,reg_num,input_reg_start,input_reg_num) \
{\
    static uint8_t _##map##_bit_array[bit_num];\
    static uint8_t _##map##_input_bit_array[input_bit_num];\
    static uint16_t _##map##_reg_array[reg_num];\
    static uint16_t _##map##_input_reg_array[input_reg_num];\
    map.status_callback = callback;\
    map.bit.start = bit_start;\
    map.bit.num = bit_num;\
    map.bit.array = _##map##_bit_array;\
    map.input_bit.start = input_bit_start;\
    map.input_bit.num   = input_bit_num;\
    map.input_bit.array = _##map##_input_bit_array;\
    map.registers.start = reg_start;\
    map.registers.num   = reg_num;\
    map.registers.array = _##map##_reg_array;\
    map.input_registers.start = input_reg_start;\
    map.input_registers.num   = input_reg_num;\
    map.input_registers.array = _##map##_input_reg_array;\
}


struct _small_modbus{
    int         status;
    int         read_timeout;
    int         write_timeout;
    uint8_t     read_buff[MODBUS_MAX_ADU_LENGTH];
    uint8_t     write_buff[MODBUS_MAX_ADU_LENGTH];
    uint8_t     slave_addr;
    uint8_t     debug_level;
    uint16_t    tid;

    small_modbus_core_t *core;
    small_modbus_port_t *port;
    small_modbus_mapping_t *mapping;
    void *port_data;
};

enum returnCode {
    MODBUS_DIS = -3,
    MODBUS_TIMEOUT = -2,
    MODBUS_FAIL = -1,
    MODBUS_OK = 0
};

#define modbus_debug(smb,level,...)     if(smb->port->debug)smb->port->debug(smb,level,__VA_ARGS__)
int _modbus_write(small_modbus_t *smb,uint8_t *data,uint16_t length);
int _modbus_read(small_modbus_t *smb,uint8_t *data,uint16_t length);
int _modbus_open(small_modbus_t *smb);
int _modbus_close(small_modbus_t *smb);
int _modbus_flush(small_modbus_t *smb);
int _modbus_select(small_modbus_t *smb,int timeout);
int _modbus_init(small_modbus_t *smb);

int modbus_connect(small_modbus_t *smb);
int modbus_disconnect(small_modbus_t *smb);
int modbus_error_recovery(small_modbus_t *smb);

//int modbus_set_read_buff(small_modbus_t *smb,int byte,uint8_t *buff);
//int modbus_set_write_buff(small_modbus_t *smb,int byte,uint8_t *buff);
int modbus_set_read_timeout(small_modbus_t *smb,int timeout_ms);
int modbus_set_write_timeout(small_modbus_t *smb,int timeout_ms);
int modbus_set_slave(small_modbus_t *smb, int slave);
int modbus_set_debug(small_modbus_t *smb, int level);

/* start request */
int modbus_start_request(small_modbus_t *smb,uint8_t *request,int function,int addr,int num,void *write_data);
/* wait for confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response);
/* handle confirmation message */
int modbus_handle_confirm(small_modbus_t *smb,uint8_t *request,uint16_t request_len,uint8_t *response,uint16_t response_len,void *read_data);

/* wait for host to query data */
int modbus_wait_poll(small_modbus_t *smb,uint8_t *request);
/* handle query data */
int modbus_handle_poll(small_modbus_t *smb,uint8_t *request,uint16_t request_len,small_modbus_mapping_t * mapping_tab);

int modbus_wait(small_modbus_t *smb,small_modbus_mapping_t * mapping_tab);

/* read */
int modbus_read_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);

/* write */
int modbus_write_bit(small_modbus_t *smb, int addr, int write_status);
int modbus_write_register(small_modbus_t *smb, int addr, int write_value);
int modbus_write_bits(small_modbus_t *smb, int addr, int num, const uint8_t *write_data);
int modbus_write_registers(small_modbus_t *smb, int addr, int num, const uint16_t *write_data);

int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask);
int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb,const uint16_t *src, int read_addr, int read_nb,uint16_t *dest);





#endif /* _SMALL_MODBUS_H_ */
