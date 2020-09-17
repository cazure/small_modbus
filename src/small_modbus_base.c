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

static const char _modbus_ver[32] = "small_modbus_v1.00";

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

int _modbus_wait(small_modbus_t *smb,int timeout)
{
    if(smb->port->wait)
        return smb->port->wait(smb,timeout);
    return MODBUS_FAIL;
}

int _modbus_init(small_modbus_t *smb)
{
    if(smb != NULL)
    {
        if(smb->read_timeout==0)
        {
            smb->read_timeout = 100;
        }
        if(smb->write_timeout==0)
        {
            smb->write_timeout = 10;
        }
        if(smb->debug_level==0)
        {
            smb->debug_level = 0;
        }
    }
    return MODBUS_FAIL;
}

int _modbus_array2bit(uint8_t *dest_modbus_bit,void *source,uint16_t bit_num)
{
    uint8_t *source_array = source;
    uint16_t index = 0;
    uint16_t byte_num =  (bit_num / 8) + ((bit_num % 8) ? 1 : 0);
    uint16_t byte_index = 0;
    uint8_t  offset,temp;

    for(index=0; index < bit_num; index++)
    {
        byte_index = (index / 8);
        offset = (index % 8);
        temp = (0x01 << offset)&0xff;
        if(source_array[index])
        {
            dest_modbus_bit[byte_index] |= temp;
        }else
        {
            dest_modbus_bit[byte_index] &= ~temp;
        }
    }
    return byte_num;
}

int _modbus_bit2array(void *dest,uint8_t *modbus_bit_array,uint16_t bit_num)
{
    uint8_t *dest_byte = dest;
    uint16_t index = 0;
    uint16_t bit_index = 0;
    uint16_t byte_num =  bit_num;
    uint8_t  offset,temp;

    for(index=0; index < bit_num; index++)
    {
        bit_index = (index / 8);
        offset = (index % 8);
        temp = (0x01 << offset)&0xff;
        if(modbus_bit_array[bit_index] & temp)
        {
            dest_byte[index] = 1;
        }else
        {
            dest_byte[index] = 0;
        }
    }
    return byte_num;
}

int _modbus_array2reg(uint8_t *dest_reg,void *source_array,uint16_t reg_num)
{
    uint16_t *source = source_array;  //u16
    uint16_t index = 0;
    uint16_t byte_num =  reg_num*2;

    for(index=0; index < reg_num; index++)
    {
        dest_reg[(index*2)] = (source[index] >> 8);
        dest_reg[(index*2)+1] = (source[index] & 0x00FF);
    }
    return byte_num;
}

