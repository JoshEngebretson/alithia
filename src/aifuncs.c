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

#include "atest.h"

static list_t* ais;

static void ai_free(void* ptr)
{
    aidata_t* ai = ptr;
    free(ptr);
}

void ai_init(void)
{
    ais = list_new();
}

void ai_shutdown(void)
{
    list_free(ais);
}

aidata_t* ai_new(entity_t* ent)
{
    aidata_t* ai = new(aidata_t);
    list_add(ais, ai);
    ai->e = ent;
    ent->ai = ai;
    return ai;
}

void ai_release(aidata_t* ai)
{
    ai_free(ai);
}

static void update_ai_moving(aidata_t* ai, float ms)
{
    entity_t* ent = ai->e;
    if (SQR(ent->p.x - ai->move_target.x) + SQR(ent->p.z - ai->move_target.z) > ai->move_speed) {
        vector_t d;
        vec_makedir(&d, &ent->p, &ai->move_target);
        ent->yrot = atan2f(d.z, d.x)*180.0f/PI - 90;
        mot_const(ent->mot, d.x*ai->move_speed, 0/* d.y*ai->move_speed*/, d.z*ai->move_speed);
    } else {
        ent_move(ent, ai->move_target.x, ai->e->p.y /*ai->move_target.y*/, ai->move_target.z);
        mot_const(ent->mot, 0, 0, 0);
        ai->moving = 0;
        if (ent->event_mask & EVMASK_MOVE_TARGET_REACHED)
            ent_call_attr(ent, "move-target-reached");
    }
}

static void update_one_ai(aidata_t* ai, float ms)
{
    if (ai->moving) update_ai_moving(ai, ms);
}

void ai_update(float ms)
{
    listitem_t* li;
    for (li=ais->first; li; li=li->next)
        update_one_ai(li->ptr, ms);
}

void ai_move_to(entity_t* ent, float tx, float ty, float tz, float speed)
{
    aidata_t* ai = ent->ai;
    ai->moving = 1;
    ai->move_target.x = tx;
    ai->move_target.y = ty;
    ai->move_target.z = tz;
    ai->move_speed = speed;
}
