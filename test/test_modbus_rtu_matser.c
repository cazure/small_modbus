#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus.h"
#include "board_virtualIO.h"

static small_modbus_t modbus_master = {0};
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug_info((&modbus_master),__VA_ARGS__)

#define UART_DEVICE_NAME "uart8"

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
//	board_uart_dir(2,on);//rtsÉèÖÃ
//	board_led_set(1,on);//led ×´Ì¬
	if(on)
	{
//		rt_pin_write(RS485_RTS_PIN, PIN_HIGH);
//		
//		rt_pin_write(RS485_STA_PIN, PIN_LOW);
		
		uart_us_delay(1000);
	}else
	{
		uart_us_delay(3000);
		
//		rt_pin_write(RS485_RTS_PIN, PIN_LOW);
//		
//		rt_pin_write(RS485_STA_PIN, PIN_HIGH);
	}
	return 0;
}

struct rt_semaphore send_modbus_sem;

void vio_lowlevel_update(void)
{
	rt_sem_release(&(send_modbus_sem));
}


static uint8_t temp_buff[256];

static void test_modbus_rtu_master_thread(void *param)
{
	int rc = 0;	
	small_modbus_t *smb_master = param;
//	
//	rt_pin_mode(RS485_RTS_PIN, PIN_MODE_OUTPUT);
//	rt_pin_mode(RS485_STA_PIN, PIN_MODE_OUTPUT);
//	
//	rt_pin_write(RS485_RTS_PIN, PIN_LOW);
	
	modbus_init(smb_master,MODBUS_CORE_RTU,modbus_port_rtdevice_create(UART_DEVICE_NAME)); // init modbus  RTU mode
	
	struct serial_configure serial_config;
	serial_config.baud_rate = BAUD_RATE_9600;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ;
	serial_config.parity = PARITY_NONE;
	modbus_rtu_set_serial_config(smb_master,&serial_config);  //config serial 
	
	modbus_rtu_set_serial_rts(smb_master,uart_rts);
	
	modbus_rtu_set_oflag(smb_master,RT_DEVICE_FLAG_INT_RX);
	//modbus_rtu_set_oflag(smb_master,RT_DEVICE_FLAG_DMA_RX);
	
	modbus_connect(smb_master);
	MODBUS_PRINTF("modbus master\n");
	
	
	rt_sem_init(&(send_modbus_sem), "sendsem", 0, RT_IPC_FLAG_FIFO);
	
	int count_ok = 0;
	int count_err = 0;
	int index = 0;
	while(1)
	{
		rt_thread_mdelay(10);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 3);
		
		rc = rt_sem_take(&(send_modbus_sem),3000);
    if(rc == RT_EOK)
		{
			vio_read_hold_coils(0,16,temp_buff);  //read
			
			rc = modbus_write_bits(smb_master, 00000 , 16, temp_buff); // modbus_write_bits
			MODBUS_PRINTF("modbus_write_bits:%d\n",rc);
			if(rc >= MODBUS_OK)
			{
				count_ok++;
			}else
			{
				count_err++;
			}
		}
	}
	
//	while (1)
//	{
//		rt_thread_mdelay(30);
//		modbus_error_recovery(smb_master);
//		modbus_set_slave(smb_master, 1);
//		rc = modbus_read_input_bits(smb_master, 10000, 8, temp_buff); // modbus_read_input_bits
//		rt_kprintf("modbus_read_input_bits:%d\n",rc);
//		if(rc >= MODBUS_OK)
//		{
//			for(index = 0; index <8;index++)
//			{
//				rt_kprintf("[%d]",dio_get_val(temp_buff,index));
//			}
//			rt_kputs("\n\r");
//			count_ok++;
//		}else
//		{
//			count_err++;
//		}
//		
//		rt_thread_mdelay(30);
//		modbus_error_recovery(smb_master);
//		modbus_set_slave(smb_master, 1);
//		rc = modbus_write_bits(smb_master, 00000 , 8, temp_buff); // modbus_write_bits
//		rt_kprintf("modbus_write_bits:%d\n",rc);
//		if(rc >= MODBUS_OK)
//		{
//			count_ok++;
//		}else
//		{
//			count_err++;
//		}
//		
//		rt_thread_mdelay(30);
//		modbus_error_recovery(smb_master);
//		modbus_set_slave(smb_master, 1);
//		rc = modbus_read_bits(smb_master, 00000 , 8, temp_buff); // modbus_read_bits
//		rt_kprintf("modbus_read_bits:%d\n",rc);
//		if(rc >= MODBUS_OK)
//		{
//			for(index = 0; index <8;index++)
//			{
//				rt_kprintf("[%d]",dio_get_val(temp_buff,index));
//			}
//			rt_kputs("\n\r");
//			count_ok++;
//		}else
//		{
//			count_err++;
//		}
//		
//		
//		rt_thread_mdelay(30);
//		modbus_error_recovery(smb_master);
//		modbus_set_slave(smb_master, 1);
//		rc = modbus_read_input_registers(smb_master, 30000 , 8, (uint16_t*)temp_buff);  // modbus_read_input_registers
//		rt_kprintf("modbus_read_input_registers:%d\n",rc);
//		if(rc >= MODBUS_OK)
//		{
//			for(index = 0; index <8;index++)
//			{
//				rt_kprintf("[%d]",aio_get_val((uint16_t*)temp_buff,index));
//			}
//			rt_kputs("\n\r");
//			count_ok++;
//		}else
//		{
//			count_err++;
//		}
//		
//		rt_thread_mdelay(30);
//		modbus_error_recovery(smb_master);
//		modbus_set_slave(smb_master, 1);
//		rc = modbus_write_registers(smb_master, 40000 , 8, (uint16_t*)temp_buff); // modbus_write_registers
//		rt_kprintf("modbus_write_registers:%d\n",rc);
//		if(rc >= MODBUS_OK)
//		{
//			count_ok++;
//		}else
//		{
//			count_err++;
//		}
//		
//		rt_thread_mdelay(30);
//		modbus_error_recovery(smb_master);
//		modbus_set_slave(smb_master, 1);
//		rc = modbus_read_registers(smb_master, 40000 , 8, (uint16_t*)temp_buff); // modbus_read_registers
//		rt_kprintf("modbus_read_registers:%d\n",rc);
//		if(rc >= MODBUS_OK)
//		{
//			for(index = 0; index <8;index++)
//			{
//				rt_kprintf("[%d]",aio_get_val((uint16_t*)temp_buff,index));
//			}
//			rt_kputs("\n\r");
//			count_ok++;
//		}else
//		{
//			count_err++;
//		}
//		
//		rt_thread_mdelay(3000);
//	}
//	modbus_disconnect(smb_master);
}

int test_modbus_rtu_master(void)
{
	rt_thread_t tid;
	
	tid = rt_thread_create("master",test_modbus_rtu_master_thread, &modbus_master,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

