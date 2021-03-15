#ifndef _SMALL_MODBUS_RTTHREAD_H_
#define _SMALL_MODBUS_RTTHREAD_H_

#include <rtthread.h>
#include <rtdevice.h>
#include "string.h"
#include "small_modbus.h"
#include "small_modbus_rtu.h"
#include "small_modbus_tcp.h"

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#endif

typedef struct _small_modbus_port_device   small_modbus_port_device_t;

struct _small_modbus_port_device
{
	struct _small_modbus_port 	base;
	const char								*device_name;
	struct rt_device					*device;
	struct rt_semaphore 			rx_sem;
	rt_size_t									rx_size;
	int												oflag;
	
  int 		(*rts_set)(int on);
};


typedef struct _small_modbus_port_socket   small_modbus_port_socket_t;

struct _small_modbus_port_socket
{
	struct _small_modbus_port base;
	const char *hostname;
	const char *hostport;
	
	void				*device;
	int32_t     socket_fd;
};

/*
*modbus port device
*/
int modbus_port_device_init(small_modbus_port_device_t *port,const char *device_name);
small_modbus_port_device_t *modbus_port_device_create(const char *device_name);
small_modbus_port_device_t *modbus_port_device_get(small_modbus_t *smb);

int modbus_set_rts(small_modbus_t *smb,int (*rts_set)(int on));
int modbus_set_serial_config(small_modbus_t *smb,struct serial_configure *serial_config);
int modbus_set_oflag(small_modbus_t *smb,int oflag);

/*
*modbus port socket
*/
int modbus_port_socket_init(small_modbus_port_socket_t *port,char *hostname,char *hostport);
small_modbus_port_socket_t *modbus_port_socket_create(char *hostname,char *hostport);
small_modbus_port_socket_t *modbus_port_socket_get(small_modbus_t *smb);

/*
*modbus_init
*/
int modbus_init(small_modbus_t *smb,uint8_t core_type,void *port);
small_modbus_t *modbus_create(uint8_t core_type,void *port);

#endif /* _SMALL_MODBUS_RTTHREAD_H_ */

