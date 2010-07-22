#!/usr/bin/python

import os
import sys
from optparse import OptionParser

# find platform
target = sys.platform
if target == 'darwin':
    target = 'macosx'
    cc = 'clang'
    objc = 'clang'
else:
    cc = 'gcc'


# build configuration
program_prefix = './'
program_name = 'atest'

# parse arguments
oparser = OptionParser(version="configure script for Alithia Engine version 0.1")
oparser.add_option("--release", action="store_true", dest="release", default=False, help="use build options for releases (enable optimizations, disable debug info, etc)")
oparser.add_option("--target", action="store", type="string", dest="target", default=target, help="target platform [default: %default]")
oparser.add_option("--c-compiler", action="store", type="string", dest="cc", default=cc, help="C compiler binary name (arguments must be GCC compatible) [default: %default]")
if target == 'macosx':
    oparser.add_option("--objc-compiler", action="store", type="string", dest="objc", default=objc, help="Objective-C compiler binary name (arguments must be GCC compatible) [default: %default]")
oparser.add_option("--bin-prefix", action="store", type="string", dest="program_prefix", default=program_prefix, help="executable program's prefix [default: %default]");
oparser.add_option("--bin-name", action="store", type="string", dest="program_name", default=program_name, help="executable program's name (note: proper suffix depending on the target platform is appended) [default: %default]")
oparser.add_option("--extra-cflags", action="store", type="string", dest="extra_cflags", default='', help="extra CFLAGS [default: %default]")
oparser.add_option("--extra-ldflags", action="store", type="string", dest="extra_ldflags", default='', help="extra LDFLAGS [default: %default]")
(options, args) = oparser.parse_args()

target = options.target
program_prefix = options.program_prefix
program_name = options.program_name
cc = options.cc
objc = options.objc
debug = not options.release

# common flags
if debug:
    common_cflags = '-Wall -g3'
    common_ldflags = '-g'
else:
    common_cflags = '-Wall -O3 -fomit-frame-pointer'
    if cc != 'clang':
        common_cflags = common_cflags + ' -ffast-math'
    common_ldflags = ''

# set platform-specific variables
exe_suffix = ''
sys_cflags = ''
sys_ldflags = ''
if target == 'win32':
    exe_suffix = '.exe'
    sys_ldflags = '-lSDL -lopengl32 -lglu32 -lgdi32'
elif target == 'macosx':
    sys_ldflags = '-framework OpenGL -framework SDL -framework Cocoa -framework Foundation'
else:
    sys_ldflags = '-lSDL -lGL -lGLU -lX11'

output_exe = program_prefix + program_name + exe_suffix
cflags = common_cflags + ' ' + sys_cflags + options.extra_cflags
ldflags = common_ldflags = ' ' + sys_ldflags + options.extra_ldflags

f = open("Makefile", "w")
f.write('# generated makefile, do not modify. Use configure.py instead.\n\n')
f.write('CC=' + cc + '\n')
f.write('OBJC=' + objc + '\n')
f.write('OUTPUT=' + output_exe + '\n')
f.write('CFLAGS=' + cflags + '\n')
f.write('LDFLAGS=' + ldflags + '\n')
f.write('SOURCES=$(wildcard src/*.c)\n')
f.write('OBJECTS=$(patsubst %.c,%.o,$(SOURCES))')
if target == 'macosx':
    f.write(' src/SDLMain.o')
f.write('\n')
f.write('''

HEADERS=$(wildcard src/*.h)

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -o $(OUTPUT) $(OBJECTS) $(LDFLAGS)

src/SDLMain.o: src/SDLMain.m $(HEADERS)
	$(OBJC) -ObjC $(CFLAGS) -o $@ -c $<
    
*.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<
	
.PHONY: clean
clean:
	$(RM) $(OUTPUT) $(OBJECTS)

''')
f.close()
