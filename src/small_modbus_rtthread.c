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
	if(level <= smb->debug_level)
	{
		va_list args;
		va_start(args, fmt);
		rt_vsnprintf(log_buf, 256, fmt, args);
		va_end(args);
		rt_kputs(log_buf);
	}
	return 0;
}

//static rt_err_t _modbus_rtdevice_rx_indicate(rt_device_t dev, rt_size_t size)
//{
//	small_modbus_port_device_t *smb_port_device = (small_modbus_port_device_t *)dev->user_data;
//	uint8_t *read_buff = smb_port_device->read_buff;
//	struct rt_ringbuffer *ringbuff = &(smb_port_device->rx_ring);
//	
//	int read_len = 0;
//	int buff_len = 0;
//	if((size > 0)&&(read_buff)&&(ringbuff))
//	{
//		read_len = rt_device_read(dev,0,read_buff,sizeof(smb_port_device->read_buff));
//		if(read_len>0)
//		{
//			buff_len = rt_ringbuffer_data_len(ringbuff);
//			if((buff_len == 0)&&(read_buff[0] == 0x00)) //uart framing error 0x00
//			{
//					rt_ringbuffer_put_force(ringbuff,read_buff+1, read_len-1);
//			}else
//			{
//					rt_ringbuffer_put_force(ringbuff,read_buff, read_len);
//			}
//			buff_len = rt_ringbuffer_data_len(ringbuff);
//			if(buff_len>0)
//			{
//				rt_sem_release(&(smb_port_device->rx_sem));
//			}
//		}
//	}
//	return RT_EOK;
//}

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};
static struct rt_messagequeue rx_mq;

static rt_err_t _modbus_rtdevice_rx_indicate(rt_device_t dev, rt_size_t size)
{
	struct rx_msg msg;
	rt_err_t result;
	msg.dev = dev;
	msg.size = size;

	return rt_mq_send(&rx_mq, &msg, sizeof(msg));
}


static void serial_thread_entry(void *parameter)
{
	struct rx_msg msg;
	rt_err_t result;
	uint8_t read_buff[RT_SERIAL_RB_BUFSZ];

	small_modbus_port_device_t *smb_port_device;
	struct rt_ringbuffer *ringbuff;
	int read_len = 0;
	int buff_len = 0;
	while (1)
	{
		rt_memset(&msg, 0, sizeof(msg));
		result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
		if (result == RT_EOK)
		{
			read_len = 0;
			buff_len = 0;
			smb_port_device = (small_modbus_port_device_t *)msg.dev->user_data;
			ringbuff = &(smb_port_device->rx_ring);
			
			if((smb_port_device)&&(ringbuff))
			{
				read_len = rt_device_read(msg.dev,0,read_buff,RT_SERIAL_RB_BUFSZ);
				if(read_len>0)
				{
					buff_len = rt_ringbuffer_data_len(ringbuff);
					if((buff_len == 0)&&(read_buff[0] == 0x00)) //uart framing error 0x00
					{
							rt_ringbuffer_put_force(ringbuff,read_buff+1, read_len-1);
					}else
					{
							rt_ringbuffer_put_force(ringbuff,read_buff, read_len);
					}
					buff_len = rt_ringbuffer_data_len(ringbuff);
					if(buff_len>0)
					{
						rt_sem_release(&(smb_port_device->rx_sem));
					}
				}
			}
		}
	}//while
}

