/*
 * Change Logs:
 * Date           Author       Notes
 * 2021-06     		chenbin      small_modbus_port_win32.c  for win32
 */
#include "small_modbus_port_win32.h"
 /*
 * modbus on win32
 */
#if SMALL_MODBUS_WIN32
#include "small_modbus_base.h"
#include "small_modbus_utils.h"
#include "small_modbus_rtu.h"
#include "small_modbus_tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <windows.h>
//#include <winsock2.h>

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif


enum win32device_ringbuffer_state win32device_ringbuffer_status(struct win32device_ringbuffer* rb)
{
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
            return WIN32DEVICE_RINGBUFFER_EMPTY;
        else
            return WIN32DEVICE_RINGBUFFER_FULL;
    }
    return WIN32DEVICE_RINGBUFFER_HALFFULL;
}

void win32device_ringbuffer_init(struct win32device_ringbuffer* rb,
    uint8_t* pool,
    int16_t size)
{
    /* initialize read and write index */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* set buffer pool and size */
    rb->buffer_ptr = pool;
    rb->buffer_size = size;
}
RTM_EXPORT(win32device_ringbuffer_init);

/**
 * put a block of data into ring buffer
 */
size_t win32device_ringbuffer_put(struct win32device_ringbuffer* rb,
    const uint8_t* ptr,
    uint16_t           length)
{
    uint16_t size;

    /* whether has enough space */
    size = win32device_ringbuffer_space_len(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;
        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
        &ptr[0],
        rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
        &ptr[rb->buffer_size - rb->write_index],
        length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    return length;
}

/**
 *  get data from ring buffer
 */
size_t win32device_ringbuffer_get(struct win32device_ringbuffer* rb,
    uint8_t* ptr,
    uint16_t           length)
{
    size_t size;

    /* whether has enough data  */
    size = win32device_ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
        &rb->buffer_ptr[rb->read_index],
        rb->buffer_size - rb->read_index);
    memcpy(&ptr[rb->buffer_size - rb->read_index],
        &rb->buffer_ptr[0],
        length - (rb->buffer_size - rb->read_index));

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buffer_size - rb->read_index);

    return length;
}

/**
 * get the size of data in rb
 */
size_t win32device_ringbuffer_data_len(struct win32device_ringbuffer* rb)
{
    switch (win32device_ringbuffer_status(rb))
    {
    case WIN32DEVICE_RINGBUFFER_EMPTY:
        return 0;
    case WIN32DEVICE_RINGBUFFER_FULL:
        return rb->buffer_size;
    case WIN32DEVICE_RINGBUFFER_HALFFULL:
    default:
        if (rb->write_index > rb->read_index)
            return rb->write_index - rb->read_index;
        else
            return rb->buffer_size - (rb->read_index - rb->write_index);
    };
}

/**
 * empty the rb
 */
void win32device_ringbuffer_reset(struct win32device_ringbuffer* rb)
{
    rb->read_mirror = 0;
    rb->read_index = 0;
    rb->write_mirror = 0;
    rb->write_index = 0;
}

uint16_t win32device_ringbuffer_get_size(struct win32device_ringbuffer* rb)
{
    return rb->buffer_size;
}


int _modbus_debug(small_modbus_t* smb, int level, const char* fmt, ...)
{
    static char log_buf[256];
    if (level <= smb->debug_level)
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(log_buf, 256, fmt, args);
        va_end(args);
        puts(log_buf);
    }
    return 0;
}
/*
*modbus_init
*/
int modbus_init(small_modbus_t* smb, uint8_t core_type, void* port)
{
    small_modbus_port_t* smb_port;
    if (smb && core_type && port)
    {
        _modbus_init(smb);
        if ((core_type == MODBUS_CORE_RTU) || (core_type == MODBUS_CORE_TCP))  // check core type
        {
            if (core_type == MODBUS_CORE_RTU)
            {
                smb->core = (small_modbus_core_t*)&_modbus_rtu_core;
            }
            if (core_type == MODBUS_CORE_TCP)
            {
                smb->core = (small_modbus_core_t*)&_modbus_tcp_core;
            }
        }
        else
        {
            return 0;
        }
        smb_port = port;
        if ((smb_port->type == MODBUS_PORT_DEVICE) || (smb_port->type == MODBUS_PORT_SOCKET))  // check port type
        {
            smb->port = smb_port;
            return 1;
        }
    }
    return 0;
}

