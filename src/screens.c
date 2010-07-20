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

#include "atest.h"

screen_t* active_screen;

screen_t* screen_new(screenproc_t proc, void* data)
{
    screen_t* scr = calloc(1, sizeof(screen_t));
    scr->proc = proc;
    screen_send(scr, SMS_INIT, data);
    return scr;
}

void screen_free(screen_t* scr)
{
    screen_send(scr, SMS_DESTROY, NULL);
    free(scr);
}

screen_t* screen_set(screen_t* scr)
{
    screen_t* prev = active_screen;
    if (active_screen)
        screen_send(active_screen, SMS_EXIT, NULL);
    active_screen = scr;
    screen_send(scr, SMS_ENTER, NULL);
    return prev;
}

void screen_send(screen_t* scr, int msg, void* data)
{
    ((screenproc_t)scr->proc)(scr, msg, data);
}
