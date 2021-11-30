#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus.h"
#include "board_virtualIO.h"

static small_modbus_t modbus_rtu_master = {0};
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug_info((&modbus_rtu_master),__VA_ARGS__)

//rtthread device name
#define UART_DEVICE_NAME "uart2"

//rtthread pin index
static  int rs485_rts_pin = 0;

//�շ��������Żص�����
static int uart_rts(int on)
{
	if(on)
	{
	    rt_pin_write(rs485_rts_pin, PIN_HIGH);
	    rt_thread_mdelay(2);  //9600 bps 3.5 ���ַ��ӳ�ʱ��
	}else
	{
		rt_thread_mdelay(2);  //9600 bps 3.5 ���ַ��ӳ�ʱ��
	    rt_pin_write(rs485_rts_pin, PIN_LOW);
	}
	return 0;
}


static int count_ok = 0;
static int count_err = 0;

#define MASTER_SEM_POLL


#ifdef MASTER_SEM_POLL

//send modbus ���ź���
struct rt_semaphore send_modbus_sem;

//ʵ��vio����֪ͨmaster����modbusдָ��
RT_WEAK void vio_lowlevel_update(void)
{
	rt_sem_release(&(send_modbus_sem));
}

//msh�����е��ú���
void test_modbus_rtu_master_sem(void)
{
	vio_lowlevel_update();
}
//msh����������
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>

MSH_CMD_EXPORT(test_modbus_rtu_master_sem, test_modbus_rtu_master sem);

#endif

#endif

static uint8_t temp_buff[256];

void master_sem_poll(small_modbus_t *smb_master)
{
	int index = 0;
	int rc = rt_sem_take(&(send_modbus_sem),3000); //�ȴ�vio_lowlevel_update�ͷ��ź���,���û�л�һֱ�ȴ�3000ms
	if(rc == RT_EOK)  //�ͷųɹ�
	{
		vio_read_hold_coils(0,16,temp_buff);  //��vio��ȡ���ּĴ���ֵ
		
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_write_bits(smb_master, 00000 , 16, temp_buff); // modbus_write_bits
		MODBUS_PRINTF("modbus_write_bits:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			count_ok++;
		}else
		{
			count_err++;
		}
		
		rt_thread_mdelay(10);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_read_input_bits(smb_master, 10000, 8, temp_buff); // modbus_read_input_bits
		MODBUS_PRINTF("modbus_read_input_bits:%d\n",rc);
		if(rc >= MODBUS_OK)
		{
			vio_lowlevel_update_input_coils(0,8,temp_buff); //��������Ĵ�����ֵ��vio
			
			for(index = 0; index <8;index++)
			{
				rt_kprintf("[%d]",dio_get_val(temp_buff,index));
			}
			rt_kputs("\n\r");
			count_ok++;
		}else
		{
			count_err++;
		}
		
	}
}


void master_poll(small_modbus_t *smb_master)
{
	int rc = 0;
	int index = 0;
	rt_thread_mdelay(30);
	modbus_error_recovery(smb_master);
	modbus_set_slave(smb_master, 1);
	rc = modbus_read_input_bits(smb_master, 10000, 8, temp_buff); // modbus_read_input_bits
	MODBUS_PRINTF("modbus_read_input_bits:%d\n",rc);
	if(rc >= MODBUS_OK)
	{
		for(index = 0; index <8;index++)
		{
			rt_kprintf("[%d]",dio_get_val(temp_buff,index));
		}
		rt_kputs("\n\r");
		count_ok++;
	}else
	{
		count_err++;
	}

	rt_thread_mdelay(30);
	modbus_error_recovery(smb_master);
	modbus_set_slave(smb_master, 1);
	rc = modbus_write_bits(smb_master, 00000 , 8, temp_buff); // modbus_write_bits
	MODBUS_PRINTF("modbus_write_bits:%d\n",rc);
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
	rc = modbus_read_bits(smb_master, 00000 , 8, temp_buff); // modbus_read_bits
	MODBUS_PRINTF("modbus_read_bits:%d\n",rc);
	if(rc >= MODBUS_OK)
	{
		for(index = 0; index <8;index++)
		{
			rt_kprintf("[%d]",dio_get_val(temp_buff,index));
		}
		rt_kputs("\n\r");
		count_ok++;
	}else
	{
		count_err++;
	}


	rt_thread_mdelay(30);
	modbus_error_recovery(smb_master);
	modbus_set_slave(smb_master, 1);
	rc = modbus_read_input_registers(smb_master, 30000 , 8, (uint16_t*)temp_buff);  // modbus_read_input_registers
	MODBUS_PRINTF("modbus_read_input_registers:%d\n",rc);
	if(rc >= MODBUS_OK)
	{
		for(index = 0; index <8;index++)
		{
			rt_kprintf("[%d]",aio_get_val((uint16_t*)temp_buff,index));
		}
		rt_kputs("\n\r");
		count_ok++;
	}else
	{
		count_err++;
	}

	rt_thread_mdelay(30);
	modbus_error_recovery(smb_master);
	modbus_set_slave(smb_master, 1);
	rc = modbus_write_registers(smb_master, 40000 , 8, (uint16_t*)temp_buff); // modbus_write_registers
	MODBUS_PRINTF("modbus_write_registers:%d\n",rc);
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
	rc = modbus_read_registers(smb_master, 40000 , 8, (uint16_t*)temp_buff); // modbus_read_registers
	MODBUS_PRINTF("modbus_read_registers:%d\n",rc);
	if(rc >= MODBUS_OK)
	{
		for(index = 0; index <8;index++)
		{
			rt_kprintf("[%d]",aio_get_val((uint16_t*)temp_buff,index));
		}
		rt_kputs("\n\r");
		count_ok++;
	}else
	{
		count_err++;
	}
}

static void test_modbus_rtu_master_thread(void *param)
{
	int rc = 0;	
	small_modbus_t *smb_master = param;
	
	rs485_rts_pin = rt_pin_get("PB.1");  //����mcuƽ̨�޸����ź�
	rt_pin_mode(rs485_rts_pin, PIN_MODE_OUTPUT);
	rt_pin_write(rs485_rts_pin, PIN_LOW);
	
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
	
	while(1)
	{
#ifdef MASTER_SEM_POLL
		master_sem_poll(smb_master);  //�����ź���������д
#else
		master_poll(smb_master);  //������ѯ�ӻ�ʾ�����룬�ò����ȽϷ�ʱ
#endif
	}
	//modbus_disconnect(smb_master);
	//���while��û��breakӦ�ò������е�����
}

int test_modbus_rtu_master(void)
{
	rt_thread_t tid;
	
	tid = rt_thread_create("master",test_modbus_rtu_master_thread, &modbus_rtu_master,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

//msh����������
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>

MSH_CMD_EXPORT(test_modbus_rtu_master, test_modbus_rtu_master);

#endif

