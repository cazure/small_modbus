/*
* Change Logs:
* Date           Author       Notes
* 2021-02-20     chenbin      
* 2021-03-20     chenbin      
*/
#include "drv_boardio.h"
static struct rt_boardio_device boardio;

extern int board_gpio_init(void);
extern int board_adc_init(void);

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


#define DO_MAX 64
static uint8_t DO_arr[DO_MAX/8] = {0};

static int boardio_read_do(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= DO_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > DO_MAX) //寻址范围超出
		{
			addr_end = DO_MAX;
		}
		while(addr_start < addr_end)
		{
			val_new = dio_get_val(DO_arr,addr_start);
			dio_set_val(buffer,index, val_new );
			addr_start++;
			index++;
		}
	}
	return num;
}
static int boardio_write_do(uint16_t addr,uint16_t num,const uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= DO_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > DO_MAX) //寻址范围超出
		{
			addr_end = DO_MAX;
		}
		while(addr_start < addr_end)
		{
			val_old = dio_get_val(DO_arr, addr_start); //设置到继电器
			val_new = dio_get_val((uint8_t *)buffer, index);
			if(val_old != val_new)
			{
				dio_set_val(DO_arr, addr_start, val_new);
				board_out_set(addr_start, val_new); //设置到继电器
				count++;
			}
			addr_start++;
			index++;
		}
	}
	if(count>0)
	{
		if(boardio.parent.rx_indicate)
		{
			boardio.parent.rx_indicate(&(boardio.parent),BIO_DO);
		}
	}
	return num;
}

#define DI_MAX 64
static uint8_t DI_arr[DI_MAX/8] = {0};

void boardio_read_di_scan(uint16_t start_addr,uint16_t end_addr,uint8_t *array)
{
	switch(start_addr)
	{
		case 0: dio_set_val(array,0,board_in_get(0)); if(end_addr == 1){break;}
		case 1:	dio_set_val(array,1,board_in_get(1));	if(end_addr == 2){break;}
		case 2: dio_set_val(array,2,board_in_get(2)); if(end_addr == 3){break;}
		case 3: dio_set_val(array,3,board_in_get(3)); if(end_addr == 4){break;}
		case 4: dio_set_val(array,4,board_in_get(4)); if(end_addr == 5){break;}
		case 5: dio_set_val(array,5,board_in_get(5)); if(end_addr == 6){break;}
		case 6: dio_set_val(array,6,board_in_get(6)); if(end_addr == 7){break;}
		case 7: dio_set_val(array,7,board_in_get(7)); if(end_addr == 8){break;}
		case 8: dio_set_val(array,8,board_in_get(8)); if(end_addr == 9){break;}
		case 9: dio_set_val(array,9,board_in_get(9)); if(end_addr == 10){break;}
	}
}
static int boardio_read_di(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= DI_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > DI_MAX) //寻址范围超出
		{
			addr_end = DI_MAX;
		}
		boardio_read_di_scan(addr_start,addr_end,DI_arr); //刷新输入
		while(addr_start < addr_end)
		{
			val_new = dio_get_val(DI_arr,addr_start);
			dio_set_val(buffer,index, val_new );
			addr_start++;
			index++;
		}
	}
	return num;
}
static int boardio_write_di(uint16_t addr,uint16_t num,const uint8_t *buffer)
{
	//uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	//int val_new = 0;
	if((0 <= addr)&&(addr <= DI_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > DI_MAX) //寻址范围超出
		{
			addr_end = DI_MAX;
		}
		while(addr_start < addr_end)
		{
			dio_set_val(DI_arr, addr_start,  dio_get_val((uint8_t *)buffer, index));
			addr_start++;
			index++;
		}
	}
	return num;
}

