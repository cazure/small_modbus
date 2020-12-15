/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#ifndef _SMALL_MODBUS_H_
#define _SMALL_MODBUS_H_

#include "small_modbus_base.h"
#include "modbus_port_rtos.h"

uint16_t modbus_crc16(uint8_t *buffer, uint16_t buffer_length);

int modbus_array2bit(uint8_t *dest_modbus_bit,void *source_array_u8,uint16_t array_num);
int modbus_array2reg(uint8_t *dest_modbus_reg,void *source_array_u16,uint16_t array_num);

int modbus_bit2array(void *dest_array_u8,uint8_t *source_modbus_bit,uint16_t modbus_num);
int modbus_reg2array(void *dest_array_u16,uint8_t *source_modbus_reg,uint16_t modbus_num);

int modbus_connect(small_modbus_t *smb);
int modbus_disconnect(small_modbus_t *smb);
int modbus_error_recovery(small_modbus_t *smb);
int modbus_error_exit(small_modbus_t *smb,int code);

//int modbus_set_read_buff(small_modbus_t *smb,int byte,uint8_t *buff);
//int modbus_set_write_buff(small_modbus_t *smb,int byte,uint8_t *buff);
int modbus_set_read_timeout(small_modbus_t *smb,int timeout_ms);
int modbus_set_write_timeout(small_modbus_t *smb,int timeout_ms);
int modbus_set_slave(small_modbus_t *smb, int slave);
int modbus_set_debug(small_modbus_t *smb, int level);

/* master mode api */
/* master start request */
int modbus_start_request(small_modbus_t *smb,uint8_t *request,int function,int addr,int num,void *write_data);
/* master wait for confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response);
/* master handle confirmation message */
int modbus_handle_confirm(small_modbus_t *smb,uint8_t *request,uint16_t request_len,uint8_t *response,uint16_t response_len,void *read_data);
/* master read */
int modbus_read_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);
/* master write */
int modbus_write_bit(small_modbus_t *smb, int addr, int write_status);
int modbus_write_register(small_modbus_t *smb, int addr, int write_value);
int modbus_write_bits(small_modbus_t *smb, int addr, int num,uint8_t *write_data);
int modbus_write_registers(small_modbus_t *smb, int addr, int num,uint16_t *write_data);
/* master write and read */
int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask);
int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb,uint16_t *src, int read_addr, int read_nb,uint16_t *dest);


/* slave callback */
typedef int(*small_modbus_slave_callback_t)(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data);

/* slave mapping table */
typedef struct _small_modbus_slave_mapping	small_modbus_slave_mapping_t;

struct _small_modbus_slave_mapping
{
	struct bit{int start;int num;uint8_t *array;}bit;
	struct input_bit{int start;int num;uint8_t *array;}input_bit;
	struct registers{int start;int num;uint16_t *array;}registers;
	struct input_registers{int start;int num;uint16_t *array;}input_registers;
};

#define modbus_slave_mapping_init(map,\
bit_start,bit_num,bit_array,    input_bit_start,input_bit_num,input_bit_array,\
reg_start,reg_num,reg_array,    input_reg_start,input_reg_num,input_reg_array)\
{\
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

#define modbus_slave_mapping_new(map,\
bit_start,bit_num,input_bit_start,input_bit_num,\
reg_start,reg_num,input_reg_start,input_reg_num) \
{\
    static uint8_t _##map##_bit_array[bit_num];\
    static uint8_t _##map##_input_bit_array[input_bit_num];\
    static uint16_t _##map##_reg_array[reg_num];\
    static uint16_t _##map##_input_reg_array[input_reg_num];\
    modbus_slave_mapping_init(map,\
        bit_start,          bit_num,            _##map##_bit_array,\
        input_bit_start,    input_bit_num,      _##map##_input_bit_array,\
        reg_start,          reg_num,            _##map##_reg_array,\
        input_reg_start,    input_reg_num,      _##map##_input_reg_array\
    );\
}

/* slave mode api */
/* slave wait query data */
int modbus_slave_wait(small_modbus_t *smb,uint8_t *request,int32_t waittime);
/* slave handle query data for callback */
int modbus_slave_handle(small_modbus_t *smb,uint8_t *request,uint16_t request_len,small_modbus_slave_callback_t slave_callback);
/* slave wait and handle query for callback */
int modbus_slave_wait_handle(small_modbus_t *smb,small_modbus_slave_callback_t slave_callback,int32_t waittime);
/* slave handle query data for mapping */
int modbus_slave_handle_mapping(small_modbus_t *smb,uint8_t *request,uint16_t request_len,small_modbus_slave_mapping_t * slave_mapping_tab);
/* slave wait and handle query for mapping */
int modbus_slave_wait_handle_mapping(small_modbus_t *smb,small_modbus_slave_mapping_t * slave_mapping_tab,int32_t waittime);


#endif /* _SMALL_MODBUS_H_ */
