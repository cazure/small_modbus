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
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave),__VA_ARGS__)

static int modbus_rtu_status_callback(small_modbus_mapping_t *mapping,int read_write,int data_type,int start,int num)
{

    return MODBUS_OK;
}

void modbus_rtu_slave_thread(void *param)
{
    int rc = 0;
    static uint8_t receive_buf[128];

    modbus_rtu_init(&modbus_slave,NULL,&uart6_config);
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
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_master),__VA_ARGS__)

void modbus_rtu_master_thread(void *param)
{
    int rc =0 ,num = 0;

    modbus_rtu_init(&modbus_master,NULL,&uart3_config);
    modbus_set_slave(&modbus_master,1);
    modbus_connect(&modbus_master);
    while (1)
    {
        modbus_error_recovery(&modbus_master);
        modbus_set_slave(&modbus_master, 1);
        rc = modbus_read_bits(&modbus_master, 0 , 16, modbus_mapping.bit.array);
        rt_kprintf("master1:%d\n",rc);

        modbus_error_recovery(&modbus_master);
        modbus_set_slave(&modbus_master, 1);
        rc = modbus_write_bits(&modbus_master, 0 , 16, modbus_mapping.bit.array);
        rt_kprintf("master2:%d\n",rc);

        modbus_error_recovery(&modbus_master);
        modbus_set_slave(&modbus_master, 1);
        rc = modbus_read_input_bits(&modbus_master, 0, 16, modbus_mapping.input_bit.array);
        rt_kprintf("master3:%d\n",rc);

        modbus_error_recovery(&modbus_master);
        modbus_set_slave(&modbus_master, 1);
        rc = modbus_read_registers(&modbus_master, 0, 16, modbus_mapping.registers.array);
        rt_kprintf("master4:%d\n",rc);

        modbus_error_recovery(&modbus_master);
        modbus_set_slave(&modbus_master, 1);
        rc = modbus_write_registers(&modbus_master, 0, 16, modbus_mapping.registers.array);
        rt_kprintf("master5:%d\n",rc);

        modbus_error_recovery(&modbus_master);
        modbus_set_slave(&modbus_master, 1);
        rc = modbus_read_input_registers(&modbus_master, 0, 16, modbus_mapping.input_registers.array);
        rt_kprintf("master6:%d\n",rc);
        delay_ms(3000);
    }
    modbus_disconnect(&modbus_master);
}


int modbus_rtu_test(void)
{
    rt_thread_t tid3,tid6;
    uint8_t type = 0;
    LOG_I("init %d",type);

    // modbus_mapping_new(modbus_mapping,modbus_rtu_status_callback,0,64,0,64,0,64,0,64);

    //controller_t * con = &controller;
    IO_mapping_t *iotable = &(controller.io_table);
    modbus_mapping_init(modbus_mapping,modbus_rtu_status_callback,
             iotable->DO.start,iotable->DO.num,iotable->DO.array,
             iotable->DI.start,iotable->DI.num,iotable->DI.array,
             iotable->AO.start,iotable->AO.num,iotable->AO.array,
             iotable->AI.start,iotable->AI.num,iotable->AI.array);


    tid3 = rt_thread_create("modbus S",modbus_rtu_slave_thread, RT_NULL,2048,20, 10);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    tid6 = rt_thread_create("modbus M",modbus_rtu_master_thread, RT_NULL,2048,20, 10);
    if (tid6 != RT_NULL)
        rt_thread_startup(tid6);
    return 0;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
int modbus_rtu_debug(int argc, char**argv)
{
    int now_level = 0;
    if(argc<2)
    {
        rt_kprintf("modbus_rtu_debug [0-2]\n0.disable 1.error  2.info\n");
    }else
    {
        now_level  = atoi(argv[1])%3;
        rt_kprintf("modbus_rtu_debug %d\n",now_level);
        modbus_set_debug(&modbus_slave,now_level);
        modbus_set_debug(&modbus_master,now_level);
    }
    return RT_EOK;
}
MSH_CMD_EXPORT(modbus_rtu_debug,modbus_rtu_debug [0-2]);
MSH_CMD_EXPORT(modbus_rtu_test,modbus rtu test);
#endif
