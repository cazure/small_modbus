/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "modbus_rtu_rtos.h"

int rtos_open(modbus_t *ctx)
{
    struct termios tios;
    speed_t speed;
    int flags;

    modbus_rtu_config_t *ctx_rtu = ctx->backend_data;

    ctx->backend->debug(0,"\n\r open %s  %d\n\r",ctx_rtu->device, ctx_rtu->baud);

    /* The O_NOCTTY flag tells UNIX that this program doesn't want
       to be the "controlling terminal" for that port. If you
       don't specify this then any input (such as keyboard abort
       signals and so forth) will affect your process

       Timeouts are ignored in canonical input mode or when the
       NDELAY option is set on the file via open or fcntl */
    //flags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
    flags = O_RDWR | O_NOCTTY  | O_EXCL;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    ctx->fd = open(ctx_rtu->device, flags);
    if (ctx->fd == -1) {
        ctx->backend->debug(0,"ERROR Can't open the device %s\n",ctx_rtu->device);
        return -1;
    }
    /* Save */
    tcgetattr(ctx->fd, &ctx_rtu->old_tios);

    memset(&tios, 0, sizeof(struct termios));

    /* C_ISPEED     Input baud (new interface)
       C_OSPEED     Output baud (new interface)
    */
    switch (ctx_rtu->baud) {
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
#ifdef B57600
    case 57600:
        speed = B57600;
        break;
#endif
#ifdef B115200
    case 115200:
        speed = B115200;
        break;
#endif
    default:
        speed = B9600;
    }

    /* Set the baud rate */
    if ((cfsetispeed(&tios, speed) < 0) ||
        (cfsetospeed(&tios, speed) < 0)) {
        close(ctx->fd);
        ctx->fd = -1;
        return -1;
    }

    /* C_CFLAG      Control options
       CLOCAL       Local line - do not change "owner" of port
       CREAD        Enable receiver
    */
    tios.c_cflag |= (CREAD | CLOCAL);
    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */

    /* Set data bits (5, 6, 7, 8 bits)
       CSIZE        Bit mask for data bits
    */
    tios.c_cflag &= ~CSIZE;
    switch (ctx_rtu->data_bit) {
    case 5:
        tios.c_cflag |= CS5;
        break;
    case 6:
        tios.c_cflag |= CS6;
        break;
    case 7:
        tios.c_cflag |= CS7;
        break;
    case 8:
    default:
        tios.c_cflag |= CS8;
        break;
    }

    /* Stop bit (1 or 2) */
    if (ctx_rtu->stop_bit == 1)
        tios.c_cflag &=~ CSTOPB;
    else /* 2 */
        tios.c_cflag |= CSTOPB;

    /* PARENB       Enable parity bit
       PARODD       Use odd parity instead of even */
    if (ctx_rtu->parity == 'N') {
        /* None */
        tios.c_cflag &=~ PARENB;
    } else if (ctx_rtu->parity == 'E') {
        /* Even */
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
    } else {
        /* Odd */
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
    }

    /* Read the man page of termios if you need more information. */

    /* This field isn't used on POSIX systems
       tios.c_line = 0;
    */

    /* C_LFLAG      Line options

       ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
       ICANON       Enable canonical input (else raw)
       XCASE        Map uppercase \lowercase (obsolete)
       ECHO Enable echoing of input characters
       ECHOE        Echo erase character as BS-SP-BS
       ECHOK        Echo NL after kill character
       ECHONL       Echo NL
       NOFLSH       Disable flushing of input buffers after
       interrupt or quit characters
       IEXTEN       Enable extended functions
       ECHOCTL      Echo control characters as ^char and delete as ~?
       ECHOPRT      Echo erased character as character erased
       ECHOKE       BS-SP-BS entire line on line kill
       FLUSHO       Output being flushed
       PENDIN       Retype pending input at next read or input char
       TOSTOP       Send SIGTTOU for background output

       Canonical input is line-oriented. Input characters are put
       into a buffer which can be edited interactively by the user
       until a CR (carriage return) or LF (line feed) character is
       received.

       Raw input is unprocessed. Input characters are passed
       through exactly as they are received, when they are
       received. Generally you'll deselect the ICANON, ECHO,
       ECHOE, and ISIG options when using raw input
    */

    /* Raw input */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* C_IFLAG      Input options

       Constant     Description
       INPCK        Enable parity check
       IGNPAR       Ignore parity errors
       PARMRK       Mark parity errors
       ISTRIP       Strip parity bits
       IXON Enable software flow control (outgoing)
       IXOFF        Enable software flow control (incoming)
       IXANY        Allow any character to start flow again
       IGNBRK       Ignore break condition
       BRKINT       Send a SIGINT when a break condition is detected
       INLCR        Map NL to CR
       IGNCR        Ignore CR
       ICRNL        Map CR to NL
       IUCLC        Map uppercase to lowercase
       IMAXBEL      Echo BEL on input line too long
    */
    if (ctx_rtu->parity == 'N') {
        /* None */
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    /* Software flow control is disabled */
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* C_OFLAG      Output options
       OPOST        Postprocess output (not set = raw output)
       ONLCR        Map NL to CR-NL

       ONCLR ant others needs OPOST to be enabled
    */

    /* Raw ouput */
    tios.c_oflag &=~ OPOST;

    /* C_CC         Control characters
       VMIN         Minimum number of characters to read
       VTIME        Time to wait for data (tenths of seconds)

       UNIX serial interface drivers provide the ability to
       specify character and packet timeouts. Two elements of the
       c_cc array are used for timeouts: VMIN and VTIME. Timeouts
       are ignored in canonical input mode or when the NDELAY
       option is set on the file via open or fcntl.

       VMIN specifies the minimum number of characters to read. If
       it is set to 0, then the VTIME value specifies the time to
       wait for every character read. Note that this does not mean
       that a read call for N bytes will wait for N characters to
       come in. Rather, the timeout will apply to the first
       character and the read call will return the number of
       characters immediately available (up to the number you
       request).

       If VMIN is non-zero, VTIME specifies the time to wait for
       the first character read. If a character is read within the
       time given, any read will block (wait) until all VMIN
       characters are read. That is, once the first character is
       read, the serial interface driver expects to receive an
       entire packet of characters (VMIN bytes total). If no
       character is read within the time allowed, then the call to
       read returns 0. This method allows you to tell the serial
       driver you need exactly N bytes and any read call will
       return 0 or N bytes. However, the timeout only applies to
       the first character read, so if for some reason the driver
       misses one character inside the N byte packet then the read
       call could block forever waiting for additional input
       characters.

       VTIME specifies the amount of time to wait for incoming
       characters in tenths of seconds. If VTIME is set to 0 (the
       default), reads will block (wait) indefinitely unless the
       NDELAY option is set on the port with open or fcntl.
    */
    /* Unused because we use open with the NDELAY option */
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(ctx->fd, TCSANOW, &tios) < 0) {
        close(ctx->fd);
        ctx->fd = -1;
        return -1;
    }

    return 0;

    return 0;
}
int rtos_close(modbus_t *ctx)
{
    modbus_rtu_config_t *ctx_rtu = ctx->backend_data;
    if (ctx->fd != -1) {
        tcsetattr(ctx->fd, TCSANOW, &ctx_rtu->old_tios);
        close(ctx->fd);
        ctx->fd = -1;
    }
    return 0;
}