int _modbus_reg2array(void *dest_array,uint8_t *source_reg,uint16_t reg_num)
{
    uint16_t *dest = dest_array;  //u16
    uint16_t index = 0;

    for(index=0; index < reg_num; index++)
    {
        dest[index] = (source_reg[index*2]<<8)|(source_reg[index*2+1]);
    }
    return reg_num;
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

//int modbus_set_read_buff(small_modbus_t *smb,int byte,uint8_t *buff)
//{
//    smb->read_buff = buff;
//    smb->read_buff_szie = byte;
//    return MODBUS_FAIL;
//}
//
//int modbus_set_write_buff(small_modbus_t *smb,int byte,uint8_t *buff)
//{
//    smb->write_buff = buff;
//    smb->write_buff_szie = byte;
//    return MODBUS_FAIL;
//}

int modbus_set_read_timeout(small_modbus_t *smb,int timeout_ms)
{
    smb->read_timeout = timeout_ms;
    return MODBUS_OK;
}

int modbus_set_write_timeout(small_modbus_t *smb,int timeout_ms)
{
    smb->write_timeout = timeout_ms;
    return MODBUS_OK;
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
    return MODBUS_OK;
}

int modbus_set_debug(small_modbus_t *smb, int level)
{
    if(smb!=NULL)
    {
       smb->debug_level = level;
    }
    return MODBUS_OK;
}

/* start request */
int modbus_start_request(small_modbus_t *smb,uint8_t *request,int function,int addr,int num,void *write_data)
{
    int len = 0;
    int slave_addr = smb->slave_addr;
    uint16_t temp = 0;
    uint16_t data_len = 0;

    len = smb->core->build_request_header(smb,request,slave_addr,function,addr,num);

    switch(function)
    {
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        {
            temp = *((uint16_t *)write_data);
            request[len-2] = temp>>8;
            request[len-1] = temp & 0x00ff;
        }break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            data_len = _modbus_array2bit(&request[len+1], write_data, num);
            request[len++] = data_len;
            len += data_len;
        }break;
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            data_len = _modbus_array2reg(&request[len+1], write_data, num);
            request[len++] = data_len;
            len += data_len;
        }break;
    }
    len = smb->core->check_send_pre(smb,request,len);
    if(len > 0)
    {
         _modbus_write(smb,request, len);
         return len;
    }
    return MODBUS_FAIL_REQUEST;
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
        rc = _modbus_wait(smb,wait_time);
        if(rc < 0)
        {
            modbus_debug_error(smb,"[%d]select(%d) \n",rc,wait_time);
            return rc;
        }
        rc = _modbus_read(smb,response + read_length , read_want);
        if(rc <= 0)
        {
            modbus_debug_error(smb,"[%d]read(%d) \n",rc,read_want);
            return rc;
        }
        if(rc != read_want)
        {
            modbus_debug_info(smb,"[%d]read less(%d) \n",rc,read_want);
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
                read_want += smb->core->len_checksum;
                if((read_want+read_length)> smb->core->len_adu_max )
                {
                    smb->port->debug(smb,1,"More than ADU %d > %d\n",(read_want+read_length),smb->core->len_adu_max);
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
    uint8_t request_function = request[smb->core->len_header];
    uint8_t response_function = response[smb->core->len_header];
    uint16_t calc_length = smb->core->len_header + smb->core->len_checksum ; // header + checksum
    uint16_t temp = 0;
    uint16_t data_num = 0;
    uint16_t byte_num = 0;
    if(response_function >= 0x80)
    {
        if((response_function - 0x80) == request_function)
        {
            modbus_debug_error(smb,"function code %d\n",request_function);
        }
        modbus_debug_error(smb,"exception code %d\n",-response_function);
        return -response_function;
    }
    if(request_function == response_function)
    {
        //clac_length
        switch (request_function)
        {
            case MODBUS_FC_READ_HOLDING_COILS:
            case MODBUS_FC_READ_INPUTS_COILS:
                {
                    temp = (request[smb->core->len_header+3]<<8)|(request[smb->core->len_header+4]);  //data length
                    calc_length += (2 + (temp / 8) + ((temp % 8) ? 1 : 0));
                }
                break;
            case MODBUS_FC_WRITE_AND_READ_REGISTERS:
            case MODBUS_FC_READ_HOLDING_REGISTERS:
            case MODBUS_FC_READ_INPUT_REGISTERS:
                {
                    temp = (request[smb->core->len_header+3]<<8)|(request[smb->core->len_header+4]);  //data length
                    calc_length += (2 + 2*temp);
                }
                break;
            case MODBUS_FC_READ_EXCEPTION_STATUS:
                calc_length += 3;
                break;
            case MODBUS_FC_MASK_WRITE_REGISTER:
                calc_length += 7;
                break;
            default:
                calc_length += 5;
        }

        if(calc_length == response_len)  //
        {
            //read data
            switch (response_function)
            {
            case MODBUS_FC_READ_HOLDING_COILS:
            case MODBUS_FC_READ_INPUTS_COILS:
                {
                    // 计算读取线圈数量
                    data_num = (request[smb->core->len_header+3]<<8)|(request[smb->core->len_header+4]);
                    // 计算字节数
                    temp = ((data_num / 8) + ((data_num % 8) ? 1 : 0));
                    // 计算实际字节数
                    byte_num = (response[smb->core->len_header+1]);
                    if((uint8_t)temp == byte_num)
                    {
                        _modbus_bit2array(read_data,&(response[smb->core->len_header+2]),data_num);
                        return MODBUS_OK;
                    }
                }
                break;
            case MODBUS_FC_WRITE_AND_READ_REGISTERS:
            case MODBUS_FC_READ_HOLDING_REGISTERS:
            case MODBUS_FC_READ_INPUT_REGISTERS:
                {
                    // 计算读取线圈数量
                    data_num = (request[smb->core->len_header+3]<<8)|(request[smb->core->len_header+4]);  //data length
                    temp = data_num*2;
                    byte_num = (response[smb->core->len_header+1]);
                    if((uint8_t)temp == byte_num)
                    {
                        _modbus_reg2array(read_data, &(response[smb->core->len_header+2]), data_num);
                        return MODBUS_OK;
                    }
                }
                break;
            case MODBUS_FC_WRITE_MULTIPLE_COILS:
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                {

                    data_num = (request[smb->core->len_header+3]<<8)|(request[smb->core->len_header+4]);  //data length
                    temp = (response[smb->core->len_header+3]<<8)|(response[smb->core->len_header+4]);  //data length
                    if(data_num == temp)
                    {
                        return MODBUS_OK;
                    }
                }
                break;
            case MODBUS_FC_READ_EXCEPTION_STATUS:
                {
                    return MODBUS_OK;
                }
            case MODBUS_FC_MASK_WRITE_REGISTER:
                {
                    return MODBUS_OK;
                }
            default:
                {
                    return MODBUS_OK;
                }
            }
        }
    }
    return MODBUS_FAIL_CONFIRM;
}

/* wait for master to query data */
int modbus_wait_poll(small_modbus_t *smb,uint8_t *request)
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
        rc = _modbus_wait(smb,wait_time);
        if(rc < 0)
        {
            modbus_debug_error(smb,"[%d]select(%d) \n",rc,wait_time);
            return rc;
        }
        rc = _modbus_read(smb,request + read_length , read_want);
        if(rc <= 0)
        {
            modbus_debug_error(smb,"[%d]read(%d) \n",rc,read_want);
            return rc;
        }
        if(rc != read_want)
        {
            modbus_debug_info(smb,"[%d]read(%d) \n",rc,read_want);
        }
        wait_time = smb->read_timeout;

        read_length += rc;  //sum byte length
        read_want -= rc;    //sub byte length

        if(read_want == 0)//read ok
        {
            if(read_position==0)/* Function code position */
            {
                function = request[smb->core->len_header];
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
                function = request[smb->core->len_header];
                switch (function)
                {
                case MODBUS_FC_WRITE_MULTIPLE_COILS:
                case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                    read_want = request[smb->core->len_header + 5];
                    break;
                case MODBUS_FC_WRITE_AND_READ_REGISTERS:
                    read_want = request[smb->core->len_header + 9];
                    break;
                default:
                    read_want = 0;
                }
                read_want += smb->core->len_checksum;
                if((read_want+read_length)> smb->core->len_adu_max )
                {
                    modbus_debug_error(smb,"More than ADU %d > %d\n",(read_want+read_length),smb->core->len_adu_max);
                    return MODBUS_FAIL;
                }
                read_position = 2;
            }
        }
    }
    return smb->core->check_wait_poll(smb,request,read_length);
}

