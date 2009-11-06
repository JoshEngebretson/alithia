import sys
import os

if sys.platform == 'win32':
    env = Environment(tools=['mingw'], env=os.environ)
else:
    env = Environment(env=os.environ)

Export('env')

SConscript('src/SConscript')
