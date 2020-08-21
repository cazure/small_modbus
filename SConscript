Import('RTT_ROOT')
from building import *

# get current directory
cwd = GetCurrentDir()

# The set of source files associated with this SConscript file.
src = Glob('src/*.c')
src += Glob('src/rtos_port/*.c')

if GetDepend(['PKG_USING_SMALL_MODBUS_TEST']):
    src += Glob('samples/*.c')

path = [cwd + '/inc']
path += [cwd + '/src/rtos_port']

group = DefineGroup('small_modbus', src, depend = ['PKG_USING_SMALL_MODBUS'], CPPPATH = path)

Return('group')
