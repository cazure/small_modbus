#ifndef __CONFIG_SMALL_MODBUS_H__
#define __CONFIG_SMALL_MODBUS_H__

/*
*RTTHREAD PORT
*/
#define SMALL_MODBUS_RTTHREAD 1
#define SMALL_MODBUS_RTTHREAD_USE_DEVICDE  1
#define SMALL_MODBUS_RTTHREAD_USE_SOCKET   1

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
#define SMALL_MODBUS_WIN32 0
#if SMALL_MODBUS_WIN32
#include "small_modbus_port_win32.h"
#endif

#define SMALL_MODBUS_CRC_BYTE_SWAP 0

#endif /* __CONFIG_SMALL_MODBUS_H__ */
