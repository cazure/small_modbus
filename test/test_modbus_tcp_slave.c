#include "stdio.h"
#include "string.h"
#include "board.h"
#include <rtthread.h>
#include "small_modbus.h"
#include "board_virtualIO.h"

//从机回调函数,当从机接收到主机的请求(数据校验和地址功能码已经解析完),在这个回调函数内填充数据，返回数据的长度即可
static int test_modbus_tcp_slave_callback(small_modbus_t *smb, int function_code, int addr, int num, void *read_write_data)
{
	int rc = 0;
	switch (function_code)
	{
	case MODBUS_FC_READ_HOLDING_COILS: //读取保持线圈,1bit代表一个线圈
	{
		if ((0 <= addr) && (addr < 10000)) //地址映射，地址从0开始
		{
			rc = vio_read_hold_coils(addr, num, read_write_data); // vio操作
		}
	}
	break;
	case MODBUS_FC_READ_INPUTS_COILS: //读取只读线圈,1bit代表一个线圈
	{
		if ((10000 <= addr) && (addr < 20000)) //地址映射，地址从10000开始
		{
			addr = addr - 10000;
			rc = vio_read_input_coils(addr, num, read_write_data); // vio操作
		}
	}
	break;
	case MODBUS_FC_READ_HOLDING_REGISTERS: //读取保持寄存器,16bit代表一个寄存器
	{
		if ((40000 <= addr) && (addr < 50000)) //地址映射，地址从40000开始
		{
			addr = addr - 40000;
			rc = vio_read_hold_regs(addr, num, read_write_data); // vio操作
		}
	}
	break;
	case MODBUS_FC_READ_INPUT_REGISTERS: //读取输入寄存器,16bit代表一个寄存器
	{
		if ((30000 <= addr) && (addr < 40000)) //地址映射，地址从30000开始
		{
			addr = addr - 30000;
			rc = vio_read_input_regs(addr, num, read_write_data); // vio操作
		}
	}
	break;
	case MODBUS_FC_WRITE_SINGLE_COIL:		 //写单个线圈,1bit代表一个线圈
	case MODBUS_FC_WRITE_MULTIPLE_COILS: //写线圈,1bit代表一个线圈
	{
		if ((0 <= addr) && (addr < 10000)) //地址映射，地址从0开始
		{
			rc = vio_write_hold_coils(addr, num, read_write_data); // vio操作
		}
	}
	break;
	case MODBUS_FC_WRITE_SINGLE_REGISTER:		 //写单个寄存器,16bit代表一个寄存器
	case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: //写寄存器,16bit代表一个寄存器
	{
		if ((40000 <= addr) && (addr < 50000)) //地址映射，地址从40000开始
		{
			addr = addr - 40000;
			rc = vio_write_hold_regs(addr, num, read_write_data); // vio操作
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

static small_modbus_t modbus_tcp_slave = {0};
//#define MODBUS_PRINTF(...)
//#define MODBUS_PRINTF(...)   modbus_debug((&modbus_tcp_slave),__VA_ARGS__)
#define MODBUS_PRINTF(...) modbus_debug_info((&modbus_tcp_slave), __VA_ARGS__)

//是否使用多路socket,多路socket需要posix select支持
#define MODBUS_TCP_SLAVE_MULTIPLEWAY_SOCKET

#ifndef MODBUS_TCP_SLAVE_MULTIPLEWAY_SOCKET

static void test_modbus_tcp_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;

	modbus_init(smb_slave, MODBUS_CORE_TCP,
							modbus_port_rtsocket_create(MODBUS_DEVICE_SLAVE, "0.0.0.0", "502")); // init modbus  TCP mode

	modbus_set_slave(smb_slave, 1); // set slave addr

	rt_kprintf("modbus slave addr:%d\n", 1);

	int server_socket = -1;
	int client_socket = -1;
	while (1)
	{
		server_socket = modbus_tcp_listen(smb_slave, 1); //
		MODBUS_PRINTF("modbus_tcp_listen:%d\n", server_socket);
		while (1)
		{
			client_socket = modbus_tcp_accept(smb_slave, server_socket);
			MODBUS_PRINTF("modbus_tcp_accept:%d\n", client_socket);

			modbus_tcp_set_socket(smb_slave, client_socket); // set client_socket
			while (modbus_tcp_status(smb_slave) == MODBUS_OK)
			{
				rc = modbus_slave_wait_handle(smb_slave, test_modbus_tcp_slave_callback, MODBUS_WAIT_FOREVER);
				if (rc > 0)
				{
					count++;
				}
				else
				{
					if (rc == MODBUS_ERROR_READ)
					{
						break; // disconnect
					}
					modbus_error_recovery(smb_slave);
				}
			}
			MODBUS_PRINTF("modbus_disconnect client :%d\n", client_socket);
			modbus_tcp_set_socket(smb_slave, client_socket); // set client_socket
			modbus_tcp_disconnect(smb_slave);								 // disconnect client_socket
			client_socket = -1;
		}

		MODBUS_PRINTF("modbus_disconnect server :%d\n", server_socket);
		modbus_tcp_set_socket(smb_slave, server_socket); // set server_socket
		modbus_tcp_disconnect(smb_slave);								 // disconnect server_socket
		server_socket = -1;
	}
}

#elseif

#ifdef RT_USING_POSIX

#include <dfs_posix.h>
#include <dfs_poll.h>
#include <dfs_select.h>

#include <sys/time.h>
#include <sal_socket.h>

//最大连接数量
#define MAX_CLIENT_NUM 3

static void test_modbus_tcp_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;

	modbus_init(smb_slave, MODBUS_CORE_TCP,
							modbus_port_rtsocket_create(MODBUS_DEVICE_SLAVE, "0.0.0.0", "502")); // init modbus  TCP mode

	modbus_set_slave(smb_slave, 1); // set slave addr

	rt_kprintf("modbus slave addr:%d\n", 1);

	int max_fd = -1;
	int server_socket = -1;
	int client_socket[MAX_CLIENT_NUM] = {-1};

	fd_set readset;
	struct timeval select_timeout;
	select_timeout.tv_sec = 1;
	select_timeout.tv_usec = 0;

	for (int i = 0; i < MAX_CLIENT_NUM; i++)
	{
		client_socket[i] = -1;
	}
	while (1)
	{
		server_socket = modbus_tcp_listen(smb_slave, MAX_CLIENT_NUM); //
		MODBUS_PRINTF("modbus_tcp_listen:%d\n", server_socket);
		while (1)
		{
			max_fd = -1;
			FD_ZERO(&readset);
			FD_SET(server_socket, &readset);

			if (max_fd < server_socket)
			{
				max_fd = server_socket;
			}

			for (int i = 0; i < MAX_CLIENT_NUM; i++)
			{
				if (client_socket[i] >= 0)
				{
					FD_SET(client_socket[i], &readset);
					if (max_fd < client_socket[i])
						max_fd = client_socket[i];
				}
			}

			rc = select(max_fd + 1, &readset, RT_NULL, RT_NULL, &select_timeout);
			if (rc < 0)
			{
				MODBUS_PRINTF("modbus_tcp_select:%d\n", rc);
				// goto _mbtcp_restart;
				break;
			}
			else if (rc > 0)
			{
				if (FD_ISSET(server_socket, &readset))
				{
					int client_sock_fd = modbus_tcp_accept(smb_slave, server_socket);
					MODBUS_PRINTF("modbus_tcp_accept:%d\n", client_sock_fd);
					if (client_sock_fd >= 0)
					{
						int index = -1;
						for (int i = 0; i < MAX_CLIENT_NUM; i++)
						{
							if (client_socket[i] < 0)
							{
								index = i;
								break;
							}
						}
						if (index >= 0)
						{
							client_socket[index] = client_sock_fd;
						}
						else
						{
							MODBUS_PRINTF("modbus client max :%d close:%d\n", MAX_CLIENT_NUM, client_sock_fd);
							modbus_tcp_set_socket(smb_slave, client_sock_fd); // set server_socket
							modbus_tcp_disconnect(smb_slave);									// disconnect server_socket
						}
					}
				}
				for (int i = 0; i < MAX_CLIENT_NUM; i++)
				{
					if (client_socket[i] >= 0)
					{
						if (FD_ISSET(client_socket[i], &readset))
						{
							modbus_tcp_set_socket(smb_slave, client_socket[i]);

							rc = modbus_slave_wait_handle(smb_slave, test_modbus_tcp_slave_callback, MODBUS_WAIT_FOREVER);
							if (rc > 0)
							{
								count++;
							}
							else
							{
								if (rc == MODBUS_ERROR_READ)
								{
									MODBUS_PRINTF("modbus_disconnect client :%d\n", client_socket[i]);
									modbus_tcp_set_socket(smb_slave, client_socket[i]); // set client_socket
									modbus_tcp_disconnect(smb_slave);										// disconnect client_socket
									client_socket[i] = -1;
								}
								modbus_error_recovery(smb_slave);
							}
						}
					}
				} // for
			}
		} // while

		for (int i = 0; i < MAX_CLIENT_NUM; i++)
		{
			if (client_socket[i] >= 0)
			{
				MODBUS_PRINTF("modbus_disconnect client :%d\n", client_socket[i]);
				modbus_tcp_set_socket(smb_slave, client_socket[i]); // set server_socket
				modbus_tcp_disconnect(smb_slave);										// disconnect server_socket
				client_socket[i] = -1;
			}
		}
		MODBUS_PRINTF("modbus_disconnect server :%d\n", server_socket);
		modbus_tcp_set_socket(smb_slave, server_socket); // set server_socket
		modbus_tcp_disconnect(smb_slave);								 // disconnect server_socket
		server_socket = -1;
	}
}

#endif

#endif

int test_modbus_tcp_slave(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("slave", test_modbus_tcp_slave_thread, &modbus_tcp_slave, 2048, 20, 10);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	return 0;
}

// msh命令行启动
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>

MSH_CMD_EXPORT(test_modbus_tcp_slave, test modbus_tcp_slave);

#endif
