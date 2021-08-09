/*
 * Change Logs:
 * Date           Author       Notes
 * 2021-03     		chenbin      small_modbus_rtthread.c  for rtthread
 */

#include "small_modbus_port_rtthread.h"
 /*
 * modbus on rtthread
 */
#if SMALL_MODBUS_RTTHREAD
#include "small_modbus_base.h"
#include "small_modbus_utils.h"
#include "small_modbus_rtu.h"
#include "small_modbus_tcp.h"

int _modbus_debug(small_modbus_t *smb,int level,const char *fmt, ...)
{
	static char log_buf[256];
	rt_enter_critical();
	if(level <= smb->debug_level)
	{
		va_list args;
		va_start(args, fmt);
		rt_vsnprintf(log_buf, 256, fmt, args);
		va_end(args);
		rt_kputs(log_buf);
	}
	rt_exit_critical();
	return 0;
}
/*
*modbus_init
*/
int modbus_init(small_modbus_t* smb, uint8_t core_type, void* port)
{
    small_modbus_port_t* smb_port = port;
    if (smb && core_type && smb_port)
    {
        _modbus_init(smb);
        if ((core_type == MODBUS_CORE_RTU) || (core_type == MODBUS_CORE_TCP))  // check core type
        {
            if (core_type == MODBUS_CORE_RTU)
            {
                smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
            }
            if (core_type == MODBUS_CORE_TCP)
            {
                smb->core = (small_modbus_core_t*)&_modbus_tcp_core;
            }
        }
        else
        {
            return 0;
        }
        if ((smb_port->type == MODBUS_PORT_DEVICE) || (smb_port->type == MODBUS_PORT_SOCKET))  // check port type
        {
            smb->port = smb_port;
            return 1;
        }
    }
    return 0;
}

small_modbus_t* modbus_create(uint8_t core_type, void* port)
{
    small_modbus_t* smb = rt_malloc_align(sizeof(small_modbus_t), 4);
    if (smb)
    {
        if (modbus_init(smb, core_type, port))
        {
            return smb;
        }
        else
        {
            rt_free_align(smb);
        }
    }
    return NULL;
}

/*
*modbus port device
*/
#ifdef SMALL_MODBUS_RTTHREAD_USE_DEVICDE

#include <rtthread.h>
#include <rtdevice.h>

static rt_err_t _modbus_rtdevice_rx_indicate(rt_device_t dev, rt_size_t size)
{
	small_modbus_port_rtdevice_t *smb_port_device = dev->user_data;
	
	smb_port_device->rx_size = size;
	
	return rt_sem_release(&(smb_port_device->rx_sem));
}

static int _modbus_rtdevice_open(small_modbus_t *smb)
{
	small_modbus_port_rtdevice_t *smb_port_device = (small_modbus_port_rtdevice_t*)smb->port;
	if(smb_port_device->device)
	{
		smb_port_device->device->user_data = smb_port_device;
		
		rt_device_set_rx_indicate(smb_port_device->device, _modbus_rtdevice_rx_indicate);
		
		rt_device_open(smb_port_device->device, smb_port_device->oflag);
		//rt_device_open(smb_port_device->device, RT_DEVICE_FLAG_INT_RX);
		//rt_device_open(smb_port_device->device, RT_DEVICE_FLAG_DMA_RX);
		
		if(smb_port_device->rts_set)
		{
			smb_port_device->rts_set(0);
		}
	}
	return 0;
}

static int _modbus_rtdevice_close(small_modbus_t *smb)
{
	small_modbus_port_rtdevice_t *smb_port_device = (small_modbus_port_rtdevice_t*)smb->port;
	if(smb_port_device->device)
	{
		rt_device_close(smb_port_device->device);
	}
	return 0;
}

static int _modbus_rtdevice_write(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_rtdevice_t *smb_port_device = (small_modbus_port_rtdevice_t*)smb->port;
	//rt_enter_critical();
	
	if(smb_port_device->rts_set)
			smb_port_device->rts_set(1);
	
	rt_enter_critical();
	
	rt_device_write(smb_port_device->device,0,data,length);
	
	rt_exit_critical();
	
	if(smb_port_device->rts_set)
			smb_port_device->rts_set(0);
	
	//rt_exit_critical();
	return length;
}

static int _modbus_rtdevice_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_rtdevice_t * smb_port_device = (small_modbus_port_rtdevice_t*)smb->port;
	
	return rt_device_read(smb_port_device->device,0,data,length);
}

