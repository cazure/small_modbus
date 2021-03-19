/*
* Change Logs:
* Date           Author       Notes
* 2021-02-20     chenbin      
*/
#include "drv_board.h"

extern int board_gpio_init(void);
extern int board_adc_init(void);

#define DI_MAX 64
static uint8_t DI_arr[8] = {0};

#define DO_MAX 64
static uint8_t DO_arr[8] = {0};

int dio_get_val(uint8_t *array,uint16_t index)
{
	uint8_t offset = (index & 0x0F); //(index%8);
	array += (index >> 0x04); //(index/8);
	return ((*array) & (0x01 << offset))?1:0;
}

void dio_set_val(uint8_t *array,uint16_t index,int status)
{
	uint8_t offset = (index & 0x0F); //(index%8);
	array += (index >> 0x04); //(index/8);
	if(status)
	{
		*array |= (0x01 << offset);
	}else
	{
		*array &= (!(0x01 << offset));
	}
}


void do_batch(uint8_t *array,uint16_t start_addr,uint16_t end_addr)
{
	switch(start_addr)
	{
		case 0: board_out_set(0,dio_get_val(array,0)); if(end_addr == 0){break;}
		case 1: board_out_set(1,dio_get_val(array,1)); if(end_addr == 1){break;}
		case 2: board_out_set(2,dio_get_val(array,2)); if(end_addr == 2){break;}
		case 3: board_out_set(3,dio_get_val(array,3)); if(end_addr == 3){break;}
		case 4: board_out_set(4,dio_get_val(array,4)); if(end_addr == 4){break;}
		case 5: board_out_set(5,dio_get_val(array,5)); if(end_addr == 5){break;}
		case 6: board_out_set(6,dio_get_val(array,6)); if(end_addr == 6){break;}
		case 7: board_out_set(7,dio_get_val(array,7)); if(end_addr == 7){break;}
	}
}

void di_batch(uint8_t *array,uint16_t start_addr,uint16_t end_addr)
{
	switch(start_addr)
	{
		case 0: dio_set_val(array,0,board_in_get(0)); if(end_addr == 0){break;}
		case 1: dio_set_val(array,1,board_in_get(1)); if(end_addr == 1){break;}
		case 2: dio_set_val(array,2,board_in_get(2)); if(end_addr == 2){break;}
		case 3: dio_set_val(array,3,board_in_get(3)); if(end_addr == 3){break;}
		case 4: dio_set_val(array,4,board_in_get(4)); if(end_addr == 4){break;}
		case 5: dio_set_val(array,5,board_in_get(5)); if(end_addr == 5){break;}
		case 6: dio_set_val(array,6,board_in_get(6)); if(end_addr == 6){break;}
		case 7: dio_set_val(array,7,board_in_get(7)); if(end_addr == 7){break;}
	}
}

static int read_do(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < DO_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (DO_MAX))
		{
			io_end = DO_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start <= io_end)
		{
			dio_set_val(buffer,index, dio_get_val(DO_arr,io_start) );
			io_start++;
			index++;
		}
	}
	return num;
}

static int read_di(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	if((0 <= addr)&&(addr < DI_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (DI_MAX))
		{
			io_end = DI_MAX;
		}else
		{
			io_end = addr+num;
		}
		di_batch(DI_arr,io_start,io_end); //刷新输入
		while(io_start <= io_end)
		{
			dio_set_val(buffer,index, dio_get_val(DI_arr,io_start) );
			io_start++;
			index++;
		}
	}
	return num;
}


static int write_do(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr < DO_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (DO_MAX))
		{
			io_end = DO_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start <= io_end)
		{
			val_old = dio_get_val(DO_arr,io_start);
			val_new = dio_get_val(buffer,index);
			if(val_old != val_new)
			{
				dio_set_val(DO_arr,io_start,val_new);
				count++;
			}
			io_start++;
			index++;
		}
	}
	if(count)
	{
		do_batch(DO_arr,addr,io_end); //刷新输出
	}
	return num;
}

static int write_di(uint16_t addr,uint16_t num,uint8_t *buffer)
{	
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr < DI_MAX))
	{ 
		io_start = addr; 
		if((addr+num) > (DI_MAX))
		{
			io_end = DI_MAX;
		}else
		{
			io_end = addr+num;
		}
		while(io_start <= io_end)
		{
			val_old = dio_get_val(DI_arr,io_start);
			val_new = dio_get_val(buffer,index);
			if(val_old != val_new)
			{
				dio_set_val(DI_arr,io_start,val_new);
				count++;
			}
			io_start++;
			index++;
		}
	}
	if(count)
	{
		//
	}
	return num;
}


