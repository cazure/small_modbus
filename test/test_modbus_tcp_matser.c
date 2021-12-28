#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus.h"
#include "board_virtualIO.h"

static small_modbus_t modbus_tcp_master = {0};
//#define MODBUS_PRINTF(...)
//#define MODBUS_PRINTF(...)   modbus_debug((&modbus_tcp_master),__VA_ARGS__)
#define MODBUS_PRINTF(...) modbus_debug_info((&modbus_tcp_master), __VA_ARGS__)

static uint8_t temp_buff[256];

static void test_modbus_tcp_master_thread(void *param)
{
	int rc = 0;
	small_modbus_t *smb_master = param;

	modbus_init(smb_master, MODBUS_CORE_TCP,
							modbus_port_rtsocket_create(MODBUS_DEVICE_MASTER, "192.168.1.98", "502")); // init modbus  TCP mode

	MODBUS_PRINTF("modbus master\n");

	int count_ok = 0;
	int count_err = 0;
	int index = 0;
	while (1)
	{
		modbus_connect(smb_master);
		MODBUS_PRINTF("modbus connect\n");

		while (modbus_tcp_status(smb_master) == MODBUS_OK) // loop query
		{
			rt_thread_mdelay(30);
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
			rc = modbus_read_input_bits(smb_master, 0, 8, temp_buff); // modbus_read_input_bits
			rt_kprintf("modbus_read_input_bits:%d\n", rc);
			if (rc >= MODBUS_OK)
			{
				for (index = 0; index < 8; index++)
				{
					rt_kprintf("[%d]", dio_get_val(temp_buff, index));
				}
				rt_kputs("\n\r");
				count_ok++;
			}
			else
			{
				count_err++;
				if (rc == MODBUS_ERROR_READ)
				{
					break; // disconnect
				}
			}

			rt_thread_mdelay(30);
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
			rc = modbus_write_bits(smb_master, 0, 8, temp_buff); // modbus_write_bits
			rt_kprintf("modbus_write_bits:%d\n", rc);
			if (rc >= MODBUS_OK)
			{
				count_ok++;
			}
			else
			{
				count_err++;
				if (rc == MODBUS_ERROR_READ)
				{
					break; // disconnect
				}
			}

			rt_thread_mdelay(30);
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
			rc = modbus_read_bits(smb_master, 0, 8, temp_buff); // modbus_read_bits
			rt_kprintf("modbus_read_bits:%d\n", rc);
			if (rc >= MODBUS_OK)
			{
				for (index = 0; index < 8; index++)
				{
					rt_kprintf("[%d]", dio_get_val(temp_buff, index));
				}
				rt_kputs("\n\r");
				count_ok++;
			}
			else
			{
				count_err++;
				if (rc == MODBUS_ERROR_READ)
				{
					break; // disconnect
				}
			}

			rt_thread_mdelay(30);
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
			rc = modbus_read_input_registers(smb_master, 0, 8, (uint16_t *)temp_buff); // modbus_read_input_registers
			rt_kprintf("modbus_read_input_registers:%d\n", rc);
			if (rc >= MODBUS_OK)
			{
				for (index = 0; index < 8; index++)
				{
					rt_kprintf("[%d]", aio_get_val((uint16_t *)temp_buff, index));
				}
				rt_kputs("\n\r");
				count_ok++;
			}
			else
			{
				count_err++;
				if (rc == MODBUS_ERROR_READ)
				{
					break; // disconnect
				}
			}

			rt_thread_mdelay(30);
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
			rc = modbus_write_registers(smb_master, 0, 8, (uint16_t *)temp_buff); // modbus_write_registers
			rt_kprintf("modbus_write_registers:%d\n", rc);
			if (rc >= MODBUS_OK)
			{
				count_ok++;
			}
			else
			{
				count_err++;
				if (rc == MODBUS_ERROR_READ)
				{
					break; // disconnect
				}
			}

			rt_thread_mdelay(30);
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
			rc = modbus_read_registers(smb_master, 0, 8, (uint16_t *)temp_buff); // modbus_read_registers
			rt_kprintf("modbus_read_registers:%d\n", rc);
			if (rc >= MODBUS_OK)
			{
				for (index = 0; index < 8; index++)
				{
					rt_kprintf("[%d]", aio_get_val((uint16_t *)temp_buff, index));
				}
				rt_kputs("\n\r");
				count_ok++;
			}
			else
			{
				count_err++;
				if (rc == MODBUS_ERROR_READ)
				{
					break; // disconnect
				}
			}
			rt_thread_mdelay(3000);
		} // loop query

		MODBUS_PRINTF("modbus disconnect\n");
		modbus_disconnect(smb_master); // disconnect
	}
}

int test_modbus_tcp_master(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("master", test_modbus_tcp_master_thread, &modbus_tcp_master, 2048, 20, 10);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	return 0;
}

// mshÃüÁîÐÐÆô¶¯
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>

MSH_CMD_EXPORT(test_modbus_tcp_master, test modbus_tcp_master);

#endif
