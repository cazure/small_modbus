#include "stdio.h"
#include "string.h"
#include "modbus_rtu_rtos.h"

small_modbus_mapping_t  *modbus_mapping = {0};
#define MODBUS_BIT_START 						 		0
#define MODBUS_INTPUT_BIT_START  		 		0
#define MODBUS_REGISTERS_START  			 	0
#define MODBUS_INPUT_REGISTERS_START  	0

#define MODBUS_BIT_UINT 						 64
#define MODBUS_INTPUT_BIT_UINT  		 64
#define MODBUS_REGISTERS_UINT  			 64
#define MODBUS_INPUT_REGISTERS_UINT  64

#define MODBUS_MAX_SLAVE_NUM  3

#define DBG_ENABLE
#define DBG_SECTION_NAME    "modbus"
#define DBG_LEVEL           DBG_LOG//DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define delay_ms        rt_thread_mdelay

void modbus_slave_thread(void *param);
void modbus_poll_thread(void *param);


static void RS485_GPIO_Init(void)
{
}

extern int rs4851_rt_set(int on);
extern int rs4852_rt_set(int on);

int _modbus_slave_rts_set(small_modbus_t *smb, int on)
{
    return rs4851_rt_set(on);
}

int _modbus_master_rts_set(small_modbus_t *smb, int on)
{
    return rs4851_rt_set(on);
}

small_modbus_t modbus_slave_ctx = {0};
modbus_rtu_config_t modbus_slave_config = {0};

#define MODBUS_PRINTF(...)   modbus_debug((&modbus_slave_ctx),2,__VA_ARGS__)

void modbus_slave_thread(void *param)
{
    int rc = 0;
    uint8_t receive_buf[128];

    modbus_rtu_init(&modbus_slave_ctx,NULL,&modbus_slave_config);
    modbus_rtu_config(&modbus_slave_ctx,"/dev/uart3", 9600, 8, 1, 'N');
    modbus_rtu_set_rts_ops(&modbus_slave_ctx,_modbus_slave_rts_set);

    modbus_set_slave(&modbus_slave_ctx,6);
    modbus_open(&modbus_slave_ctx);
    while (1)
    {
        rc = modbus_wait_poll(&modbus_slave_ctx, receive_buf);
			//rc = modbus_receive(&modbus_slave_ctx, receive_buf);
			//rc = modbus_read(query_buf,64);
			if (rc > 0)
			{
			    MODBUS_PRINTF("\n[1]------ receive %d --------\n",rc);
				//rc = modbus_reply(&modbus_slave_ctx, receive_buf, rc, modbus_mapping);
				rc = modbus_handle_poll(&modbus_slave_ctx, modbus_mapping, receive_buf, rc);
			}else
			{
			    MODBUS_PRINTF("\n[1]------ receive failed ---------\n");
				delay_ms(10);
				modbus_flush(&modbus_slave_ctx);
			}
    }
    modbus_close(&modbus_slave_ctx);
}


small_modbus_t modbus_master_ctx = {0};
modbus_rtu_config_t modbus_master_config = {0};
#undef MODBUS_PRINTF
#define MODBUS_PRINTF(...)   modbus_debug((&modbus_master_ctx),2,__VA_ARGS__)

void modbus_master_thread(void *param)
{
    uint16_t tab_reg[64] = {0};

//    modbus_rtu_init(&modbus_ctx,NULL);
//    modbus_rtu_set_read_timeout(&modbus_ctx,-1);
//    modbus_rtu_set_write_timeout(&modbus_ctx,100);

    modbus_rtu_init(&modbus_master_ctx,NULL,&modbus_master_config);
    modbus_rtu_config(&modbus_master_ctx,"/dev/uart6", 9600, 8, 1, 'N');
    modbus_rtu_set_rts_ops(&modbus_master_ctx,_modbus_master_rts_set);

    modbus_set_slave(&modbus_master_ctx,6);
    modbus_open(&modbus_master_ctx);
    int regs =0 ,num = 0;
    while (1)
    {
        memset(tab_reg, 0, 64 * 2);
        modbus_set_slave(&modbus_master_ctx,6);

        MODBUS_PRINTF("\n[2]------ read registers [0x00] -------\n");
        regs = modbus_read_registers(&modbus_master_ctx, 3, 0x02, tab_reg);
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
            modbus_write_register(&modbus_master_ctx,0x08,num);

        }else
        {
            MODBUS_PRINTF("\n[2]------ receive failed -------\n");
            delay_ms(10);
            modbus_flush(&modbus_master_ctx);
        }
        delay_ms(6000);
    }
    modbus_close(&modbus_master_ctx);
}


int app_modbus_init(void)
{
    rt_thread_t tid3,tid6;
    uint8_t type = 0;
    LOG_I("init %d",type);
    RS485_GPIO_Init();

    modbus_mapping->bit.start = 0;

//
//    modbus_mapping = modbus_mapping_new_start_address(
//                                        MODBUS_BIT_START  ,MODBUS_BIT_UINT * MODBUS_MAX_SLAVE_NUM,
//                                        MODBUS_INTPUT_BIT_START,MODBUS_INTPUT_BIT_UINT * MODBUS_MAX_SLAVE_NUM,
//                                        MODBUS_REGISTERS_START,MODBUS_REGISTERS_UINT * MODBUS_MAX_SLAVE_NUM,
//                                        MODBUS_INPUT_REGISTERS_START,MODBUS_INPUT_REGISTERS_UINT * MODBUS_MAX_SLAVE_NUM);

    tid3 = rt_thread_create("modbus S",modbus_slave_thread, RT_NULL,2048,20, 10);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    tid6 = rt_thread_create("modbus M",modbus_master_thread, RT_NULL,2048,20, 10);
    if (tid6 != RT_NULL)
        rt_thread_startup(tid6);
    return 0;
}
MSH_CMD_EXPORT(app_modbus_init,modbus test)
