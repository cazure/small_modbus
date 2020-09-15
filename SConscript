Import('RTT_ROOT')
from building import *

cwd = GetCurrentDir()

src = Glob('src/*.c')

path = [cwd + '/inc']

if GetDepend(['PKG_SMALL_MODBUS_RTOS']):
    src += Glob('port_rtos/*.c')
    path += [cwd + '/port_rtos']

if GetDepend(['PKG_USING_SMALL_MODBUS_RTU_TEST']):
    src += Glob('test/modbus_rtu_test.c')
    
if GetDepend(['PKG_USING_SMALL_MODBUS_TCP_TEST']):
    src += Glob('test/modbus_tcp_test.c')
    
group = DefineGroup('small_modbus', src, depend = ['PKG_USING_SMALL_MODBUS'], CPPPATH = path)

Return('group')