/* handle query data */
int modbus_handle_poll(small_modbus_t *smb,uint8_t *request,uint16_t request_len,small_modbus_mapping_t * mapping_tab)
{
    //uint8_t *request = smb->read_buff;
    int response_exception = MODBUS_OK;
    uint16_t response_len = 0;
    uint8_t *response = smb->write_buff;
    uint8_t query_slave = smb->slave_addr;//request[smb->core->len_header-1];
    uint8_t query_function = request[smb->core->len_header];
    uint16_t query_address = 0;
    uint16_t query_num = 0;
    uint16_t map_address = 0;
    uint16_t map_num = 0;
    uint8_t *map_8array;
    uint16_t *map_16array;
    uint16_t temp = 0;

    /* Data are flushed on illegal number of values errors. */
    switch (query_function)
    {
        case MODBUS_FC_READ_HOLDING_COILS:
        case MODBUS_FC_READ_INPUTS_COILS:
        {
            if(query_function == MODBUS_FC_READ_HOLDING_COILS)
            {
                map_address = mapping_tab->bit.start;
                map_num = mapping_tab->bit.num;
                map_8array = mapping_tab->bit.array;
            }else
            {
                map_address = mapping_tab->input_bit.start;
                map_num = mapping_tab->input_bit.num;
                map_8array = mapping_tab->input_bit.array;
            }
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            if((map_address <= query_address) && (query_address <= (map_address + map_num)) ) //起始地址
            {
                int diff = query_address - map_address;
                query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];//查询数量
                if((0  < query_num )&&( query_num <= (map_num-diff)))
                {
                    response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
                    temp = _modbus_array2bit(&(response[response_len+1]), map_8array+diff, query_num);  //数组转modbus bit
                    response[response_len++] = temp;
                    response_len += temp;
                }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;}
            }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;}
        }break;

        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
        {
            if(query_function == MODBUS_FC_READ_HOLDING_REGISTERS)
            {
                map_address = mapping_tab->registers.start;
                map_num = mapping_tab->registers.num;
                map_16array = mapping_tab->registers.array;
            }else
            {
                map_address = mapping_tab->input_registers.start;
                map_num = mapping_tab->input_registers.num;
                map_16array = mapping_tab->input_registers.array;
            }
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            if((map_address <= query_address) && (query_address <= (map_address + map_num)) ) //起始地址
            {
                int diff = query_address - map_address;
                query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];//查询数量
                if((0  < query_num )&&( query_num <= (map_num-diff)))
                {
                    response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
                    temp = _modbus_array2reg(&(response[response_len+1]), map_16array+diff, query_num);  //数组转modbus reg
                    response[response_len++] = temp;
                    response_len += temp;
                }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;}
            }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;}
        }break;

        case MODBUS_FC_WRITE_SINGLE_COIL:
        {
            map_address =   mapping_tab->bit.start;
            map_num =       mapping_tab->bit.num;
            map_8array =    mapping_tab->bit.array;
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            if((map_address <= query_address) && (query_address <= (map_address + map_num)) ) //起始地址
            {
                int diff = query_address - map_address;
                query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];//数据
                if (query_num == 0xFF00 || query_num == 0x0)
                {
                    map_8array[diff] = query_num ? 1 : 0;
                    response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
                    response[response_len++] = (query_address>>8);
                    response[response_len++] = (query_address&0x00ff);
                    response[response_len++] = (query_num>>8);
                    response[response_len++] = (query_num&0x00ff);
                }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;}
            }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;}
        }break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        {
            map_address =   mapping_tab->registers.start;
            map_num =       mapping_tab->registers.num;
            map_16array =    mapping_tab->registers.array;
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            if((map_address <= query_address) && (query_address <= (map_address + map_num)) ) //起始地址
            {
                int diff = query_address - map_address;
                query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];//数据
                map_16array[diff] = query_num;
                response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
                response[response_len++] = (query_address>>8);
                response[response_len++] = (query_address&0x00ff);
                response[response_len++] = (query_num>>8);
                response[response_len++] = (query_num&0x00ff);
            }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;}
        }break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            map_address =   mapping_tab->bit.start;
            map_num =       mapping_tab->bit.num;
            map_8array =    mapping_tab->bit.array;
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            if((map_address <= query_address) && (query_address <= (map_address + map_num)) ) //起始地址
            {
                int diff = query_address - map_address;
                query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];//查询数量
                temp = (query_num / 8) + ((query_num % 8) ? 1 : 0); //数据长度
                if((0  < query_num )&&( query_num <= (map_num-diff)) && (temp == request[smb->core->len_header + 5]) )
                {
                    temp = _modbus_bit2array(map_8array+diff,&(response[smb->core->len_header + 6]),query_num);   //modbus bit 转数组
                    response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
                    response[response_len++] = (query_address>>8);
                    response[response_len++] = (query_address&0x00ff);
                    response[response_len++] = (query_num>>8);
                    response[response_len++] = (query_num&0x00ff);
                }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;}
            }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;}
        }break;
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            map_address =   mapping_tab->registers.start;
            map_num =       mapping_tab->registers.num;
            map_16array =    mapping_tab->registers.array;
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            if((map_address <= query_address) && (query_address <= (map_address + map_num)) ) //起始地址
            {
                int diff = query_address - map_address;
                query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];//查询数量
                temp = (query_num * 2); //数据长度
                if((0  < query_num )&&( query_num <= (map_num-diff)) && (temp == request[smb->core->len_header + 5]) )
                {
                    temp = _modbus_reg2array(map_16array+diff,&(response[smb->core->len_header + 6]),query_num);   //modbus bit 转数组
                    response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
                    response[response_len++] = (query_address>>8);
                    response[response_len++] = (query_address&0x00ff);
                    response[response_len++] = (query_num>>8);
                    response[response_len++] = (query_num&0x00ff);
                }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;}
            }else{response_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;}
        }break;
        case MODBUS_FC_REPORT_SLAVE_ID:
        {
            response_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
//            temp = strlen(_modbus_ver);
//            response_len = smb->core->build_response_header(smb,response,query_slave,query_function);
//            response[response_len++] =  temp+2;
//            response[response_len++] =  smb->slave_addr;
//            /* Run indicator status to ON */
//            response[response_len++] =  0XFF;
//            memcpy(response + response_len,_modbus_ver, temp);
//            response_len += temp;
        }break;
        case MODBUS_FC_READ_EXCEPTION_STATUS:
        {
            response_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        }break;
        case MODBUS_FC_MASK_WRITE_REGISTER:
        {
            response_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        }break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        {
            response_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        }break;
        default:
        {
            response_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        }break;
    }

    if(response_exception != MODBUS_OK)
    {
        switch(response_exception)
        {
            case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE:
            {
                modbus_debug_error(smb,"slave:0x%0X,function:0x%0X,Illegal num:%d not at [1-%d]\n",query_slave,query_function,query_num,map_num);
            }break;
            case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
            {
                modbus_debug_error(smb,"slave:0x%0X,function:0x%0X,Illegal data:%d not at [1-%d]\n",query_slave,query_function,query_address,map_address);
            }break;
            default:
            {
                modbus_debug_error(smb,"slave:0x%0X,Unknown Modbus function code: 0x%0X\n",query_slave,query_function);
             }break;
        }
        response_len = smb->core->build_response_header(smb,response,query_slave,query_function+0x80);
        response[response_len++] = response_exception;
    }

    if(response_len)
    {
        response_len = smb->core->check_send_pre(smb,response,response_len);
        if(response_len > 0)
        {
             _modbus_write(smb,response, response_len);
             return response_len;
        }
    }
    return MODBUS_FAIL_POLL;
}

