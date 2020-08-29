#include "stdio.h"
#include "string.h"
#include "modbus_tcp_rtos.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_common.h"
#include "stdio.h"
#include "string.h"

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#endif

#include "controller.h"

#define DBG_TAG "mbtcp"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define delay_ms        rt_thread_mdelay

small_modbus_mapping_t modbus_mapping = {0};

small_modbus_t modbus_slave = {0};
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),2,__VA_ARGS__)

#define MAX_CLIENT_NUM  3
#define CLIENT_TIMEOUT  10      //单位 s

typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;

modbus_mapping_t mb_mapping ={0};
client_session_t client_session[MAX_CLIENT_NUM];
uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
fd_set readset;

static void modbus_tcp_thread(void *param)
{
    int server_fd = -1;
    modbus_t *ctx = NULL;
//    modbus_mapping_t *mb_mapping = NULL;
    int max_fd = -1;
    int rc;
    struct timeval select_timeout;

    rt_thread_mdelay(3000);

    for (int i = 0; i < MAX_CLIENT_NUM; i++)
    {
        client_session[i].fd = -1;
        client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
    }

    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;

    ctx = modbus_new_tcp(RT_NULL, 1502, AF_INET);
    RT_ASSERT(ctx != RT_NULL);
//    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
//                                    MODBUS_MAX_READ_REGISTERS, 0);
//    RT_ASSERT(mb_mapping != RT_NULL);
    mb_mapping.start_bits = con.io.DO.start;
    mb_mapping.nb_bits =   con.io.DO.num;
    mb_mapping.tab_bits = con.io.DO.array;

    mb_mapping.start_input_bits = con.io.DI.start;
    mb_mapping.nb_input_bits = con.io.DI.num;
    mb_mapping.tab_input_bits = con.io.DI.array;

    mb_mapping.start_registers = con.io.AO.start;
    mb_mapping.nb_registers = con.io.AO.num;
    mb_mapping.tab_registers = con.io.AO.array;

    mb_mapping.start_input_registers = con.io.AI.start;
    mb_mapping.nb_input_registers = con.io.AI.num;
    mb_mapping.tab_input_registers = con.io.AI.array;

_mbtcp_start:
    server_fd = modbus_tcp_listen(ctx, 1);
    if (server_fd < 0)
        goto _mbtcp_restart;

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

        rc = select(max_fd + 1, &readset, RT_NULL, RT_NULL, &select_timeout);
        if(rc < 0)
        {
            goto _mbtcp_restart;
        }
        else if(rc > 0)
        {
            if(FD_ISSET(server_fd, &readset))
            {
                int client_sock_fd = modbus_tcp_accept(ctx, &server_fd);
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
                        modbus_set_socket(ctx, client_session[i].fd);

                        rc = modbus_receive(ctx, query);
                        if (rc > 0)
                        {
                            rc = modbus_reply(ctx, query, rc, &mb_mapping);
                            if(rc < 0)
                            {
                                close(client_session[i].fd);
                                client_session[i].fd = -1;
                            }
                            else
                            {
                                client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                            }
                        }
                        else
                        {
                            close(client_session[i].fd);
                            client_session[i].fd = -1;
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
    if(server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }

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

    modbus_free(ctx);
}


//extern void * _stack_mtcp;

int app_modbus_tcp_start(void)
{
    LOG_I("init,mac:%s,ip:%s",dev_get_mac(),dev_get_ip());

//    static struct rt_thread thread_mbtcp;
//    rt_thread_init(&thread_mbtcp,"mbtcp", modbus_tcp_thread,NULL, _stack_mtcp,4*1024, 5, 10);
//    rt_thread_startup(&thread_mbtcp);

    rt_thread_t tid;
    tid = rt_thread_create("mbtcp",modbus_tcp_thread, RT_NULL,2048,12, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
//INIT_APP_EXPORT(modbus_tcp_init);




#define DBG_ENABLE
#define DBG_SECTION_NAME    "modbus"
#define DBG_LEVEL           DBG_LOG//DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define delay_ms        rt_thread_mdelay

small_modbus_mapping_t modbus_mapping = {0};

small_modbus_t modbus_slave = {0};
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),2,__VA_ARGS__)

void modbus_slave_thread(void *param)
{
    int rc = 0;
    uint8_t receive_buf[128];
    modbus_rtu_init(&modbus_slave,NULL,&uart6_config);
    modbus_set_slave(&modbus_slave,6);
    modbus_connect(&modbus_slave);
    while (1)
    {
            rc = modbus_wait_poll(&modbus_slave, receive_buf);
			if (rc > 0)
			{
			    MODBUS_PRINTF("\n[1]------ receive %d --------\n",rc);
				rc = modbus_handle_poll(&modbus_slave,receive_buf, rc,&modbus_mapping);
			}else
			{
			    MODBUS_PRINTF("\n[1]------ receive failed ---------\n");
				delay_ms(10);
				modbus_error_recovery(&modbus_slave);
			}
    }
    modbus_disconnect(&modbus_slave);
}

small_modbus_t modbus_master = {0};
#undef MODBUS_PRINTF
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_master),2,__VA_ARGS__)

void modbus_master_thread(void *param)
{
    uint16_t tab_reg[64] = {0};
    modbus_rtu_init(&modbus_master,NULL,&uart3_config);
    modbus_set_slave(&modbus_master,6);
    modbus_connect(&modbus_master);
    int regs =0 ,num = 0;
    while (1)
    {
        memset(tab_reg, 0, 64 * 2);
        modbus_set_slave(&modbus_master,6);
        regs = modbus_read_registers(&modbus_master, 3, 0x02, tab_reg);
        if(regs>0)
        {
            MODBUS_PRINTF("[%4d][read num = %d]\n", num, regs);
            int i;
            for (i = 0; i < 10; i++)
            {
              MODBUS_PRINTF("<%#x>", tab_reg[i]);
            }
            MODBUS_PRINTF("\n");

            delay_ms(50);
            num++;
            MODBUS_PRINTF("\n[2]------ write register [0x08]:%d -------\n",num);
            modbus_write_register(&modbus_master,0x08,num);

        }else
        {
            MODBUS_PRINTF("\n[2]------ receive failed -------\n");
            delay_ms(10);
            modbus_error_recovery(&modbus_master);
        }
        delay_ms(6000);
    }
    modbus_disconnect(&modbus_master);
}


int app_modbus_init(void)
{
    rt_thread_t tid3,tid6;
    uint8_t type = 0;
    LOG_I("init %d",type);

    modbus_mapping_init(modbus_mapping,modbus_status_callback,0,64,0,64,0,64,0,64);

    tid3 = rt_thread_create("modbus S",modbus_slave_thread, RT_NULL,2048,20, 10);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    tid6 = rt_thread_create("modbus M",modbus_master_thread, RT_NULL,2048,20, 10);
    if (tid6 != RT_NULL)
        rt_thread_startup(tid6);
    return 0;
}
MSH_CMD_EXPORT(app_modbus_init,modbus test)