static int _modbus_rtdevice_flush(small_modbus_t *smb)
{
	small_modbus_port_rtdevice_t* smb_port_device = (small_modbus_port_rtdevice_t*)smb->port;
	
	int rc = rt_device_read(smb_port_device->device,0,smb->read_buff,MODBUS_MAX_ADU_LENGTH);
	
	rt_sem_control(&(smb_port_device->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
	return rc;
}

static int _modbus_rtdevice_wait(small_modbus_t *smb,int timeout)
{
	int rc = -1;
	small_modbus_port_rtdevice_t * smb_port_device = (small_modbus_port_rtdevice_t*)smb->port;
	
	rc = rt_sem_take(&(smb_port_device->rx_sem),timeout);
	if(rc < RT_EOK)
	{
		return MODBUS_TIMEOUT;
	}
	if(smb_port_device->rx_size == 0)
	{
		return MODBUS_ERROR_READ;
	}
	return rc;
}


small_modbus_port_rtdevice_t * modbus_port_rtdevice_get(small_modbus_t *smb)
{
	if( (smb->port->type == MODBUS_PORT_DEVICE) && (smb->port->magic == MODBUS_PORT_MAGIC) )
	{
		return (small_modbus_port_rtdevice_t*)smb->port;
	}
	return NULL;
}

int modbus_port_rtdevice_init(small_modbus_port_rtdevice_t *port,const char *device_name)
{
	(*(uint32_t *)&(port->base.type)) = MODBUS_PORT_DEVICE;
	(*(uint32_t *)&(port->base.magic)) = MODBUS_PORT_MAGIC;
	port->base.open = _modbus_rtdevice_open;
	port->base.close = _modbus_rtdevice_close;
	port->base.read = _modbus_rtdevice_read;
	port->base.write = _modbus_rtdevice_write;
	port->base.flush = _modbus_rtdevice_flush;
	port->base.wait = _modbus_rtdevice_wait;
	
	port->device_name = device_name;
	port->device = rt_device_find(device_name);
	if(port->device)
	{
		rt_sem_init(&(port->rx_sem), device_name, 0, RT_IPC_FLAG_FIFO);
	}
	return 0;
}

int modbus_port_rtdevice_deinit(small_modbus_port_rtdevice_t *port)
{
	if(port->device)
	{
		rt_device_close(port->device);
	}
	return 0;
}

small_modbus_port_rtdevice_t *modbus_port_rtdevice_create(const char *device_name)
{
	small_modbus_port_rtdevice_t *smb_port_device = rt_malloc_align(sizeof(small_modbus_port_rtdevice_t),4);
	if(smb_port_device)
	{
		rt_memset(smb_port_device,0,sizeof(small_modbus_port_rtdevice_t));
		modbus_port_rtdevice_init(smb_port_device,device_name);
		return smb_port_device;
	}
	return NULL;
}

int modbus_port_rtdevice_delete(void *port)
{
	small_modbus_port_rtdevice_t *port_device = port;
	if( port_device && (port_device->base.type == MODBUS_PORT_DEVICE) && (port_device->base.magic == MODBUS_PORT_MAGIC))
	{
		modbus_port_rtdevice_deinit(port_device);
		rt_free_align(port_device);
	}
	return 0;
}

int modbus_rtu_set_serial_rts(small_modbus_t *smb,int (*rts_set)(int on))
{
	small_modbus_port_rtdevice_t *smb_port_device = modbus_port_rtdevice_get(smb);
	if(smb_port_device)
	{
		smb_port_device->rts_set = rts_set;
	}
	return 0;
}

int modbus_rtu_set_serial_name(small_modbus_t *smb,const char *device_name)
{
	small_modbus_port_rtdevice_t *smb_port_device = modbus_port_rtdevice_get(smb);
	if(smb_port_device)
	{
		modbus_port_rtdevice_init(smb_port_device,device_name);
	}
	return 0;
}

int modbus_rtu_set_serial_config(small_modbus_t *smb,struct serial_configure *serial_config)
{
	small_modbus_port_rtdevice_t *smb_port_device = modbus_port_rtdevice_get(smb);
	if(smb_port_device)
	{
		if(smb_port_device->device)
		{
			rt_device_control(smb_port_device->device, RT_DEVICE_CTRL_CONFIG, serial_config);
		}
	}
	return 0;
}
int modbus_rtu_set_oflag(small_modbus_t *smb,int oflag)
{
	small_modbus_port_rtdevice_t *smb_port_device = modbus_port_rtdevice_get(smb);
	if(smb_port_device)
	{
		smb_port_device->oflag = oflag;
	}
	return 0;
}

#endif

/*
*modbus port socket
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

static int _modbus_rtsocket_open(small_modbus_t *smb)
{
	return modbus_tcp_connect(smb);
}

static int _modbus_rtsocket_close(small_modbus_t *smb)
{
	return modbus_tcp_disconnect(smb);
}

static int _modbus_rtsocket_write(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_rtsocket_t *smb_port_socket = (small_modbus_port_rtsocket_t *)smb->port;

	struct timeval tv = {
			smb->timeout_frame / 1000,
			(smb->timeout_frame % 1000) * 1000
	};

	if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec <= 0)){
			tv.tv_sec = 0;
			tv.tv_usec = 1000*10;
	}
	setsockopt(smb_port_socket->socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));

	send(smb_port_socket->socket_fd, data, length, 0);

	return length;
}

static int _modbus_rtsocket_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_rtsocket_t *smb_port_socket = (small_modbus_port_rtsocket_t *)smb->port;

	uint32_t read_len = rt_ringbuffer_data_len(&(smb_port_socket->rx_ringbuff));
	if (read_len > length)
	{
			read_len = length; //min
	}
	if (read_len > 0)
	{
		 rt_ringbuffer_get(&(smb_port_socket->rx_ringbuff), data, read_len);
	}
	return read_len;
}

static int _modbus_rtsocket_flush(small_modbus_t *smb)
{
	small_modbus_port_rtsocket_t *smb_port_socket = (small_modbus_port_rtsocket_t *)smb->port;
	
	uint32_t read_len = rt_ringbuffer_data_len(&(smb_port_socket->rx_ringbuff));
	
	rt_ringbuffer_reset(&(smb_port_socket->rx_ringbuff));
	
	return read_len;
}

static int _modbus_rtsocket_wait(small_modbus_t *smb,int timeout)
{
	int rc = -1;
	int socket_status = 0;
	socklen_t slen;
	small_modbus_port_rtsocket_t *smb_port_socket = (small_modbus_port_rtsocket_t *)smb->port;
	
	struct timeval tv = {
			 timeout / 1000,
			(timeout % 1000) * 1000
	};

	if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec <= 0)) {
			tv.tv_sec = 0;
			tv.tv_usec = 1000*10;
	}
	setsockopt(smb_port_socket->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(struct timeval));

	rc = recv(smb_port_socket->socket_fd, smb_port_socket->rx_temp, sizeof(smb_port_socket->rx_temp), 0);
	if(rc <= 0)
	{
		getsockopt(smb_port_socket->socket_fd, SOL_SOCKET, SO_ERROR, (void *)&socket_status , &slen );
		
		if(!(socket_status == EINTR || socket_status == EWOULDBLOCK || socket_status == EAGAIN))
		{
				modbus_debug_error(smb,"rtsocket[%d] recv err(%d)\n", smb_port_socket->socket_fd, socket_status);
				rc = MODBUS_ERROR_READ;
		}else
		{
				//rt_kprintf("rtsocket timeout :%d\n",rc);
				rc = MODBUS_TIMEOUT;
		}
	}else
	{
		rt_ringbuffer_put(&(smb_port_socket->rx_ringbuff), smb_port_socket->rx_temp, rc);
	}
	return rc;
}


small_modbus_port_rtsocket_t * modbus_port_rtsocket_get(small_modbus_t *smb)
{
	if( (smb->port->type == MODBUS_PORT_SOCKET) && (smb->port->magic == MODBUS_PORT_MAGIC))
	{
		return (small_modbus_port_rtsocket_t*)smb->port;
	}
	return NULL;
}

int modbus_port_rtsocket_init(small_modbus_port_rtsocket_t *port,int devicemode,char *hostname,char *hostport)
{
	(*(uint32_t *)&(port->base.type)) = MODBUS_PORT_SOCKET;
	(*(uint32_t *)&(port->base.magic)) = MODBUS_PORT_MAGIC;
	port->base.open = _modbus_rtsocket_open;
	port->base.close = _modbus_rtsocket_close;
	port->base.read = _modbus_rtsocket_read;
	port->base.write = _modbus_rtsocket_write;
	port->base.flush = _modbus_rtsocket_flush;
	port->base.wait = _modbus_rtsocket_wait;
	
	rt_ringbuffer_init( &(port->rx_ringbuff) ,port->rx_ringbuff_pool, sizeof(port->rx_ringbuff_pool) );  //static init ringbuff
	
	if(hostname)
	{
		port->hostname = hostname;
	}
	if(hostport)
	{
		port->hostport = hostport;
	}
	port->devicemode = devicemode;
	port->socket_fd = -1;
	return 0;
}

int modbus_port_rtsocket_deinit(small_modbus_port_rtsocket_t *port)
{
	if(port->socket_fd >= 0)
	{
		closesocket( port->socket_fd );
		port->socket_fd = -1;
	}
	return 0;
}

small_modbus_port_rtsocket_t *modbus_port_rtsocket_create(int devicemode, char *hostname, char *hostport)
{
	small_modbus_port_rtsocket_t *port_socket = rt_malloc_align(sizeof(small_modbus_port_rtsocket_t),4);
	if(port_socket)
	{
		rt_memset(port_socket,0,sizeof(small_modbus_port_rtsocket_t));
		modbus_port_rtsocket_init(port_socket, devicemode, hostname, hostport);
		return port_socket;
	}
	return NULL;
}

int modbus_port_rtsocket_delete(void *port)
{
	small_modbus_port_rtsocket_t *port_socket = port;
	if( port_socket && (port_socket->base.type == MODBUS_PORT_SOCKET) && (port_socket->base.magic == MODBUS_PORT_MAGIC))
	{
		modbus_port_rtsocket_deinit(port_socket);
		rt_free_align(port_socket);
	}
	return 0;
}


/*
* modbus tcp api
*/
int modbus_tcp_status(small_modbus_t *smb)
{
	small_modbus_port_rtsocket_t *port_socket = (small_modbus_port_rtsocket_t *)smb->port;
	
	int socket_status = 0;
	socklen_t slen;
	
	if( (port_socket) && (port_socket->socket_fd >= 0) )
	{
		getsockopt(port_socket->socket_fd, SOL_SOCKET, SO_ERROR, (void *)&socket_status , &slen );
	}
	return socket_status;
}


int modbus_tcp_disconnect(small_modbus_t *smb)
{
	small_modbus_port_rtsocket_t *port_socket = (small_modbus_port_rtsocket_t *)smb->port;
	if( (port_socket) && (port_socket->socket_fd >= 0) )
	{
		closesocket( port_socket->socket_fd );
		port_socket->socket_fd = -1;
	}
	return 0;
}

int modbus_tcp_connect(small_modbus_t *smb)
{
	small_modbus_port_rtsocket_t *port_socket = modbus_port_rtsocket_get(smb);
	if( (port_socket) && (port_socket->socket_fd < 0) )
	{
		int ret = 0;
		unsigned long mode = 0;
		struct addrinfo hints, *addr_list, *cur;
		struct timeval timeout;
		
		struct sockaddr_in * socket_addr;
		char addr_str[64];
		
		/* Do name resolution with both IPv6 and IPv4 */
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		//hints.ai_protocol = PROTOCOL_TLS;

		ret = getaddrinfo(port_socket->hostname, port_socket->hostport, &hints, &addr_list);
		if (ret != 0)
		{
				modbus_debug_info(smb,"rtsocket getaddrinfo fail [%d]\n",ret);
				return ret;
		}
		for (cur = addr_list; cur != NULL; cur = cur->ai_next)
		{
			port_socket->socket_fd = socket(cur->ai_family,SOCK_STREAM, IPPROTO_TCP);
			if (port_socket->socket_fd < 0)
			{
				continue;
			}
			mode = 0;
			ioctlsocket(port_socket->socket_fd, FIONBIO, &mode);
			
			ret = connect(port_socket->socket_fd, cur->ai_addr, cur->ai_addrlen);
		
			socket_addr = (struct sockaddr_in *)(cur->ai_addr);
			
			inet_ntoa_r(socket_addr->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
			
			modbus_debug_info(smb,"rtsocket[%d] connect: %s:%d  ret:%d\n", port_socket->socket_fd, addr_str ,ntohs(socket_addr->sin_port),ret);
			if (ret == 0)
			{
				break;//连接成功
			}
			closesocket(port_socket->socket_fd);
		}
		freeaddrinfo(addr_list);
		
		timeout.tv_sec = smb->timeout_frame /1000;
		timeout.tv_usec = (smb->timeout_frame%1000)*1000;
		
		setsockopt(port_socket->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (void *) &timeout,sizeof(timeout));
		
		setsockopt(port_socket->socket_fd, SOL_SOCKET, SO_SNDTIMEO, (void *) &timeout,sizeof(timeout));
	}
	return port_socket->socket_fd;
}

int modbus_tcp_listen(small_modbus_t *smb, int max_connection)
{
	small_modbus_port_rtsocket_t *port_socket = modbus_port_rtsocket_get(smb);
	if( (port_socket) && (port_socket->socket_fd < 0) )
	{
		int ret = 0;
		unsigned long mode = 0;
		struct addrinfo hints, *addr_list, *cur;
		
		struct sockaddr_in * socket_addr;
		char addr_str[64];
		
		/* Do name resolution with both IPv6 and IPv4 */
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		//hints.ai_protocol = PROTOCOL_TLS;

		ret = getaddrinfo(port_socket->hostname, port_socket->hostport, &hints, &addr_list);
		if (ret != 0)
		{
				modbus_debug_info(smb,"rtsocket getaddrinfo fail [%d]\n",ret);
				return ret;
		}
		for (cur = addr_list; cur != NULL; cur = cur->ai_next)
		{
			port_socket->socket_fd = socket(cur->ai_family,SOCK_STREAM, IPPROTO_TCP);
			if (port_socket->socket_fd < 0)
			{
				continue;
			}
			mode = 0;
			ioctlsocket(port_socket->socket_fd, FIONBIO, &mode);
			mode = 1;
			setsockopt(port_socket->socket_fd, SOL_SOCKET, SO_REUSEADDR,(void *)&mode, sizeof (mode));
			
			ret = bind(port_socket->socket_fd, cur->ai_addr, cur->ai_addrlen);
			if(ret == 0)
			{
				ret = listen(port_socket->socket_fd, max_connection);
				
				socket_addr = (struct sockaddr_in *)(cur->ai_addr);
				inet_ntoa_r(socket_addr->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
				
				modbus_debug_info(smb,"rtsocket[%d] listen: %s:%d  ret:%d\n", port_socket->socket_fd, addr_str ,ntohs(socket_addr->sin_port),ret);
				if (ret == 0)
				{
					break;//连接成功
				}
			}else
			{
				modbus_debug_info(smb,"rtsocket[%d] bind:err ret:%d\n", port_socket->socket_fd, ret);
			}
			closesocket(port_socket->socket_fd);
		}
		freeaddrinfo(addr_list);
	}
	return port_socket->socket_fd;
}

int modbus_tcp_accept(small_modbus_t *smb,int socket_fd)
{
	small_modbus_port_rtsocket_t *port_socket = modbus_port_rtsocket_get(smb);
	
	struct sockaddr_storage addr;
  socklen_t addrlen;
	int ret_socket = -1;
	
	if( (port_socket) && (socket_fd >= 0) )
	{
		ret_socket = accept( socket_fd, (struct sockaddr *)&addr, &addrlen);
		if(ret_socket < 0)
		{
			closesocket(ret_socket);
		}
	}
	return ret_socket;
}



int modbus_tcp_get_socket(small_modbus_t *smb)
{
	small_modbus_port_rtsocket_t *port_socket = modbus_port_rtsocket_get(smb);
	if(port_socket)
	{
		return port_socket->socket_fd;
	}
	return 0;
}

int modbus_tcp_set_socket(small_modbus_t *smb,int socket)
{
	small_modbus_port_rtsocket_t *port_socket = modbus_port_rtsocket_get(smb);
	if(port_socket)
	{
		port_socket->socket_fd = socket;
	}
	return 0;
}

int modbus_tcp_set_socket_block(small_modbus_t *smb,int isblock)
{
	small_modbus_port_rtsocket_t *port_socket = modbus_port_rtsocket_get(smb);
	unsigned long mode = 0;
	if(port_socket)
	{
		if(isblock)
		{	
			mode = 0;
		}else
		{
			mode = 1;
		}
		ioctlsocket(port_socket->socket_fd, FIONBIO, &mode);
	}
	return 0;
}

#endif

#endif

