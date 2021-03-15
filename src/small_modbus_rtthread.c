/*
 * Change Logs:
 * Date           Author       Notes
 * 2021-03     		chenbin      small_modbus_rtthread.c  for rtthread
 */
#include "small_modbus_rtthread.h"
#ifdef RT_USING_DEVICE
#include <rtdevice.h>
#endif

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

static rt_err_t _modbus_rtdevice_rx_indicate(rt_device_t dev, rt_size_t size)
{
	small_modbus_port_device_t *smb_port_device = dev->user_data;
	
	smb_port_device->rx_size = size;
	
	return rt_sem_release(&(smb_port_device->rx_sem));
}

static int _modbus_rtdevice_open(small_modbus_t *smb)
{
	small_modbus_port_device_t *smb_port_device = (small_modbus_port_device_t *)smb->port;
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
	small_modbus_port_device_t *smb_port_device = (small_modbus_port_device_t *)smb->port;
	if(smb_port_device->device)
	{
		rt_device_close(smb_port_device->device);
	}
	return 0;
}

static int _modbus_rtdevice_write(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_device_t *smb_port_device = (small_modbus_port_device_t *)smb->port;
	rt_enter_critical();
	
	if(smb_port_device->rts_set)
			smb_port_device->rts_set(1);
		
	rt_device_write(smb_port_device->device,0,data,length);
	
	if(smb_port_device->rts_set)
			smb_port_device->rts_set(0);
	
	rt_exit_critical();
	return length;
}

static int _modbus_rtdevice_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
	
	return rt_device_read(port_device->device,0,data,length);
}

