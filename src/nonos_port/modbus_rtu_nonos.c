/*
 * Copyright ? 2001-2011 St¨¦phane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "modbus_rtu_nonos.h"

int rtos_open(modbus_t *ctx)
{
	bus_myuart_init();
    return 0;
}
int rtos_close(modbus_t *ctx)
{
    modbus_rtu_config_t *ctx_rtu = ctx->backend_data;
//    if (ctx->fd != -1) {
//        tcsetattr(ctx->fd, TCSANOW, &ctx_rtu->old_tios);
//        close(ctx->fd);
//        ctx->fd = -1;
//    }
    return 0;
}

int rtos_read(modbus_t *ctx,uint8_t *data, uint16_t length)
{
    int rc = 0;
    //modbus_rtu_config_t *ctx_rtu = ctx->backend_data;
 //   rc =  read(ctx->fd, data, length);

//    int i;
//    rt_kprintf("read %d,%d :",rc,length);
//    for (i = 0; i < rc; i++)
//    {
//            rt_kprintf("<%02X>", data[i]);
//    }
//    rt_kprintf("\n");
		rc = myuart_read(&myuart2,data,length,1000);

    return rc;
}
int rtos_write(modbus_t *ctx,uint8_t *data, uint16_t length)
{
    modbus_rtu_config_t *ctx_rtu = ctx->backend_data;
    if(ctx_rtu->rts_set)
        ctx_rtu->rts_set(ctx,1);

 //   write(ctx->fd, data, length);

//    int i;
//    rt_kprintf("write %d :",length);
//    for (i = 0; i < length; i++)
//    {
//            rt_kprintf("<%02X>", data[i]);
//    }
//    rt_kprintf("\n");
		
		
		myuart_write(&myuart2,data,length,1000);

    if(ctx_rtu->rts_set)
        ctx_rtu->rts_set(ctx,0);
    return length;
}
int rtos_flush(modbus_t *ctx)
{
	myuart_flush(&myuart2);
	return 0;
}

int rtos_select(modbus_t *ctx,int timeout_ms)
{
    int rc = 0;
		rc = myuart_select(&myuart2,timeout_ms);
    return rc;
}

#include "stdio.h"
#include "string.h"
static uint8_t now_level = 0;


void rtos_debug(int level,const char *fmt, ...)
{
    static char log_buf[32];
    if(level < now_level)
    {
//        va_list args;
//        va_start(args, fmt);
//        rt_vsnprintf(log_buf, 32, fmt, args);
//        va_end(args);
//        printf(log_buf);
    }
}


modbus_backend_t modbus_rtu_rtos_backend =
{
    .read_timeout = 500,
    .write_timeout = 100,
    .open =  rtos_open,
    .close = rtos_close,
    .read =  rtos_read,
    .write = rtos_write,
    .flush = rtos_flush,
    .select = rtos_select,
    .debug = rtos_debug
};


int modbus_rtu_init(modbus_t *ctx,modbus_backend_t *backend,void *config)
{
    ctx->core = (modbus_core_t*)&modbus_rtu_core;
    ctx->backend_data = config;
    if(backend == NULL)
    {
        ctx->backend = &modbus_rtu_rtos_backend;
    }
    return 0;
}

int modbus_rtu_config(modbus_t *ctx,char *device,int baud,uint8_t data_bit, uint8_t stop_bit,char parity)
{
    modbus_rtu_config_t * config = ctx->backend_data;
    return 0;
}

int modbus_rtu_set_rts_ops(modbus_t *ctx,int (*rts_set)(modbus_t *ctx, int on))
{
    modbus_rtu_config_t * config = ctx->backend_data;
    config->rts_set = rts_set;
    return 0;
}

int modbus_rtu_set_open_ops(modbus_t *ctx, int (*open)(modbus_t *ctx))
{
    ctx->backend->open = open;
    return 0;
}

int modbus_rtu_set_close_ops(modbus_t *ctx, int (*close)(modbus_t *ctx))
{
    ctx->backend->close = close;
    return 0;
}

int modbus_rtu_set_read_ops(modbus_t *ctx, int (*read)(modbus_t *ctx,uint8_t *data,uint16_t length))
{
    ctx->backend->read = read;
    return 0;
}

int modbus_rtu_set_write_ops(modbus_t *ctx, int (*write)(modbus_t *ctx,uint8_t *data,uint16_t length))
{
    ctx->backend->write = write;
    return 0;
}

int modbus_rtu_set_flush_ops(modbus_t *ctx, int (*flush)(modbus_t *ctx))
{
    ctx->backend->flush = flush;
    return 0;
}

int modbus_rtu_set_select_ops(modbus_t *ctx, int (*select)(modbus_t *ctx,int timeout_ms))
{
    ctx->backend->select = select;
    return 0;
}

int modbus_rtu_set_read_timeout(modbus_t *ctx,int timeout_ms)
{
    ctx->backend->read_timeout = timeout_ms;
    return timeout_ms;
}

int modbus_rtu_set_write_timeout(modbus_t *ctx,int timeout_ms)
{
    ctx->backend->write_timeout = timeout_ms;
    return timeout_ms;
}





