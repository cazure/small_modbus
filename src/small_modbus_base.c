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

int _modbus_write(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
    if(smb->port->write)
        return smb->port->write(smb,data,length);
    return MODBUS_FAIL;
}

int _modbus_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
    if(smb->port->read)
        return smb->port->read(smb,data,length);
    return MODBUS_FAIL;
}

int _modbus_open(small_modbus_t *smb)
{
    if(smb->port->open)
        return smb->port->open(smb);
    return MODBUS_FAIL;
}

int _modbus_close(small_modbus_t *smb)
{
    if(smb->port->close)
        return smb->port->close(smb);
    return MODBUS_FAIL;
}

int _modbus_flush(small_modbus_t *smb)
{
    if(smb->port->flush)
        return smb->port->flush(smb);
    return MODBUS_FAIL;
}

int _modbus_select(small_modbus_t *smb,int timeout)
{
    if(smb->port->select)
        return smb->port->select(smb,timeout);
    return MODBUS_FAIL;
}

/*
 * *
 */
int modbus_connect(small_modbus_t *smb)
{
    if(smb != NULL)
    {
        return _modbus_open(smb);
    }
    return MODBUS_FAIL;
}

int modbus_disconnect(small_modbus_t *smb)
{
    if(smb != NULL)
    {
        return _modbus_close(smb);
    }
    return MODBUS_FAIL;
}

int modbus_error_recovery(small_modbus_t *smb)
{
    if(smb!=NULL)
    {
        return _modbus_flush(smb);
    }
    return MODBUS_FAIL;
}

int modbus_set_read_buff(small_modbus_t *smb,int byte,uint8_t *buff)
{
    return MODBUS_FAIL;
}

int modbus_set_write_buff(small_modbus_t *smb,int byte,uint8_t *buff)
{
    return MODBUS_FAIL;
}

int modbus_set_read_timeout(small_modbus_t *smb,int timeout_ms)
{
    return MODBUS_FAIL;
}

int modbus_set_write_timeout(small_modbus_t *smb,int timeout_ms)
{
    return MODBUS_FAIL;
}


int modbus_set_slave(small_modbus_t *smb, int slave)
{
    if(smb!=NULL)
    {
        if((slave > 0)&&(slave < 247))
        {
            smb->slave_addr = slave;
            return MODBUS_OK;
        }
    }
    return MODBUS_FAIL;
}

int modbus_set_debug(small_modbus_t *smb, int level)
{
    if(smb!=NULL)
    {
       smb->debug_level = level;
    }
    return MODBUS_FAIL;
}

/* start requset */
int modbus_start_requset(small_modbus_t *smb,uint8_t *request,int slave_addr,int function,int addr,int num,void *write_data)
{
    int len = 0;
    uint8_t *u8p = write_data;
    uint16_t *u16p = write_data;
    switch(function)
    {
    case MODBUS_FC_WRITE_SINGLE_COIL:
        len = smb->core->build_request_header(smb,request,slave_addr,function,addr,num);
        break;
    case MODBUS_FC_WRITE_SINGLE_REGISTER:

        break;
    }

    len = smb->core->check_send_pre(smb,smb->write_buff,len);
    if(len >0)
    {
        return _modbus_write(smb, smb->write_buff, len);
    }
    return MODBUS_FAIL;
}
/* wait for confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response)
{

    return MODBUS_FAIL;
}
/* handle confirmation message */
int modbus_handle_confirm(small_modbus_t *smb,uint8_t *request,uint8_t *response,void *read_data)
{

    return MODBUS_FAIL;
}

/* wait for host to query data */
int modbus_wait_poll(small_modbus_t *smb,uint8_t *request)
{

    return MODBUS_FAIL;
}
/* handle query data */
int modbus_handle_poll(small_modbus_t *smb,uint8_t *request,int length,small_modbus_mapping_t * mapping_tab)
{

    return MODBUS_FAIL;
}

/* read */
int modbus_read_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data)
{
    int rc = 0;
    uint8_t *requset = smb->write_buff;

    rc = modbus_start_requset(smb,requset,smb->slave_addr,MODBUS_FC_READ_COILS,addr,num,NULL);
    len = smb->core->check_send_pre(smb,smb->write_buff,len);
    if(len >0)
    {
        return _modbus_write(smb, smb->write_buff, len);
    }
    return MODBUS_FAIL;
}
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);

/* write */
int modbus_write_bit(small_modbus_t *smb, int addr, int write_status);
int modbus_write_register(small_modbus_t *smb, int addr, int write_value);
int modbus_write_bits(small_modbus_t *smb, int addr, int num, const uint8_t *write_data);
int modbus_write_registers(small_modbus_t *smb, int addr, int num, const uint16_t *write_data);



int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask)
{
    return MODBUS_FAIL;
}

int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb,const uint16_t *src, int read_addr, int read_nb,uint16_t *dest)
{
    return MODBUS_FAIL;
}

