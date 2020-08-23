/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     Administrator       the first version
 */

#include "small_modbus.h"


int modbus_write(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
    if(smb->port->write)
        return smb->port->write(smb,data,length);
    return MODBUS_FAIL;
}

int modbus_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
    if(smb->port->read)
        return smb->port->read(smb,data,length);
    return MODBUS_FAIL;
}

int modbus_open(small_modbus_t *smb)
{
    if(smb->port->open)
        return smb->port->open(smb);
    return MODBUS_FAIL;
}

int modbus_close(small_modbus_t *smb)
{
    if(smb->port->close)
        return smb->port->close(smb);
    return MODBUS_FAIL;
}

int modbus_flush(small_modbus_t *smb)
{
    if(smb->port->close)
        return smb->port->close(smb);
    return MODBUS_FAIL;
}

int modbus_select(small_modbus_t *smb,int timeout)
{
    if(smb->port->select)
        return smb->port->select(smb,smb->port->read_timeout);
    return MODBUS_FAIL;
}

/* Waiting for reply confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response)
{
    return MODBUS_FAIL;
}

/* Waiting for host to query data */
int modbus_wait_poll(small_modbus_t *smb,uint8_t *request)
{
    return MODBUS_FAIL;
}

int modbus_handle_poll(small_modbus_t *smb,small_modbus_mapping_t * mapping_tab,uint8_t *request,int length)
{
    return MODBUS_FAIL;
}

int modbus_set_slave(small_modbus_t *smb, int slave)
{
    return MODBUS_FAIL;
}

int modbus_set_debug(small_modbus_t *smb, int level)
{
    return MODBUS_FAIL;
}



int modbus_read_bits(small_modbus_t *smb, int addr, int nb, uint8_t *dest)
{
    return MODBUS_FAIL;
}

int modbus_read_input_bits(small_modbus_t *smb, int addr, int nb, uint8_t *dest)
{
    return MODBUS_FAIL;
}

int modbus_read_registers(small_modbus_t *smb, int addr, int nb, uint16_t *dest)
{
    return MODBUS_FAIL;
}

int modbus_read_input_registers(small_modbus_t *smb, int addr, int nb, uint16_t *dest)
{
    return MODBUS_FAIL;
}

int modbus_write_bit(small_modbus_t *smb, int addr, int status)
{
    return MODBUS_FAIL;
}

int modbus_write_register(small_modbus_t *smb, int addr, int value)
{
    return MODBUS_FAIL;
}

int modbus_write_bits(small_modbus_t *smb, int addr, int nb, const uint8_t *data)
{
    return MODBUS_FAIL;
}

int modbus_write_registers(small_modbus_t *smb, int addr, int nb, const uint16_t *data)
{
    return MODBUS_FAIL;
}


int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask)
{
    return MODBUS_FAIL;
}

int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb,const uint16_t *src, int read_addr, int read_nb,uint16_t *dest)
{
    return MODBUS_FAIL;
}

