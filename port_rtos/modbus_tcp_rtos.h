/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-28     Administrator       the first version
 */
#ifndef _PORT_RTOS_MODBUS_TCP_RTOS_H_
#define _PORT_RTOS_MODBUS_TCP_RTOS_H_

#include "small_modbus_tcp.h"
#include <rtthread.h>
#include <rtdevice.h>

//typedef struct _rtt_uart
//{
//    char name[8];
//    struct rt_device *dev;
//    struct serial_configure config;
//    struct rt_semaphore rx_sem;
//    struct rt_ringbuffer rx_ring;
//} rtt_uart_t;
//
//extern rtt_uart_t rtt_uart3;
//extern rtt_uart_t rtt_uart6;

typedef struct _modbus_tcp_config
{

    int         fd;
    uint16_t    tid;
    char name[8];
    uint8_t _read_buff[256];
    uint8_t _rx_buff[256];
    struct rt_device *dev;
    struct serial_configure config;
    struct rt_semaphore rx_sem;
    struct rt_ringbuffer rx_ring;
    int (*rts_set)(small_modbus_t *ctx, int on);
} modbus_tcp_config_t;

extern modbus_tcp_config_t tcp_config;

int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port,void *config);
int modbus_tcp_listen(small_modbus_t *smb,int port);
int modbus_tcp_accept(small_modbus_t *smb,int port);
int modbus_tcp_set_socket(small_modbus_t *smb,int fd);



#endif /* _PORT_RTOS_MODBUS_TCP_RTOS_H_ */
