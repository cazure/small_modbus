#ifndef _MODBUS_PORT_RTOS_H_
#define _MODBUS_PORT_RTOS_H_

#include "small_modbus_rtu.h"
#include "small_modbus_tcp.h"
//#include <rtthread.h>
//#include <rtdevice.h>
#include "hw_port.h"

//slave server
//int modbus_tcp_accept(small_modbus_t *smb);
//int modbus_tcp_select(small_modbus_t *smb,int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
//int modbus_tcp_set_socket(small_modbus_t *smb,int socket_fd);

int modbus_rtu_init(small_modbus_t *smb,void *port);
int modbus_tcp_init(small_modbus_t *smb,void *port);


#endif /* _MODBUS_PORT_RTOS_H_ */

#ifndef _HW_PORT_H_
#define _HW_PORT_H_

#include "board.h"

#ifdef RT_USING_DEVICE
#include <rtdevice.h>
#endif

#ifdef RT_USING_DFS
#ifdef SAL_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <dfs_select.h>
#include "sys/time.h"
#endif
#endif

#ifdef RT_USING_SAL
#include <sys/socket.h>
#else
#ifdef SAL_USING_LWIP
#include <lwip/sockets.h>
#endif
#endif

#define USING_HWPORT_UART
//#define USING_HWPORT_CAN
//#define USING_HWPORT_TCP
//#define USING_HWPORT_UDP
/*
*端口 定义
*/
enum PORT_ID
{
	PORT_NONE = 0,
	PORT_RS485_1 = 3,
	PORT_RS485_2 = 6,
	PORT_UART_1 = 1,
	PORT_UART_2 = 2,
	PORT_UART_3 = 3,
	PORT_UART_4 = 4,
	PORT_UART_5 = 5,
	PORT_UART_6 = 6,
	PORT_UART_7 = 7,
	PORT_UART_8 = 8,
	PORT_UART_9 = 9,
	PORT_UART_10 = 10,
	PORT_CAN_1 = 11,
	PORT_CAN_2 = 12,
	PORT_TCP_1 = 21,
	PORT_TCP_2 = 22,
	PORT_UDP_1 = 31,
	PORT_UDP_2 = 32
};

/*
*端口 定义
*/
enum PORT_TYPE
{
	PORT_TYPE_NONE = 0,
	PORT_TYPE_UART = 1,
	PORT_TYPE_RS485 = 1,
	PORT_TYPE_CAN = 2,
	PORT_TYPE_TCP = 3,
	PORT_TYPE_UDP = 4
};

typedef struct _hwport		hwport_t;
struct _hwport
{
	uint8_t type;		//端口类型
	uint8_t id;			//端口ID
	uint8_t rx;
	uint8_t tx;
	void * port_data;										//端口配置数据
	void * user_data;										//用户数据
	int (*init) (hwport_t *port);				//初始化端口
	int (*ioctl)(hwport_t *port,int cmd,void *args);	//控制
	int (*open) (hwport_t *port);
	int (*close)(hwport_t *port);
	int (*take) (hwport_t *port);				//获取端口资源
	int (*release)(hwport_t *port);				//释放端口资源
	int (*read) (hwport_t *port,uint8_t *data,uint16_t length);				//读
	int (*write)(hwport_t *port,uint8_t *data,uint16_t length);				//写
	int (*flush)(hwport_t *port);																			//清空buff
	int (*wait)(hwport_t *port,int timeout);													//等待数据
};

typedef struct
{
	uint16_t num;
	hwport_t *list;
}HWport_list;

#ifdef USING_HWPORT_UART
typedef struct _hwport_uart
{
	const char *name;
	hwport_t	*parent;
	struct rt_mutex 	lock;
  struct rt_device *dev;
	uint16_t tx_size;
	uint8_t * tx_buff;
	struct rt_ringbuffer tx_ring;
	struct rt_semaphore tx_sem;
	uint16_t rx_size;
	uint8_t	* rx_buff;
	struct rt_ringbuffer rx_ring;
	struct rt_semaphore rx_sem;
  struct serial_configure config;
  int (*rts_set)(int on);
}hwport_uart_t;
#endif

#ifdef USING_HWPORT_CAN
typedef struct _hwport_can
{
	const char *name;
	hwport_t	*parent;
	struct rt_mutex 	lock;
  struct rt_device *dev;
	uint16_t tx_size;
	uint8_t * tx_buff;
	struct rt_ringbuffer tx_ring;
	struct rt_semaphore tx_sem;
	uint16_t rx_size;
	uint8_t	* rx_buff;
	struct rt_ringbuffer rx_ring;
	struct rt_semaphore rx_sem;
  struct can_configure config;
}hwport_can_t;
#endif

#ifdef USING_HWPORT_TCP
typedef struct _hwport_tcp
{
	const char *name;
	hwport_t	*parent;
	struct rt_mutex 	lock;
  struct rt_device *dev;
	uint16_t tx_size;
	uint8_t * tx_buff;
	struct rt_ringbuffer tx_ring;
	struct rt_semaphore tx_sem;
	uint16_t rx_size;
	uint8_t	* rx_buff;
	struct rt_ringbuffer rx_ring;
	struct rt_semaphore rx_sem;
	
	uint16_t    transfer_id;
	uint16_t    protocol_id;
	int32_t     socket_fd;
	char         ip[16];
	uint16_t     port;
	uint16_t     isSlave;
	int          socket;
	struct  timeval     tv;
	struct sockaddr_in  socket_addr;
}hwport_tcp_t;
#endif

int hwport_init(hwport_t *port);
int hwport_ioctl(hwport_t *port,int cmd,void *args);
int hwport_open(hwport_t *port);
int hwport_close(hwport_t *port);
int hwport_take(hwport_t *port);
int hwport_release(hwport_t *port);
int hwport_read(hwport_t *port,uint8_t *data,uint16_t length);
int hwport_write(hwport_t *port,uint8_t *data,uint16_t length);
int hwport_flush(hwport_t *port);
int hwport_wait(hwport_t *port,int timeout);

int hwport_count(void);
hwport_t * hwport_get_by_id(uint8_t id);
int hwport_init_all(HWport_list *list);

#endif /* _HW_PORT_H_ */



