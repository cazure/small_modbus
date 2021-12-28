/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#ifndef _SMALL_MODBUS_UTILS_H_
#define _SMALL_MODBUS_UTILS_H_

#include "stdint.h"
#include "stdio.h"
#include "string.h"

/**
 * UTILS FUNCTIONS
 **/
uint16_t modbus_crc16(uint8_t *buffer, uint16_t buffer_length);
int modbus_check_addr_num(uint8_t function, uint16_t address, uint16_t num);
void modbus_byte_copy(uint8_t *des, uint8_t *src, int num);
void modbus_coil_h2m(uint8_t *des, uint8_t *src, int coil_num);
void modbus_coil_m2h(uint8_t *des, uint8_t *src, int coil_num);
void modbus_reg_h2m(void *dest_modbus_reg, void *source_host, int reg_num);
void modbus_reg_m2h(void *dest_host, void *source_modbus_reg, int reg_num);

int dio_get_val(uint8_t *array, uint16_t index);
void dio_set_val(uint8_t *array, uint16_t index, int status);
int aio_get_val(uint16_t *array, uint16_t index);
void aio_set_val(uint16_t *array, uint16_t index, int status);

#endif /* _SMALL_MODBUS_BASE_H_ */
