/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#include "small_modbus_base.h"
#include "small_modbus_utils.h"
#include "stdint.h"
#include "string.h"

uint16_t modbus_crc16(uint8_t *buffer, uint16_t buffer_length)
{
	uint16_t CRC = 0XFFFF;
	uint16_t CRC_count = 0;
	uint16_t i = 0;
	for (CRC_count = 0; CRC_count < buffer_length; CRC_count++)
	{
		CRC = CRC ^ *(buffer + CRC_count);
		for (i = 0; i < 8; i++)
		{
			if (CRC & 1)
			{
				CRC >>= 1;
				CRC ^= 0xA001;
			}
			else
			{
				CRC >>= 1;
			}
		}
	}
	return CRC;
}

int dio_get_val(uint8_t *array, uint16_t index)
{
	uint8_t offset_bit = (index & 0x07);	//(index%8);  //
	uint8_t offset_arr = (index >> 0x03); //(index/8);  //
	return (array[offset_arr] & (0x01 << offset_bit)) ? 1 : 0;
}

void dio_set_val(uint8_t *array, uint16_t index, int status)
{
	uint8_t offset_bit = (index & 0x07);	//(index%8);  //
	uint8_t offset_arr = (index >> 0x03); //(index/8);  //
	if (status)
	{
		array[offset_arr] |= (0x01 << offset_bit);
	}
	else
	{
		array[offset_arr] &= ~(0x01 << offset_bit);
	}
}

int aio_get_val(uint16_t *array, uint16_t index)
{
	return array[index];
}

void aio_set_val(uint16_t *array, uint16_t index, int status)
{
	array[index] = status;
}

int modbus_check_addr_num(uint8_t function, uint16_t address, uint16_t num)
{
	switch (function)
	{
	case MODBUS_FC_READ_HOLDING_COILS:
	case MODBUS_FC_READ_INPUTS_COILS:
	{
		if ((0 < num) && (num <= MODBUS_MAX_READ_BITS)) //读取到的线圈数量大于0且小于2000个
		{
			return 1;
		}
	}
	break;
	case MODBUS_FC_READ_HOLDING_REGISTERS:
	case MODBUS_FC_READ_INPUT_REGISTERS:
	{
		if ((0 < num) && (num <= MODBUS_MAX_READ_REGISTERS)) //读取到的寄存器数量大于0且小于125个
		{
			return 1;
		}
	}
	break;
	case MODBUS_FC_WRITE_MULTIPLE_COILS:
	{
		if ((0 < num) && (num <= MODBUS_MAX_WRITE_BITS)) //读取到的寄存器数量大于0且小于1968个
		{
			return 1;
		}
	}
	break;
	case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
	{
		if ((0 < num) && (num <= MODBUS_MAX_WRITE_REGISTERS)) //读取到的寄存器数量大于0且小于123个
		{
			return 1;
		}
	}
	break;
	case MODBUS_FC_WRITE_SINGLE_COIL:
	case MODBUS_FC_WRITE_SINGLE_REGISTER:
	{
		return 1;
	}
	}
	return 0;
}

void modbus_byte_copy(uint8_t *des, uint8_t *src, int num)
{
	while (num--)
	{
		*des = *src;
		des++;
		src++;
	}
}

void modbus_coil_h2m(uint8_t *des, uint8_t *src, int coil_num)
{
	int num = (coil_num / 8) + ((coil_num % 8) ? 1 : 0);
	do
	{
		*des = *src;
		des++;
		src++;
	} while (--num);
}

void modbus_coil_m2h(uint8_t *des, uint8_t *src, int coil_num)
{
	int num = (coil_num / 8) + ((coil_num % 8) ? 1 : 0);
	do
	{
		*des = *src;
		des++;
		src++;
	} while (--num);
}

#define MODBUS_REG_SWAP(x) ((((x)&0x00ffUL) << 8) | (((x)&0xff00UL) >> 8))
/*
host uint16_t >> modbus_reg
*/
void modbus_reg_h2m(void *dest_modbus_reg, void *source_host, int reg_num)
{
	uint16_t *source_host_u16 = source_host;
	uint16_t *dest_modbus_u16 = dest_modbus_reg;
	uint16_t temp = 0;

	do
	{
		temp = *source_host_u16;
		*dest_modbus_u16 = MODBUS_REG_SWAP(temp);
		source_host_u16++;
		dest_modbus_u16++;
	} while (--reg_num);
}
/*
modbus_reg >> host uint16_t
*/
void modbus_reg_m2h(void *dest_host, void *source_modbus_reg, int reg_num)
{
	uint16_t *source_modbus_u16 = source_modbus_reg;
	uint16_t *dest_host_u16 = dest_host;
	uint16_t temp = 0;

	do
	{
		temp = *source_modbus_u16;
		*dest_host_u16 = MODBUS_REG_SWAP(temp);
		source_modbus_u16++;
		dest_host_u16++;
	} while (--reg_num);
}
