/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-28     Administrator       the first version
 */
#include "modbus_tcp_rtos.h"

static int tcp_open(small_modbus_t *smb)
{
    modbus_tcp_config_t *config = smb->port_data;
//    smb->port->debug(smb,0,"open:%s baud:%d \n",config->name,config->config.baud_rate);
//    rt_ringbuffer_reset(&(config->rx_ring));
    return 0;
}

static int tcp_close(small_modbus_t *smb)
{
    modbus_tcp_config_t *config = smb->port_data;
//    smb->port->debug(smb,0,"close:%s\n",config->name);
    return 0;
}

static int tcp_read(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    int rc = 0;
    modbus_tcp_config_t *config = smb->port_data;

//    rc = rt_ringbuffer_get(&(config->rx_ring), data, length);
//    //rc = rt_device_read(ctx_config->serial,0,data,length);
//
//    //if(smb->debug_level == 0)
//    {
//        int i;
//        rt_kprintf("read %d,%d :",rc,length);
//        for (i = 0; i < rc; i++)
//        {
//                rt_kprintf("<%02X>", data[i]);
//        }
//        rt_kprintf("\n");
//    }
    return rc;
}
static int tcp_write(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    modbus_tcp_config_t *config = smb->port_data;
//    if(config->rts_set)
//        config->rts_set(smb,1);
//
//    //write(ctx->fd, data, length);
//
//    rt_device_write(config->dev, 0, data, length);
//
//    if(config->rts_set)
//        config->rts_set(smb,0);
//
//   // if(smb->debug_level == 0)
//    {
//        int i;
//        rt_kprintf("write %d :",length);
//        for (i = 0; i < length; i++)
//        {
//                rt_kprintf("<%02X>", data[i]);
//        }
//        rt_kprintf("\n");
//    }
//
//    return length;
    return 0;
}
static int tcp_flush(small_modbus_t *smb)
{
    modbus_tcp_config_t *config = smb->port_data;
//    int rc = rt_ringbuffer_data_len(&(config->rx_ring));
//    rt_ringbuffer_reset(&(config->rx_ring));
//    return rc;
    return 0;
}
static int tcp_select(small_modbus_t *smb,int timeout_ms)
{
    modbus_tcp_config_t *config = smb->port_data;
//    int rc = rt_ringbuffer_data_len(&(config->rx_ring));
//    if(rc>0)
//    {
//        return MODBUS_OK;
//    }
//    rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
//    if(rt_sem_take(&(config->rx_sem), timeout_ms) == RT_EOK)
//    {
//        return MODBUS_OK;
//    }else
//    {
//       return MODBUS_TIMEOUT;
//    }
    return 0;
}
static int tcp_debug(small_modbus_t *smb,int level,const char *fmt, ...)
{
    //modbus_tcp_config_t *config = smb->port_data;
    static char log_buf[32];
    if(level <= smb->debug_level)
    {
        va_list args;
        va_start(args, fmt);
        rt_vsnprintf(log_buf, 32, fmt, args);
        va_end(args);
        rt_kprintf(log_buf);
    }
    return 0;
}

small_modbus_port_t _modbus_tcp_rtos_port =
{
    .open =  tcp_open,
    .close = tcp_close,
    .read =  tcp_read,
    .write = tcp_write,
    .flush = tcp_flush,
    .select = tcp_select,
    .debug =  tcp_debug
};

int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port,void *config)
{
    _modbus_init(smb);
    smb->core = (small_modbus_core_t*)&_modbus_tcp_core;
    smb->port_data = config;
    if(port ==NULL)
    {
        smb->port = &_modbus_tcp_rtos_port;
    }else {
        smb->port = port;
    }
    return 0;
}
int modbus_tcp_listen(small_modbus_t *smb,uint16_t port)
{
    return 0;
}
int modbus_tcp_accept(small_modbus_t *smb,int socket_fd)
{
    return 0;
}
int modbus_tcp_set_socket(small_modbus_t *smb,int socket_fd)
{
    return 0;
}
