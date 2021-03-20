#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"

//#define DO_MASK		0x10000000
//#define DI_MASK		0x20000000
//#define AO_MASK		0x40000000
//#define AI_MASK		0x80000000
//static rt_device_t bio_dev = {0};  //test device

static small_modbus_t modbus_slave = {0}; 
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),__VA_ARGS__)

#define HOLD_COILS_MAX 64
uint8_t hold_coils[HOLD_COILS_MAX/8] = {0};

#define INPUT_COILS_MAX 64
uint8_t input_coils[INPUT_COILS_MAX/8] = {0};

int user_read_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
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
			val_new = dio_get_val(hold_coils,io_start);
			dio_set_val(buffer,index, val_new );
			io_start++;
			index++;
		}
	}
	return num;
}

int user_read_input_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
{
	uint16_t index = 0;
	uint16_t io_start = 0;
	uint16_t io_end = 0;
	int val_old = 0;
	int val_new = 0;
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
		//刷新输入
		while(io_start < io_end)
		{
			val_new = dio_get_val(input_coils,io_start);
			dio_set_val(buffer,index,val_new);
			io_start++;
			index++;
		}
	}
	return num;
}

int user_write_hold_coils(uint16_t addr,uint16_t num,uint8_t *buffer)
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
		//刷新输出
	}
	return num;
}

#define HOLD_REG_MAX 16
uint16_t hold_regs[HOLD_REG_MAX] = {0};

#define INPUT_REG_MAX 16
uint16_t input_regs[INPUT_REG_MAX] = {0};

