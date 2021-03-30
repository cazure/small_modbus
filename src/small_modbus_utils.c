/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#include "small_modbus_base.h"
#include "small_modbus_utils.h"
#include "stdint.h"
#include "string.h"

/* Table of CRC values for high-order byte */
static const uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

uint16_t modbus_crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i; /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }
    return (crc_hi << 8 | crc_lo);
}

int modbus_check_addr_num(uint8_t function,uint16_t address,uint16_t num)
{
	switch (function)
	{
		case MODBUS_FC_READ_HOLDING_COILS:
		case MODBUS_FC_READ_INPUTS_COILS:
		{
			if((0 < num)&&(num <= MODBUS_MAX_READ_BITS)) //读取到的线圈数量大于0且小于2000个
			{
				return 1;
			}
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:
		case MODBUS_FC_READ_INPUT_REGISTERS:
		{
			if((0 < num)&&(num <= MODBUS_MAX_READ_REGISTERS)) //读取到的寄存器数量大于0且小于125个
			{
				return 1;
			}
		}break;
		case MODBUS_FC_WRITE_MULTIPLE_COILS:
		{
			if((0 < num)&&(num <= MODBUS_MAX_WRITE_BITS)) //读取到的寄存器数量大于0且小于1968个
			{
				return 1;
			}
		}break;
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
		{
			if((0 < num)&&(num <= MODBUS_MAX_WRITE_REGISTERS)) //读取到的寄存器数量大于0且小于123个
			{
				return 1;
			}
		}break;
	}
	return 0;
}

void modbus_byte_copy(uint8_t *des,uint8_t *src,int num)
{
	while(num--)
	{
		*des = *src;
		des++;
		src++;
	}
}

void modbus_coil_h2m(uint8_t *des,uint8_t *src,int coil_num)
{
	int num  = (coil_num / 8) + ((coil_num % 8) ? 1 : 0);
	do{
		*des = *src;
		des++;
		src++;
	}while(--num);
}

void modbus_coil_m2h(uint8_t *des,uint8_t *src,int coil_num)
{
	int num  = (coil_num / 8) + ((coil_num % 8) ? 1 : 0);
	do{
		*des = *src;
		des++;
		src++;
	}while(--num);
}

#define MODBUS_REG_SWAP(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
/*
host uint16_t >> modbus_reg
*/
void modbus_reg_h2m(void *dest_modbus_reg,void *source_host,int reg_num)
{
	uint16_t *source_host_u16 = source_host;
	uint16_t *dest_modbus_u16 = dest_modbus_reg;
	uint16_t temp = 0;
	
	do{
		temp = *source_host_u16;
		*dest_modbus_u16 = MODBUS_REG_SWAP(temp);
		source_host_u16++;
		dest_modbus_u16++;
	}while(--reg_num);
}
/*
modbus_reg >> host uint16_t
*/
void modbus_reg_m2h(void *dest_host,void *source_modbus_reg,int reg_num)
{
	uint16_t *source_modbus_u16 = source_modbus_reg;
	uint16_t *dest_host_u16 = dest_host;
	uint16_t temp = 0;
	
	do{
		temp = *source_modbus_u16;
		*dest_host_u16 = MODBUS_REG_SWAP(temp);
		source_modbus_u16++;
		dest_host_u16++;
	}while(--reg_num);
}


int dio_get_val(uint8_t *array,uint16_t index)
{
	uint8_t offset_bit = (index & 0x07); //(index%8);  //
	uint8_t offset_arr = (index >> 0x03); //(index/8);  //
	return (array[offset_arr] & (0x01 << offset_bit))?1:0;
}

void dio_set_val(uint8_t *array,uint16_t index,int status)
{
	uint8_t offset_bit = (index & 0x07); //(index%8);  //
	uint8_t offset_arr = (index >> 0x03); //(index/8);  //
	if(status)
	{
		array[offset_arr] |= (0x01 << offset_bit);
	}else
	{
		array[offset_arr] &= ~(0x01 << offset_bit);
	}
}

int aio_get_val(uint16_t *array,uint16_t index)
{
	return array[index];
}

void aio_set_val(uint16_t *array,uint16_t index,int status)
{
	array[index] = status;
}


static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static inline uint32_t bswap_32(uint32_t x)
{
    return (bswap_16(x & 0xffff) << 16) | (bswap_16(x >> 16));
}

#define modbus_ntohl(x)     (x)
#define modbus_htonl(x)     (x)
/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    int i;

    for (i=0; i < 8; i++) {
        dest[idx+i] = (value & (1 << i)) ? 1 : 0;
    }
}

/* Sets many bits from a table of bytes (only the bits between idx and
   idx + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,const uint8_t *tab_byte)
{
    unsigned int i;
    int shift = 0;

    for (i = idx; i < idx + nb_bits; i++) {
        dest[i] = tab_byte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}

/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx,unsigned int nb_bits)
{
    unsigned int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        nb_bits = 8;
    }

    for (i=0; i < nb_bits; i++) {
        value |= (src[idx+i] << i);
    }

    return value;
}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl(((uint32_t)src[0] << 16) + src[1]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) without any conversion (ABCD) */
long modbus_get_long_abcd(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl(((uint32_t)src[0] << 16) + src[1]);
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl(bswap_32((((uint32_t)src[0]) << 16) + src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) in inversed format (DCBA) */
long modbus_get_long_dcba(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl(bswap_32((((uint32_t)src[0]) << 16) + src[1]));
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl((uint32_t)(bswap_16(src[0]) << 16) + bswap_16(src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) with swapped bytes (BADC) */
long modbus_get_long_badc(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl((uint32_t)(bswap_16(src[0]) << 16) + bswap_16(src[1]));
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl((((uint32_t)src[1]) << 16) + src[0]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) with swapped words (CDAB) */
long modbus_get_long_cdab(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl((((uint32_t)src[1]) << 16) + src[0]);
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = (((uint32_t)src[1]) << 16) + src[0];
    memcpy(&f, &i, sizeof(float));

    return f;
}


/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a long to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_long_abcd(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = bswap_32(modbus_htonl(i));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a long to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_long_dcba(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = bswap_32(modbus_htonl(i));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)bswap_16(i >> 16);
    dest[1] = (uint16_t)bswap_16(i & 0xFFFF);
}

/* Set a long to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_long_badc(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)bswap_16(i >> 16);
    dest[1] = (uint16_t)bswap_16(i & 0xFFFF);
}

/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}

/* Set a long to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_long_cdab(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}

/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}