int rtos_read(modbus_t *ctx,uint8_t *data, uint16_t length)
{
    int rc = 0;
    //modbus_rtu_config_t *ctx_rtu = ctx->backend_data;
    rc =  read(ctx->fd, data, length);

//    int i;
//    rt_kprintf("read %d,%d :",rc,length);
//    for (i = 0; i < rc; i++)
//    {
//            rt_kprintf("<%02X>", data[i]);
//    }
//    rt_kprintf("\n");

    return rc;
}
int rtos_write(modbus_t *ctx,uint8_t *data, uint16_t length)
{
    modbus_rtu_config_t *ctx_rtu = ctx->backend_data;
    if(ctx_rtu->rts_set)
        ctx_rtu->rts_set(ctx,1);

    write(ctx->fd, data, length);

//    int i;
//    rt_kprintf("write %d :",length);
//    for (i = 0; i < length; i++)
//    {
//            rt_kprintf("<%02X>", data[i]);
//    }
//    rt_kprintf("\n");

    if(ctx_rtu->rts_set)
        ctx_rtu->rts_set(ctx,0);
    return length;
}
int rtos_flush(modbus_t *ctx)
{
    return tcflush(ctx->fd, TCIOFLUSH);
}

int rtos_select(modbus_t *ctx,int timeout_ms)
{
    fd_set rset;
    struct timeval tv;
    int rc;

    /* Add a file descriptor to the set */
    FD_ZERO(&rset);
    FD_SET(ctx->fd, &rset);

    if(timeout_ms == -1)
    {
        rc = select(ctx->fd+1, &rset, NULL, NULL, NULL);    //
    }else {
        tv.tv_sec = (timeout_ms/1000);
        tv.tv_usec = (timeout_ms%1000)*1000;

        rc = select(ctx->fd+1, &rset, NULL, NULL, &tv);
    }
    if(rc<0)
    {
        return MODBUS_FAIL;
    }else if(rc == 0)
    {
        return MODBUS_TIMEOUT;
    }
//    else {
//        if (FD_ISSET(ctx->fd, &rset))
//        {
//
//        }
//    }
    return rc;
}

static uint8_t now_level = 0;

void rtos_debug(int level,const char *fmt, ...)
{
    static char log_buf[32];
    if(level < now_level)
    {
        va_list args;
        va_start(args, fmt);
        rt_vsnprintf(log_buf, 32, fmt, args);
        va_end(args);
        printf(log_buf);
    }
}

int debug_modbus(int argc, char**argv)
{
    if(argc<2)
    {
        rt_kprintf("debug_modbus [0-2]");
    }else
    {
        now_level  = atoi(argv[1])%3;
    }
    return RT_EOK;
}
MSH_CMD_EXPORT(debug_modbus,debug_modbus [0-5])

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
    config->device = device;
    config->baud = baud;
    config->data_bit = data_bit;
    config->stop_bit = stop_bit;
    config->parity = parity;
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