int user_read_hold_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < HOLD_REG_MAX))
	{ 
		if((addr+num) > (HOLD_REG_MAX))
		{
			num = HOLD_REG_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
			*buffer = hold_regs[addr];
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}

int user_read_input_regs(uint16_t addr,uint16_t num,uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < INPUT_REG_MAX))
	{ 
		if((addr+num) > (INPUT_REG_MAX))
		{
			num = INPUT_REG_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
//			switch(addr)
//			{
//				case 0: input_regs[addr] = board_adc_read_vref();break;
//				case 1: input_regs[addr] = board_adc_read_temp();break;
//				case 2: input_regs[addr] = board_adc_read_power();break;
//			}
			*buffer = input_regs[addr];
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}


int user_write_hold_regs(uint16_t addr,uint16_t num,const uint16_t *buffer)
{
	uint16_t index = 0;
	if((0 <= addr)&&(addr < HOLD_REG_MAX))
	{ 
		if((addr+num) > (HOLD_REG_MAX))
		{
			num = HOLD_REG_MAX;
		}else
		{
			num = addr+num;
		}
		while(addr < num)
		{
			hold_regs[addr] = *buffer;
			buffer++;
			addr++;
			index++;
		}
	}
	return num;
}

//从机回调函数,当从机接收到主机的请求(数据校验和地址功能码已经解析完),在这个回调函数内填充数据，返回数据的长度即可
static int test_modbus_rtu_slave_callback(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data)
{
	int rc = 0;
	switch(function_code)
	{
		case MODBUS_FC_READ_HOLDING_COILS:	//读取保持线圈,1bit代表一个线圈
		{
			if((0 <= addr)&&(addr < 10000))	//地址映射，地址从0开始
			{
				rc = user_read_hold_coils(addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_READ_INPUTS_COILS:	//读取只读线圈,1bit代表一个线圈
		{
			if((10000 <= addr)&&(addr < 20000)) //地址映射，地址从10000开始
			{
				addr = addr - 10000;
				rc = user_read_input_coils(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:	//读取保持寄存器,16bit代表一个寄存器
		{
			if((40000 <= addr)&&(addr < 50000)) //地址映射，地址从40000开始
			{
				addr = addr - 40000;
				rc = user_read_hold_regs(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_READ_INPUT_REGISTERS:	//读取输入寄存器,16bit代表一个寄存器
		{
			if((30000 <= addr)&&(addr < 40000)) //地址映射，地址从30000开始
			{
				addr = addr - 30000;
				rc = user_read_input_regs(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_COIL:	//写单个线圈,1bit代表一个线圈
		case MODBUS_FC_WRITE_MULTIPLE_COILS:		//写线圈,1bit代表一个线圈
		{
			if((0 <= addr)&&(addr < 10000))	//地址映射，地址从0开始
			{
				rc = user_write_hold_coils(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:	//写单个寄存器,16bit代表一个寄存器
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:	//写寄存器,16bit代表一个寄存器
		{	
			if((40000 <= addr)&&(addr < 50000))	//地址映射，地址从40000开始
			{
				addr = addr - 40000;
				rc = user_write_hold_regs(addr,num,read_write_data); 
			}
		}break;
	}	
	if(rc<0)
	{
		//MODBUS_PRINTF("callback fail %d\n",rc);
	}
	return rc;
}

////从机回调函数,当从机接收到主机的请求(数据校验和地址功能码已经解析完),在这个回调函数内填充数据，返回数据的长度即可
//static int test_modbus_rtu_slave_callback(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data)
//{
//	int rc = 0;
//	switch(function_code)
//	{
//		case MODBUS_FC_READ_HOLDING_COILS:	//读取保持线圈,1bit代表一个线圈
//		{
//			if((0 <= addr)&&(addr < 10000))	//地址映射，地址从0开始
//			{
//				rc = rt_device_read(bio_dev,DO_MASK+addr,read_write_data,num); 
//				
//			}
//		}break;
//		case MODBUS_FC_READ_INPUTS_COILS:	//读取只读线圈,1bit代表一个线圈
//		{
//			if((10000 <= addr)&&(addr < 20000)) //地址映射，地址从10000开始
//			{
//				addr = addr - 10000;
//				rc = rt_device_read(bio_dev,DI_MASK+addr,read_write_data,num);  
//			}
//		}break;
//		case MODBUS_FC_READ_HOLDING_REGISTERS:	//读取保持寄存器,16bit代表一个寄存器
//		{
//			if((40000 <= addr)&&(addr < 50000)) //地址映射，地址从40000开始
//			{
//				addr = addr - 40000;
//				rc = rt_device_read(bio_dev,AO_MASK+addr,read_write_data,num);  
//			}
//		}break;
//		case MODBUS_FC_READ_INPUT_REGISTERS:	//读取输入寄存器,16bit代表一个寄存器
//		{
//			if((30000 <= addr)&&(addr < 40000)) //地址映射，地址从30000开始
//			{
//				addr = addr - 30000;
//				rc = rt_device_read(bio_dev,AI_MASK+addr,read_write_data,num);  
//			}
//		}break;
//		case MODBUS_FC_WRITE_SINGLE_COIL:	//写单个线圈,1bit代表一个线圈
//		case MODBUS_FC_WRITE_MULTIPLE_COILS:		//写线圈,1bit代表一个线圈
//		{
//			if((0 <= addr)&&(addr < 10000))	//地址映射，地址从0开始
//			{
//				rc = rt_device_write(bio_dev,DO_MASK+addr,read_write_data,num);
//			}
//		}break;
//		case MODBUS_FC_WRITE_SINGLE_REGISTER:	//写单个寄存器,16bit代表一个寄存器
//		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:	//写寄存器,16bit代表一个寄存器
//		{	
//			if((40000 <= addr)&&(addr < 50000))	//地址映射，地址从40000开始
//			{
//				addr = addr - 40000;
//				rc = rt_device_write(bio_dev,AO_MASK+addr,read_write_data,num);
//			}
//		}break;
//	}	
//	if(rc<0)
//	{
//		MODBUS_PRINTF("callback fail %d\n",rc);
//	}
//	return rc;
//}

#define UART_DEVICE_NAME "uart3"

static int uart_rts(int on)
{
	board_uart_dir(3,on);//rts设置
	board_led_set(1,on);//led 状态
	return 0;
}

static void test_modbus_rtu_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;
	
//	bio_dev = rt_device_find("bio");
//	rt_device_open(bio_dev,0);
	
	modbus_init(smb_slave,MODBUS_CORE_RTU,modbus_port_device_create(UART_DEVICE_NAME)); // init modbus  RTU mode
	//modbus_init(smb_slave,MODBUS_CORE_TCP,modbus_port_device_create(UART_DEVICE_NAME)); // init modbus  TCP mode
	
	struct serial_configure serial_config;
	serial_config.baud_rate = BAUD_RATE_9600;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ;
	serial_config.parity = PARITY_NONE;
	modbus_set_serial_config(smb_slave,&serial_config);  //config serial 
	
	modbus_set_rts(smb_slave,uart_rts);
	
	modbus_set_oflag(smb_slave,RT_DEVICE_FLAG_INT_RX);
	//modbus_set_oflag(smb_slave,RT_DEVICE_FLAG_DMA_RX);
	
	modbus_set_slave(smb_slave,1); //set slave addr
	
	modbus_connect(smb_slave);
	rt_kprintf("modbus slave addr:%d\n",1);
	
	while (1)
	{
		rc = modbus_slave_wait_handle(smb_slave,test_modbus_rtu_slave_callback,MODBUS_WAIT_FOREVER);
		if (rc > 0)
		{
			count++;
		}else
		{
			modbus_error_recovery(smb_slave);
		}
	}
	//modbus_disconnect(&modbus_slave);
}

int test_modbus_rtu_slave(void)
{
	rt_thread_t tid;
	
	tid = rt_thread_create("slave",test_modbus_rtu_slave_thread, &modbus_slave,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