#define AI_MAX 16
static uint16_t AI_arr[AI_MAX] = {0};

#define AO_MAX 16
static uint16_t AO_arr[AO_MAX] = {0};

static int read_ao(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < AO_MAX))
	{ 
		if((addr+num) > (AO_MAX))
		{
			num = AO_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
			//AO_arr[addr] = 
			*buffer = AO_arr[addr];
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}

static int read_ai(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < AI_MAX))
	{ 
		if((addr+num) > (AI_MAX))
		{
			num = AI_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
			//AI_arr[addr] = 
			switch(addr)
			{
				case 0: AI_arr[addr] = board_adc_read_vref();break;
				case 1: AI_arr[addr] = board_adc_read_temp();break;
				case 2: AI_arr[addr] = board_adc_read_power();break;
			}
			*buffer = AI_arr[addr];
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}


static int write_ao(uint16_t addr,uint16_t num,const uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < AO_MAX))
	{ 
		if((addr+num) > (AO_MAX))
		{
			num = AO_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
			AO_arr[addr] = *buffer;
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}

static int write_ai(uint16_t addr,uint16_t num,const uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < AI_MAX))
	{ 
		if((addr+num) > (AI_MAX))
		{
			num = AI_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
			AI_arr[addr] = *buffer;
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}


int boardio_read(uint32_t mask_addr,uint32_t num,void *buffer)
{
	uint32_t type = mask_addr & 0xF0000000;
	uint32_t addr = mask_addr & 0x0FFFFFFF;
	switch(type)
	{
		case DO_MASK:
			return read_do(addr,num,buffer);
			break;
		case DI_MASK:
			return read_di(addr,num,buffer);
			break;
		case AO_MASK:
			return read_ao(addr,num,buffer);
			break;
		case AI_MASK:
			return read_ai(addr,num,buffer);
			break;
	}
	return 0;
}
int boardio_write(uint32_t mask_addr,uint32_t num,void *buffer)
{
	uint32_t type = mask_addr & 0xF0000000;
	uint32_t addr = mask_addr & 0x0FFFFFFF;
	switch(type)
	{
		case DO_MASK:
			return write_do(addr,num,buffer);
			break;
		case DI_MASK:
			return write_di(addr,num,buffer);
			break;
		case AO_MASK:
			return write_ao(addr,num,buffer);
			break;
		case AI_MASK:
			return write_ai(addr,num,buffer);
			break;
	}
	return 0;
}


/* RT-Thread Device Interface */
/*
 * This function initializes board io device.
 */
static rt_err_t rt_boardio_init(struct rt_device *dev)
{
	rt_err_t result = RT_EOK;
	struct rt_boardio_device *boardio;

	RT_ASSERT(dev != RT_NULL);
	boardio = (struct rt_boardio_device *)dev;

	return result;
}

static rt_err_t rt_boardio_open(struct rt_device *dev, rt_uint16_t oflag)
{
	rt_uint16_t stream_flag = 0;
	struct rt_boardio_device *boardio;

	RT_ASSERT(dev != RT_NULL);
	boardio = (struct rt_boardio_device *)dev;

	return RT_EOK;
}

static rt_err_t rt_boardio_close(struct rt_device *dev)
{
    struct rt_boardio_device *boardio;

    RT_ASSERT(dev != RT_NULL);
    boardio = (struct rt_boardio_device *)dev;

    /* this device has more reference count */
    if (dev->ref_count > 1) return RT_EOK;
	
    return RT_EOK;
}

static rt_size_t rt_boardio_read(struct rt_device *dev,rt_off_t pos,void *buffer,rt_size_t size)
{
	if (size == 0) return 0;
	return boardio_read(pos,size,buffer);
}

static rt_size_t rt_boardio_write(struct rt_device *dev,rt_off_t pos,const void *buffer,rt_size_t size)
{
	if (size == 0) return 0;
	return boardio_write(pos,size,(void *)buffer);
}


static rt_err_t rt_boardio_control(struct rt_device *dev,int cmd,void *args)
{
	rt_err_t ret = RT_EOK;
	struct rt_boardio_device *boardio;

	RT_ASSERT(dev != RT_NULL);
	boardio = (struct rt_boardio_device *)dev;
	return ret;
}

#ifdef RT_USING_DEVICE_OPS
	const static struct rt_device_ops boardio_ops =
	{
		rt_boardio_init,
		rt_boardio_open,
		rt_boardio_close,
		rt_boardio_read,
		rt_boardio_write,
		rt_boardio_control,
	};
#endif

/*
 * boardio register
 */
rt_err_t rt_hw_boardio_register(struct rt_boardio_device *boardio,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data)
{
    rt_err_t ret;
    struct rt_device *device;
    RT_ASSERT(boardio != RT_NULL);
	
    device = &(boardio->parent);

    device->type        = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &boardio_ops;
#else
    device->init        = rt_boardio_init;
    device->open        = rt_boardio_open;
    device->close       = rt_boardio_close;
    device->read        = rt_boardio_read;
    device->write       = rt_boardio_write;
    device->control     = rt_boardio_control;
#endif
    device->user_data   = data;

    /* register a character device */
    ret = rt_device_register(device, name, flag);

#if defined(RT_USING_POSIX)
    /* set fops */
    device->fops        = &_boardio_fops;
#endif

    return ret;
}

static struct rt_boardio_device boardio;

void boardio_init(void)
{
	rt_hw_boardio_register(&boardio,"bio",0,NULL);
}

int boardio_get_addr(int tab,int addr)
{
	switch(tab)
	{
		case 0:
			return (DO_MASK | (addr & 0x0FFFFFFF));
			break;
		case 1:
			return (DI_MASK | (addr & 0x0FFFFFFF));
			break;
		case 2:
			return (AO_MASK | (addr & 0x0FFFFFFF));
			break;
		case 3:
			return (AI_MASK | (addr & 0x0FFFFFFF));
			break;
	}
	return 0;
}

int boardio_digital_get_val(uint8_t *array,uint8_t max,uint16_t index)
{
	if((index) < max)
	{
		return (array[(index/8)] & (0x01 << (index%8)))?1:0;
	}
	return -1;
}

int boardio_analog_get_val(uint16_t *array,uint8_t max,uint16_t index)
{
	if((index) < max)
	{
		return array[index];
	}
	return -1;
}

void boardio_show(int tab)
{
	uint8_t DI_buff[4] = {0};
	uint8_t DO_buff[4] = {0};
	uint16_t AI_buff[32] = {0};
	uint16_t AO_buff[32] = {0};
	uint8_t rc_di,rc_do,rc_ai,rc_ao;
	uint16_t index = 0;
	if(tab == -1)
	{
		rc_do = boardio_read(DO_MASK,32,DO_buff);
		rc_di = boardio_read(DI_MASK,32,DI_buff);
		rc_ao = boardio_read(AO_MASK,32,AO_buff);
		rc_ai = boardio_read(AI_MASK,32,AI_buff);
		rt_kprintf("DO\tDI\tAO\tAI\n");
		for(index=0;index<32;index++)
		{
			rt_kprintf("%d\t",boardio_digital_get_val(DO_buff,rc_do,index));
			rt_kprintf("%d\t",boardio_digital_get_val(DI_buff,rc_di,index));
			rt_kprintf("%d\t",boardio_analog_get_val(AO_buff,rc_ao,index));
			rt_kprintf("%d\t",boardio_analog_get_val(AI_buff,rc_ai,index));
			rt_kputs("\n");
		}
	}
}


//**************************//
//*** 控制台调用函数  *****//
//************************//
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static int BIO(uint8_t argc, char **argv)
{
	int  tab = 0,addr = 0,status = 0,rc=0;
	if (argc == 4)
	{
		addr = boardio_get_addr(atoi(argv[1]),atoi(argv[2]));
		status = atoi(argv[3]);
		rc = boardio_write(addr,1,&status);
		rt_kprintf("BIO write: %d %d\n",0x0FFFFFFF&addr,status);
	}else if(argc == 3)
	{
		addr = boardio_get_addr(atoi(argv[1]),atoi(argv[2]));
		rc = boardio_read(addr,1,&status);
		rt_kprintf("BIO read: %d %d\n",0x0FFFFFFF&addr,status);
	}else if(argc == 2)
	{
		boardio_show(atoi(argv[1]));
	}else
	{
		boardio_show(-1);
	}
	return 0;
}
MSH_CMD_EXPORT(BIO, BIO: [TAB] [addr] [num]);

#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */


