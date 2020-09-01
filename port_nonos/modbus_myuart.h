#ifndef __MODBUS_MYUART_H
#define __MODBUS_MYUART_H

#include "main.h"
#include "stdio.h"
#include "modbus_ringbuffer.h"

//#######################################
//#define USE_MYUART1
#define USE_MYUART2
//#define USE_MYUART3
//#define USE_MYUART4
//#define USE_MYUART5

//#######################################

#define EVENT_SERIAL1			(1<<1)
#define EVENT_SERIAL2			(1<<2)
#define EVENT_SERIAL3			(1<<3)
#define EVENT_SERIAL4			(1<<4)
#define EVENT_SERIAL5			(1<<5)

typedef struct _myuart{
	char name[6];
	UART_HandleTypeDef *huart;
	IRQn_Type irq_type;
	uint32_t rx_event;
	uint32_t tx_event;
	void (*rx_callback)(void);
	void (*tx_callback)(void);
	struct modbus_ringbuffer  rx_rbuff;
	struct modbus_ringbuffer  tx_rbuff;
}MYUART_t;

int bus_myuart_init(void);

int myuart_close(MYUART_t * myuart);
int myuart_open(MYUART_t * myuart);
int myuart_read(MYUART_t * myuart,uint8_t *data,uint16_t len,int32_t timeout);
int myuart_write(MYUART_t * myuart,uint8_t *data,uint16_t len,int32_t timeout);
int myuart_flush(MYUART_t * myuart);
int myuart_select(MYUART_t * myuart,int32_t timeout);

//extern struct rt_event myevent;
#ifdef USE_MYUART1
extern MYUART_t myuart1;
#endif
#ifdef USE_MYUART2
extern MYUART_t myuart2;
#endif
#ifdef USE_MYUART3
extern MYUART_t myuart3;
#endif
#ifdef USE_MYUART4
extern MYUART_t myuart4;
#endif
#ifdef USE_MYUART5
extern MYUART_t myuart5;
#endif


#endif

