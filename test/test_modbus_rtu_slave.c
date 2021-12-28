#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus.h"
#include "board_virtualIO.h"

//�ӻ��ص�����,���ӻ����յ�����������(����У��͵�ַ�������Ѿ�������),������ص�������������ݣ��������ݵĳ��ȼ���
static int test_modbus_rtu_slave_callback(small_modbus_t *smb, int function_code, int addr, int num, void *read_write_data)
{
	int rc = 0;
	switch (function_code)
	{
	case MODBUS_FC_READ_HOLDING_COILS: //��ȡ������Ȧ,1bit����һ����Ȧ
	{
		if ((0 <= addr) && (addr < 10000)) //��ַӳ�䣬��ַ��0��ʼ
		{
			rc = vio_read_hold_coils(addr, num, read_write_data);
		}
	}
	break;
	case MODBUS_FC_READ_INPUTS_COILS: //��ȡֻ����Ȧ,1bit����һ����Ȧ
	{
		if ((10000 <= addr) && (addr < 20000)) //��ַӳ�䣬��ַ��10000��ʼ
		{
			addr = addr - 10000;
			rc = vio_read_input_coils(addr, num, read_write_data);
		}
	}
	break;
	case MODBUS_FC_READ_HOLDING_REGISTERS: //��ȡ���ּĴ���,16bit����һ���Ĵ���
	{
		if ((40000 <= addr) && (addr < 50000)) //��ַӳ�䣬��ַ��40000��ʼ
		{
			addr = addr - 40000;
			rc = vio_read_hold_regs(addr, num, read_write_data);
		}
	}
	break;
	case MODBUS_FC_READ_INPUT_REGISTERS: //��ȡ����Ĵ���,16bit����һ���Ĵ���
	{
		if ((30000 <= addr) && (addr < 40000)) //��ַӳ�䣬��ַ��30000��ʼ
		{
			addr = addr - 30000;
			rc = vio_read_input_regs(addr, num, read_write_data);
		}
	}
	break;
	case MODBUS_FC_WRITE_SINGLE_COIL:		 //д������Ȧ,1bit����һ����Ȧ
	case MODBUS_FC_WRITE_MULTIPLE_COILS: //д��Ȧ,1bit����һ����Ȧ
	{
		if ((0 <= addr) && (addr < 10000)) //��ַӳ�䣬��ַ��0��ʼ
		{
			rc = vio_write_hold_coils(addr, num, read_write_data);
		}
	}
	break;
	case MODBUS_FC_WRITE_SINGLE_REGISTER:		 //д�����Ĵ���,16bit����һ���Ĵ���
	case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: //д�Ĵ���,16bit����һ���Ĵ���
	{
		if ((40000 <= addr) && (addr < 50000)) //��ַӳ�䣬��ַ��40000��ʼ
		{
			addr = addr - 40000;
			rc = vio_write_hold_regs(addr, num, read_write_data);
		}
	}
	break;
	}
	if (rc < 0)
	{
		// MODBUS_PRINTF("callback fail %d\n",rc);
	}
	return rc;
}

static small_modbus_t modbus_rtu_slave = {0};
//#define MODBUS_PRINTF(...)
#define MODBUS_PRINTF(...) modbus_debug((&modbus_rtu_slave), __VA_ARGS__)

// rtthread device name
#define UART_DEVICE_NAME "uart1"

// rtthread pin index
static int rs485_rts_pin = 0;

//�շ��������Żص�����
static int uart_rts(int on)
{
	if (on)
	{
		rt_pin_write(rs485_rts_pin, PIN_HIGH);
		rt_thread_mdelay(2); // 9600 bps 3.5 ���ַ��ӳ�ʱ��
	}
	else
	{
		rt_thread_mdelay(2); // 9600 bps 3.5 ���ַ��ӳ�ʱ��
		rt_pin_write(rs485_rts_pin, PIN_LOW);
	}
	return 0;
}

static void test_modbus_rtu_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;

	rs485_rts_pin = rt_pin_get("PB.0"); //����mcuƽ̨�޸����ź�
	rt_pin_mode(rs485_rts_pin, PIN_MODE_OUTPUT);
	rt_pin_write(rs485_rts_pin, PIN_LOW);

	modbus_init(smb_slave, MODBUS_CORE_RTU, modbus_port_rtdevice_create(UART_DEVICE_NAME)); // init modbus  RTU mode

	struct serial_configure serial_config;
	serial_config.baud_rate = BAUD_RATE_9600;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_SERIAL_RB_BUFSZ;
	serial_config.parity = PARITY_NONE;
	modbus_rtu_set_serial_config(smb_slave, &serial_config); // config serial

	modbus_rtu_set_serial_rts(smb_slave, uart_rts); // set serial rts callback

	modbus_rtu_set_oflag(smb_slave, RT_DEVICE_FLAG_INT_RX);
	// modbus_rtu_set_oflag(smb_slave,RT_DEVICE_FLAG_DMA_RX);

	modbus_set_slave(smb_slave, 1); // set slave addr

	modbus_connect(smb_slave);
	rt_kprintf("modbus slave addr:%d\n", 1);

	while (1)
	{
		rc = modbus_slave_wait_handle(smb_slave, test_modbus_rtu_slave_callback, MODBUS_WAIT_FOREVER);
		if (rc > 0)
		{
			count++;
		}
		else
		{
			modbus_error_recovery(smb_slave);
		}
	}
	// modbus_disconnect(smb_slave);
	//���while��û��breakӦ�ò������е�����
}

int test_modbus_rtu_slave(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("slave", test_modbus_rtu_slave_thread, &modbus_rtu_slave, 2048, 10, 10);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	return 0;
}

// msh����������
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>

MSH_CMD_EXPORT(test_modbus_rtu_slave, test modbus_rtu_slave);

#endif
