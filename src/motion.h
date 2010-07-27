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

#ifndef __MOTION_H_INCLUDED__
#define __MOTION_H_INCLUDED__

typedef struct _motion_t
{
    entity_t* ent;
    vector_t f; /* accumulated force */
    vector_t c; /* constant motion */
    vector_t g; /* gravity */
    float airf; /* air friction multiplier */
    float wldf; /* world friction multiplier */
    float bncf; /* bounce multiplier */
    float frf; /* force receive factor */
    float fdf; /* force damp factor */
    float xff; /* extra force factor */
    float slideup; /* slide up units */
    int slideupc; /* slide up count */
    int sliding;
    int frozen;
    int frozctr;
    int onground;
} motion_t;

void mot_init(void);
void mot_shutdown(void);
void mot_reset(void);
motion_t* mot_new(entity_t* ent);
void mot_free(motion_t* mot);
void mot_for_char(motion_t* mot);
void mot_force(motion_t* mot, float x, float y, float z);
void mot_const(motion_t* mot, float x, float y, float z);
void mot_freeze(motion_t* mot);
void mot_unfreeze(motion_t* mot);
void mot_update(float ms);

#endif
