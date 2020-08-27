/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     Administrator       the first version
 */
#include "small_modbus_tcp.h"

/* Builds a TCP request header */
int _tcp_build_request_header(small_modbus_t *smb,uint8_t *buff,int slave,int fun,int reg,int num)
{
    smb->tid++;
    buff[0] = smb->tid >> 8;
    buff[1] = smb->tid & 0x00ff;

    /* Protocol Modbus */
    buff[2] = 0;
    buff[3] = 0;

    /* Length will be defined later by set_req_length_tcp at offsets 4
       and 5 */

    buff[6] = slave;
    buff[7] = fun;
    buff[8] = reg >> 8;
    buff[9] = reg & 0x00ff;
    buff[10] = num >> 8;
    buff[11] = num & 0x00ff;

    return _MODBUS_TCP_PRESET_REQ_LENGTH;
}

/* Builds a TCP response header */
int _tcp_build_response_header(small_modbus_t *smb,uint8_t *buff,int slave,int fun)
{
    /* Extract from MODBUS Messaging on TCP/IP Implementation
       Guide V1.0b (page 23/46):
       The transaction identifier is used to associate the future
       response with the request. */
    buff[0] = smb->tid >> 8;
    buff[1] = smb->tid & 0x00ff;

    /* Protocol Modbus */
    buff[2] = 0;
    buff[3] = 0;

    /* Length will be set later by send_msg (4 and 5) */

    /* The slave ID is copied from the indication */
    buff[6] = slave;
    buff[7] = fun;

    return _MODBUS_TCP_PRESET_RSP_LENGTH;
}

int _tcp_check_send_pre(small_modbus_t *smb,uint8_t *buff,int length)
{
    /* Substract the header length to the message length */
    int rc = length - 6;

    buff[4] = rc >> 8;
    buff[5] = rc & 0x00FF;

    return length;
}

int _tcp_check_wait_poll(small_modbus_t *smb,uint8_t *buff,int length)
{
    return length;
}

int _tcp_check_wait_confirm(small_modbus_t *smb,uint8_t *buff,int length)
{
    uint16_t tt_id = (uint16_t)buff[0] + (uint16_t)buff[1]*256;
    return MODBUS_OK;
}

const small_modbus_core_t modbus_tcp_core =
{
    .type           = 1,
    .len_header     = _MODBUS_TCP_HEADER_LENGTH,
    .len_checksum   = _MODBUS_TCP_CHECKSUM_LENGTH,
    .len_max        = _MODBUS_TCP_MAX_ADU_LENGTH,
    .build_request_header   = _tcp_build_request_header,
    .build_response_header  = _tcp_build_response_header,
    .check_send_pre     = _tcp_check_send_pre,
    .check_wait_poll    = _tcp_check_wait_poll,
    .check_wait_confirm   = _tcp_check_wait_confirm
};


