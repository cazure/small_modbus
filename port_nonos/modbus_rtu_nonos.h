#ifndef MODBUS_RTU_NONOS_H
#define MODBUS_RTU_NONOS_H

#include "small_modbus_rtu.h"

#include "modbus_myuart.h"

typedef struct _modbus_rtu_config
{
    char name[8];
		MYUART_t *myuart;
    int (*rts_set)(small_modbus_t *ctx, int on);
} modbus_rtu_config_t;

extern modbus_rtu_config_t uart2_config;

int modbus_rtu_init(small_modbus_t *smb,small_modbus_port_t *port,void *config);

#endif
