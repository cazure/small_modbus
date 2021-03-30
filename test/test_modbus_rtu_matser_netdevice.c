#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"

#define SOCKET_NAME "socket1"

static void netdevice_config_init(void)
{
	rt_device_t socket_dev;
	int val = 0;
	
	socket_dev = rt_device_find(SOCKET_NAME);
	
	val = NETSOCKET_PROTOCOL_TCP;
	rt_device_control(socket_dev,NETSOCKET_CMD_PROTOCOL_SET,&val);
	
	val = NETSOCKET_TYPE_CLIENT;  //客户端模式
	rt_device_control(socket_dev,NETSOCKET_CMD_TYPE_SET,&val);
	
	//目标IP
	rt_device_control(socket_dev,NETSOCKET_CMD_HOSTNAME_SET,"192.168.1.98");
	//目标端口
	rt_device_control(socket_dev,NETSOCKET_CMD_PORTNAME_SET,"1502");
	
//	ip_addr_t ip;
//	ip.u8[0] = 192;
//	ip.u8[1] = 168;
//	ip.u8[2] = 1;
//	ip.u8[3] = 98; 
//	rt_device_control(socket0_dev,NETSOCKET_CMD_IP_SET,&(ip.u32));
//	
//	uint16_t port = 4567;
//	rt_device_control(socket0_dev,NETSOCKET_CMD_PORT_SET,&port);
//	
//	port = 1502;
//	rt_device_control(socket0_dev,NETSOCKET_CMD_SPORT_SET,&port);
}

static small_modbus_t modbus_master = {0};
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_master),__VA_ARGS__)

static int uart_rts(int on)
{
	board_uart_dir(4,on);
	return 0;
}

static uint8_t temp_buff[256];

static void test_modbus_rtu_master_thread(void *param)
{
	int rc = 0;	
	small_modbus_t *smb_master = param;
	
	netdevice_config_init();
	
	modbus_init(smb_master,MODBUS_CORE_RTU,modbus_port_device_create(SOCKET_NAME)); // init modbus  RTU mode
	//modbus_init(smb_master,MODBUS_CORE_TCP,modbus_port_device_create(SOCKET_NAME)); // init modbus  TCP mode
	
	modbus_set_oflag(smb_master,RT_DEVICE_FLAG_INT_RX);
	//modbus_set_oflag(smb_master,RT_DEVICE_FLAG_DMA_RX);
	
	modbus_connect(smb_master);
	rt_kprintf("modbus master\n");
	
	int count_ok = 0;
	int count_err = 0;
	int index = 0;
	while (1)
	{
		rt_thread_mdelay(30);
		modbus_error_recovery(smb_master);
		modbus_set_slave(smb_master, 1);
		rc = modbus_read_input_bits(smb_master, 10000, 8, temp_buff); // modbus_read_input_bits
		rt_kprintf("modbus_read_input_bits:%d\n",rc);
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
		rc = modbus_read_bits(smb_master, 00000 , 8, temp_buff); // modbus_read_bits
		rt_kprintf("modbus_read_bits:%d\n",rc);
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
		rt_kprintf("modbus_read_input_registers:%d\n",rc);
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
		rc = modbus_read_registers(smb_master, 40000 , 8, (uint16_t*)temp_buff); // modbus_read_registers
		rt_kprintf("modbus_read_registers:%d\n",rc);
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
		
		rt_thread_mdelay(3000);
	}
	//modbus_disconnect(&modbus_slave);
}

int test_modbus_rtu_master_netdevice(void)
{
	rt_thread_t tid;
	
	tid = rt_thread_create("master1",test_modbus_rtu_master_thread, &modbus_master,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

