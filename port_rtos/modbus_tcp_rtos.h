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

typedef struct _modbus_tcp_config
{
    int      server_socket;
    uint16_t server_port;

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


int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port,void *config);
int modbus_tcp_listen(small_modbus_t *smb,uint16_t port);
int modbus_tcp_accept(small_modbus_t *smb,int socket_fd);
int modbus_tcp_set_socket(small_modbus_t *smb,int socket_fd);



#endif /* _PORT_RTOS_MODBUS_TCP_RTOS_H_ */