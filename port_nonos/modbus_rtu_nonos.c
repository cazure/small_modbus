/*
 * Copyright ? 2001-2011 St¨¦phane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "modbus_rtu_nonos.h"

int rts_set(small_modbus_t *ctx, int on)
{
	if(on)
	{
		HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin,GPIO_PIN_RESET);
		HAL_Delay(1);
	}else
	{
		HAL_Delay(1);
		HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin,GPIO_PIN_SET);
	}
	return 0;
}

modbus_rtu_config_t uart2_config =
{
	.name = "uart2",
	.myuart = &myuart2,
	.rts_set = rts_set
};

int hw_uart_init(void)
{
	bus_myuart_init();
}

int rtu_open(small_modbus_t *smb)
{
	modbus_rtu_config_t *config = smb->port_data;
	smb->port->debug(smb,0,"open:%s\n",config->name);
	if(config->rts_set)
			config->rts_set(smb,0);
	return 0;
}
int rtu_close(small_modbus_t *smb)
{
	modbus_rtu_config_t *config = smb->port_data;
	smb->port->debug(smb,0,"close:%s\n",config->name);
	if(config->rts_set)
			config->rts_set(smb,0);
	return 0;
}

int rtu_read(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    int rc = 0;
    modbus_rtu_config_t *config = smb->port_data;
		rc = myuart_read(config->myuart,data,length,1000);
//    int i;
//    rt_kprintf("read %d,%d :",rc,length);
//    for (i = 0; i < rc; i++)
//    {
//            rt_kprintf("<%02X>", data[i]);
//    }
//    rt_kprintf("\n");

    return rc;
}
int rtu_write(small_modbus_t *smb,uint8_t *data, uint16_t length)
{
    modbus_rtu_config_t *config = smb->port_data;
    if(config->rts_set)
        config->rts_set(smb,1);
		
		myuart_write(config->myuart,data,length,1000);

    if(config->rts_set)
        config->rts_set(smb,0);
		
//    int i;
//    rt_kprintf("write %d :",length);
//    for (i = 0; i < length; i++)
//    {
//            rt_kprintf("<%02X>", data[i]);
//    }
//    rt_kprintf("\n");
    return length;
}
int rtu_flush(small_modbus_t *smb)
{
	modbus_rtu_config_t *config = smb->port_data;
	myuart_flush(config->myuart);
	return 0;
}

int rtu_select(small_modbus_t *smb,int timeout_ms)
{
	int rc = 0;	
	modbus_rtu_config_t *config = smb->port_data;
	rc = myuart_select(config->myuart,timeout_ms);
	return rc;
}

#include "stdio.h"
#include "string.h"

static int rtu_debug(small_modbus_t *smb,int level,const char *fmt, ...)
{
	modbus_rtu_config_t *config = smb->port_data;
//	static char log_buf[32];
//	if(level <= smb->debug_level)
//	{
//			va_list args;
//			va_start(args, fmt);
//			rt_vsnprintf(log_buf, 32, fmt, args);
//			va_end(args);
//			printf(log_buf);
//	}
	return 0;
}

small_modbus_port_t _modbus_rtu_nonos_port =
{
    .open =  rtu_open,
    .close = rtu_close,
    .read =  rtu_read,
    .write = rtu_write,
    .flush =  rtu_flush,
    .select = rtu_select,
    .debug =  rtu_debug
};

int modbus_rtu_init(small_modbus_t *smb,small_modbus_port_t *port,void *config)
{
    _modbus_init(smb);
    smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
    smb->port_data = config;
    if(port ==NULL)
    {
        smb->port = &_modbus_rtu_nonos_port;
    }else {
        smb->port = port;
    }
    return 0;
}




