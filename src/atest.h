#ifndef __ATEST_H_INCLUDED__
#define __ATEST_H_INCLUDED__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#ifdef main
#undef main
#endif

extern int running;
extern int argc;
extern char** argv;

#include "defines.h"
#include "utils.h"
#include "argparse.h"
#include "vidmode.h"
#include "textures.h"
#include "models.h"
#include "world.h"

#endif
