/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#ifndef _SMALL_MODBUS_TCP_H_
#define _SMALL_MODBUS_TCP_H_

#include "small_modbus_base.h"
extern const small_modbus_core_t _modbus_tcp_core;

#define _MODBUS_TCP_HEADER_LENGTH 7
#define _MODBUS_TCP_PRESET_REQ_LENGTH 12
#define _MODBUS_TCP_PRESET_RSP_LENGTH 8

#define _MODBUS_TCP_CHECKSUM_LENGTH 0

#define MODBUS_TCP_DEFAULT_PORT 502
#define MODBUS_TCP_SLAVE 0xFF

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes
 */
#define _MODBUS_TCP_MAX_ADU_LENGTH 260

#endif /* _SMALL_MODBUS_TCP_H_ */
