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
int modbus_check_addr_num(uint8_t function,uint16_t address,uint16_t num);
void modbus_byte_copy(uint8_t *des,uint8_t *src,int num);
void modbus_coil_h2m(uint8_t *des,uint8_t *src,int coil_num);
void modbus_coil_m2h(uint8_t *des,uint8_t *src,int coil_num);
void modbus_reg_h2m(void *dest_modbus_reg,void *source_host,int reg_num);
void modbus_reg_m2h(void *dest_host,void *source_modbus_reg,int reg_num);

void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value);
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,
                                       const uint8_t *tab_byte);
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits);
float modbus_get_float(const uint16_t *src);
float modbus_get_float_abcd(const uint16_t *src);
float modbus_get_float_dcba(const uint16_t *src);
float modbus_get_float_badc(const uint16_t *src);
float modbus_get_float_cdab(const uint16_t *src);

long modbus_get_long_abcd(const uint16_t *src);
long modbus_get_long_dcba(const uint16_t *src);
long modbus_get_long_badc(const uint16_t *src);
long modbus_get_long_cdab(const uint16_t *src);

void modbus_set_float(float f, uint16_t *dest);
void modbus_set_float_abcd(float f, uint16_t *dest);
void modbus_set_float_dcba(float f, uint16_t *dest);
void modbus_set_float_badc(float f, uint16_t *dest);
void modbus_set_float_cdab(float f, uint16_t *dest);

void modbus_set_long_abcd(long l, uint16_t *dest);
void modbus_set_long_dcba(long l, uint16_t *dest);
void modbus_set_long_badc(long l, uint16_t *dest);
void modbus_set_long_cdab(long l, uint16_t *dest);

#define MODBUS_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE(data) ((data) & 0xFF)
#define MODBUS_GET_INT64_FROM_INT16(tab_int16, index) \
    (((int64_t)tab_int16[(index)    ] << 48) + \
     ((int64_t)tab_int16[(index) + 1] << 32) + \
     ((int64_t)tab_int16[(index) + 2] << 16) + \
      (int64_t)tab_int16[(index) + 3])
#define MODBUS_GET_INT32_FROM_INT16(tab_int16, index) ((tab_int16[(index)] << 16) + tab_int16[(index) + 1])
#define MODBUS_GET_INT16_FROM_INT8(tab_int8, index) ((tab_int8[(index)] << 8) + tab_int8[(index) + 1])
#define MODBUS_SET_INT16_TO_INT8(tab_int8, index, value) \
    do { \
        tab_int8[(index)] = (value) >> 8;  \
        tab_int8[(index) + 1] = (value) & 0xFF; \
    } while (0)
#define MODBUS_SET_INT32_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 16; \
        tab_int16[(index) + 1] = (value); \
    } while (0)
#define MODBUS_SET_INT64_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 48; \
        tab_int16[(index) + 1] = (value) >> 32; \
        tab_int16[(index) + 2] = (value) >> 16; \
        tab_int16[(index) + 3] = (value); \
    } while (0)

#endif /* _SMALL_MODBUS_BASE_H_ */