small_modbus_t* modbus_create(uint8_t core_type, void* port)
{
    small_modbus_t* smb = malloc(sizeof(small_modbus_t), 4);
    if (smb)
    {
        if (modbus_init(smb, core_type, port))
        {
            return smb;
        }
        else
        {
           free(smb);
        }
    }
    return NULL;
}

/* Some references here:
 * http://msdn.microsoft.com/en-us/library/aa450602.aspx
 */

static int _modbus_win32device_open(small_modbus_t* smb)
{
    small_modbus_port_win32device_t* smb_port_device = (small_modbus_port_win32device_t*)smb->port;
    DCB dcb;

    /* ctx_rtu->device should contain a string like "COMxx:" xx being a decimal
    * number */

    smb_port_device->fd  = CreateFileA(smb_port_device->device_name,
        GENERIC_READ | GENERIC_WRITE ,0, NULL,
        OPEN_EXISTING, 0, NULL);

    /* Error checking */
    if (smb_port_device->fd == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "ERROR Can't open the device %s (LastError %d)\n",
            smb_port_device->device_name,(int)GetLastError());
        return -1;
    }
    /* Save params */
    smb_port_device->old_dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(smb_port_device->fd, & (smb_port_device->old_dcb) ))
    {
        fprintf(stderr, "ERROR Error getting configuration (LastError %d)\n",(int)GetLastError());
        CloseHandle(smb_port_device->fd);
        smb_port_device->fd = INVALID_HANDLE_VALUE;
        return -1;
    }

    /* Build new configuration (starting from current settings) */
    dcb = smb_port_device->old_dcb;

    /* Speed setting */
    switch (smb_port_device->serial_config.baud_rate)
    {
    case 110:
        dcb.BaudRate = CBR_110;
        break;
    case 300:
        dcb.BaudRate = CBR_300;
        break;
    case 600:
        dcb.BaudRate = CBR_600;
        break;
    case 1200:
        dcb.BaudRate = CBR_1200;
        break;
    case 2400:
        dcb.BaudRate = CBR_2400;
        break;
    case 4800:
        dcb.BaudRate = CBR_4800;
        break;
    case 9600:
        dcb.BaudRate = CBR_9600;
        break;
    case 14400:
        dcb.BaudRate = CBR_14400;
        break;
    case 19200:
        dcb.BaudRate = CBR_19200;
        break;
    case 38400:
        dcb.BaudRate = CBR_38400;
        break;
    case 57600:
        dcb.BaudRate = CBR_57600;
        break;
    case 115200:
        dcb.BaudRate = CBR_115200;
        break;
    case 230400:
        /* CBR_230400 - not defined */
        dcb.BaudRate = 230400;
        break;
    case 250000:
        dcb.BaudRate = 250000;
        break;
    case 460800:
        dcb.BaudRate = 460800;
        break;
    case 500000:
        dcb.BaudRate = 500000;
        break;
    case 921600:
        dcb.BaudRate = 921600;
        break;
    case 1000000:
        dcb.BaudRate = 1000000;
        break;
    default:
        dcb.BaudRate = CBR_9600;
        fprintf(stderr, "WARNING Unknown baud rate %d for %s (B9600 used)\n",
            smb_port_device->serial_config.baud_rate, smb_port_device->device_name);

    }

    /* Data bits */
    switch (smb_port_device->serial_config.data_bits)
    {
        case DATA_BITS_5:
            dcb.ByteSize = 5;
            break;
        case DATA_BITS_6:
            dcb.ByteSize = 6;
            break;
        case DATA_BITS_7:
            dcb.ByteSize = 7;
            break;
        case DATA_BITS_8:
        default:
            dcb.ByteSize = 8;
            break;
    }

    /* Stop bits */
    if (smb_port_device->serial_config.stop_bits == STOP_BITS_1)
    {
        dcb.StopBits = ONESTOPBIT;
    } 
    else /* 2 */
    {
        dcb.StopBits = TWOSTOPBITS;
    }
    /* Parity */
    if (smb_port_device->serial_config.parity == PARITY_NONE)
    {
        dcb.Parity = NOPARITY;
        dcb.fParity = FALSE;
    }else
    if (smb_port_device->serial_config.parity == PARITY_EVEN)
    {
        dcb.Parity = EVENPARITY;
        dcb.fParity = TRUE;
    }else
    {
        ///* odd */
        //dcb.Parity = ODDPARITY;
        //dcb.fParity = TRUE;
        dcb.Parity = NOPARITY;
        dcb.fParity = FALSE;
    }

    /* Hardware handshaking left as default settings retrieved */

    /* No software handshaking */
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    /* Binary mode (it's the only supported on Windows anyway) */
    dcb.fBinary = TRUE;

    /* Don't want errors to be blocking */
    dcb.fAbortOnError = FALSE;

    /* Setup port */
    if (!SetCommState(smb_port_device->fd, &dcb))
    {
        fprintf(stderr, "ERROR Error setting new configuration (LastError %d)\n",(int)GetLastError());
        CloseHandle(smb_port_device->fd);
        smb_port_device->fd = INVALID_HANDLE_VALUE;
        return -1;
    }
    if (smb_port_device->rts_set)
    {
        smb_port_device->rts_set(0);
    }
    return 0;
}

