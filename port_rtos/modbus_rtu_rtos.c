/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "modbus_rtu_rtos.h"
#include "string.h"

#include "modbus_rtu_rtos.h"


int _modbus1_rts(small_modbus_t *ctx, int on)
{
    hw_led_set(1,on);
    hw_rs485_set(1,on);
    return 0;
}

int _modbus2_rts(small_modbus_t *ctx, int on)
{
    hw_led_set(1,on);
    hw_rs485_set(2,on);
    return 0;
}


modbus_rtu_config_t uart3_config =
{
        .name = "uart3",
        .rts_set = _modbus1_rts
};
modbus_rtu_config_t uart6_config =
{
        .name = "uart6",
        .rts_set = _modbus2_rts
};

/* 接收数据回调函数 */
static rt_err_t uart3_indicate(rt_device_t dev, rt_size_t size)
{
//    if(size)
//        rt_sem_release(&(uart3_config.rx_sem));
    if (size > 0)
    {
        int rc = rt_device_read(dev,0,uart3_config._read_buff,256);
        if(rc>0)
        {
            int len = rt_ringbuffer_data_len(&(uart3_config.rx_ring));
            if((len == 0)&&(uart3_config._read_buff[0] == 0x00))
            {
                rt_ringbuffer_put(&(uart3_config.rx_ring), uart3_config._read_buff+1, rc-1);
            }else
            {
                rt_ringbuffer_put(&(uart3_config.rx_ring), uart3_config._read_buff, rc);
            }
            rt_sem_release(&(uart3_config.rx_sem));
        }
    }
    return RT_EOK;
}
static rt_err_t uart6_indicate(rt_device_t dev, rt_size_t size)
{
//    if(size)
//        rt_sem_release(&(uart6_config.rx_sem));
    if (size > 0)
    {
        int rc = rt_device_read(dev,0,uart6_config._read_buff,256);
        if(rc>0)
        {
            int len = rt_ringbuffer_data_len(&(uart6_config.rx_ring));
            if((len == 0)&&(uart6_config._read_buff[0] == 0x00))
            {
                rt_ringbuffer_put(&(uart6_config.rx_ring), uart6_config._read_buff+1, rc-1);
            }else
            {
                rt_ringbuffer_put(&(uart6_config.rx_ring), uart6_config._read_buff, rc);
            }
            rt_sem_release(&(uart6_config.rx_sem));
        }
    }
    return RT_EOK;
}

int hw_uart_init(void)
{
    uart3_config.dev = rt_device_find("uart3");
    if(uart3_config.dev != NULL)
    {
        rt_sem_init(&(uart3_config.rx_sem), "uart3", 0, RT_IPC_FLAG_FIFO);
        rt_ringbuffer_init(&(uart3_config.rx_ring), uart3_config._rx_buff, 256);

        uart3_config.config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
        uart3_config.config.data_bits = DATA_BITS_8;           //数据位 8
        uart3_config.config.stop_bits = STOP_BITS_1;           //停止位 1
        uart3_config.config.bufsz     = 256;                   //修改缓冲区 buff size 为 128
        uart3_config.config.parity    = PARITY_NONE;           //无奇偶校验位

        rt_device_control(uart3_config.dev, RT_DEVICE_CTRL_CONFIG, &(uart3_config.config));

        /* 以中断接收及轮询发送模式打开串口设备 */
        rt_device_open(uart3_config.dev, RT_DEVICE_FLAG_RDWR|RT_DEVICE_FLAG_DMA_RX);
        /* 设置接收回调函数 */
        rt_device_set_rx_indicate(uart3_config.dev, uart3_indicate);
    }

    uart6_config.dev = rt_device_find("uart6");
    if(uart6_config.dev != NULL)
    {
        rt_sem_init(&(uart6_config.rx_sem), "uart6", 0, RT_IPC_FLAG_FIFO);
        rt_ringbuffer_init(&(uart6_config.rx_ring), uart6_config._rx_buff, 256);

        uart6_config.config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
        uart6_config.config.data_bits = DATA_BITS_8;           //数据位 8
        uart6_config.config.stop_bits = STOP_BITS_1;           //停止位 1
        uart6_config.config.bufsz     = 256;                   //修改缓冲区 buff size 为 128
        uart6_config.config.parity    = PARITY_NONE;           //无奇偶校验位

        rt_device_control(uart6_config.dev, RT_DEVICE_CTRL_CONFIG, &(uart6_config.config));

        /* 以中断接收及轮询发送模式打开串口设备 */
        rt_device_open(uart6_config.dev, RT_DEVICE_FLAG_RDWR|RT_DEVICE_FLAG_DMA_RX);
        /* 设置接收回调函数 */
        rt_device_set_rx_indicate(uart6_config.dev, uart6_indicate);
    }
    return 0;
}


