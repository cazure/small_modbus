#ifndef _SMALL_MODBUS_PORT_RTTHREAD_H_
#define _SMALL_MODBUS_PORT_RTTHREAD_H_

#include "stdint.h"
#include "string.h"
#include "small_modbus_base.h"
#include "config_small_modbus.h"

/*
* modbus on rtthread
*/
#if SMALL_MODBUS_RTTHREAD
/*
*modbus_init
*/
int modbus_init(small_modbus_t* smb, uint8_t core_type, void* port);
small_modbus_t* modbus_create(uint8_t core_type, void* port);

/*
*modbus port rtthread device
*/
#if SMALL_MODBUS_RTTHREAD_USE_DEVICDE

#include <rtthread.h>
#include <rtdevice.h>

typedef struct small_modbus_port_rtdevice   small_modbus_port_rtdevice_t;

struct small_modbus_port_rtdevice
{
	struct _small_modbus_port 	base;
  struct serial_configure     serial_config;
	const char                  *device_name;
	struct rt_device            *device;
	struct rt_semaphore         rx_sem;
	rt_size_t                   rx_size;
	int                         oflag;
	int                         (*rts_set)(int on);
};

small_modbus_port_rtdevice_t *modbus_port_rtdevice_get(small_modbus_t *smb);

int modbus_port_rtdevice_init(small_modbus_port_rtdevice_t *port,const char *device_name);
int modbus_port_rtdevice_deinit(small_modbus_port_rtdevice_t *port);

small_modbus_port_rtdevice_t *modbus_port_rtdevice_create(const char *device_name);
int modbus_port_rtdevice_delete(void *port);

int modbus_rtu_set_serial_rts(small_modbus_t *smb,int (*rts_set)(int on));
int modbus_rtu_set_serial_name(small_modbus_t *smb,const char *device_name);
int modbus_rtu_set_serial_config(small_modbus_t *smb,struct serial_configure *serial_config);
int modbus_rtu_set_oflag(small_modbus_t *smb,int oflag);

#endif

/*
*modbus port rtthread socket
*/
#if SMALL_MODBUS_RTTHREAD_USE_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(RT_USING_LIBC) || defined(RT_USING_MINILIBC) || defined(RT_LIBC_USING_TIME)
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#endif

/* support both enable and disable "SAL_USING_POSIX" */
#if defined(RT_USING_SAL)
#include <netdb.h>
#include <sys/socket.h>
#else
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#endif /* RT_USING_SAL */

#include "ipc/ringbuffer.h"

typedef struct small_modbus_port_rtsocket   small_modbus_port_rtsocket_t;

struct small_modbus_port_rtsocket
{
	struct _small_modbus_port 	base;
	int32_t     								socket_fd;
	int													devicemode;
	const char *								hostname;
	const char *								hostport;
	
	struct rt_ringbuffer 			rx_ringbuff;
	uint8_t 									rx_temp[256];
	uint8_t 									rx_ringbuff_pool[256];
};


int modbus_tcp_status(small_modbus_t *smb);
int modbus_tcp_disconnect(small_modbus_t *smb);
int modbus_tcp_connect(small_modbus_t *smb);
int modbus_tcp_listen(small_modbus_t *smb, int max_connection);
int modbus_tcp_accept(small_modbus_t *smb,int socket_fd);

int modbus_tcp_get_socket(small_modbus_t *smb);
int modbus_tcp_set_socket(small_modbus_t *smb,int socket);
int modbus_tcp_set_socket_block(small_modbus_t *smb,int isblock);


small_modbus_port_rtsocket_t * modbus_port_rtsocket_get(small_modbus_t *smb);

int modbus_port_rtsocket_init(small_modbus_port_rtsocket_t *port,int devicemode,char *hostname,char *hostport);
int modbus_port_rtsocket_deinit(small_modbus_port_rtsocket_t *port);

small_modbus_port_rtsocket_t *modbus_port_rtsocket_create(int devicemode, char *hostname, char *hostport);
int modbus_port_rtsocket_delete(void *port);

#endif

#endif  /* SMALL_MODBUS_RTTHREAD */

#endif /* _SMALL_MODBUS_RTTHREAD_H_ */

