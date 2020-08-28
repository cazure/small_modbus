/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-28     Administrator       the first version
 */


small_modbus_port_t modbus_tcp_rtos_port = {0};
//{
//    .open =  rtos_open,
//    .close = rtos_close,
//    .read =  rtos_read,
//    .write = rtos_write,
//    .flush = rtos_flush,
//    .select = rtos_select,
//    .debug =  rtos_debug
//};


int modbus_tcp_init(small_modbus_t *smb,small_modbus_port_t *port,void *config)
{
    _modbus_init(smb);
    smb->core = (small_modbus_core_t*)&modbus_tcp_core;
    smb->port_data = config;
    if(port ==NULL)
    {
        smb->port = &modbus_tcp_rtos_port;
    }else {
        smb->port = port;
    }
    return 0;
}
