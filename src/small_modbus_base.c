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


/* start request */
int modbus_start_request(small_modbus_t *smb,uint8_t *request,int function,int addr,int num,void *write_data)
{
    int len = 0;
    int slave_addr = smb->slave_addr;
    uint8_t *u8p = write_data;
    uint16_t *u16p = write_data;
    uint16_t value = 0;
    uint8_t n = 0;
    uint8_t b = 0;
    uint8_t cnt = 0;
    uint8_t byte_len = 0;

    len = smb->core->build_request_header(smb,request,slave_addr,function,addr,num);

    switch(function)
    {
        case MODBUS_FC_WRITE_SINGLE_COIL:
            value = *u16p;
            request[len-2] = value>>8;
            request[len-1] = value & 0x00ff;
            break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            value = *u16p;
            request[len-2] = value>>8;
            request[len-1] = value & 0x00ff;
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            n = 0;
            b = 0;
            cnt = 0;
            byte_len = (num / 8) + ((num % 8) ? 1 : 0);
            request[len]= byte_len;
            len++;
            for (n = 0; n < byte_len; n++)
            {
                request[len] = 0;
                for(b=0x01;(b & 0xFF);(b<<=1))
                {
                    if(cnt < num)
                    {
                        if (u8p[cnt++])
                        {
                            request[len] |= b;
                        }
                        else
                        {
                            request[len] &=~ b;
                        }
                    }
                }
                len++;
            }
        }break;
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            n = 0;
            byte_len = num * 2;
            request[len]= byte_len;
            len++;
            for (n = 0; n < num; n++)
            {
                request[len++] = u16p[n] >> 8;
                request[len++] = u16p[n] & 0x00FF;
            }
        }break;
    }
    len = smb->core->check_send_pre(smb,request,len);
    if(len > 0)
    {
         _modbus_write(smb,request, len);
         return len;
    }
    return MODBUS_FAIL;
}
/* wait for confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response)
{
    int rc = 0;
    int wait_time = 0;
    int read_want = 0;
    int read_length = 0;
    int read_position = 0;
    int function = 0;

    wait_time = smb->read_timeout;
    read_want = smb->core->len_header + 1;  //header + function code

    while (read_want != 0)
    {
        rc = smb->port->select(smb,wait_time);
        if(rc <= 0)
        {
            smb->port->debug(smb,0,"[%d]select(%d) \n",rc,wait_time);
            return rc;
        }
        rc = smb->port->read(smb,response + read_length , read_want);
        if(rc <= 0)
        {
            smb->port->debug(smb,0,"[%d]read(%d) \n",rc,read_want);
            return rc;
        }
        if(rc != read_want)
        {
            smb->port->debug(smb,0,"[%d]read(%d) \n",rc,read_want);
        }

        read_length += rc;  //sum byte length
        read_want -= rc;    //sub byte length

        if(read_want == 0)//read ok
        {
            if(read_position==0)/* Function code position */
            {
                function = response[smb->core->len_header];
                switch (function)
                {
                case MODBUS_FC_READ_HOLDING_COILS:
                case MODBUS_FC_READ_INPUTS_COILS:
                case MODBUS_FC_READ_HOLDING_REGISTERS:
                case MODBUS_FC_READ_INPUT_REGISTERS:
                    read_want = 1; //read data length(1)
                    break;
                case MODBUS_FC_WRITE_SINGLE_COIL:
                case MODBUS_FC_WRITE_SINGLE_REGISTER:
                case MODBUS_FC_WRITE_MULTIPLE_COILS:
                case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                    read_want = 4;  //write addr(2)+num(2)
                    break;
                case MODBUS_FC_MASK_WRITE_REGISTER:
                    read_want = 6;  //data length
                    break;
                default:
                    read_want = 1;  //read data length(1)
                }
                read_position = 1;
            }
            else if(read_position==1)/* Data */
            {
                function = response[smb->core->len_header];
                switch(function)
                {
                case MODBUS_FC_READ_HOLDING_COILS:
                case MODBUS_FC_READ_INPUTS_COILS:
                case MODBUS_FC_READ_HOLDING_REGISTERS:
                case MODBUS_FC_READ_INPUT_REGISTERS:
                    read_want = response[smb->core->len_header+1];
                    break;
                default:
                    read_want = 0;
                }
                if((read_want+read_length)> smb->core->len_max )
                {
                    smb->port->debug(smb,0,"More than ADU %d > %d\n",(read_want+read_length),smb->core->len_max);
                    return MODBUS_FAIL;
                }
                read_position = 2;
            }
        }
    }
    return smb->core->check_wait_confirm(smb,response,read_length);
}
/* handle confirmation message */
int modbus_handle_confirm(small_modbus_t *smb,uint8_t *request,uint16_t request_len,uint8_t *response,uint16_t response_len,void *read_data)
{
    int request_function = request[smb->core->len_header];
    int response_function = response[smb->core->len_header];

    if(request_function == response_function)
    {
        switch (response_function)
        {
        case MODBUS_FC_READ_HOLDING_COILS:
        case MODBUS_FC_READ_INPUTS_COILS:
        {
            data_length = request[smb->core->len_header+1];  //data length
            frame_length = data_length
            /* Header + nb values (code from write_bits) */
            int nb = (req[offset + 3] << 8) | req[offset + 4];
            length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
        }
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
            /* Header + 2 * nb values */
            length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
            break;
        case MODBUS_FC_READ_EXCEPTION_STATUS:
            length = 3;
            break;
        case MODBUS_FC_REPORT_SLAVE_ID:
            /* The response is device specific (the header provides the
               length) */
            return MSG_LENGTH_UNDEFINED;
        case MODBUS_FC_MASK_WRITE_REGISTER:
            length = 7;
            break;
        default:
            length = 5;
        }
    }
//    int data_length = 0;
//    int frame_length = 0;
//    switch (function)
//    {
//    case MODBUS_FC_READ_HOLDING_COILS:
//    case MODBUS_FC_READ_INPUTS_COILS:
//    {
//        data_length = request[smb->core->len_header+1];  //data length
//        frame_length = data_length
//        /* Header + nb values (code from write_bits) */
//        int nb = (req[offset + 3] << 8) | req[offset + 4];
//        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
//    }
//        break;
//    case MODBUS_FC_WRITE_AND_READ_REGISTERS:
//    case MODBUS_FC_READ_HOLDING_REGISTERS:
//    case MODBUS_FC_READ_INPUT_REGISTERS:
//        /* Header + 2 * nb values */
//        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
//        break;
//    case MODBUS_FC_READ_EXCEPTION_STATUS:
//        length = 3;
//        break;
//    case MODBUS_FC_REPORT_SLAVE_ID:
//        /* The response is device specific (the header provides the
//           length) */
//        return MSG_LENGTH_UNDEFINED;
//    case MODBUS_FC_MASK_WRITE_REGISTER:
//        length = 7;
//        break;
//    default:
//        length = 5;
//    }
//
//    return offset + length + ctx->backend->checksum_length;

    return MODBUS_FAIL;
}

