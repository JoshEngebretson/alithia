#!/usr/bin/python

import os
import sys

# build configuration
prefix = './'
program_name = 'atest'

# find platform
target = sys.platform
if target == 'darwin':
    target = 'macosx'

# common flags
#common_cflags = '-Wall -O3 -fomit-frame-pointer -ffast-math'
common_cflags = '-Wall -g3 -fomit-frame-pointer -ffast-math'
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

output_exe = prefix + program_name + exe_suffix
cflags = common_cflags + ' ' + sys_cflags
ldflags = common_ldflags = ' ' + sys_ldflags

f = open("Makefile", "w")
f.write('# generated makefile, do not modify. Use configure.py instead.\n\n')
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
	gcc -ObjC $(CFLAGS) -o $@ -c $<
    
*.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<
	
.PHONY: clean
clean:
	$(RM) $(OUTPUT) $(OBJECTS)

''')
f.close()