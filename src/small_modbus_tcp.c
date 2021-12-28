/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#include "small_modbus_tcp.h"
#include "small_modbus_utils.h"
#include "config_small_modbus.h"

/* Builds a TCP request header */
static int _tcp_build_request_header(small_modbus_t *smb, uint8_t *buff, int slave, int fun, int reg, int num)
{
    smb->transfer_id++;
    buff[0] = smb->transfer_id >> 8;
    buff[1] = smb->transfer_id & 0x00ff;

    /* Protocol Modbus */
    //    buff[2] = config->protocol_id >> 8;
    //    buff[3] = config->protocol_id & 0x00ff;
    buff[2] = 0x00;
    buff[3] = 0x00;

    /* Length will be defined later by set_req_length_tcp at offsets 4
       and 5 */

    buff[6] = slave;
    buff[7] = fun;
    buff[8] = reg >> 8;
    buff[9] = reg & 0x00ff;
    buff[10] = num >> 8;
    buff[11] = num & 0x00ff;

    return _MODBUS_TCP_PRESET_REQ_LENGTH;
}

/* Builds a TCP response header */
static int _tcp_build_response_header(small_modbus_t *smb, uint8_t *buff, int slave, int fun)
{
    /* Extract from MODBUS Messaging on TCP/IP Implementation
       Guide V1.0b (page 23/46):
       The transaction identifier is used to associate the future
       response with the request. */
    buff[0] = smb->transfer_id >> 8;
    buff[1] = smb->transfer_id & 0x00ff;

    /* Protocol Modbus */
    //    buff[2] = config->protocol_id >> 8;
    //    buff[3] = config->protocol_id & 0x00ff;
    buff[2] = 0x00;
    buff[3] = 0x00;

    /* Length will be set later by send_msg (4 and 5) */

    /* The slave ID is copied from the indication */
    buff[6] = slave;
    buff[7] = fun;

    return _MODBUS_TCP_PRESET_RSP_LENGTH;
}

static int _tcp_check_send_pre(small_modbus_t *smb, uint8_t *buff, int length)
{
    /* Substract the header length to the message length */
    int rc = length - 6;

    buff[4] = rc >> 8;
    buff[5] = rc & 0x00FF;

    return length;
}

static int _tcp_check_wait_request(small_modbus_t *smb, uint8_t *buff, int length)
{
    int check_len = (buff[4] << 8) + (buff[5]);
    int addr = buff[6];
    if ((buff[2] == 0x00) && (buff[3] == 0x00)) // check Protocol ID
    {
        if (((length - check_len) == 6)) // check data length
        {
            if ((addr == smb->slave_addr) || (addr == MODBUS_BROADCAST_ADDRESS)) // check addr
            {
                smb->transfer_id = (buff[0] << 8) + (buff[1]); // save transfer_id
                return length;
            }
            else
            {
                modbus_debug_error(smb, "slave adrr: 0x%0X != 0x%0X\n", addr, smb->slave_addr);
                return MODBUS_FAIL_ADRR;
            }
        }
    }
    modbus_debug_error(smb, "not is modbus tcp data\n");
    return MODBUS_FAIL_CHECK;
}

static int _tcp_check_wait_response(small_modbus_t *smb, uint8_t *buff, int length)
{
    int check_len = (buff[4] << 8) + (buff[5]);
    int addr = buff[6];
    uint16_t transfer_id = (buff[0] << 8) + (buff[1]); // transfer_id
    if (transfer_id == smb->transfer_id)
    {
        if ((buff[2] == 0x00) && (buff[3] == 0x00)) // check Protocol ID
        {
            if (((length - check_len) == 6)) // check data length
            {
                if ((addr == smb->slave_addr) || (addr == MODBUS_BROADCAST_ADDRESS)) // check addr
                {
                    return length;
                }
                else
                {
                    modbus_debug_error(smb, "slave adrr: 0x%0X != 0x%0X\n", addr, smb->slave_addr);
                    return MODBUS_FAIL_ADRR;
                }
            }
        }
    }
    modbus_debug_error(smb, "not is modbus tcp data\n");
    return MODBUS_FAIL_CHECK;
}

const small_modbus_core_t _modbus_tcp_core =
    {
        .magic = MODBUS_CORE_MAGIC,
        .type = MODBUS_CORE_TCP,
        .len_header = _MODBUS_TCP_HEADER_LENGTH,
        .len_checksum = _MODBUS_TCP_CHECKSUM_LENGTH,
        .len_adu_max = _MODBUS_TCP_MAX_ADU_LENGTH,
        .build_request_header = _tcp_build_request_header,
        .build_response_header = _tcp_build_response_header,
        .check_send_pre = _tcp_check_send_pre,
        .check_wait_request = _tcp_check_wait_request,
        .check_wait_response = _tcp_check_wait_response};