int serial_thread_init(void)
{
	static char msg_pool[256];
	static uint32_t init_flag = 0;
	if(init_flag==0)
	{
		 /* 初始化消息队列 */
    rt_mq_init(&rx_mq, "rx_mq",msg_pool,sizeof(struct rx_msg),sizeof(msg_pool),RT_IPC_FLAG_FIFO);        /* 如果有多个线程等待，按照先来先得到的方法分配消息 */
		
		/* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 2048, 25, 6);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
	}
	return 0;
}

static int _modbus_rtdevice_open(small_modbus_t *smb)
{
	small_modbus_port_device_t *smb_port_device = (small_modbus_port_device_t *)smb->port;
	
	//port_device->device = rt_device_find(port_device->device_name);
	if(smb_port_device->device)
	{
		rt_device_open(smb_port_device->device, RT_DEVICE_FLAG_DMA_RX);
		
		smb_port_device->old_user_data = smb_port_device->device->user_data;   // backup old
		
		smb_port_device->device->user_data = smb_port_device; //user_data
		
		smb_port_device->old_rx_indicate = smb_port_device->device->rx_indicate;  // backup old
		
		rt_device_set_rx_indicate(smb_port_device->device, _modbus_rtdevice_rx_indicate);  // set new
	}
	return 0;
}

static int _modbus_rtdevice_close(small_modbus_t *smb)
{
	small_modbus_port_device_t *smb_port_device = (small_modbus_port_device_t *)smb->port;
	if(smb_port_device->device)
	{
		rt_device_set_rx_indicate(smb_port_device->device, smb_port_device->old_rx_indicate); // recovery
		
		smb_port_device->device->user_data = smb_port_device->old_user_data; // recovery
		
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
	//return rt_device_write(port_device->device,0,data,length);
}

static int _modbus_rtdevice_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
	
	//return rt_device_read(port_device->device,0,data,length);
	int rc = 0;
  rc = rt_ringbuffer_get(&(port_device->rx_ring), data, length);
	return rc;
}

static int _modbus_rtdevice_flush(small_modbus_t *smb)
{
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
	
	int rc = rt_ringbuffer_data_len(&(port_device->rx_ring));
	rt_ringbuffer_reset(&(port_device->rx_ring));
	rt_sem_control(&(port_device->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
	
	return 0;
}

static int _modbus_rtdevice_wait(small_modbus_t *smb,int timeout)
{
//	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
//	return rt_sem_take(&(port_device->rx_sem),timeout);
	
	int rc = -1;
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t *)smb->port;
	rc = rt_ringbuffer_data_len(&(port_device->rx_ring));
	if(rc == 0)
	{
		rt_sem_control(&(port_device->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
		rc = rt_sem_take(&(port_device->rx_sem), timeout);
		if(rc == RT_EOK)
		{
			rc = rt_ringbuffer_data_len(&(port_device->rx_ring));
			if(rc == 0)
			{
				rc = -9; //no data
			}
		}
	}
	return rc;
}

small_modbus_port_t _port_device_default = 
{
	.type = MODBUS_PORT_DEVICE,
	.port = MODBUS_PORT_DEVICE,
	.open = _modbus_rtdevice_open,
	.close = _modbus_rtdevice_close,
	.read = _modbus_rtdevice_read,
	.write = _modbus_rtdevice_write,
	.flush = _modbus_rtdevice_flush,
	.wait = _modbus_rtdevice_wait
};


small_modbus_port_t _port_socket_default = 
{
	.type = MODBUS_PORT_SOCKET,
	.port = MODBUS_PORT_SOCKET,
	.open = _modbus_rtdevice_open,
	.close = _modbus_rtdevice_close,
	.read = _modbus_rtdevice_read,
	.write = _modbus_rtdevice_write,
	.flush = _modbus_rtdevice_flush,
	.wait = _modbus_rtdevice_wait
};


int modbus_rtu_init(small_modbus_t *smb,small_modbus_port_t *port)
{
	if(smb&&port)
	{
		_modbus_init(smb);
		smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
		smb->port = port;
	}
	return 0;
}

int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port)
{
	if(smb&&port)
	{
		_modbus_init(smb);
		smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
		smb->port = port;
	}
	return 0;
}

/*
*device
*/
int modbus_port_device_init(small_modbus_port_device_t *port,const char *device_name)
{
	rt_memcpy(&port->port,&_port_device_default,sizeof(small_modbus_port_t));
	port->device_name = device_name;
	port->device = rt_device_find(device_name);
	if(port->device)
	{
		serial_thread_init();
		rt_sem_init(&(port->rx_sem), device_name, 0, RT_IPC_FLAG_FIFO);
		rt_ringbuffer_init(&(port->rx_ring), port->_ring_buff, sizeof(port->_ring_buff));
	}
	return 0;
}
small_modbus_port_t *modbus_port_device_create(const char *device_name)
{
	small_modbus_port_device_t *port_device = rt_malloc_align(sizeof(small_modbus_port_device_t),4);
	if(port_device)
	{
		modbus_port_device_init(port_device,device_name);
		return (small_modbus_port_t *)port_device;
	}
	return NULL;
}

int modbus_port_device_set_config(small_modbus_t *smb,struct serial_configure *serial_config)
{
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t*)smb->port;
//		smb_port_device->serial_config.baud_rate = BAUD_RATE_9600,
//		smb_port_device->serial_config.data_bits = DATA_BITS_8,
//		smb_port_device->serial_config.stop_bits = STOP_BITS_1,
//		smb_port_device->serial_config.bufsz = 256,
//		smb_port_device->serial_config.parity = PARITY_NONE,
	rt_device_control(port_device->device, RT_DEVICE_CTRL_CONFIG, serial_config);
	return 0;
}

int modbus_port_device_set_rts(small_modbus_t *smb,int (*rts_set)(int on))
{
	small_modbus_port_device_t *port_device = (small_modbus_port_device_t*)smb->port;
	port_device->rts_set = rts_set;
	return 0;
}


/*
*socket
*/
int modbus_port_socket_init(small_modbus_port_socket_t *port,char *hostname,char *hostport)
{
	rt_memcpy(&port->port,&_port_socket_default,sizeof(small_modbus_port_t));
	port->hostname = hostname;
	port->hostport = hostport;
	port->socket_fd = 0;
	
	return 0;
}

small_modbus_port_t *modbus_port_socket_create(char *hostname,char *hostport)
{
	small_modbus_port_socket_t *port_socket = rt_malloc_align(sizeof(small_modbus_port_socket_t),4);
	if(port_socket)
	{
		modbus_port_socket_init(port_socket,hostname,hostport);
		return (small_modbus_port_t *)port_socket;
	}
	return NULL;
}



small_modbus_t *modbus_rtu_create(const char *device_name)
{
	small_modbus_port_t *port;
	small_modbus_t *smb = rt_malloc_align(sizeof(small_modbus_t),4);
	if(smb)
	{
		port = modbus_port_device_create(device_name);
		if(port)
		{
			modbus_rtu_init(smb,port);
			return smb;
		}else
		{
			rt_free_align(smb);
		}
	}
	return NULL;
}


small_modbus_t *modbus_tcp_create(char *hostname,char *hostport)
{
	small_modbus_port_t *port;
	small_modbus_t *smb = rt_malloc_align(sizeof(small_modbus_t),4);
	if(smb)
	{
		port = modbus_port_socket_create(hostname,hostport);
		if(port)
		{
			modbus_tcp_init(smb,port);
			return smb;
		}else
		{
			rt_free_align(smb);
		}
	}
	return NULL;
}