#define DM_MAX 64
static uint8_t DM_arr[DM_MAX/8] = {0};
static int boardio_read_dm(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= DM_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > DM_MAX) //寻址范围超出
		{
			addr_end = DM_MAX;
		}
		while(addr_start < addr_end)
		{
			val_new = dio_get_val(DM_arr,addr_start);
			dio_set_val(buffer,index, val_new );
			addr_start++;
			index++;
		}
	}
	return num;
}
static int boardio_write_dm(uint16_t addr,uint16_t num,const uint8_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= DM_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > DM_MAX) //寻址范围超出
		{
			addr_end = DM_MAX;
		}
		while(addr_start < addr_end)
		{
//			dio_set_val(DM_arr, addr_start,  dio_get_val((uint8_t *)buffer, index));
//			addr_start++;
//			index++;
			val_old = dio_get_val(DM_arr, addr_start);
			val_new = dio_get_val((uint8_t *)buffer, index);
			if(val_old != val_new)
			{
				dio_set_val(DM_arr, addr_start, val_new);
				//设置到继电器
				count++;
			}
			addr_start++;
			index++;
		}
	}
	if(boardio.parent.rx_indicate)
	{
		boardio.parent.rx_indicate(&(boardio.parent),BIO_DM);
	}
	return num;
}

#define AO_MAX 16
static uint16_t AO_arr[AO_MAX] = {0};

static int boardio_read_ao(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= AO_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > AO_MAX) //寻址范围超出
		{
			addr_end = AO_MAX;
		}
		while(addr_start < addr_end)
		{
			val_new = aio_get_val(AO_arr,addr_start);
			aio_set_val(buffer,index, val_new );
			addr_start++;
			index++;
		}
	}
	return num;
}
static int boardio_write_ao(uint16_t addr,uint16_t num,const uint16_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= AO_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > AO_MAX) //寻址范围超出
		{
			addr_end = AO_MAX;
		}
		while(addr_start < addr_end)
		{
			val_old = aio_get_val(AO_arr, addr_start);
			val_new = aio_get_val((uint16_t *)buffer, index);
			if(val_old != val_new)
			{
				aio_set_val(AO_arr, addr_start, val_new);
				//更新硬件
				count++;
			}
			addr_start++;
			index++;
		}
	}
	return num;
}


#define AI_MAX 16
static uint16_t AI_arr[AI_MAX] = {0};

float vol;// = (adc * 3.3 / 4096.0 / 0.1); //外部输入电压

void boardio_read_ai_scan(uint16_t start_addr,uint16_t end_addr,uint16_t *array)
{
	int status = 0;
	switch(start_addr)
	{
		case 0: AI_arr[0] = board_adc_read_vref(); if(end_addr == 1){break;}
		case 1:	AI_arr[1] = board_adc_read_temp();	if(end_addr == 2){break;}
		case 2: AI_arr[2] = board_adc_read_power(); if(end_addr == 3){break;}
		case 3: AI_arr[3] = (uint16_t)((AI_arr[2] * 3.3 / 4096.0 / 0.1)*100); //外部输入电压; if(end_addr == 3){break;}
	}
}

static int boardio_read_ai(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= AI_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > AI_MAX) //寻址范围超出
		{
			addr_end = AI_MAX;
		}
		boardio_read_ai_scan(addr_start,addr_end,AI_arr); //刷新输入
		while(addr_start < addr_end)
		{
			val_new = aio_get_val(AI_arr,addr_start);
			aio_set_val(buffer,index, val_new );
			addr_start++;
			index++;
		}
	}
	return num;
}

static int boardio_write_ai(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	//uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	//int val_new = 0;
	if((0 <= addr)&&(addr <= AI_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > AI_MAX) //寻址范围超出
		{
			addr_end = AI_MAX;
		}
		while(addr_start < addr_end)
		{
			aio_set_val(AI_arr, addr_start,  aio_get_val(buffer, index));
			addr_start++;
			index++;
		}
	}
	return num;
}


