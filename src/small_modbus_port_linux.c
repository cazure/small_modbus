/*
 * Change Logs:
 * Date           Author       Notes
 * 2021-06     		chenbin      small_modbus_port_linux.c  for linux
 */

#include "small_modbus_port_linux.h"
 /*
 * modbus on linux
 */
#if SMALL_MODBUS_LINUX

int _modbus_debug(small_modbus_t* smb, int level, const char* fmt, ...)
{
    return 0;
}

#endif

