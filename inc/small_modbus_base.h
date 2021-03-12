/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 */
#ifndef _SMALL_MODBUS_BASE_H_
#define _SMALL_MODBUS_BASE_H_

#include "stdint.h"
#include "stdio.h"
#include "string.h"

/* function codes */
enum functionCode {
    MODBUS_FC_READ_HOLDING_COILS        =0x01,
    MODBUS_FC_READ_INPUTS_COILS         =0x02,
    MODBUS_FC_READ_HOLDING_REGISTERS    =0x03,
    MODBUS_FC_READ_INPUT_REGISTERS      =0x04,
    MODBUS_FC_WRITE_SINGLE_COIL         =0x05,
    MODBUS_FC_WRITE_SINGLE_REGISTER     =0x06,
    MODBUS_FC_READ_EXCEPTION_STATUS     =0x07,
    MODBUS_FC_WRITE_MULTIPLE_COILS      =0x0F,
    MODBUS_FC_WRITE_MULTIPLE_REGISTERS  =0x10,
    MODBUS_FC_REPORT_SLAVE_ID           =0x11,
    MODBUS_FC_MASK_WRITE_REGISTER       =0x16,
    MODBUS_FC_WRITE_AND_READ_REGISTERS  =0x17,
};

///* Protocol exceptions */
//enum exceptionsCode{
//    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
//    MODBUS_EXCEPTION_ACKNOWLEDGE,
//    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
//    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
//    MODBUS_EXCEPTION_MEMORY_PARITY,
//    MODBUS_EXCEPTION_NOT_DEFINED,
//    MODBUS_EXCEPTION_GATEWAY_PATH,
//    MODBUS_EXCEPTION_GATEWAY_TARGET,
//    MODBUS_EXCEPTION_MAX
//};

enum returnCode
{
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = -0x83,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = -0x82,
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = -0x81,
    MODBUS_EXCEPTION = -0x80,
    MODBUS_ERROR_WAIT = -9,
    MODBUS_ERROR_READ = -8,
    MODBUS_FAIL_CHECK = -7,
    MODBUS_FAIL_ADRR = -6,
    MODBUS_FAIL_HANDLE = -5,
    MODBUS_FAIL_CONFIRM = -4,
    MODBUS_FAIL_REQUEST = -3,
    MODBUS_TIMEOUT  = -2,
    MODBUS_FAIL     = -1,
    MODBUS_OK = 0
};

enum waitCode
{
    MODBUS_WAIT_FOREVER		= -1,
    MODBUS_WAIT_NO				= 0
};

enum coreType
{
	MODBUS_CORE_NONE	= 0,
	MODBUS_CORE_RTU		= 1,
	MODBUS_CORE_TCP		= 2
};

enum portType
{
	MODBUS_PORT_NONE	= 0,
	MODBUS_PORT_DEVICE		= 1,
	MODBUS_PORT_SOCKET		= 2
};

#define MODBUS_BROADCAST_ADDRESS    0

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* The size of the MODBUS PDU is limited by the size constraint inherited from
 * the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
 * bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
 * address (1 byte) - CRC (2 bytes) = 253 bytes.
 */
#define MODBUS_MAX_PDU_LENGTH              253

/* Consequently:
 * - RTU MODBUS ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256
 *   bytes.
 * - TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes.
 * so the maximum of both backend in 260 bytes. This size can used to allocate
 * an array of bytes to store responses and it will be compatible with the two
 * backends.
 */
#define MODBUS_MAX_ADU_LENGTH              260

//#define MODBUS_BROADCAST_ADDRESS 0xFF

typedef struct _small_modbus        small_modbus_t;
typedef struct _small_modbus_core   small_modbus_core_t;
typedef struct _small_modbus_port   small_modbus_port_t;

struct _small_modbus_core
{
	const uint16_t type;
	const uint16_t len_header;
	const uint16_t len_checksum;
	const uint16_t len_adu_max;
	int (*build_request_header)(small_modbus_t *smb,uint8_t *buff,int slave,int fun,int reg,int num);
	int (*build_response_header)(small_modbus_t *smb,uint8_t *buff,int slave,int fun);
	int (*check_send_pre)(small_modbus_t *smb,uint8_t *buff,int length);
	int (*check_wait_request)(small_modbus_t *smb,uint8_t *buff,int length);
	int (*check_wait_response)(small_modbus_t *smb,uint8_t *buff,int length);
};

struct _small_modbus_port
{
	const uint16_t type;
	const uint16_t port;
	int (*open)(small_modbus_t *smb);
	int (*close)(small_modbus_t *smb);
	int (*read)(small_modbus_t *smb,uint8_t *data,uint16_t length);
	int (*write)(small_modbus_t *smb,uint8_t *data,uint16_t length);
	int (*flush)(small_modbus_t *smb);
	int (*wait)(small_modbus_t *smb,int timeout);
};

struct _small_modbus
{
	uint8_t			mode;  //
	uint8_t     slave_addr;
	uint8_t     debug_level;
	uint16_t    transfer_id;
	uint16_t    protocol_id;
	
	int         status;
	int         error_code;
	uint32_t		timeout_frame;
	uint32_t		timeout_byte;
	uint8_t     read_buff[MODBUS_MAX_ADU_LENGTH];
	uint8_t     write_buff[MODBUS_MAX_ADU_LENGTH];
	small_modbus_core_t *core;
	small_modbus_port_t *port;
};

int _modbus_init(small_modbus_t *smb);
int _modbus_debug(small_modbus_t *smb,int level,const char *fmt, ...);

#define modbus_debug(smb,...)           _modbus_debug(smb,2,__VA_ARGS__)
#define modbus_debug_info(smb,...)      _modbus_debug(smb,1,__VA_ARGS__)
#define modbus_debug_error(smb,...)     _modbus_debug(smb,0,__VA_ARGS__)

#endif /* _SMALL_MODBUS_BASE_H_ */
