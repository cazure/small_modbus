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
	
	val = NETSOCKET_TYPE_SERVER;  //�����
	rt_device_control(socket_dev,NETSOCKET_CMD_TYPE_SET,&val);
	
	ip_addr_t ip;
	ip.u8[0] = 192;
	ip.u8[1] = 168;
	ip.u8[2] = 1;
	ip.u8[3] = 98;
	rt_device_control(socket_dev,NETSOCKET_CMD_IP_SET,&(ip.u32));
	
	uint16_t port = 4567;
	rt_device_control(socket_dev,NETSOCKET_CMD_PORT_SET,&port);
	
	port = 1502;
	rt_device_control(socket_dev,NETSOCKET_CMD_SPORT_SET,&port);
}


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
				rc = vio_read(VIO_DO,addr,num,read_write_data);
			}else
			if((20000 <= addr)&&(addr < 30000))	//��ַӳ�䣬��ַ��20000��ʼ
			{
				addr = addr - 20000;
				rc = vio_read(VIO_DM,addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_READ_INPUTS_COILS:	//��ȡֻ����Ȧ,1bit����һ����Ȧ
		{
			if((10000 <= addr)&&(addr < 20000)) //��ַӳ�䣬��ַ��10000��ʼ
			{
				addr = addr - 10000;
				rc = vio_read(VIO_DI,addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:	//��ȡ���ּĴ���,16bit����һ���Ĵ���
		{
			if((40000 <= addr)&&(addr < 50000)) //��ַӳ�䣬��ַ��40000��ʼ
			{
				addr = addr - 40000;
				rc = vio_read(VIO_AO,addr,num,read_write_data);
			}else
			if((50000 <= addr)&&(addr < 60000)) //��ַӳ�䣬��ַ��50000��ʼ
			{
				addr = addr - 50000;
				rc = vio_read(VIO_AM,addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_READ_INPUT_REGISTERS:	//��ȡ����Ĵ���,16bit����һ���Ĵ���
		{
			if((30000 <= addr)&&(addr < 40000)) //��ַӳ�䣬��ַ��30000��ʼ
			{
				addr = addr - 30000;
				rc = vio_read(VIO_AI,addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_COIL:	//д������Ȧ,1bit����һ����Ȧ
		case MODBUS_FC_WRITE_MULTIPLE_COILS:		//д��Ȧ,1bit����һ����Ȧ
		{
			if((0 <= addr)&&(addr < 10000))	//��ַӳ�䣬��ַ��0��ʼ
			{
				rc = vio_write(VIO_DO,addr,num,read_write_data);
			}else
			if((20000 <= addr)&&(addr < 30000))	//��ַӳ�䣬��ַ��20000��ʼ
			{
				addr = addr - 20000;
				rc = vio_write(VIO_DM,addr,num,read_write_data);
			}
		}break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:	//д�����Ĵ���,16bit����һ���Ĵ���
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:	//д�Ĵ���,16bit����һ���Ĵ���
		{	
			if((40000 <= addr)&&(addr < 50000))	//��ַӳ�䣬��ַ��40000��ʼ
			{
				addr = addr - 40000;
				rc = vio_write(VIO_AO,addr,num,read_write_data);
			}else
			if((50000 <= addr)&&(addr < 60000))	//��ַӳ�䣬��ַ��50000��ʼ
			{
				addr = addr - 50000;
				rc = vio_write(VIO_AM,addr,num,read_write_data);
			}
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
	
	modbus_init(smb_slave,MODBUS_CORE_RTU,modbus_port_device_create(SOCKET_NAME)); // init modbus  RTU mode
	//modbus_init(smb_slave,MODBUS_CORE_TCP,modbus_port_device_create(SOCKET_NAME)); // init modbus  TCP mode
	
	modbus_set_oflag(&modbus_slave,RT_DEVICE_FLAG_INT_RX);
	
	modbus_set_slave(smb_slave,1); //set slave addr
	
	while(1)
	{
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
				break;
			}
		}
		modbus_disconnect(smb_slave);
	}
}

int test_modbus_rtu_slave_netdevice(void)
{
	rt_thread_t tid;
	
	tid = rt_thread_create("slave1",test_modbus_rtu_slave_thread, &modbus_slave,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}