int modbus_wait(small_modbus_t *smb,small_modbus_mapping_t * mapping_tab)
{
    int rc = 0;
    uint8_t *confirm = smb->write_buff;
    uint8_t *request = smb->read_buff;
    rc = modbus_wait_poll(smb,request);
    if(rc > 0)
    {
        rc = modbus_handle_poll(smb,request,rc,mapping_tab);
    }
    return rc;
}

/* read */
int modbus_read_bits(small_modbus_t *smb, int addr, int num,uint8_t *read_data)
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
        return response_len;
    }
    return request_len;
}
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num,uint8_t *read_data)
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
        return response_len;
    }
    return request_len;
}
int modbus_read_registers(small_modbus_t *smb, int addr, int num,uint16_t *read_data)
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
        return response_len;
    }
    return request_len;
}
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num,uint16_t *read_data)
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
        return response_len;
    }
    return request_len;
}

/* write */
int modbus_write_bit(small_modbus_t *smb, int addr,int write_status)
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
        return response_len;
    }
    return request_len;
}
int modbus_write_register(small_modbus_t *smb, int addr,int write_value)
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
        return response_len;
    }
    return request_len;
}
int modbus_write_bits(small_modbus_t *smb, int addr, int num,uint8_t *write_data)
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
        return response_len;
    }
    return request_len;
}
int modbus_write_registers(small_modbus_t *smb, int addr, int num,uint16_t *write_data)
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
        return response_len;
    }
    return request_len;
}

int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask)
{
    return MODBUS_FAIL;
}

int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb,uint16_t *src, int read_addr, int read_nb,uint16_t *dest)
{
    return MODBUS_FAIL;
}

