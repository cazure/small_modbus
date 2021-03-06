#ifndef _SMALL_MODBUS_RTTHREAD_H_
#define _SMALL_MODBUS_RTTHREAD_H_

#include <rtthread.h>
#include <rtdevice.h>
#include "string.h"
#include "small_modbus.h"
#include "small_modbus_rtu.h"
#include "small_modbus_tcp.h"

typedef struct _small_modbus_port_device   small_modbus_port_device_t;

struct _small_modbus_port_device
{
	struct _small_modbus_port port;
	const char 					*device_name;
	struct rt_device 		*device;
	struct rt_semaphore 	rx_sem;
	struct rt_ringbuffer 	rx_ring;
  struct serial_configure serial_config;
  int 		(*rts_set)(int on);
//	uint8_t read_buff[64];
	uint8_t _ring_buff[256];
	int			oflag;
	void 		*old_user_data;
	rt_err_t (*old_rx_indicate)(rt_device_t dev, rt_size_t size);
};


typedef struct _small_modbus_port_socket   small_modbus_port_socket_t;

struct _small_modbus_port_socket
{
	struct _small_modbus_port port;
	const char *hostname;
	const char *hostport;
	
	void				*device;
	int32_t     socket_fd;
};

//slave server
//int modbus_tcp_accept(small_modbus_t *smb);
//int modbus_tcp_select(small_modbus_t *smb,int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
//int modbus_tcp_set_socket(small_modbus_t *smb,int socket_fd);

//int modbus_rtu_init(small_modbus_t *smb,void *port);
//int modbus_tcp_init(small_modbus_t *smb,void *port);


int modbus_rtu_init(small_modbus_t *smb,small_modbus_port_t *port);
int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port);
/*
*device
*/
int modbus_port_device_init(small_modbus_port_device_t *port,const char *device_name);
small_modbus_port_t *modbus_port_device_create(const char *device_name);

int modbus_port_device_set_config(small_modbus_t *smb,struct serial_configure *serial_config);
int modbus_port_device_set_rts(small_modbus_t *smb,int (*rts_set)(int on));

/*
*socket
*/
int modbus_port_socket_init(small_modbus_port_socket_t *port,char *hostname,char *hostport);
small_modbus_port_t *modbus_port_socket_create(char *hostname,char *hostport);


small_modbus_t *modbus_rtu_create(const char *device_name);
small_modbus_t *modbus_tcp_create(char *hostname,char *hostport);

#endif /* _SMALL_MODBUS_RTTHREAD_H_ */

