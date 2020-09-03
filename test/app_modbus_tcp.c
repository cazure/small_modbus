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
#define MAX_CLIENT_NUM  3
#define CLIENT_TIMEOUT  10      //单位 s

typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;

static client_session_t client_session[MAX_CLIENT_NUM];

static small_modbus_mapping_t modbus_tcp_mapping = {0};
static small_modbus_t modbus_tcp_slave = {0};
static modbus_tcp_config_t modbus_config = {0};
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_tcp_slave),2,__VA_ARGS__)

static int modbus_tcp_status_callback(small_modbus_mapping_t *mapping,int read_write,int data_type,int start,int num)
{

    return MODBUS_OK;
}

static void modbus_tcp_slave_thread(void *param)
{
    int server_fd = -1;
    int max_fd = -1;
    int rc;
    fd_set readset;
    struct timeval select_timeout;

    rt_thread_mdelay(3000);
    controller_t * con = &controller;

    //modbus_mapping_new(modbus_mapping,modbus_status_callback,0,64,0,64,0,64,0,64);
    modbus_mapping_init(modbus_tcp_mapping,modbus_tcp_status_callback,
            con->io.DO.start,con->io.DO.num,con->io.DO.array,
            con->io.DI.start,con->io.DI.num,con->io.DI.array,
            con->io.AO.start,con->io.AO.num,con->io.AO.array,
            con->io.AI.start,con->io.AI.num,con->io.AI.array);

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

int modbus_tcp_test(void)
{
    LOG_I("init,mac:%s,ip:%s",dev_get_mac(),dev_get_ip());

//    static struct rt_thread thread_mbtcp;
//    rt_thread_init(&thread_mbtcp,"mbtcp", modbus_tcp_thread,NULL, _stack_mtcp,4*1024, 5, 10);
//    rt_thread_startup(&thread_mbtcp);

    rt_thread_t tid;
    tid = rt_thread_create("mbtcp",modbus_tcp_slave_thread, RT_NULL,2048,12, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
MSH_CMD_EXPORT(modbus_tcp_test,modbus tcp test)
