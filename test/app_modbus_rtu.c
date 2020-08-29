#include "stdio.h"
#include "string.h"
#include "modbus_rtu_rtos.h"

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
