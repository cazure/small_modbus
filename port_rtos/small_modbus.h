/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#ifndef _SMALL_MODBUS_H_
#define _SMALL_MODBUS_H_

#include "modbus_port_rtos.h"

uint16_t crc16(uint8_t *buffer, uint16_t buffer_length);

#define modbus_mapping_init(map,callback,\
bit_start,bit_num,bit_array,    input_bit_start,input_bit_num,input_bit_array,\
reg_start,reg_num,reg_array,    input_reg_start,input_reg_num,input_reg_array)\
{\
    map.status_callback = callback;\
    map.bit.start = bit_start;\
    map.bit.num = bit_num;\
    map.bit.array = bit_array;\
    map.input_bit.start = input_bit_start;\
    map.input_bit.num   = input_bit_num;\
    map.input_bit.array = input_bit_array;\
    map.registers.start = reg_start;\
    map.registers.num   = reg_num;\
    map.registers.array = reg_array;\
    map.input_registers.start = input_reg_start;\
    map.input_registers.num   = input_reg_num;\
    map.input_registers.array = input_reg_array;\
}

#define modbus_mapping_new(map,callback,\
bit_start,bit_num,input_bit_start,input_bit_num,\
reg_start,reg_num,input_reg_start,input_reg_num) \
{\
    static uint8_t _##map##_bit_array[bit_num];\
    static uint8_t _##map##_input_bit_array[input_bit_num];\
    static uint16_t _##map##_reg_array[reg_num];\
    static uint16_t _##map##_input_reg_array[input_reg_num];\
    modbus_mapping_init(map,callback,\
        bit_start,          bit_num,            _##map##_bit_array,\
        input_bit_start,    input_bit_num,      _##map##_input_bit_array,\
        reg_start,          reg_num,            _##map##_reg_array,\
        input_reg_start,    input_reg_num,      _##map##_input_reg_array\
    );\
}

#endif /* _SMALL_MODBUS_H_ */
