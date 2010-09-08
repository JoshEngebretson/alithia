#!/usr/bin/python

import os
import sys
import glob
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
if os.path.isdir('../lil'):
    lil_path = '../lil'
elif os.path.isdir('../../lil'):
    lil_path = '../../lil'
else:
    lil_path = './lil';
# parse arguments
oparser = OptionParser(version="configure script for Alithia Engine version 0.1")
oparser.add_option("--release", action="store_true", dest="release", default=False, help="use build options for releases (enable optimizations, disable debug info, etc)")
oparser.add_option("--optimize", action="store_true", dest="optimize", default=False, help="include optimization (like --release but does not remove debug info)")
oparser.add_option("--profiling", action="store_true", dest="profiling", default=False, help="use build options for profiling")
oparser.add_option("--target", action="store", type="string", dest="target", default=target, help="target platform [default: %default]")
oparser.add_option("--c-compiler", action="store", type="string", dest="cc", default=cc, help="C compiler binary name (arguments must be GCC compatible) [default: %default]")
if target == 'macosx':
    oparser.add_option("--objc-compiler", action="store", type="string", dest="objc", default=objc, help="Objective-C compiler binary name (arguments must be GCC compatible) [default: %default]")
oparser.add_option("--bin-prefix", action="store", type="string", dest="program_prefix", default=program_prefix, help="executable program's prefix [default: %default]");
oparser.add_option("--bin-name", action="store", type="string", dest="program_name", default=program_name, help="executable program's name (note: proper suffix depending on the target platform is appended) [default: %default]")
oparser.add_option("--extra-cflags", action="store", type="string", dest="extra_cflags", default='', help="extra CFLAGS [default: %default]")
oparser.add_option("--extra-ldflags", action="store", type="string", dest="extra_ldflags", default='', help="extra LDFLAGS [default: %default]")
oparser.add_option("--lil-path", action="store", type="string", dest="lil_path", default=lil_path, help="path to the LIL source code [default: %default]");
(options, args) = oparser.parse_args()

target = options.target
program_prefix = options.program_prefix
program_name = options.program_name
cc = options.cc
if target == 'macosx':
    objc = options.objc
debug = not options.release
profiling = options.profiling
optimize = options.optimize
lil_path = options.lil_path

# check paths
if not os.path.isdir(lil_path):
    print("The LIL path '" + lil_path + "' is not a directory");
    sys.exit()
if not os.path.isfile(lil_path + "/liblil.a"):
    print("The LIL library 'liblil.a' was not found in '" + lil_path + "'");
    sys.exit()
if not os.path.isfile(lil_path + "/lil.h"):
    print("The LIL header file 'lil.h' was not found in '" + lil_path + "'");
    sys.exit()

# common flags
if profiling and cc == 'clang':
    cc = 'gcc'
if debug:
    common_cflags = '-m32 -Wall -g3 -I' + lil_path
    if (not optimize):
        common_cflags = common_cflags + ' -O0'
    common_ldflags = '-m32 -g -L' + lil_path + ' -llil'
else:
    common_cflags = '-m32 -Wall -fomit-frame-pointer -I' + lil_path
    common_ldflags = '-m32 -L' + lil_path + ' -llil'
if optimize or (not debug):
    if cc != 'clang':
        common_cflags = common_cflags + ' -O3 -ffast-math'
	if target != 'win32':
		common_cflags = common_cflags + ' -ftree-vectorize'
    else:
        common_cflags = common_cflags + ' -O4'
if profiling:
    common_cflags = common_cflags + ' -pg'
    common_ldflags = common_ldflags + ' -pg'

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
cflags = common_cflags + ' ' + sys_cflags + ' ' + options.extra_cflags
ldflags = common_ldflags + ' ' + sys_ldflags + ' ' + options.extra_ldflags

f = open("Makefile", "w")
f.write('# generated makefile, do not modify. Use configure.py instead.\n\n')
f.write('CC=' + cc + '\n')
if target == 'macosx':
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

''')
if target == 'macosx':
    f.write('''
src/SDLMain.o: src/SDLMain.m $(HEADERS)
	$(OBJC) -ObjC $(CFLAGS) -o $@ -c $<
''')

for cfile in glob.glob("src/*.c"):
    ofile = cfile.replace('.c', '.o')
    f.write(ofile + ": " + cfile + " $(HEADERS)\n\t$(CC) $(CFLAGS) -o $@ -c $<\n")
f.write('''
.PHONY: clean
clean:
	$(RM) $(OUTPUT) $(OBJECTS)

''')
f.close()