static int _modbus_win32device_close(small_modbus_t* smb)
{
    small_modbus_port_win32device_t* smb_port_device = (small_modbus_port_win32device_t*)smb->port;
    /* Revert settings */
    if (!SetCommState(smb_port_device->fd, &(smb_port_device->old_dcb)  ) )
    {
        fprintf(stderr, "ERROR Couldn't revert to configuration (LastError %d)\n",(int)GetLastError());
    }
    if (!CloseHandle(smb_port_device->fd) )
    {
        fprintf(stderr, "ERROR Error while closing handle (LastError %d)\n",(int)GetLastError());
    }
    return 0;
}

static int _modbus_win32device_write(small_modbus_t* smb, uint8_t* data, uint16_t length)
{
    small_modbus_port_win32device_t* smb_port_device = (small_modbus_port_win32device_t*)smb->port;

    if (smb_port_device->rts_set)
        smb_port_device->rts_set(1);

    DWORD n_bytes = 0;
    WriteFile(smb_port_device->fd, data, length, &n_bytes, NULL);

    if (smb_port_device->rts_set)
        smb_port_device->rts_set(0);

    return length;
}

static int _modbus_win32device_read(small_modbus_t* smb, uint8_t* data, uint16_t length)
{
    small_modbus_port_win32device_t* smb_port_device = (small_modbus_port_win32device_t*)smb->port;

    //uint32_t read_len = smb_port_device->read_buff_len;

    //if (read_len > length)
    //{
    //    read_len = length; //min
    //}
    //if (read_len > 0)
    //{
    //    memcpy(data, smb_port_device->read_buff+ smb_port_device->read_buff_pos, read_len);
    //}
    //smb_port_device->read_buff_len -= read_len;
    //smb_port_device->read_buff_pos += read_len;
    uint32_t read_len = win32device_ringbuffer_data_len(&(smb_port_device->rx_ringbuff));
    if (read_len > length)
    {
        read_len = length; //min
    }
    if (read_len > 0)
    {
       win32device_ringbuffer_get(&(smb_port_device->rx_ringbuff), data, read_len);
    }
    return read_len;
}

static int _modbus_win32device_flush(small_modbus_t* smb)
{
    small_modbus_port_win32device_t* smb_port_device = (small_modbus_port_win32device_t*)smb->port;

    win32device_ringbuffer_reset(&(smb_port_device->rx_ringbuff));

    smb_port_device->read_buff_len = 0;
    smb_port_device->read_buff_pos = 0;
    PurgeComm(smb_port_device->fd, PURGE_RXCLEAR);
    return 0;
}

