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
#include <dfs_posix.h>

#ifdef RT_USING_SAL
#include <sys/socket.h>
#else
#include <lwip/sockets.h>
#endif

typedef struct _modbus_tcp_config
{
    uint16_t    transfer_id;
    uint16_t    protocol_id;
    int32_t     socket_fd;

    char         ip[16];
    uint16_t     port;
    uint16_t     isSlave;
    int          socket;
    struct  timeval     tv;
    struct sockaddr_in  socket_addr;
} modbus_tcp_config_t;


int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port,void *config);
int modbus_tcp_config(small_modbus_t *smb,uint16_t isSlave,char *ip,uint16_t port);

//slave server
int modbus_tcp_accept(small_modbus_t *smb);
int modbus_tcp_select(small_modbus_t *smb,int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int modbus_tcp_set_socket(small_modbus_t *smb,int socket_fd);



#endif /* _PORT_RTOS_MODBUS_TCP_RTOS_H_ */