/* wait for master to query data */
int modbus_wait_poll(small_modbus_t *smb,uint8_t *response)
{
    int rc = 0;
    int wait_time = 0;
    int read_want = 0;
    int read_length = 0;
    int read_position = 0;
    int function = 0;

    wait_time = -1; //
    read_want = smb->core->len_header + 1;  //header + function code

    while (read_want != 0)
    {
        rc = smb->port->select(smb,wait_time);
        if(rc <= 0)
        {
            smb->port->debug(smb,0,"[%d]select(%d) \n",rc,wait_time);
            return rc;
        }
        rc = smb->port->read(smb,response + read_length , read_want);
        if(rc <= 0)
        {
            smb->port->debug(smb,0,"[%d]read(%d) \n",rc,read_want);
            return rc;
        }
        if(rc != read_want)
        {
            smb->port->debug(smb,0,"[%d]read(%d) \n",rc,read_want);
        }
        wait_time = smb->read_timeout;

        read_length += rc;  //sum byte length
        read_want -= rc;    //sub byte length

        if(read_want == 0)//read ok
        {
            if(read_position==0)/* Function code position */
            {
                function = response[smb->core->len_header];
                if(function <= MODBUS_FC_WRITE_SINGLE_REGISTER)
                {
                    read_want = 4;
                }else if((function == MODBUS_FC_WRITE_MULTIPLE_COILS) ||
                     (function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS) )
                {
                    read_want = 5;
                }else if(function == MODBUS_FC_MASK_WRITE_REGISTER)
                {
                    read_want = 6;
                }else if(function == MODBUS_FC_WRITE_AND_READ_REGISTERS)
                {
                    read_want = 9;
                }else
                {
                    read_want = 0;
                }
                read_position = 1;
            }
            else if(read_position==1)/* Data */
            {
                function = response[smb->core->len_header];
                switch (function)
                {
                case MODBUS_FC_WRITE_MULTIPLE_COILS:
                case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                    read_want = response[smb->core->len_header + 5];
                    break;
                case MODBUS_FC_WRITE_AND_READ_REGISTERS:
                    read_want = response[smb->core->len_header + 9];
                    break;
                default:
                    read_want = 0;
                }
                if((read_want+read_length)> smb->core->len_max )
                {
                    smb->port->debug(smb,0,"More than ADU %d > %d\n",(read_want+read_length),smb->core->len_max);
                    return MODBUS_FAIL;
                }
                read_position = 2;
            }
        }
    }
    return smb->core->check_wait_poll(smb,response,read_length);
}
/* handle query data */
int modbus_handle_poll(small_modbus_t *smb,uint8_t *response,uint16_t response_len,small_modbus_mapping_t * mapping_tab)
{

    return MODBUS_FAIL;
}

