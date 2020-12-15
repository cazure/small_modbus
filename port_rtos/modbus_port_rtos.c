/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-10     chenbin       	modbus_port_rtos.c  for rtthread
 */
#include "modbus_port_rtos.h"
#include "string.h"
int _modbus_write(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	hwport_t *port = smb->port;
	
	return hwport_write(port,data,length);
}

int _modbus_read(small_modbus_t *smb,uint8_t *data,uint16_t length)
{
	hwport_t *port = smb->port;
	
	return hwport_read(port,data,length);
}

int _modbus_open(small_modbus_t *smb)
{
	hwport_t *port = smb->port;
	return hwport_open(port);
}

int _modbus_close(small_modbus_t *smb)
{
	hwport_t *port = smb->port;
	return hwport_close(port);
}

int _modbus_flush(small_modbus_t *smb)
{
	hwport_t *port = smb->port;
	rt_thread_mdelay(smb->write_timeout);
	return hwport_flush(port);
}

int _modbus_wait(small_modbus_t *smb,int timeout)
{
	hwport_t *port = smb->port;
	return hwport_wait(port,timeout);
}

int _modbus_debug(small_modbus_t *smb,int level,const char *fmt, ...)
{
	static char log_buf[256];
	if(level <= smb->debug_level)
	{
		va_list args;
		va_start(args, fmt);
		rt_vsnprintf(log_buf, 256, fmt, args);
		va_end(args);
		rt_kprintf(log_buf);
	}
	return 0;
}

int modbus_rtu_init(small_modbus_t *smb,void *port)
{
	_modbus_init(smb);
	smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
	smb->port = port;
	return 0;
}

int modbus_tcp_init(small_modbus_t *smb,void *port)
{
	_modbus_init(smb);
	smb->core = (small_modbus_core_t*)&_modbus_tcp_core;
	smb->port = port;
	return 0;
}


#ifndef _HW_PORT_H_
#define _HW_PORT_H_

#include "board.h"
#include "hw_port.h"
#include "hw_api.h"


#ifdef USING_HWPORT_UART
int uart3_rts(int on)
{
    hw_led_set(1,on);
    hw_rs485_set(1,on);
    return 0;
}

int uart6_rts(int on)
{
    hw_led_set(1,on);
    hw_rs485_set(2,on);
    return 0;
}

static uint8_t uart3_rbuff[256];
static hwport_uart_t uart3_config = 
{
	.name = "uart3" ,
	.rx_size = 256,
	.rx_buff = uart3_rbuff,
	.tx_size = 0,
	.config.baud_rate = BAUD_RATE_9600,
	.config.data_bits = DATA_BITS_8,
	.config.stop_bits = STOP_BITS_1,
	.config.bufsz = 256,
	.config.parity = PARITY_NONE,
	.rts_set = uart3_rts
};
static uint8_t uart6_rbuff[256];
static hwport_uart_t uart6_config = 
{
	.name = "uart6" ,
	.rx_size = 256,
	.rx_buff = uart6_rbuff,
	.tx_size = 0,
	.config.baud_rate = BAUD_RATE_9600,
	.config.data_bits = DATA_BITS_8,
	.config.stop_bits = STOP_BITS_1,
	.config.bufsz = 256,
	.config.parity = PARITY_NONE,
	.rts_set = uart6_rts
};

static rt_err_t uart3_rx_indicate(rt_device_t dev, rt_size_t size)
{
	static uint8_t read_buff[256];
	hwport_uart_t *uart_config = &uart3_config;
	int rc;
	if (size > 0)
	{
		rc = rt_device_read(dev,0,read_buff,256);
		if(rc>0)
		{
				int len = rt_ringbuffer_data_len(&(uart_config->rx_ring));
				if((len == 0)&&(read_buff[0] == 0x00)) //uart framing error 0x00
				{
						rt_ringbuffer_put(&(uart_config->rx_ring), read_buff+1, rc-1);
				}else
				{
						rt_ringbuffer_put(&(uart_config->rx_ring), read_buff, rc);
				}
				rt_sem_release(&(uart_config->rx_sem));
		}
	}
	return RT_EOK;
}

