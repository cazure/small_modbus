#ifndef MODBUS_RTU_RTOS_H
#define MODBUS_RTU_RTOS_H
#include "small_modbus_rtu.h"
#include <rtthread.h>
#include <rtdevice.h>

typedef struct _modbus_rtu_config
{
    char name[8];
    uint8_t _read_buff[256];
    uint8_t _rx_buff[256];
    struct rt_device *dev;
    struct serial_configure config;
    struct rt_semaphore rx_sem;
    struct rt_ringbuffer rx_ring;
    struct rt_mutex 	lock;
    int (*rts_set)(small_modbus_t *ctx, int on);
} modbus_rtu_config_t;

extern modbus_rtu_config_t uart3_config;
extern modbus_rtu_config_t uart6_config;

int modbus_rtu_init(small_modbus_t *smb,small_modbus_port_t *port,void *config);

int modbus_rtu_lock(small_modbus_t *smb);
int modbus_rtu_unlock(small_modbus_t *smb);

//int modbus_rtu_config(small_modbus_t *smb,char *device_name,rtt_uart_t *uart);
//int modbus_rtu_set_rts_ops(small_modbus_t *smb,int (*rts_set)(small_modbus_t *ctx, int on));
//int modbus_rtu_set_open_ops(small_modbus_t *smb, int (*open)(modbus_t *ctx));
//int modbus_rtu_set_close_ops(small_modbus_t *smb, int (*close)(modbus_t *ctx));
//int modbus_rtu_set_read_ops(small_modbus_t *smb, int (*read)(modbus_t *ctx,uint8_t *data,uint16_t length));
//int modbus_rtu_set_write_ops(small_modbus_t *smb, int (*write)(modbus_t *ctx,uint8_t *data,uint16_t length));
//int modbus_rtu_set_flush_ops(small_modbus_t *smb, int (*flush)(modbus_t *ctx));
//int modbus_rtu_set_select_ops(small_modbus_t *smb, int (*select)(modbus_t *ctx,int timeout_ms));
//int modbus_rtu_set_read_timeout(small_modbus_t *smb,int timeout_ms);
//int modbus_rtu_set_write_timeout(small_modbus_t *smb,int timeout_ms);

#endif
