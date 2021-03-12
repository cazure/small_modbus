#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"

enum netdevice_staus_type
{
	NETDEVICE_STATUS_NULL = 0,
	NETDEVICE_STATUS_ON = 1,
	NETDEVICE_STATUS_OFF = 2,
};

typedef enum
{
	NETDEVICE_CMD_STATUS_SET =				0x01,
	NETDEVICE_CMD_STATUS_GET =				0x02,
	NETDEVICE_CMD_DHCP_SET =					0x03,
	NETDEVICE_CMD_DHCP_GET =					0x04,
	NETDEVICE_CMD_CALLBACK_SET =			0x05,
	NETDEVICE_CMD_CALLBACK_GET =			0x06,
	NETDEVICE_CMD_PING =							0x07,
	NETDEVICE_CMD_NETSTAT =						0x08,
	
	NETDEVICE_CMD_IPADDR_SET =				0x11,
	NETDEVICE_CMD_IPADDR_GET =				0x12,
	NETDEVICE_CMD_NETMASK_SET =				0x13,
	NETDEVICE_CMD_NETMASK_GET =				0x14,
	NETDEVICE_CMD_GATEWAY_SET =				0x15,
	NETDEVICE_CMD_GATEWAY_GET =				0x16,
	NETDEVICE_CMD_DNSSERVER_SET =			0x17,
	NETDEVICE_CMD_DNSSERVER_GET =			0x18,
}netdevice_cmd_t;

typedef enum
{
	NETSOCKET_PROTOCOL_TCP = 1,
	NETSOCKET_PROTOCOL_UDP,
	NETSOCKET_PROTOCOL_MACRAW,
	NETSOCKET_PROTOCOL_IPRAW,
}netsocket_protocol_t;

typedef enum
{
	NETSOCKET_TYPE_SERVER = 1,
	NETSOCKET_TYPE_CLIENT,
}netsocket_type_t;

typedef enum
{
	NETSOCKET_STATE_INIT = 0,
	NETSOCKET_STATE_ESTABLISHED = 1,
	NETSOCKET_STATE_CLOSED = 2,
	NETSOCKET_STATE_LISTEN = 3,
	NETSOCKET_STATE_DISCONNING = 4,
	NETSOCKET_STATE_CLOSING = 5,
}netsocket_state_t;

typedef enum
{
	NETSOCKET_CMD_PROTOCOL_SET =	0x01,
	NETSOCKET_CMD_PROTOCOL_GET =	0x02,
	NETSOCKET_CMD_TYPE_SET =			0x03,
	NETSOCKET_CMD_TYPE_GET =			0x04,
	
	NETSOCKET_CMD_IP_SET =				0x11,
	NETSOCKET_CMD_IP_GET =				0x12,
	NETSOCKET_CMD_PORT_SET =			0x13,
	NETSOCKET_CMD_PORT_GET =			0x14,
	NETSOCKET_CMD_SIP_SET =				0x15,
	NETSOCKET_CMD_SIP_GET =				0x16,
	NETSOCKET_CMD_SPORT_SET =			0x17,
	NETSOCKET_CMD_SPORT_GET =			0x18,
	
	NETSOCKET_CMD_HOSTNAME_SET =			0x21,
	NETSOCKET_CMD_HOSTNAME_GET =			0x22,
	NETSOCKET_CMD_HOSTPORT_SET =			0x23,
	NETSOCKET_CMD_HOSTPORT_GET =			0x24,
}netsocket_cmd_t;

typedef union
{
   uint32_t u32;
   uint8_t  u8[4];
}ip_addr_t;