static int _modbus_rtdevice_flush(small_modbus_t *smb)
{
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
	
	int rc = rt_device_read(port_device->device,0,smb->read_buff,MODBUS_MAX_ADU_LENGTH);
	
	rt_sem_control(&(port_device->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
	return rc;
}

static int _modbus_rtdevice_wait(small_modbus_t *smb,int timeout)
{
	int rc = -1;
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
	
	rc = rt_sem_take(&(port_device->rx_sem),timeout);
	if(rc < RT_EOK)
	{
		return MODBUS_TIMEOUT;
	}
	if(port_device->rx_size == 0)
	{
		return MODBUS_ERROR_READ;
	}
	return rc;
}
/*
*modbus port device
*/
//small_modbus_port_t _port_device_default = 
//{
//	.type = MODBUS_PORT_DEVICE,
//	.open = _modbus_rtdevice_open,
//	.close = _modbus_rtdevice_close,
//	.read = _modbus_rtdevice_read,
//	.write = _modbus_rtdevice_write,
//	.flush = _modbus_rtdevice_flush,
//	.wait = _modbus_rtdevice_wait
//};
int modbus_port_device_init(small_modbus_port_device_t *port,const char *device_name)
{
	//rt_memcpy(&port->base,&_port_device_default,sizeof(small_modbus_port_t));
	
	(*(uint32_t *)&(port->base.type)) = MODBUS_PORT_DEVICE;
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
small_modbus_port_device_t *modbus_port_device_create(const char *device_name)
{
	small_modbus_port_device_t *port_device = rt_malloc_align(sizeof(small_modbus_port_device_t),4);
	if(port_device)
	{
		rt_memset(port_device,0,sizeof(small_modbus_port_device_t));
		modbus_port_device_init(port_device,device_name);
		return port_device;
	}
	return NULL;
}
small_modbus_port_device_t * modbus_port_device_get(small_modbus_t *smb)
{
	if(smb->port->type == MODBUS_PORT_DEVICE)
	{
		return (small_modbus_port_device_t*)smb->port;
	}
	return NULL;
}

int modbus_set_rts(small_modbus_t *smb,int (*rts_set)(int on))
{
	small_modbus_port_device_t *port_device = modbus_port_device_get(smb);
	if(port_device)
	{
		port_device->rts_set = rts_set;
	}
	return 0;
}
int modbus_set_serial_config(small_modbus_t *smb,struct serial_configure *serial_config)
{
	small_modbus_port_device_t *port_device = modbus_port_device_get(smb);
	if(port_device)
	{
		if(port_device->device)
		{
			rt_device_control(port_device->device, RT_DEVICE_CTRL_CONFIG, serial_config);
		}
	}
	return 0;
}
int modbus_set_oflag(small_modbus_t *smb,int oflag)
{
	small_modbus_port_device_t *port_device = modbus_port_device_get(smb);
	if(port_device)
	{
		port_device->oflag = oflag;
	}
	return 0;
}


/*
*modbus port socket
*/
//small_modbus_port_t _port_socket_default = 
//{
//	.type = MODBUS_PORT_SOCKET,
//	.open = _modbus_rtdevice_open,
//	.close = _modbus_rtdevice_close,
//	.read = _modbus_rtdevice_read,
//	.write = _modbus_rtdevice_write,
//	.flush = _modbus_rtdevice_flush,
//	.wait = _modbus_rtdevice_wait
//};
int modbus_port_socket_init(small_modbus_port_socket_t *port,char *hostname,char *hostport)
{
//	rt_memcpy(&port->base,&_port_socket_default,sizeof(small_modbus_port_t));
	//rt_memcpy(&port->base,&_port_device_default,sizeof(small_modbus_port_t));
	
	(*(uint32_t *)&(port->base.type)) = MODBUS_PORT_DEVICE;
	port->base.open = _modbus_rtdevice_open;
	port->base.close = _modbus_rtdevice_close;
	port->base.read = _modbus_rtdevice_read;
	port->base.write = _modbus_rtdevice_write;
	port->base.flush = _modbus_rtdevice_flush;
	port->base.wait = _modbus_rtdevice_wait;
	
	port->hostname = hostname;
	port->hostport = hostport;
	port->socket_fd = 0;
	
	return 0;
}

small_modbus_port_socket_t *modbus_port_socket_create(char *hostname,char *hostport)
{
	small_modbus_port_socket_t *port_socket = rt_malloc_align(sizeof(small_modbus_port_socket_t),4);
	if(port_socket)
	{
		rt_memset(port_socket,0,sizeof(small_modbus_port_socket_t));
		modbus_port_socket_init(port_socket,hostname,hostport);
		return port_socket;
	}
	return NULL;
}

small_modbus_port_socket_t * modbus_port_socket_get(small_modbus_t *smb)
{
	if(smb->port->type == MODBUS_PORT_SOCKET)
	{
		return (small_modbus_port_socket_t*)smb->port;
	}
	return NULL;
}


/*
*modbus_init
*/
int modbus_init(small_modbus_t *smb,uint8_t core_type,void *port)
{
	small_modbus_port_t *smb_port;
	if(smb&&core_type&&port)
	{
		_modbus_init(smb);
		if((core_type == MODBUS_CORE_RTU)||(core_type == MODBUS_CORE_TCP))  // check core type
		{
			if(core_type == MODBUS_CORE_RTU)
			{
				smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
			}
			if(core_type == MODBUS_CORE_TCP)
			{
				smb->core = (small_modbus_core_t*)&_modbus_tcp_core;
			}
		}else
		{
			return 0;
		}
		smb_port = port;
		if((smb_port->type == MODBUS_PORT_DEVICE)||(smb_port->type == MODBUS_PORT_SOCKET))  // check port type
		{
			smb->port = smb_port;
			return 1;
		}
	}
	return 0;
}

small_modbus_t *modbus_create(uint8_t core_type,void *port)
{
	small_modbus_t *smb = rt_malloc_align(sizeof(small_modbus_t),4);
	if(smb)
	{
		if(modbus_init(smb,core_type,port))
		{
			return smb;
		}else
		{
			rt_free_align(smb);
		}
	}
	return NULL;
}

//int modbus_rtu_init(small_modbus_t *smb,void *port)
//{
//	small_modbus_port_t *smb_port;
//	if(smb&&port)
//	{
//		_modbus_init(smb);
//		smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
//		
//		smb_port = port;
//		if((smb_port->type == MODBUS_PORT_DEVICE)||(smb_port->type == MODBUS_PORT_SOCKET))
//		{
//			smb->port = smb_port;
//		}
//	}
//	return 0;
//}
//int modbus_tcp_init(small_modbus_t *smb,void *port)
//{
//	small_modbus_port_t *smb_port;
//	if(smb&&port)
//	{
//		_modbus_init(smb);
//		smb->core = (small_modbus_core_t*)&_modbus_tcp_core;
//		
//		smb_port = port;
//		if((smb_port->type == MODBUS_PORT_DEVICE)||(smb_port->type == MODBUS_PORT_SOCKET))
//		{
//			smb->port = smb_port;
//		}
//	}
//	return 0;
//}

