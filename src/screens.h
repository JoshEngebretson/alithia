/*
 * Alithia Engine
 * Copyright (C) 2009-2010 Kostas Michalopoulos
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

#ifndef __SCREENS_H_INCLUDED__
#define __SCREENS_H_INCLUDED__

#define SMS_DESTROY 0
#define SMS_INIT 1
#define SMS_ENTER 2
#define SMS_EXIT 3
#define SMS_SDL_EVENT 4
#define SMS_RENDER 5
#define SMS_UPDATE 6

typedef struct _screen_t
{
    int do_clear;
    int draw_gui;
    int draw_world;
    int draw_mouse;
    void* proc;
} screen_t;

typedef int (*screenproc_t)(struct _screen_t* scr, int msg, void* data);

extern screen_t* active_screen;

screen_t* screen_new(screenproc_t proc, void* data);
void screen_free(screen_t* scr);
void screen_set(screen_t* scr);
void screen_send(screen_t* scr, int msg, void* data);

#endif
