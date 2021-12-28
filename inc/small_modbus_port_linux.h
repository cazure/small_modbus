#ifndef _SMALL_MODBUS_PORT_LINUX_H_
#define _SMALL_MODBUS_PORT_LINUX_H_

#include "stdint.h"
#include "string.h"
#include "small_modbus_base.h"
#include "config_small_modbus.h"
/*
 * modbus on linux
 */
#if SMALL_MODBUS_LINUX

int modbus_init(small_modbus_t *smb, uint8_t core_type, void *port);
small_modbus_t *modbus_create(uint8_t core_type, void *port);

#endif

#endif /* _SMALL_MODBUS_PORT_LINUX_H_ */