//static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
//{
//    modbus_rtu_config_t *config = dev->user_data;
//    static uint8_t buff[128];
//    if (size > 0)
//    {
//        int rc = rt_device_read(config->serial,0,buff,128);
//        if(rc>0)
//        {
//            rt_ringbuffer_put(&(config->rx_ring), buff, rc);
//            rt_sem_release(&(config->rx_sem));
//        }
//    }
//    return RT_EOK;
//}


int rtos_open(small_modbus_t *smb)
{
    modbus_rtu_config_t *config = smb->port_data;
    smb->port->debug(smb,0,"open:%s baud:%d \n",config->name,config->config.baud_rate);
    rt_ringbuffer_reset(&(config->rx_ring));
    return 0;
}

int rtos_close(small_modbus_t *smb)
{
    modbus_rtu_config_t *config = smb->port_data;
    smb->port->debug(smb,0,"close:%s\n",config->name);
    return 0;
}

int rtos_read(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    int rc = 0;
    modbus_rtu_config_t *config = smb->port_data;

    rc = rt_ringbuffer_get(&(config->rx_ring), data, length);
    //rc = rt_device_read(ctx_config->serial,0,data,length);

    if(smb->debug_level == 0)
    {
        int i;
        rt_kprintf("read %d,%d :",rc,length);
        for (i = 0; i < rc; i++)
        {
                rt_kprintf("<%02X>", data[i]);
        }
        rt_kprintf("\n");
    }
    return rc;
}
int rtos_write(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    modbus_rtu_config_t *config = smb->port_data;
    if(config->rts_set)
        config->rts_set(smb,1);

    //write(ctx->fd, data, length);

    rt_device_write(config->dev, 0, data, length);

    if(config->rts_set)
        config->rts_set(smb,0);

    if(smb->debug_level == 0)
    {
        int i;
        rt_kprintf("write %d :",length);
        for (i = 0; i < length; i++)
        {
                rt_kprintf("<%02X>", data[i]);
        }
        rt_kprintf("\n");
    }

    return length;
}
int rtos_flush(small_modbus_t *smb)
{
    modbus_rtu_config_t *config = smb->port_data;
    int rc = rt_ringbuffer_data_len(&(config->rx_ring));
    rt_ringbuffer_reset(&(config->rx_ring));
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

void rtos_debug(small_modbus_t *smb,int level,const char *fmt, ...)
{
    //modbus_rtu_config_t *config = smb->port_data;
    static char log_buf[32];
    if(level <= smb->debug_level)
    {
        va_list args;
        va_start(args, fmt);
        rt_vsnprintf(log_buf, 32, fmt, args);
        va_end(args);
        rt_kprintf(log_buf);
    }
}



small_modbus_port_t modbus_rtu_rtos_port =
{
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
    _modbus_init(smb);
    smb->core = (small_modbus_core_t*)&modbus_rtu_core;
    smb->port_data = config;
    if(port ==NULL)
    {
        smb->port = &modbus_rtu_rtos_port;
    }else {
        smb->port = port;
    }
    return 0;
}
//int modbus_rtu_config(small_modbus_t *smb,char *device_name,rtt_uart_t *uart)
//{
//    modbus_rtu_config_t *config = smb->port_data;
//
//    memcpy(config->name,device_name,6);
//    config->uart = uart;
//    return 0;
//}
//
//int modbus_rtu_set_rts_ops(small_modbus_t *smb,int (*rts_set)(small_modbus_t *smb, int on))
//{
//    modbus_rtu_config_t *config = smb->port_data;
//    config->rts_set = rts_set;
//    return 0;
//}

//int debug_modbus(int argc, char**argv)
//{
//    if(argc<2)
//    {
//        rt_kprintf("debug_modbus [0-2]\n");
//    }else
//    {
//        now_level  = atoi(argv[1])%3;
//    }
//    return RT_EOK;
//}
//MSH_CMD_EXPORT(debug_modbus,debug_modbus [0-5])


