/*
 * Change Logs:
 * Date           Author       Notes
 * 2021-06     		chenbin      small_modbus_port_win32.c  for win32
 */
#include "small_modbus_port_win32.h"
 /*
 * modbus on win32
 */
#if SMALL_MODBUS_WIN32

#include <windows.h>
#include <winsock2.h>

/* Add this for macros that defined unix flavor */
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif


#endif

