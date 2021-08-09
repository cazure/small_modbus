#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus.h"
#include "board_virtualIO.h"

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
				rc = vio_read_hold_coils(addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_READ_INPUTS_COILS:	//读取只读线圈,1bit代表一个线圈
		{
			if((10000 <= addr)&&(addr < 20000)) //地址映射，地址从10000开始
			{
				addr = addr - 10000;
				rc = vio_read_input_coils(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:	//读取保持寄存器,16bit代表一个寄存器
		{
			if((40000 <= addr)&&(addr < 50000)) //地址映射，地址从40000开始
			{
				addr = addr - 40000;
				rc = vio_read_hold_regs(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_READ_INPUT_REGISTERS:	//读取输入寄存器,16bit代表一个寄存器
		{
			if((30000 <= addr)&&(addr < 40000)) //地址映射，地址从30000开始
			{
				addr = addr - 30000;
				rc = vio_read_input_regs(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_COIL:	//写单个线圈,1bit代表一个线圈
		case MODBUS_FC_WRITE_MULTIPLE_COILS:		//写线圈,1bit代表一个线圈
		{
			if((0 <= addr)&&(addr < 10000))	//地址映射，地址从0开始
			{
				rc = vio_write_hold_coils(addr,num,read_write_data); 
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:	//写单个寄存器,16bit代表一个寄存器
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:	//写寄存器,16bit代表一个寄存器
		{	
			if((40000 <= addr)&&(addr < 50000))	//地址映射，地址从40000开始
			{
				addr = addr - 40000;
				rc = vio_write_hold_regs(addr,num,read_write_data); 
			}
		}break;
	}	
	if(rc<0)
	{
		//MODBUS_PRINTF("callback fail %d\n",rc);
	}
	return rc;
}

static small_modbus_t modbus_slave = {0}; 
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),__VA_ARGS__)

#define UART_DEVICE_NAME "uart3"

#include "drv_gpio.h"
#define RS485_RTS_PIN			GET_PIN(1, 24)
#define RS485_STA_PIN			GET_PIN(1, 20)

static void uart_us_delay(rt_uint32_t us)
{
	int temp;
	while(us--)
	{
		temp = 300;
		while(temp--);
	}
}

static int uart_rts(int on)
{
//	board_uart_dir(2,on);//rts设置
//	board_led_set(1,on);//led 状态
	if(on)
	{
		rt_pin_write(RS485_RTS_PIN, PIN_HIGH);
		
		rt_pin_write(RS485_STA_PIN, PIN_LOW);
		
		uart_us_delay(1000);
	}else
	{
		uart_us_delay(3000);
		
		rt_pin_write(RS485_RTS_PIN, PIN_LOW);
		
		rt_pin_write(RS485_STA_PIN, PIN_HIGH);
	}
	return 0;
}

static void test_modbus_rtu_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;
	
	rt_pin_mode(RS485_RTS_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RS485_STA_PIN, PIN_MODE_OUTPUT);
	
	rt_pin_write(RS485_RTS_PIN, PIN_LOW);
	
	modbus_init(smb_slave,MODBUS_CORE_RTU,modbus_port_rtdevice_create(UART_DEVICE_NAME)); // init modbus  RTU mode
	
	struct serial_configure serial_config;
	serial_config.baud_rate = BAUD_RATE_9600;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ;
	serial_config.parity = PARITY_NONE;
	modbus_rtu_set_serial_config(smb_slave,&serial_config);  //config serial 
	
	modbus_rtu_set_serial_rts(smb_slave,uart_rts);
	
	modbus_rtu_set_oflag(smb_slave,RT_DEVICE_FLAG_INT_RX);
	//modbus_rtu_set_oflag(smb_slave,RT_DEVICE_FLAG_DMA_RX);
	
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
	
	tid = rt_thread_create("slave",test_modbus_rtu_slave_thread, &modbus_slave,2048,10, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