static int _modbus_win32device_wait(small_modbus_t* smb, int timeout)
{
    small_modbus_port_win32device_t* smb_port_device = (small_modbus_port_win32device_t*)smb->port;
    COMMTIMEOUTS comm_to;
    unsigned int msec = 0;

    /* Check if some data still in the buffer to be consumed */
    if (smb_port_device->read_buff_len > 0)
    {
        return 1;
    }
    /* Setup timeouts like select() would do.
      FIXME Please someone on Windows can look at this?
      Does it possible to use WaitCommEvent?
      When tv is NULL, MAXDWORD isn't infinite!
    */
    if (timeout <= MODBUS_WAIT_FOREVER)
    {
        msec = MAXDWORD;
    }
    else if(timeout == MODBUS_WAIT_NO)
    {
        msec = 0;
    }
    else
    {
        msec = timeout;
    }
    if (timeout != 0)
    {
        comm_to.ReadIntervalTimeout = msec;
        comm_to.ReadTotalTimeoutMultiplier = 0;
        comm_to.ReadTotalTimeoutConstant = msec;
        comm_to.WriteTotalTimeoutMultiplier = 0;
        comm_to.WriteTotalTimeoutConstant = 1000;
        SetCommTimeouts(smb_port_device->fd, &comm_to);
    }

    int max_len = PY_BUF_SIZE;
    int read_len = 0;

    if (ReadFile(smb_port_device->fd, smb_port_device->read_buff, max_len, &read_len, NULL))
    {
        /* Check if some bytes available */
        if (read_len > 0)
        {
            printf("RX[%d] ", read_len);
            for (int i = 0; i < read_len; i++)
            {
                printf("%02X ", smb_port_device->read_buff[i]);
            }
            printf("\n");
            //smb_port_device->read_buff_len = read_len;
            //smb_port_device->read_buff_pos = 0;

            win32device_ringbuffer_put(&(smb_port_device->rx_ringbuff), smb_port_device->read_buff, read_len);

            /* Some bytes read */
            return 1;
        }
        else {
            /* Just timed out */
            return MODBUS_TIMEOUT;
        }
    }
    else {
        /* Some kind of error */
        return MODBUS_ERROR_READ;
    }
}

int modbus_port_win32device_init(small_modbus_port_win32device_t* port, const char* device_name)
{
    struct serial_configure config_temp = SERIAL_CONFIG_DEFAULT;

    (*(uint32_t*)&(port->base.type)) = MODBUS_PORT_DEVICE;
    port->base.open = _modbus_win32device_open;
    port->base.close = _modbus_win32device_close;
    port->base.read = _modbus_win32device_read;
    port->base.write = _modbus_win32device_write;
    port->base.flush = _modbus_win32device_flush;
    port->base.wait = _modbus_win32device_wait;

    port->device_name = device_name;
    port->serial_config = config_temp;

    win32device_ringbuffer_init(&(port->rx_ringbuff), port->__rx_ringbuff_data,256);
    return 0;
}
small_modbus_port_win32device_t* modbus_port_win32device_create(const char* device_name)
{
    small_modbus_port_win32device_t* port_device = malloc(sizeof(small_modbus_port_win32device_t));
    if (port_device)
    {
        memset(port_device, 0, sizeof(small_modbus_port_win32device_t));
        modbus_port_win32device_init(port_device, device_name);
        return port_device;
    }
    return NULL;
}
small_modbus_port_win32device_t* modbus_port_win32device_get(small_modbus_t* smb)
{
    if (smb->port->type == MODBUS_PORT_DEVICE)
    {
        return (small_modbus_port_win32device_t*)smb->port;
    }
    return NULL;
}

int modbus_set_rts(small_modbus_t* smb, int (*rts_set)(int on))
{
    small_modbus_port_win32device_t* port_device = modbus_port_win32device_get(smb);
    if (port_device)
    {
        port_device->rts_set = rts_set;
    }
    return 0;
}
int modbus_set_serial_config(small_modbus_t* smb, struct serial_configure* serial_config)
{
    small_modbus_port_win32device_t* port_device = modbus_port_win32device_get(smb);
    if (port_device)
    {
        port_device->serial_config = *serial_config;
    }
    return 0;
}

#endif

