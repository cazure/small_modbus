#include "stdio.h"
#include "string.h"
#include "modbus_rtu_rtos.h"

#include "controller.h"
#define DBG_ENABLE
#define DBG_SECTION_NAME    "modbus"
#define DBG_LEVEL           DBG_LOG//DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define delay_ms        rt_thread_mdelay

static small_modbus_mapping_t modbus_mapping = {0};
static small_modbus_t modbus_slave = {0};
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),2,__VA_ARGS__)

static int modbus_rtu_status_callback(small_modbus_mapping_t *mapping,int read_write,int data_type,int start,int num)
{

    return MODBUS_OK;
}

void modbus_rtu_slave_thread(void *param)
{
    int rc = 0;
    static uint8_t receive_buf[128];

   // modbus_mapping_new(modbus_mapping,modbus_rtu_status_callback,0,64,0,64,0,64,0,64);
    controller_t * con = &controller;

    modbus_mapping_init(modbus_mapping,modbus_rtu_status_callback,
            con->io.DO.start,con->io.DO.num,con->io.DO.array,
            con->io.DI.start,con->io.DI.num,con->io.DI.array,
            con->io.AO.start,con->io.AO.num,con->io.AO.array,
            con->io.AI.start,con->io.AI.num,con->io.AI.array);


    modbus_rtu_init(&modbus_slave,NULL,&uart3_config);
    modbus_set_slave(&modbus_slave,8);
    modbus_connect(&modbus_slave);
    while (1)
    {
            rc = modbus_wait_poll(&modbus_slave, receive_buf);
            MODBUS_PRINTF("[slave]%d\n",rc);
			if (rc > 0)
			{
				rc = modbus_handle_poll(&modbus_slave,receive_buf, rc,&modbus_mapping);
				if(rc)
				{
		            MODBUS_PRINTF("[slave] ok %d\n",rc);
				}
			}else
			{
				modbus_error_recovery(&modbus_slave);
			}
    }
    modbus_disconnect(&modbus_slave);
}

small_modbus_t modbus_master = {0};
#undef MODBUS_PRINTF
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_master),2,__VA_ARGS__)

void modbus_rtu_master_thread(void *param)
{
    uint16_t tab_reg[64] = {0};
    modbus_rtu_init(&modbus_master,NULL,&uart6_config);
    modbus_set_slave(&modbus_master,9);
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


int modbus_rtu_test(void)
{
    rt_thread_t tid3,tid6;
    uint8_t type = 0;
    LOG_I("init %d",type);


    tid3 = rt_thread_create("modbus S",modbus_rtu_slave_thread, RT_NULL,2048,20, 10);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

//    tid6 = rt_thread_create("modbus M",modbus_rtu_master_thread, RT_NULL,2048,20, 10);
//    if (tid6 != RT_NULL)
//        rt_thread_startup(tid6);
    return 0;
}
MSH_CMD_EXPORT(modbus_rtu_test,modbus rtu test)
