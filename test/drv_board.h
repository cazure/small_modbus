/*
* Change Logs:
* Date           Author       Notes
* 2021-02-20     chenbin      
*/
#ifndef __DRV_BOARD_H__
#define __DRV_BOARD_H__

#include "main.h"
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

#define DO_MASK		0x10000000
#define DI_MASK		0x20000000
#define AO_MASK		0x40000000
#define AI_MASK		0x80000000

//board io
struct rt_boardio_device
{
	struct rt_device	parent;
	uint16_t	di_max;
	uint16_t	do_max;
	uint16_t	ai_max;
	uint16_t	ao_max;
};
typedef struct rt_boardio_device rt_boardio_t;

#endif /* __DRV_BOARD_H__ */