#define AM_MAX 16
static uint16_t AM_arr[AM_MAX] = {0};
static int boardio_read_am(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	int val_new = 0;
	if((0 <= addr)&&(addr <= AM_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > AM_MAX) //寻址范围超出
		{
			addr_end = AM_MAX;
		}
		while(addr_start < addr_end)
		{
			val_new = aio_get_val(AM_arr,addr_start);
			aio_set_val(buffer,index, val_new );
			addr_start++;
			index++;
		}
	}
	return num;
}
static int boardio_write_am(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	//uint16_t count = 0;
	uint16_t index = 0;
	uint16_t addr_start = 0;
	uint16_t addr_end = 0;
	//int val_old = 0;
	//int val_new = 0;
	if((0 <= addr)&&(addr <= AM_MAX)) //检查首地址
	{
		addr_start = addr;
		addr_end = addr+num;
		if(addr_end > AM_MAX) //寻址范围超出
		{
			addr_end = AM_MAX;
		}
		while(addr_start < addr_end)
		{
			aio_set_val(AM_arr, addr_start,  aio_get_val(buffer, index));
			addr_start++;
			index++;
		}
	}
	return num;
}

/* RT-Thread Device Interface */
/*
 * This function initializes board io device.
 */
static rt_err_t rt_boardio_init(struct rt_device *dev)
{
	return RT_EOK;
}