static rt_err_t uart6_rx_indicate(rt_device_t dev, rt_size_t size)
{
	static uint8_t read_buff[256];
	hwport_uart_t *uart_config = &uart6_config;
	int rc;
	if (size > 0)
	{
		rc = rt_device_read(dev,0,read_buff,256);
		if(rc>0)
		{
				int len = rt_ringbuffer_data_len(&(uart_config->rx_ring));
				if((len == 0)&&(read_buff[0] == 0x00)) //uart framing error 0x00
				{
						rt_ringbuffer_put(&(uart_config->rx_ring), read_buff+1, rc-1);
				}else
				{
						rt_ringbuffer_put(&(uart_config->rx_ring), read_buff, rc);
				}
				rt_sem_release(&(uart_config->rx_sem));
		}
	}
	return RT_EOK;
}

static int uart_init(hwport_t *port)
{
	hwport_uart_t *uart_config;
	if(port->type == PORT_TYPE_UART)
	{
		uart_config = port->port_data;
		uart_config->parent = port;
		uart_config->dev = rt_device_find(uart_config->name);
		rt_kprintf("init %s  %d\n",uart_config->name,uart_config->dev->device_id);
		//lock
		rt_mutex_init(&(uart_config->lock),uart_config->name,RT_IPC_FLAG_FIFO);
		//rx
		if(uart_config->rx_size)
		{
			rt_sem_init(&(uart_config->rx_sem), uart_config->name, 0, RT_IPC_FLAG_FIFO);
			rt_ringbuffer_init(&(uart_config->rx_ring), uart_config->rx_buff, uart_config->rx_size);
		}
		//tx
		if(uart_config->tx_size)
		{
			rt_sem_init(&(uart_config->tx_sem), uart_config->name, 0, RT_IPC_FLAG_FIFO);
			rt_ringbuffer_init(&(uart_config->tx_ring), uart_config->tx_buff, uart_config->tx_size);
		}
		rt_device_control(uart_config->dev, RT_DEVICE_CTRL_CONFIG, &(uart_config->config));
		//rt_device_open(uart_config->dev, RT_DEVICE_FLAG_RDWR|RT_DEVICE_FLAG_DMA_RX);
		rt_device_open(uart_config->dev, RT_DEVICE_FLAG_DMA_RX);
		if(port->id == PORT_UART_3)
		{
			rt_device_set_rx_indicate(uart_config->dev, uart3_rx_indicate);
		}else if(port->id == PORT_UART_6)
		{
			rt_device_set_rx_indicate(uart_config->dev, uart6_rx_indicate);
		}
		if(uart_config->rts_set)
				uart_config->rts_set(0);
	}
	return 0;
}
static int uart_ioctl(hwport_t *port,int cmd,void *args)
{
	hwport_uart_t *config = port->port_data;
	if(cmd == 0)
	{
		memcpy(args,config->name,8);
	}
	return 0;
}
static int uart_open(hwport_t *port)
{
	hwport_uart_t *config = port->port_data;
	return 0;
}
static int uart_close(hwport_t *port)
{
	hwport_uart_t *config = port->port_data;
	return 0;
}
static int uart_take(hwport_t *port)
{
	hwport_uart_t *config = port->port_data;
	return rt_mutex_take(&(config->lock), RT_WAITING_FOREVER);
}
static int uart_release(hwport_t *port)
{
	hwport_uart_t *config = port->port_data;
	return rt_mutex_release(&(config->lock));
}
static int uart_read(hwport_t *port,uint8_t *data,uint16_t length)
{
	hwport_uart_t *config = port->port_data;
	int rc = 0;
  rc = rt_ringbuffer_get(&(config->rx_ring), data, length);
	//rc = rt_device_read(config->dev,0,data,length);
	return rc;
}
static int uart_write(hwport_t *port,uint8_t *data,uint16_t length)
{
	hwport_uart_t *config = port->port_data;
	if(config->rts_set)
			config->rts_set(1);

	rt_device_write(config->dev, 0, data, length);

	if(config->rts_set)
			config->rts_set(0);
	return length;
}
static int uart_flush(hwport_t *port)
{
	hwport_uart_t *config = port->port_data;
	int rc = rt_ringbuffer_data_len(&(config->rx_ring));
	rt_ringbuffer_reset(&(config->rx_ring));
	rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
	return rc;
}
static int uart_wait(hwport_t *port,int timeout)
{
	hwport_uart_t *config = port->port_data;
	int rc = rt_ringbuffer_data_len(&(config->rx_ring));
	if(rc==0)
	{
		rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
		rc = rt_sem_take(&(config->rx_sem), timeout);
		if(rc == RT_EOK)
		{
			rc = rt_ringbuffer_data_len(&(config->rx_ring));
		}
	}
	return rc;
}
#endif


#ifdef USING_HWPORT_CAN
/*
*can
*/
static uint8_t can1_rbuff[256];
static hwport_can_t can1_config = 
{
	.name = "can1" ,
	.rx_size = 256,
	.rx_buff = can1_rbuff,
	.tx_size = 0,
	.config.baud_rate =  CAN500kBaud,
	.config.msgboxsz =  RT_CANMSG_BOX_SZ,
	.config.sndboxnumber =  RT_CANSND_BOX_NUM,
	.config.mode =  RT_CAN_MODE_NORMAL,
};
static uint8_t can2_rbuff[256];
static hwport_can_t can2_config = 
{
	.name = "can2" ,
	.rx_size = 256,
	.rx_buff = can2_rbuff,
	.tx_size = 0,
	.config.baud_rate =  CAN500kBaud,
	.config.msgboxsz =  RT_CANMSG_BOX_SZ,
	.config.sndboxnumber =  RT_CANSND_BOX_NUM,
	.config.mode =  RT_CAN_MODE_NORMAL,
};

/* 接收数据回调函数 */
static rt_err_t can1_rx_indicate(rt_device_t dev, rt_size_t size)
{
	static struct rt_can_msg can_read_buff = {0}; 
	hwport_can_t *can_config = &can1_config;
	int rc;
	if (size > 0)
	{
		rc = rt_device_read(dev,0,&can_read_buff,sizeof(struct rt_can_msg));
		if(rc>0)
		{
				rt_ringbuffer_put(&(can_config->rx_ring),(uint8_t*)&can_read_buff,sizeof(struct rt_can_msg));
				rt_sem_release(&(can_config->rx_sem));
		}
	}
	return RT_EOK;
}
/* 接收数据回调函数 */
static rt_err_t can2_rx_indicate(rt_device_t dev, rt_size_t size)
{
	static struct rt_can_msg can_read_buff = {0}; 
	hwport_can_t *can_config = &can2_config;
	int rc;
	if (size > 0)
	{
		rc = rt_device_read(dev,0,&can_read_buff,sizeof(struct rt_can_msg));
		if(rc>0)
		{
				rt_ringbuffer_put(&(can_config->rx_ring),(uint8_t*)&can_read_buff,sizeof(struct rt_can_msg));
				rt_sem_release(&(can_config->rx_sem));
		}
	}
	return RT_EOK;
}

static int can_init(hwport_t *port)
{
	hwport_can_t *can_config;
	rt_uint32_t set_val = 0;
	if(port->type == PORT_TYPE_CAN)
	{
		can_config = port->port_data;
		can_config->parent = port;
		can_config->dev = rt_device_find(can_config->name);
		rt_kprintf("init %s  %d\n",can_config->name,can_config->dev->device_id);
		//lock
		rt_mutex_init(&(can_config->lock),can_config->name,RT_IPC_FLAG_FIFO);
		//rx
		if(can_config->rx_size)
		{
			rt_sem_init(&(can_config->rx_sem), can_config->name, 0, RT_IPC_FLAG_FIFO);
			rt_ringbuffer_init(&(can_config->rx_ring), can_config->rx_buff, can_config->rx_size);
		}
		//tx
		if(can_config->tx_size)
		{
			rt_sem_init(&(can_config->tx_sem), can_config->name, 0, RT_IPC_FLAG_FIFO);
			rt_ringbuffer_init(&(can_config->tx_ring), can_config->tx_buff, can_config->tx_size);
		}
		set_val = can_config->config.baud_rate;
		rt_device_control(can_config->dev, RT_CAN_CMD_SET_BAUD, &(set_val));
		set_val = can_config->config.mode;
		rt_device_control(can_config->dev, RT_CAN_CMD_SET_MODE, &(set_val));
		//rt_device_control(can_config->dev, RT_DEVICE_CTRL_CONFIG, &(can_config->config));
		rt_device_open(can_config->dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
		if(port->id == PORT_CAN_1)
		{
			rt_device_set_rx_indicate(can_config->dev, can1_rx_indicate);
		}else if(port->id == PORT_CAN_2)
		{
			rt_device_set_rx_indicate(can_config->dev, can2_rx_indicate);
		}
#ifdef RT_CAN_USING_HDR
    struct rt_can_filter_item items[5] =
    {
        RT_CAN_FILTER_ITEM_INIT(0x100, 0, 0, 1, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x100~0x1ff，hdr 为 - 1，设置默认过滤表 */
        RT_CAN_FILTER_ITEM_INIT(0x300, 0, 0, 1, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x300~0x3ff，hdr 为 - 1 */
        RT_CAN_FILTER_ITEM_INIT(0x211, 0, 0, 1, 0x7ff, RT_NULL, RT_NULL), /* std,match ID:0x211，hdr 为 - 1 */
        RT_CAN_FILTER_STD_INIT(0x486, RT_NULL, RT_NULL),                  /* std,match ID:0x486，hdr 为 - 1 */
        {0x555, 0, 0, 1, 0x7ff, 7,}                                       /* std,match ID:0x555，hdr 为 7，指定设置 7 号过滤表 */
    };
    struct rt_can_filter_config cfg = {5, 1, items}; /* 一共有 5 个过滤表 */
    /* 设置硬件过滤表 */
    res = rt_device_control(can_config->dev, RT_CAN_CMD_SET_FILTER, &cfg);
    RT_ASSERT(res == RT_EOK);
#endif
	}
	return 0;
}
static int can_ioctl(hwport_t *port,int cmd,void *args)
{
	hwport_can_t *config = port->port_data;
	if(cmd == 0)
	{
		memcpy(args,config->name,8);
	}
	return 0;
}
static int can_open(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	return 0;
}
static int can_close(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	return 0;
}
static int can_take(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	return rt_mutex_take(&(config->lock), RT_WAITING_FOREVER);
}
static int can_release(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	return rt_mutex_release(&(config->lock));
}
static int can_read(hwport_t *port,uint8_t *data,uint16_t length)
{
	hwport_can_t *config = port->port_data;
	int rc = 0;
  rc = rt_ringbuffer_get(&(config->rx_ring), data, length);
	//rc = rt_device_read(config->dev,0,data,length);
	return rc;
}
static int can_write(hwport_t *port,uint8_t *data,uint16_t length)
{
	hwport_can_t *config = port->port_data;
	rt_device_write(config->dev, 0, data, length);
	return length;
}
static int can_flush(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	int rc = rt_ringbuffer_data_len(&(config->rx_ring));
	rt_ringbuffer_reset(&(config->rx_ring));
	rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
	return rc;
}
static int can_wait(hwport_t *port,int timeout)
{
	hwport_can_t *config = port->port_data;
	int rc = rt_ringbuffer_data_len(&(config->rx_ring));
	if(rc==0)
	{
		rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
		rc = rt_sem_take(&(config->rx_sem), timeout);
		if(rc == RT_EOK)
		{
			rc = rt_ringbuffer_data_len(&(config->rx_ring));
		}
	}
	return rc;
}
#endif

#ifdef USING_HWPORT_TCP
/*
*TCP
*/
static int tcp_init(hwport_t *port)
{
	hwport_can_t *can_config;
	rt_uint32_t set_val = 0;
	if(port->type == PORT_TYPE_TCP)
	{
		
	}
	return 0;
}
static int tcp_ioctl(hwport_t *port,int cmd,void *args)
{
	hwport_can_t *config = port->port_data;
	if(cmd == 0)
	{
		memcpy(args,config->name,8);
	}
	return 0;
}
static int tcp_open(hwport_t *port)
{
	hwport_tcp_t *config = port->port_data;
	return 0;
}
static int tcp_close(hwport_t *port)
{
	hwport_tcp_t *config = port->port_data;
	return 0;
}
static int tcp_take(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	return rt_mutex_take(&(config->lock), RT_WAITING_FOREVER);
}
static int tcp_release(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	return rt_mutex_release(&(config->lock));
}
static int tcp_read(hwport_t *port,uint8_t *data,uint16_t length)
{
	hwport_can_t *config = port->port_data;
	int rc = 0;
  rc = rt_ringbuffer_get(&(config->rx_ring), data, length);
	//rc = rt_device_read(config->dev,0,data,length);
	return rc;
}
static int tcp_write(hwport_t *port,uint8_t *data,uint16_t length)
{
	hwport_can_t *config = port->port_data;
	rt_device_write(config->dev, 0, data, length);
	return length;
}
static int tcp_flush(hwport_t *port)
{
	hwport_can_t *config = port->port_data;
	int rc = rt_ringbuffer_data_len(&(config->rx_ring));
	rt_ringbuffer_reset(&(config->rx_ring));
	rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
	return rc;
}
static int tcp_wait(hwport_t *port,int timeout)
{
	hwport_can_t *config = port->port_data;
	int rc = rt_ringbuffer_data_len(&(config->rx_ring));
	if(rc==0)
	{
		rt_sem_control(&(config->rx_sem), RT_IPC_CMD_RESET, RT_NULL);
		rc = rt_sem_take(&(config->rx_sem), timeout);
		if(rc == RT_EOK)
		{
			rc = rt_ringbuffer_data_len(&(config->rx_ring));
		}
	}
	return rc;
}
#endif


static hwport_t hwport_list[] =
{
#ifdef USING_HWPORT_UART
	{.type=PORT_TYPE_UART	,.id=PORT_UART_3	,.port_data=&uart3_config,
				.init=uart_init ,.ioctl=uart_ioctl	,.open=uart_open		,.close=uart_close		,.take=uart_take	,.release=uart_release	,.read=uart_read	,.write=uart_write	,.flush=uart_flush	,.wait=uart_wait},
	{.type=PORT_TYPE_UART	,.id=PORT_UART_6	,.port_data=&uart6_config,
				.init=uart_init ,.ioctl=uart_ioctl	,.open=uart_open		,.close=uart_close		,.take=uart_take	,.release=uart_release	,.read=uart_read	,.write=uart_write	,.flush=uart_flush	,.wait=uart_wait},
#endif
#ifdef USING_HWPORT_CAN
	{.type=PORT_TYPE_CAN		,.id=PORT_CAN_1	,.port_data=&can1_config,
				.init=can_init	,.ioctl=can_ioctl	,.open=can_open		,.close=can_close		,.take=can_take	,.release=can_release	,.read=can_read	,.write=can_write	,.flush=can_flush	,.wait=can_wait},
	{.type=PORT_TYPE_CAN		,.id=PORT_CAN_2	,.port_data=&can2_config,
				.init=can_init	,.ioctl=can_ioctl	,.open=can_open		,.close=can_close		,.take=can_take	,.release=can_release	,.read=can_read	,.write=can_write	,.flush=can_flush	,.wait=can_wait},
#endif
//	{.type=PORT_TYPE_TCP		,.id=PORT_TCP_1	,},
//	{.type=PORT_TYPE_TCP		,.id=PORT_TCP_2	,},
};


int hwport_init(hwport_t *port)
{
	if(port->init)
		return port->init(port);
	return 0;
}
int hwport_ioctl(hwport_t *port,int cmd,void *args)
{
	if(port->ioctl)
		return port->ioctl(port,cmd,args);
	return 0;
}
int hwport_open(hwport_t *port)
{
	if(port->open)
		return port->open(port);
	return 0;
}
int hwport_close(hwport_t *port)
{
	if(port->close)
		return port->close(port);
	return 0;
}
int hwport_take(hwport_t *port)
{
	if(port->take)
		return port->take(port);
	return 0;
}
int hwport_release(hwport_t *port)
{
	if(port->release)
		return port->release(port);
	return 0;
}
int hwport_read(hwport_t *port,uint8_t *data,uint16_t length)
{
	if(port->read)
		return port->read(port,data,length);
	return 0;
}
int hwport_write(hwport_t *port,uint8_t *data,uint16_t length)
{
	if(port->write)
		return port->write(port,data,length);
	return 0;
}
int hwport_flush(hwport_t *port)
{
	if(port->flush)
		return port->flush(port);
	return 0;
}
int hwport_wait(hwport_t *port,int timeout)
{
	if(port->wait)
		return port->wait(port,timeout);
	return 0;
}

int hwport_count(void)
{
	uint32_t port_count = sizeof(hwport_list)/sizeof(hwport_t);
	return port_count;
}

hwport_t * hwport_get_by_id(uint8_t id)
{	
	uint32_t port_count = sizeof(hwport_list)/sizeof(hwport_t);
	hwport_t *port = hwport_list;
	do{
		if(port->id == id)
		{
			return port;
		}
		++ port;
	}while( (--port_count) != 0x00);
	return NULL;
}

int hwport_init_all(HWport_list *list)
{
	uint32_t port_count = sizeof(hwport_list)/sizeof(hwport_t);
	hwport_t *item;
	list->num = port_count;
	list->list = hwport_list;
	item = hwport_list;
	
	do{
		hwport_init(item);
		item++;
	}while((--port_count)!=0x00);
	return 0;
}



//int hwport_init_all(HWport_list *list)
//{
//	uint32_t index = 0;
//	uint32_t num = 0;
//	list->num = sizeof(hwport_list)/sizeof(hwport_t);
//	list->list = hwport_list;
//	
//	for(index=0;index <(list->num);index++)
//	{
//		hwport_init(&(list->list[index]));
//	}
//	return 0;
//}

#endif


