#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"

static small_modbus_t modbus_master = {0};
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_master),__VA_ARGS__)

static int uart4_rts(int on)
{
	board_uart_dir(4,on);
	return 0;
}

static uint8_t temp_buff[256];

static void test_modbus_rtu_master_thread(void *param)
{
	int rc = 0;	
	small_modbus_t *smb_master = param;
	
	modbus_init(smb_master,MODBUS_CORE_RTU,modbus_port_device_create("uart4")); // init modbus
	
	struct serial_configure serial_config;
	serial_config.baud_rate = BAUD_RATE_9600;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ;
	serial_config.parity = PARITY_NONE;
	modbus_set_serial_config(smb_master,&serial_config);  //config serial 
	
	modbus_set_rts(smb_master,uart4_rts);
	
	//modbus_set_oflag(&modbus_slave,RT_DEVICE_FLAG_INT_RX);
	modbus_set_oflag(smb_master,RT_DEVICE_FLAG_DMA_RX);
	
	modbus_connect(smb_master);
	rt_kprintf("modbus master\n");
	
	int count_ok = 0;
	int count_err = 0;
	while (1)
	{
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_read_input_bits(smb_master, 0, 8, temp_buff); // modbus_read_input_bits
		rt_kprintf("modbus_read_input_bits:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_write_bits(smb_master, 0 , 8, temp_buff); // modbus_write_bits
		rt_kprintf("modbus_write_bits:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_read_bits(smb_master, 0 , 8, temp_buff); // modbus_read_bits
		rt_kprintf("modbus_read_bits:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_read_input_registers(smb_master, 0 , 8, (uint16_t*)temp_buff);  // modbus_read_input_registers
		rt_kprintf("modbus_read_input_registers:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_write_registers(smb_master, 0 , 8, (uint16_t*)temp_buff); // modbus_write_registers
		rt_kprintf("modbus_write_registers:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_read_registers(smb_master, 0 , 8, (uint16_t*)temp_buff); // modbus_read_registers
		rt_kprintf("modbus_read_registers:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		rt_thread_mdelay(3000);
	}
	//modbus_disconnect(&modbus_slave);
}

int test_modbus_rtu_master(void)
{
	rt_thread_t tid;
	
	tid = rt_thread_create("master",test_modbus_rtu_master_thread, &modbus_master,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

