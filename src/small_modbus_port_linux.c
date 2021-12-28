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

int _modbus_debug(small_modbus_t *smb, int level, const char *fmt, ...)
{
    static char log_buf[256];
    if (level <= smb->debug_level)
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(log_buf, 256, fmt, args);
        va_end(args);
        puts(log_buf);
    }
    return 0;
}

#endif