static rt_err_t rt_boardio_open(struct rt_device *dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_boardio_close(struct rt_device *dev)
{
	return RT_EOK;
}

static rt_size_t rt_boardio_read(struct rt_device *dev,rt_off_t pos,void *buffer,rt_size_t size)
{
	uint32_t type = BIO_TYPE(pos);
	uint32_t addr = BIO_ADDR(pos);
	if((size == 0)||(type == 0))
	{
		return 0;
	}
	switch(type)
	{
		case BIO_DI:
			return boardio_read_di(addr,size,(void *)buffer);
		case BIO_DM:
			return boardio_read_dm(addr,size,(void *)buffer);
		case BIO_DO:
			return boardio_read_do(addr,size,(void *)buffer);
		case BIO_AI:
			return boardio_read_ai(addr,size,(void *)buffer);
		case BIO_AM:
			return boardio_read_am(addr,size,(void *)buffer);
		case BIO_AO:
			return boardio_read_ao(addr,size,(void *)buffer);
		default:
			return 0;
	}
}

static rt_size_t rt_boardio_write(struct rt_device *dev,rt_off_t pos,const void *buffer,rt_size_t size)
{
	uint32_t type = BIO_TYPE(pos);
	uint32_t addr = BIO_ADDR(pos);
	if((size == 0)||(type == 0))
	{
		return 0;
	}
	switch(type)
	{
		case BIO_DI:
			return boardio_write_di(addr,size,(void *)buffer);
		case BIO_DM:
			return boardio_write_dm(addr,size,(void *)buffer);
		case BIO_DO:
			return boardio_write_do(addr,size,(void *)buffer);
		case BIO_AI:
			return boardio_write_ai(addr,size,(void *)buffer);
		case BIO_AM:
			return boardio_write_am(addr,size,(void *)buffer);
		case BIO_AO:
			return boardio_write_ao(addr,size,(void *)buffer);
		default:
			return 0;
	}
}


static rt_err_t rt_boardio_control(struct rt_device *dev,int cmd,void *args)
{
	return RT_EOK;
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

void boardio_init(void)
{
	rt_hw_boardio_register(&boardio,"bio",0,NULL);
}


int boardio_read(uint32_t type,uint32_t addr,uint32_t num,void *buffer)
{
	switch(type)
	{
		case BIO_DI:
			return boardio_read_di(addr,num,buffer);
		case BIO_DM:
			return boardio_read_dm(addr,num,buffer);
		case BIO_DO:
			return boardio_read_do(addr,num,buffer);
		case BIO_AI:
			return boardio_read_ai(addr,num,buffer);
		case BIO_AM:
			return boardio_read_am(addr,num,buffer);
		case BIO_AO:
			return boardio_read_ao(addr,num,buffer);
	}
	return 0;
}
int boardio_write(uint32_t type,uint32_t addr,uint32_t num,void *buffer)
{
	switch(type)
	{
		case BIO_DI:
			return boardio_write_di(addr,num,buffer);
		case BIO_DM:
			return boardio_write_dm(addr,num,buffer);
		case BIO_DO:
			return boardio_write_do(addr,num,buffer);
		case BIO_AI:
			return boardio_write_ai(addr,num,buffer);
		case BIO_AM:
			return boardio_write_am(addr,num,buffer);
		case BIO_AO:
			return boardio_write_ao(addr,num,buffer);
	}
	return 0;
}

const char *BIO_string(uint16_t type)
{
	switch(type)
	{
		case BIO_DI:
			return "DI";
		case BIO_DM:
			return "DM";
		case BIO_DO:
			return "DO";
		case BIO_AI:
			return "AI";
		case BIO_AM:
			return "AM";
		case BIO_AO:
			return "AO";
		default:
			return NULL;
	}
}
int BIO_iomax(uint16_t type)
{
	switch(type)
	{
		case BIO_DI:
			return DI_MAX;
		case BIO_DM:
			return DM_MAX;
		case BIO_DO:
			return DO_MAX;
		case BIO_AI:
			return AI_MAX;
		case BIO_AM:
			return AM_MAX;
		case BIO_AO:
			return AO_MAX;
		default:
			return NULL;
	}
}

#define GET_MAX(n1,n2) (n1>n2)?n1:n2
#define GET_MIN(n1,n2) (n1<n2)?n1:n2

void boardio_show(int type)
{
	uint8_t rc_di,rc_do,rc_ai,rc_ao;
	uint16_t index = 0;
	
	int iomax = GET_MIN(DI_MAX,DI_MAX);
	iomax = GET_MIN(iomax,DM_MAX);
	iomax = GET_MIN(iomax,AI_MAX);
	iomax = GET_MIN(iomax,AO_MAX);
	iomax = GET_MIN(iomax,AM_MAX);
	
	if(BIO_string(type))
	{
		rt_kprintf("BIO\t%s\n",BIO_string(type));
		iomax = BIO_iomax(type);
		for(index=0;index<iomax;index++)
		{
			rt_kprintf("%02d>\t",index);
			if(type==BIO_DO)rt_kprintf("%d\t",dio_get_val(DO_arr,index));
			else if(type==BIO_DM)rt_kprintf("%d\t",dio_get_val(DM_arr,index));
			else if(type==BIO_DI)rt_kprintf("%d\t",dio_get_val(DI_arr,index));
			else if(type==BIO_AO)rt_kprintf("%d\t",aio_get_val(AO_arr,index));
			else if(type==BIO_AM)rt_kprintf("%d\t",aio_get_val(AM_arr,index));
			else if(type==BIO_AI)rt_kprintf("%d\t",aio_get_val(AI_arr,index));
			rt_kputs("\n");
		}
		
	}else
	{
		rt_kprintf("BIO\tDI(1)\tDM(2)\tDO(3)\tAI(4)\tAM(5)\tAO(6)\n");
		rt_kprintf("---\t---\t---\t---\t---\t---\t---\n");
		for(index=0;index<iomax;index++)
		{
			rt_kprintf("%02d>\t",index);
			rt_kprintf("%d\t",dio_get_val(DI_arr,index));
			rt_kprintf("%d\t",dio_get_val(DM_arr,index));
			rt_kprintf("%d\t",dio_get_val(DO_arr,index));
			rt_kprintf("%d\t",aio_get_val(AI_arr,index));
			rt_kprintf("%d\t",aio_get_val(AM_arr,index));
			rt_kprintf("%d\t",aio_get_val(AO_arr,index));
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
	int  type = 0,addr = 0,status = 0,rc=0;
	if (argc == 4)
	{
		type = atoi(argv[1]);
		addr = atoi(argv[2]);
		status = atoi(argv[3]);
		rc = boardio_write(type,addr,1,&status);
		rt_kprintf("BIO: %s[%d]=%d rc:%d\n",BIO_string(type),addr,status,rc);
	}else if(argc == 3)
	{
		type = atoi(argv[1]);
		addr = atoi(argv[2]);
		status = 0;
		rc = boardio_read(type,addr,1,&status);
		rt_kprintf("BIO: %s[%d]=%d\n",BIO_string(type),addr,status);
	}else if(argc == 2)
	{
		boardio_show(atoi(argv[1]));
	}else
	{
		boardio_show(0);
	}
	return 0;
}
MSH_CMD_EXPORT(BIO, BIO: [DI(1) DM(2) DO(3) AI(4) AM(5) AO(6)] [addr] [num]);

#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */


