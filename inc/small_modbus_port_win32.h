#ifndef _SMALL_MODBUS_PORT_WIN32_H_
#define _SMALL_MODBUS_PORT_WIN32_H_

#include "stdint.h"
#include "string.h"
#include "small_modbus_base.h"
#include "small_modbus_port.h"
/*
* modbus on win32
*/
#if SMALL_MODBUS_WIN32

int modbus_init(small_modbus_t *smb,uint8_t core_type,void *port);
small_modbus_t *modbus_create(uint8_t core_type,void *port);

#endif

#endif /* _SMALL_MODBUS_PORT_WIN32_H_ */

