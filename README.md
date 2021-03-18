# small modbus



基于了libmodbus重构，可以实现主机和从机大部分功能，可以多实例

将modbus功能和操作系统数据端口分离，分为：

modbus核心（rtu、tcp），

modbus端口（rtthread device、rtthread sal socket）



## 通用函数：

```c
int modbus_connect(small_modbus_t *smb);
int modbus_disconnect(small_modbus_t *smb);
int modbus_write(small_modbus_t *smb,uint8_t *data,uint16_t length);
int modbus_read(small_modbus_t *smb,uint8_t *data,uint16_t length);
int modbus_flush(small_modbus_t *smb);
int modbus_wait(small_modbus_t *smb,int timeout);
int modbus_error_recovery(small_modbus_t *smb);
int modbus_error_exit(small_modbus_t *smb,int code);

int modbus_set_frame_timeout(small_modbus_t *smb,int timeout_ms);
int modbus_set_byte_timeout(small_modbus_t *smb,int timeout_ms);
int modbus_set_slave(small_modbus_t *smb, int slave);
int modbus_set_debug(small_modbus_t *smb, int level);
```





## 主机端函数：

```c
/* master start request */
int modbus_start_request(small_modbus_t *smb,uint8_t *request,int function,int addr,int num,void *write_data);
/* master wait for confirmation message */
int modbus_wait_confirm(small_modbus_t *smb,uint8_t *response);
/* master handle confirmation message */
int modbus_handle_confirm(small_modbus_t *smb,uint8_t *request,uint16_t request_len,uint8_t *response,uint16_t response_len,void *read_data);
/* master read */
int modbus_read_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data);
int modbus_read_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data);
/* master write */
int modbus_write_bit(small_modbus_t *smb, int addr, int write_status);
int modbus_write_register(small_modbus_t *smb, int addr, int write_value);
int modbus_write_bits(small_modbus_t *smb, int addr, int num,uint8_t *write_data);
```



## 从机端函数：

```C
/* slave callback */
typedef int(*small_modbus_slave_callback_t)(small_modbus_t *smb,int function_code,int addr,int num,void *read_write_data);
/* slave mode api */
/* slave wait query data */
int modbus_slave_wait(small_modbus_t *smb,uint8_t *request,int32_t waittime);
/* slave handle query data for callback */
int modbus_slave_handle(small_modbus_t *smb,uint8_t *request,uint16_t request_len,small_modbus_slave_callback_t slave_callback);
/* slave wait and handle query for callback */
int modbus_slave_wait_handle(small_modbus_t *smb,small_modbus_slave_callback_t slave_callback,int32_t waittime);
```





## rtthread平台接口函数：

```c
/*
*modbus port device
*/
int modbus_port_device_init(small_modbus_port_device_t *port,const char *device_name);
small_modbus_port_device_t *modbus_port_device_create(const char *device_name);
small_modbus_port_device_t *modbus_port_device_get(small_modbus_t *smb);
int modbus_set_rts(small_modbus_t *smb,int (*rts_set)(int on));
int modbus_set_serial_config(small_modbus_t *smb,struct serial_configure *serial_config);
int modbus_set_oflag(small_modbus_t *smb,int oflag);
/*
*modbus port socket
*/
int modbus_port_socket_init(small_modbus_port_socket_t *port,char *hostname,char *hostport);
small_modbus_port_socket_t *modbus_port_socket_create(char *hostname,char *hostport);
small_modbus_port_socket_t *modbus_port_socket_get(small_modbus_t *smb);
/*
*modbus_init
*/
int modbus_init(small_modbus_t *smb,uint8_t core_type,void *port);
small_modbus_t *modbus_create(uint8_t core_type,void *port);
```



















