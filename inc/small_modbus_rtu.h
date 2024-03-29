/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#ifndef _SMALL_MODBUS_RTU_H_
#define _SMALL_MODBUS_RTU_H_

#include "small_modbus_base.h"
extern const small_modbus_core_t _modbus_rtu_core;

#define _MODBUS_RTU_HEADER_LENGTH 1
#define _MODBUS_RTU_PRESET_REQ_LENGTH 6
#define _MODBUS_RTU_PRESET_RSP_LENGTH 2

#define _MODBUS_RTU_CHECKSUM_LENGTH 2

#define _MODBUS_RTU_MAX_ADU_LENGTH 260

#endif /* _SMALL_MODBUS_RTU_H_ */
