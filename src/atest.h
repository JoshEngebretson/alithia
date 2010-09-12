/*
 * Alithia Engine
 * Copyright (C) 2009 Kostas Michalopoulos
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Kostas Michalopoulos <badsector@runtimeterror.com>
 */

#ifndef __ATEST_H_INCLUDED__
#define __ATEST_H_INCLUDED__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif
#include <SDL/SDL.h>
#ifdef WIN32
#undef main
#endif

#include "lil.h"
#include "defines.h"
#include "utils.h"
#include "resio.h"
#include "mathstuff.h"
#include "argparse.h"
#include "vidmode.h"
#include "textures.h"
#include "fonts.h"
#include "gui.h"
#include "models.h"
#include "world.h"
#include "motion.h"
#include "screens.h"
#include "editor.h"
#include "script.h"
#include "aifuncs.h"

extern int running;
extern int argc;
extern char** argv;
extern int key[];
extern int button[];
extern float mouse_x;
extern float mouse_y;
extern screen_t* gamescreen;
extern int camx, camy;
extern int down;
extern float mouse_x;
extern float mouse_y;
extern int mouse_sx;
extern int mouse_sy;
extern float pla, pll, plfov;
extern float frustum[6][4];
extern float proj[16];
extern float modl[16];
extern float clip[16];
extern vector_t centeraya, centerayb;
extern int disable_occlusion;
extern entity_t* camera_ent;
extern float sensitivity;

void console_clear(void);
void console_write(const char* txt);
void console_newline(void);

void cell_vertices(cell_t* c, int x, int y, float* vx, float* vy, float* vz, int floor);
void map_update_cell(int x, int y);

void move_towards(float ix, float iy, float iz);

#endif
