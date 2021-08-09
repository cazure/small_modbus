#ifndef _VIO_H_
#define _VIO_H_

#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "small_modbus.h"

/*
* coils
*/
int vio_read_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer);
int vio_read_input_coils(uint16_t addr,uint16_t num,uint8_t *buffer);

int vio_write_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer);


int vio_lowlevel_update_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer);

int vio_lowlevel_update_input_coils(uint16_t addr,uint16_t num,uint8_t *buffer);


/*
* regs
*/
int vio_read_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer);
int vio_read_input_regs(uint16_t addr,uint16_t num,uint16_t *buffer);

int vio_write_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer);



int vio_lowlevel_update_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer);

int vio_lowlevel_update_input_regs(uint16_t addr,uint16_t num,uint16_t *buffer);


#endif

