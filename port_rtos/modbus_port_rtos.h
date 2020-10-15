#ifndef _MODBUS_PORT_RTOS_H_
#define _MODBUS_PORT_RTOS_H_

#include "small_modbus_rtu.h"
#include "small_modbus_tcp.h"
#include <rtthread.h>
#include <rtdevice.h>

#include "hw_port.h"

//slave server
int modbus_tcp_accept(small_modbus_t *smb);
int modbus_tcp_select(small_modbus_t *smb,int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int modbus_tcp_set_socket(small_modbus_t *smb,int socket_fd);


int modbus_rtu_init(small_modbus_t *smb,void *port);
int modbus_tcp_init(small_modbus_t *smb,void *port);

#endif /* _MODBUS_PORT_RTOS_H_ */
