import sys
import os

AddOption('--target',
    dest='target',
    type='string',
    nargs=1,
    action='store',
    help='override target system. Currently only crosswin32 can be used')  

if GetOption('target') == 'crossmingw':
    env = Environment(tools=['crossmingw'], toolpath=['scons-tools'])
    target = 'win32'
else:
    if sys.platform == 'win32':
        env = Environment(tools=['mingw'], env=os.environ)
    else:
        env = Environment(env=os.environ)
    target = sys.platform
    if target == 'darwin':
        target = 'macosx'

Export('env')
Export('target')

SConscript('src/SConscript')
