/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#include "small_modbus_rtu.h"
#include "small_modbus_utils.h"
#include "small_modbus_port.h"

/* Builds a RTU request header */
static int _rtu_build_request_header(small_modbus_t *smb,uint8_t *buff,int slave,int fun,int reg,int num)
{
    buff[0] = slave;
    buff[1] = fun;
    buff[2] = reg >> 8;
    buff[3] = reg & 0x00ff;
    buff[4] = num >> 8;
    buff[5] = num & 0x00ff;
    return _MODBUS_RTU_PRESET_REQ_LENGTH;
}

/* Builds a RTU response header */
static int _rtu_build_response_header(small_modbus_t *smb,uint8_t *buff,int slave,int fun)
{
    buff[0] = slave;
    buff[1] = fun;
    return _MODBUS_RTU_PRESET_RSP_LENGTH;
}

static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static int _rtu_check_send_pre(small_modbus_t *smb,uint8_t *buff,int length)
{
    uint16_t crc = modbus_crc16(buff,length);
#if (SMALL_MODBUS_CRC_BYTE_SWAP)
    crc = bswap_16(crc);
#endif
    buff[length++] = crc & 0x00FF; //LOW BYTE 
    buff[length++] = crc >> 8;   //HIGH BYTE
    return length;
}

static int _rtu_check_wait_request(small_modbus_t *smb,uint8_t *buff,int length)
{
    uint16_t crc_recv = ( (uint16_t)(buff[length - 2]&0x00FF) + (uint16_t)(buff[length - 1] << 8) ); //LOW BYTE HIGH BYTE
    uint16_t crc_cal = modbus_crc16(buff, length - 2);
#if (SMALL_MODBUS_CRC_BYTE_SWAP)
    crc_cal = bswap_16(crc_cal);
#endif
    if (crc_cal != crc_recv)
    {
        modbus_debug_error(smb,"crc  0x%04X != 0x%04X\n", crc_cal, crc_recv );
        return MODBUS_FAIL_CHECK;
    }
    if (buff[0] != smb->slave_addr && buff[0] != MODBUS_BROADCAST_ADDRESS)
    {
        modbus_debug_error(smb,"slave adrr: 0x%02X != 0x%02X\n", buff[0],smb->slave_addr);
        return MODBUS_FAIL_ADRR;
    }
    return length;
}

static int _rtu_check_wait_response(small_modbus_t *smb,uint8_t *buff,int length)
{
    uint16_t crc_recv = ( (uint16_t)(buff[length - 2]&0x00FF) + (uint16_t)(buff[length - 1] << 8) ); //LOW BYTE HIGH BYTE
    uint16_t crc_cal = modbus_crc16(buff, length - 2);
#if (SMALL_MODBUS_CRC_BYTE_SWAP)
    crc_cal = bswap_16(crc_cal);
#endif
    if (crc_cal != crc_recv)
    {
        modbus_debug_error(smb, "crc  0x%04X != 0x%04X\n", crc_cal, crc_recv);
        return MODBUS_FAIL_CHECK;
    }
    if (buff[0] != smb->slave_addr && buff[0] != MODBUS_BROADCAST_ADDRESS)
    {
        modbus_debug_error(smb,"slave adrr: 0x%02X != 0x%02X\n", buff[0],smb->slave_addr);
        return MODBUS_FAIL_ADRR;
    }
    return length;
}

const small_modbus_core_t _modbus_rtu_core =
{
    .type           = MODBUS_CORE_RTU,
    .len_header     = _MODBUS_RTU_HEADER_LENGTH,
    .len_checksum   = _MODBUS_RTU_CHECKSUM_LENGTH,
    .len_adu_max    = _MODBUS_RTU_MAX_ADU_LENGTH,
    .build_request_header   = _rtu_build_request_header,
    .build_response_header  = _rtu_build_response_header,
    .check_send_pre     = _rtu_check_send_pre,
    .check_wait_request   = _rtu_check_wait_request,
    .check_wait_response   = _rtu_check_wait_response
};