int modbus_wait(small_modbus_t *smb,small_modbus_mapping_t * mapping_tab)
{
    int rc = 0;
    uint8_t *confirm = smb->write_buff;
    uint8_t *response = smb->read_buff;
    rc = modbus_wait_poll(smb,response);
    rc = modbus_handle_poll(smb,response,rc,mapping_tab);
    return rc;
}

/* read */
int modbus_read_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    request_len = modbus_start_request(smb,request,MODBUS_FC_READ_HOLDING_COILS,addr,num,NULL);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, read_data);
        }
    }
    return MODBUS_FAIL;
}
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    request_len = modbus_start_request(smb,request,MODBUS_FC_READ_INPUTS_COILS,addr,num,NULL);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, read_data);
        }
    }
    return MODBUS_FAIL;
}
int modbus_read_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    request_len = modbus_start_request(smb,request,MODBUS_FC_READ_HOLDING_REGISTERS,addr,num,NULL);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, read_data);
        }
    }
    return MODBUS_FAIL;
}
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    request_len = modbus_start_request(smb,request,MODBUS_FC_READ_INPUT_REGISTERS,addr,num,NULL);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, read_data);
        }
    }
    return MODBUS_FAIL;
}

/* write */
int modbus_write_bit(small_modbus_t *smb, int addr, int write_status)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;
    int status = write_status?0xFF00:0x0;

    request_len = modbus_start_request(smb,request,MODBUS_FC_WRITE_SINGLE_COIL,addr,1,&status);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, NULL);
        }
    }
    return MODBUS_FAIL;
}
int modbus_write_register(small_modbus_t *smb, int addr, int write_value)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;
    int value = write_value;

    request_len = modbus_start_request(smb,request,MODBUS_FC_WRITE_SINGLE_REGISTER,addr,1,&value);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, NULL);
        }
    }
    return MODBUS_FAIL;
}
int modbus_write_bits(small_modbus_t *smb, int addr, int num, const uint8_t *write_data)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    request_len = modbus_start_request(smb,request,MODBUS_FC_WRITE_MULTIPLE_COILS,addr,num,write_data);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, NULL);
        }
    }
    return MODBUS_FAIL;
}
int modbus_write_registers(small_modbus_t *smb, int addr, int num, const uint16_t *write_data)
{
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    request_len = modbus_start_request(smb,request,MODBUS_FC_WRITE_MULTIPLE_REGISTERS,addr,num,write_data);
    if(request_len >= 0)
    {
        response_len = modbus_wait_confirm(smb, response);
        if(response_len > 0)
        {
            return modbus_handle_confirm(smb, request,request_len, response,response_len, NULL);
        }
    }
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

