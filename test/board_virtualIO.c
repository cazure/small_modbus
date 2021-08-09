#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus.h"
#include "board_virtualIO.h"

void vio_lowlevel_update(void);

#define INPUT_COILS_MAX 64
uint8_t input_coils[INPUT_COILS_MAX/8] = {0};

int vio_read_input_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < INPUT_COILS_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (INPUT_COILS_MAX))
		{
			io_end = INPUT_COILS_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			dio_set_val(buffer,index,  dio_get_val(input_coils,io_start)  );
			io_start++;
			index++;
		}
	}
	return index;
}

int vio_lowlevel_update_input_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < INPUT_COILS_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (INPUT_COILS_MAX))
		{
			io_end = INPUT_COILS_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			dio_set_val(input_coils,io_start, dio_get_val(buffer,index));
			io_start++;
			index++;
		}
	}
	return index;
}



#define HOLD_COILS_MAX 64
uint8_t hold_coils[HOLD_COILS_MAX/8] = {0};

int vio_read_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < HOLD_COILS_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (HOLD_COILS_MAX))
		{
			io_end = HOLD_COILS_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			dio_set_val(buffer,index,  dio_get_val(hold_coils,io_start)  );
			io_start++;
			index++;
		}
	}
	return index;
}

int vio_write_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr < HOLD_COILS_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (HOLD_COILS_MAX))
		{
			io_end = HOLD_COILS_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			val_old = dio_get_val(hold_coils,io_start);
			val_new = dio_get_val(buffer,index);
			if(val_old != val_new)
			{
				dio_set_val(hold_coils,io_start,val_new);
				count++;
			}
			io_start++;
			index++;
		}
	}
	if(count)
	{
		//通知硬件更新输出
		vio_lowlevel_update();
	}
	return index;
}

int vio_lowlevel_update_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < HOLD_COILS_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (HOLD_COILS_MAX))
		{
			io_end = HOLD_COILS_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			dio_set_val(hold_coils,io_start, dio_get_val(buffer,index));
			io_start++;
			index++;
		}
	}
	return index;
}




#define INPUT_REG_MAX 16
uint16_t input_regs[INPUT_REG_MAX] = {0};

int vio_read_input_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < INPUT_REG_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (INPUT_REG_MAX))
		{
			io_end = INPUT_REG_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			aio_set_val(buffer,index,  aio_get_val(input_regs,io_start)  );
			io_start++;
			index++;
		}
	}
	return index;
}

int vio_lowlevel_update_input_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < INPUT_REG_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (INPUT_REG_MAX))
		{
			io_end = INPUT_REG_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			aio_set_val(input_regs,io_start, aio_get_val(buffer,index));
			io_start++;
			index++;
		}
	}
	return index;
}



#define HOLD_REG_MAX 16
uint16_t hold_regs[HOLD_REG_MAX] = {0};

int vio_read_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < HOLD_REG_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (HOLD_REG_MAX))
		{
			io_end = HOLD_REG_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			aio_set_val(buffer,index,  aio_get_val(hold_regs,io_start)  );
			io_start++;
			index++;
		}
	}
	return index;
}

int vio_write_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr < HOLD_REG_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (HOLD_REG_MAX))
		{
			io_end = HOLD_REG_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			val_old = aio_get_val(hold_regs,io_start);
			val_new = aio_get_val(buffer,index);
			if(val_old != val_new)
			{
				aio_set_val(hold_regs,io_start,val_new);
				count++;
			}
			io_start++;
			index++;
		}
	}
	if(count)
	{
		//通知硬件更新输出
		vio_lowlevel_update();
	}
	return index;
}

int vio_lowlevel_update_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < HOLD_REG_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (HOLD_REG_MAX))
		{
			io_end = HOLD_REG_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start < io_end)
		{
			aio_set_val(hold_regs,io_start, aio_get_val(buffer,index));
			io_start++;
			index++;
		}
	}
	return index;
}


