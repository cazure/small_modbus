#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"

#define DO_MASK		0x10000000
#define DI_MASK		0x20000000
#define AO_MASK		0x40000000
#define AI_MASK		0x80000000
static rt_device_t bio_dev = {0};  //test device

static small_modbus_t modbus_slave = {0}; 
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),__VA_ARGS__)

//�ӻ��ص�����,���ӻ����յ�����������(����У��͵�ַ�������Ѿ�������),������ص�������������ݣ��������ݵĳ��ȼ���
static int test_modbus_rtu_slave_callback(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data)
{
	int rc = 0;
	switch(function_code)
	{
		case MODBUS_FC_READ_HOLDING_COILS:	//��ȡ������Ȧ,1bit����һ����Ȧ
		{
			if((0 <= addr)&&(addr < 10000))	//��ַӳ�䣬��ַ��0��ʼ
			{
				rc = rt_device_read(bio_dev,DO_MASK+addr,read_write_data,num);  
			}
		}break;
		case MODBUS_FC_READ_INPUTS_COILS:	//��ȡֻ����Ȧ,1bit����һ����Ȧ
		{
			if((10000 <= addr)&&(addr < 20000)) //��ַӳ�䣬��ַ��10000��ʼ
			{
				addr = addr - 10000;
				rc = rt_device_read(bio_dev,DI_MASK+addr,read_write_data,num);  
			}
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:	//��ȡ���ּĴ���,16bit����һ���Ĵ���
		{
			if((40000 <= addr)&&(addr < 50000)) //��ַӳ�䣬��ַ��40000��ʼ
			{
				addr = addr - 40000;
				rc = rt_device_read(bio_dev,AO_MASK+addr,read_write_data,num);  
			}
		}break;
		case MODBUS_FC_READ_INPUT_REGISTERS:	//��ȡ����Ĵ���,16bit����һ���Ĵ���
		{
			if((30000 <= addr)&&(addr < 40000)) //��ַӳ�䣬��ַ��30000��ʼ
			{
				addr = addr - 30000;
				rc = rt_device_read(bio_dev,AI_MASK+addr,read_write_data,num);  
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_COIL:	//д������Ȧ,1bit����һ����Ȧ
		case MODBUS_FC_WRITE_MULTIPLE_COILS:		//д��Ȧ,1bit����һ����Ȧ
		{
			if((0 <= addr)&&(addr < 10000))	//��ַӳ�䣬��ַ��0��ʼ
			{
				rc = rt_device_write(bio_dev,DO_MASK+addr,read_write_data,num);
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:	//д�����Ĵ���,16bit����һ���Ĵ���
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:	//д�Ĵ���,16bit����һ���Ĵ���
		{	
			if((40000 <= addr)&&(addr < 50000))	//��ַӳ�䣬��ַ��40000��ʼ
			{
				addr = addr - 40000;
				rc = rt_device_write(bio_dev,AO_MASK+addr,read_write_data,num);
			}
		}break;
	}	
	if(rc<0)
	{
		MODBUS_PRINTF("callback fail %d\n",rc);
	}
	return rc;
}

static int uart_rts(int on)
{
	board_uart_dir(4,on);//rts����
	board_led_set(1,on);//led ״̬
	return 0;
}

static void test_modbus_rtu_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;
	
	bio_dev = rt_device_find("bio");
	rt_device_open(bio_dev,0);
	
	modbus_init(smb_slave,MODBUS_CORE_RTU,modbus_port_device_create("uart4")); // init modbus
	
	struct serial_configure serial_config;
	serial_config.baud_rate = BAUD_RATE_9600;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ;
	serial_config.parity = PARITY_NONE;
	modbus_set_serial_config(smb_slave,&serial_config);  //config serial 
	
	modbus_set_rts(smb_slave,uart_rts);
	
	//modbus_set_oflag(&modbus_slave,RT_DEVICE_FLAG_INT_RX);
	modbus_set_oflag(smb_slave,RT_DEVICE_FLAG_DMA_RX);
	
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

