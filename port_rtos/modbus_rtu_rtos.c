/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "modbus_rtu_rtos.h"

static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    modbus_rtu_config_t *config = dev->user_data;
    static uint8_t buff[128];
    if (size > 0)
    {
        int rc = rt_device_read(config->serial,0,buff,128);
        if(rc>0)
        {
            rt_ringbuffer_put(&(config->rx_ring), buff, rc);
            rt_sem_release(&(config->rx_sem));
        }
    }
    return RT_EOK;
}

uint8_t _rx_buff[128];

int rtos_open(small_modbus_t *smb)
{
    modbus_rtu_config_t *config = smb->port_data;

    config->serial = rt_device_find(config->device_name);
    if(config->serial!=NULL)
    {
        config->serial->user_data = smb->port_data;

        rt_sem_init(&(config->rx_sem), config->device_name , 0, RT_IPC_FLAG_FIFO);
        rt_ringbuffer_init(&(config->rx_ring), _rx_buff, 128);

        config->serial_config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
        config->serial_config.data_bits = DATA_BITS_8;           //数据位 8
        config->serial_config.stop_bits = STOP_BITS_1;           //停止位 1
        config->serial_config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
        config->serial_config.parity    = PARITY_NONE;           //无奇偶校验位

        rt_device_control(config->serial, RT_DEVICE_CTRL_CONFIG, &(config->serial_config));

        rt_device_open(config->serial,RT_DEVICE_FLAG_DMA_RX);

        rt_device_set_rx_indicate(config->serial, uart_rx_ind);

        smb->port->debug(smb,0,"open:%s baud:%d\n",config->device_name,config->serial_config.baud_rate);
        return 0;
    }
    smb->port->debug(smb,0,"open:err %p",config->serial);
    return 0;
}

int rtos_close(small_modbus_t *smb)
{
    modbus_rtu_config_t *config = smb->port_data;
    if(config->serial!=NULL)
    {
        rt_device_close(config->serial);
        smb->port->debug(smb,0,"close:%s\n",config->device_name);
    }
    return 0;
}

int rtos_read(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    int rc = 0;
    modbus_rtu_config_t *config = smb->port_data;

    rc = rt_ringbuffer_get(&(config->rx_ring), data, length);
    //rc = rt_device_read(ctx_config->serial,0,data,length);

    int i;
    rt_kprintf("read %d,%d :",rc,length);
    for (i = 0; i < rc; i++)
    {
            rt_kprintf("<%02X>", data[i]);
    }
    rt_kprintf("\n");

    return rc;
}
int rtos_write(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    modbus_rtu_config_t *config = smb->port_data;
    if(config->rts_set)
        config->rts_set(smb,1);

    //write(ctx->fd, data, length);

    rt_device_write(config->serial, 0, data, length);

    if(config->rts_set)
        config->rts_set(smb,0);

    int i;
    rt_kprintf("write %d :",length);
    for (i = 0; i < length; i++)
    {
            rt_kprintf("<%02X>", data[i]);
    }
    rt_kprintf("\n");

    return length;
}
int rtos_flush(small_modbus_t *smb)
{
    modbus_rtu_config_t *config = smb->port_data;
    int rc = rt_ringbuffer_data_len(&(config->rx_ring));
    rt_ringbuffer_reset(&(config->rx_ring));
    //return rt_device_control(ctx_config->serial,TCFLSH,TCIOFLUSH);
    return rc;
}

int rtos_select(small_modbus_t *smb,int timeout_ms)
{
    modbus_rtu_config_t *config = smb->port_data;

    int rc = rt_ringbuffer_data_len(&(config->rx_ring));
    if(rc>0)
    {
        return MODBUS_OK;
    }
    rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
    if(rt_sem_take(&(config->rx_sem), timeout_ms) == RT_EOK)
    {
        return MODBUS_OK;
    }else
    {
        return MODBUS_TIMEOUT;
    }
}

static uint8_t now_level = 0;

void rtos_debug(small_modbus_t *smb,int level,const char *fmt, ...)
{
    modbus_rtu_config_t *config = smb->port_data;
    static char log_buf[32];
    if(level <= now_level)
    {
        va_list args;
        va_start(args, fmt);
        rt_vsnprintf(log_buf, 32, fmt, args);
        va_end(args);
        rt_kprintf(log_buf);
    }
}

int debug_modbus(int argc, char**argv)
{
    if(argc<2)
    {
        rt_kprintf("debug_modbus [0-2]\n");
    }else
    {
        now_level  = atoi(argv[1])%3;
    }
    return RT_EOK;
}
MSH_CMD_EXPORT(debug_modbus,debug_modbus [0-5])

small_modbus_port_t modbus_rtu_rtos_port =
{
    .read_timeout = 300,
    .write_timeout = 300,
    .open =  rtos_open,
    .close = rtos_close,
    .read =  rtos_read,
    .write = rtos_write,
    .flush = rtos_flush,
    .select = rtos_select,
    .debug =  rtos_debug
};


int modbus_rtu_init(small_modbus_t *smb,small_modbus_port_t *port,void *config)
{
    smb->core = (small_modbus_core_t*)&modbus_rtu_core;
    smb->port_data = config;
    smb->port = port;
    return 0;
}

int modbus_rtu_config(small_modbus_t *smb,char *device_name,int baud,uint8_t data_bit, uint8_t stop_bit,uint8_t parity)
{
    modbus_rtu_config_t *config = smb->port_data;

    memcpy(config->device_name,device_name,6);

    config->serial_config.baud_rate = baud;
    config->serial_config.data_bits = data_bit;
    config->serial_config.stop_bits = stop_bit;
    config->serial_config.parity = parity;
    config->serial_config.bufsz = 128;

//    ctx_config->serial_config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
//    ctx_config->serial_config.data_bits = DATA_BITS_8;           //数据位 8
//    ctx_config->serial_config.stop_bits = STOP_BITS_1;           //停止位 1
//    ctx_config->serial_config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
//    ctx_config->serial_config.parity    = PARITY_NONE;           //无奇偶校验位
    return 0;
}

int modbus_rtu_set_rts_ops(small_modbus_t *smb,int (*rts_set)(small_modbus_t *smb, int on))
{
    modbus_rtu_config_t *config = smb->port_data;
    config->rts_set = rts_set;
    return 0;
}
int modbus_rtu_set_read_timeout(small_modbus_t *smb,int timeout_ms)
{
    smb->port->read_timeout = timeout_ms;
    return timeout_ms;
}

int modbus_rtu_set_write_timeout(small_modbus_t *smb,int timeout_ms)
{
    smb->port->write_timeout = timeout_ms;
    return timeout_ms;
}





