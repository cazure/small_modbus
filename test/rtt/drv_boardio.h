/*
* Change Logs:
* Date           Author       Notes
* 2021-02-20     chenbin      
*/
#ifndef __DRV_BOARDIO_H__
#define __DRV_BOARDIO_H__

#include "main.h"
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

enum BIO_TYPE
{
	BIO_NONE 	= 0x00,
	BIO_DI		= 0x01,
	BIO_DM		= 0x02,
	BIO_DO		= 0x03,
	BIO_AI		= 0x04,
	BIO_AM		= 0x05,
	BIO_AO		= 0x06,
	BIO_MAX		= 0x07
};

#define BIO_TYPE_MASK		0xFFFF0000
#define BIO_ADDR_MASK		0x0000FFFF

#define BIO_MASK(type,addr) (((type<<16)&BIO_TYPE_MASK)|(addr&BIO_ADDR_MASK))

#define BIO_TYPE(mask) (((mask)&BIO_TYPE_MASK)>>16)

#define BIO_ADDR(mask) (((mask)&BIO_ADDR_MASK))

//board io
struct rt_boardio_device
{
	struct rt_device	parent;
};
typedef struct rt_boardio_device rt_boardio_t;

//int dio_get_val(uint8_t *array,uint16_t index)
//{
//	uint8_t offset_bit = (index & 0x07); //(index%8);  //
//	uint8_t offset_arr = (index >> 0x03); //(index/8);  //
//	return (array[offset_arr] & (0x01 << offset_bit))?1:0;
//}

//void dio_set_val(uint8_t *array,uint16_t index,int status)
//{
//	uint8_t offset_bit = (index & 0x07); //(index%8);  //
//	uint8_t offset_arr = (index >> 0x03); //(index/8);  //
//	if(status)
//	{
//		array[offset_arr] |= (0x01 << offset_bit);
//	}else
//	{
//		array[offset_arr] &= ~(0x01 << offset_bit);
//	}
//}

//int aio_get_val(uint16_t *array,uint16_t index)
//{
//	return array[index];
//}

#endif /* __DRV_BOARD_H__ */
