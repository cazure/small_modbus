#ifndef __SMALL_MODBUS_PORT_H__
#define __SMALL_MODBUS_PORT_H__

/*
*RTTHREAD PORT
*/
#define SMALL_MODBUS_RTTHREAD 0
#define SMALL_MODBUS_RTTHREAD_USE_DEVICDE  0
#define SMALL_MODBUS_RTTHREAD_USE_SOCKET   0

#if SMALL_MODBUS_RTTHREAD
#include "small_modbus_port_rtthread.h"
#endif

/*
*LINUX PORT
*/
#define SMALL_MODBUS_LINUX 0
#if SMALL_MODBUS_LINUX
#include "small_modbus_port_linux.h"
#endif

/*
*WIN32 PORT  
*/
#define SMALL_MODBUS_WIN32 1
#if SMALL_MODBUS_WIN32
#include "small_modbus_port_win32.h"
#endif


#endif /* __SMALL_MODBUS_PORT_H__ */
