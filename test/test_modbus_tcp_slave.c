#include "stdio.h"
#include "string.h"
#include "board.h"
#include "small_modbus_rtthread.h"


#define delay_ms        rt_thread_mdelay
#define MAX_CLIENT_NUM  3
#define CLIENT_TIMEOUT  10      //单位 s

typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;

static client_session_t client_session[MAX_CLIENT_NUM];


#define DO_MASK		0x10000000
#define DI_MASK		0x20000000
#define AO_MASK		0x40000000
#define AI_MASK		0x80000000
static rt_device_t bio_dev = {0};  //test device

static small_modbus_t modbus_tcp_slave = {0}; 
//#define MODBUS_PRINTF(...) 
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_tcp_slave),__VA_ARGS__)

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

static void test_modbus_tcp_slave_thread(void *param)
{
    int server_fd = -1;
    int max_fd = -1;
    int rc;
    fd_set readset;
    struct timeval select_timeout;

    rt_thread_mdelay(3000);

    for (int i = 0; i < MAX_CLIENT_NUM; i++)
    {
        client_session[i].fd = -1;
        client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
    }

    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;

    modbus_tcp_init(&modbus_tcp_slave,NULL,&modbus_config);
    modbus_tcp_config(&modbus_tcp_slave, 1,"0.0.0.0", 502);
    modbus_set_slave(&modbus_tcp_slave, 1);
_mbtcp_start:
    modbus_connect(&modbus_tcp_slave);

    server_fd = modbus_config.socket;
    while (1)
    {
        max_fd = -1;
        FD_ZERO(&readset);
        FD_SET(server_fd, &readset);

        if(max_fd < server_fd)
            max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENT_NUM; i++)
        {
            if(client_session[i].fd >= 0)
            {
                FD_SET(client_session[i].fd, &readset);
                if(max_fd < client_session[i].fd)
                    max_fd = client_session[i].fd;
            }
        }

        rc = modbus_tcp_select(&modbus_tcp_slave,max_fd + 1, &readset, RT_NULL, RT_NULL, &select_timeout);
        if(rc < 0)
        {
            goto _mbtcp_restart;
        }
        else if(rc > 0)
        {
            if(FD_ISSET(server_fd, &readset))
            {
                int client_sock_fd = modbus_tcp_accept(&modbus_tcp_slave);
                if(client_sock_fd >= 0)
                {
                    int index = -1;
                    for (int i = 0; i < MAX_CLIENT_NUM; i++)
                    {
                        if(client_session[i].fd < 0)
                        {
                            index = i;
                            break;
                        }
                    }
                    if(index >= 0)
                    {
                        client_session[index].fd = client_sock_fd;
                        client_session[index].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                    }
                    else
                    {
                        close(client_sock_fd);
                    }
                }
            }

            for (int i = 0; i < MAX_CLIENT_NUM; i++)
            {
                if(client_session[i].fd >= 0)
                {
                    if(FD_ISSET(client_session[i].fd, &readset))
                    {
                        modbus_tcp_set_socket(&modbus_tcp_slave, client_session[i].fd);
                        rc = modbus_wait(&modbus_tcp_slave, &modbus_tcp_mapping);
                        if(rc < 0)
                        {
                            rt_kprintf("modbus_wait:%d\n",rc);
                            close(client_session[i].fd);
                            client_session[i].fd = -1;
                        }
                        else
                        {
                            client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                        }
                    }
                }
            }
        }

        // 客户端超时未收到数据断开
        for(int i =0;i<MAX_CLIENT_NUM;i++)
        {
            if(client_session[i].fd >= 0)
            {
                //超时
                if((rt_tick_get() - client_session[i].tick_timeout) < (RT_TICK_MAX / 2))
                {
                    close(client_session[i].fd);
                    client_session[i].fd = -1;
                }
            }
        }
    }

_mbtcp_restart:
    modbus_disconnect(&modbus_tcp_slave);

    for(int i =0;i<MAX_CLIENT_NUM;i++)
    {
        if(client_session[i].fd >= 0)
        {
            close(client_session[i].fd);
            client_session[i].fd = -1;
        }
    }
    rt_thread_mdelay(5000);
    goto _mbtcp_start;


}

int test_modbus_tcp_slave(void)
{
	rt_thread_t tid;
	
	bio_dev = rt_device_find("bio");
	rt_device_open(bio_dev,0);
	
	tid = rt_thread_create("slave",test_modbus_tcp_slave_thread, &modbus_tcp_slave,2048,20, 10);
	if (tid != RT_NULL)
			rt_thread_startup(tid);
	return 0;
}
