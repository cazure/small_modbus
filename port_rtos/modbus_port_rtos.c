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
	static char log_buf[32];
	if(level <= smb->debug_level)
	{
		va_list args;
		va_start(args, fmt);
		rt_vsnprintf(log_buf, 32, fmt, args);
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