static void netdevice_config_init(void)
{
	rt_device_t ch9121_dev;
	rt_device_t socket0_dev;
	int val = 0;
	
	ch9121_dev = rt_device_find("ch9121");
	
	val = NETDEVICE_STATUS_ON;
	rt_device_control(ch9121_dev,NETDEVICE_CMD_DHCP_SET,&val);
	
	rt_device_open(ch9121_dev,0);
	
	socket0_dev = rt_device_find("socket0");
	
	val = NETSOCKET_PROTOCOL_TCP;
	rt_device_control(socket0_dev,NETSOCKET_CMD_PROTOCOL_SET,&val);
	
	val = NETSOCKET_TYPE_SERVER;
	rt_device_control(socket0_dev,NETSOCKET_CMD_TYPE_SET,&val);
	
	ip_addr_t ip;
	ip.u8[0] = 192;
	ip.u8[1] = 168;
	ip.u8[2] = 1;
	ip.u8[3] = 98;
	rt_device_control(socket0_dev,NETSOCKET_CMD_IP_SET,&(ip.u32));
	
	uint16_t port = 4567;
	rt_device_control(socket0_dev,NETSOCKET_CMD_PORT_SET,&port);
	
	port = 1502;
	rt_device_control(socket0_dev,NETSOCKET_CMD_SPORT_SET,&port);
}



#define DO_MASK		0x10000000
#define DI_MASK		0x20000000
#define AO_MASK		0x40000000
#define AI_MASK		0x80000000
static rt_device_t bio_dev = {0};  //test device

static small_modbus_t modbus_slave = {0}; 
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),__VA_ARGS__)

static uint8_t temp_buff[256];

static int test_modbus_rtu_slave_callback(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data)
{
	int rc = 0;
	switch(function_code)
	{
		case MODBUS_FC_READ_HOLDING_COILS:
		{
			rc = rt_device_read(bio_dev,DO_MASK+addr,temp_buff,num);
			rc = modbus_array2bit(read_write_data, temp_buff, rc);
		}break;
		case MODBUS_FC_READ_INPUTS_COILS:
		{
			rc = rt_device_read(bio_dev,DI_MASK+addr,temp_buff,num);
			rc = modbus_array2bit(read_write_data, temp_buff, rc);
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:
		{
			rc = rt_device_read(bio_dev,AO_MASK+addr,temp_buff,num);
			rc = modbus_array2reg(read_write_data, temp_buff, rc);
		}break;
		case MODBUS_FC_READ_INPUT_REGISTERS:
		{
			rc = rt_device_read(bio_dev,AI_MASK+addr,temp_buff,num);
			rc = modbus_array2reg(read_write_data, temp_buff, rc);
		}break;
		
		case MODBUS_FC_WRITE_SINGLE_COIL:
		{
			uint8_t value1 = num?1:0;
			rc = rt_device_write(bio_dev,DO_MASK+addr,&value1,1);
		}break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:
		{
			uint16_t value2 = num;
			rc = rt_device_write(bio_dev,AO_MASK+addr,&value2,1);
		}break;
		
		case MODBUS_FC_WRITE_MULTIPLE_COILS:
		{
			rc = modbus_bit2array(temp_buff,read_write_data,num);
			rc = rt_device_write(bio_dev,DO_MASK+addr,temp_buff,num);
		}break;
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
		{
			rc = modbus_reg2array(temp_buff,read_write_data,num);
			rc = rt_device_write(bio_dev,AO_MASK+addr,temp_buff,num);
		}break;
	}	
	if(rc<0)
	{
		MODBUS_PRINTF("callback fail %d\n",rc);
	}
	return rc;
}

static void test_modbus_rtu_slave_thread(void *param)
{
	int rc = 0;
	int count = 0;
	small_modbus_t *smb_slave = param;
	
	netdevice_config_init();
	
	modbus_init(smb_slave,MODBUS_CORE_RTU,modbus_port_device_create("socket0")); // init modbus  RTU mode
	//modbus_init(smb_slave,MODBUS_CORE_TCP,modbus_port_device_create("socket0")); // init modbus  TCP mode
	
	modbus_set_oflag(&modbus_slave,RT_DEVICE_FLAG_INT_RX);
	
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

int test_modbus_rtu_slave_netdevice(void)
{
	rt_thread_t tid;
	
	bio_dev = rt_device_find("bio");
	rt_device_open(bio_dev,0);
	
	tid = rt_thread_create("slave1",test_modbus_rtu_slave_thread, &modbus_slave,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

