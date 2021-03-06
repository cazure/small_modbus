#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"

#define delay_ms        rt_thread_mdelay

#include "drv_board.h"
static rt_device_t bio_dev;
//static uint8_t DI_buff[32];
//static uint8_t DO_buff[32];

static small_modbus_t modbus_slave = {0};
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),__VA_ARGS__)

static uint8_t temp_buff[256];

static int modbus_rtu_slave_callback(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data)
{
	int rc = 0;
	switch(function_code)
	{
		case MODBUS_FC_READ_HOLDING_COILS:
		{
			rc = rt_device_read(bio_dev,DO_MASK+addr,temp_buff,num);
			rc = modbus_array2bit(read_write_data, temp_buff, rc);
//			rc = master_read(IOTAB_DO,addr,num,temp_buff);
//			rc = modbus_array2bit(read_write_data, temp_buff, rc);
		}break;
		case MODBUS_FC_READ_INPUTS_COILS:
		{
			rc = rt_device_read(bio_dev,DI_MASK+addr,temp_buff,num);
			rc = modbus_array2bit(read_write_data, temp_buff, rc);
//			rc = master_read(IOTAB_DI,addr,num,temp_buff);
//			rc = modbus_array2bit(read_write_data, temp_buff, rc);
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:
		{
			rc = rt_device_read(bio_dev,AO_MASK+addr,temp_buff,num);
			rc = modbus_array2reg(read_write_data, temp_buff, rc);
//			rc = master_read(IOTAB_AO,addr,num,temp_buff);
//			rc = modbus_array2reg(read_write_data, temp_buff, rc);
		}break;
		case MODBUS_FC_READ_INPUT_REGISTERS:
		{
			rc = rt_device_read(bio_dev,AI_MASK+addr,temp_buff,num);
			rc = modbus_array2reg(read_write_data, temp_buff, rc);
//			rc = master_read(IOTAB_AI,addr,num,temp_buff);
//			rc = modbus_array2reg(read_write_data, temp_buff, rc);
		}break;
		
		case MODBUS_FC_WRITE_SINGLE_COIL:
		{
			uint8_t value = num?1:0;
			rt_device_write(bio_dev,DO_MASK+addr,&value,1);
//			uint8_t value = num?1:0;
//			rc = master_write(IOTAB_DO,addr,1,&value);
		}break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:
		{
			uint8_t value = num?1:0;
			rt_device_write(bio_dev,AO_MASK+addr,&value,1);
//			uint16_t value = num;
//			rc = master_write(IOTAB_AO,addr,1,&value);
		}break;
		
		case MODBUS_FC_WRITE_MULTIPLE_COILS:
		{
			rc = modbus_bit2array(temp_buff,read_write_data,num);
			rc = rt_device_write(bio_dev,DO_MASK+addr,temp_buff,num);
//			rc = modbus_bit2array(temp_buff,read_write_data,num);
//			rc = master_write(IOTAB_DO,addr,num,temp_buff);
		}break;
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
		{
			rc = modbus_reg2array(temp_buff,read_write_data,num);
			rc = rt_device_write(bio_dev,AO_MASK+addr,temp_buff,num);
//			rc = modbus_reg2array(temp_buff,read_write_data,num);
//			rc = master_write(IOTAB_AO,addr,num,temp_buff);
		}break;
	}	
	if(rc<0)
	{
		MODBUS_PRINTF("callback fail %d\n",rc);
	}
	return rc;
}

int uart4_rts(int on)
{
	board_uart_dir(4,on);
	return 0;
}


void modbus_rtu_slave_thread(void *param)
{
	int rc = 0;
	modbus_rtu_init(&modbus_slave,modbus_port_device_create("uart4"));
	
	struct serial_configure serial_config;
	
	serial_config.baud_rate = BAUD_RATE_9600,
	serial_config.data_bits = DATA_BITS_8,
	serial_config.stop_bits = STOP_BITS_1,
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ,
	serial_config.parity = PARITY_NONE,
	
	modbus_port_device_set_config(&modbus_slave,&serial_config);
	modbus_port_device_set_rts(&modbus_slave,uart4_rts);
	
	modbus_set_slave(&modbus_slave,1);
	modbus_connect(&modbus_slave);
	rt_kprintf("modbus slave addr:%d\n",1);
	
	bio_dev = rt_device_find("bio");
	
	rt_device_open(bio_dev,0);
	
	while (1)
	{
		rc = modbus_slave_wait_handle(&modbus_slave,modbus_rtu_slave_callback,MODBUS_WAIT_FOREVER);
		if (rc > 0)
		{
			
		}else
		{
			modbus_error_recovery(&modbus_slave);
		}
	}
	//modbus_disconnect(&modbus_slave);
}


int modbus_rtu_test(void)
{
	rt_thread_t tid3,tid6;
	uint8_t type = 0;
	rt_kprintf("init %d\n",type);

	tid3 = rt_thread_create("Mslave",modbus_rtu_slave_thread, RT_NULL,2048,20, 10);
	if (tid3 != RT_NULL)
			rt_thread_startup(tid3);

//	tid6 = rt_thread_create("modbus M",modbus_rtu_master_thread, RT_NULL,2048,20, 10);
//	if (tid6 != RT_NULL)
//		rt_thread_startup(tid6);
	return 0;
}

//#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
//#include <finsh.h>
//int modbus_rtu_debug(int argc, char**argv)
//{
//    int now_level = 0;
//    if(argc<2)
//    {
//        rt_kprintf("modbus_rtu_debug [0-2]\n0.disable 1.error  2.info\n");
//    }else
//    {
//        now_level  = atoi(argv[1])%3;
//        rt_kprintf("modbus_rtu_debug %d\n",now_level);
//        modbus_set_debug(&modbus_slave,now_level);
//        modbus_set_debug(&modbus_master,now_level);
//    }
//    return RT_EOK;
//}
//MSH_CMD_EXPORT(modbus_rtu_debug,modbus_rtu_debug [0-2]);
//MSH_CMD_EXPORT(modbus_rtu_test,modbus rtu test);
//#endif
