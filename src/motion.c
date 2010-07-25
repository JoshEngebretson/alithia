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

#define CT_AIR 0
#define CT_WORLD 1
#define CT_ENTITY 2

typedef struct _collcheck_t
{
    int type;
    aabb_t aabb;
    motion_t* mot;
    entity_t* ent;
    entity_t* other_ent; /* for CT_ENTITY */
    vector_t np;
    vector_t nf;
    int bounce;
} collcheck_t;

static list_t* motions;

void mot_init(void)
{
    motions = list_new();
}

void mot_shutdown(void)
{
    while (motions->first)
        mot_free(motions->first->ptr);
    list_free(motions);
}

void mot_reset(void)
{
    mot_shutdown();
    mot_init();
}

motion_t* mot_new(entity_t* ent)
{
    motion_t* mot = new(motion_t);
    mot->ent = ent;
    mot->airf = 0.98f;
    mot->bncf = 0.1f;
    mot->g.y = -8;
    mot->frf = 0.45f;
    mot->fdf = 0.45f;
    mot->frif = 0.95f;
    ent->mot = mot;
    list_add(motions, mot);
    return mot;
}

void mot_free(motion_t* mot)
{
    mot->ent->mot = NULL;
    free(mot);
    list_remove(motions, mot, 0);
}

void mot_for_char(motion_t* mot)
{
    mot->bncf = 0;
    mot->frf = 0;
    mot->xff = 1;
    mot->slideup = 4;
    mot->slideupc = 10;
}

void mot_force(motion_t* mot, float x, float y, float z)
{
    mot->f.x += x;
    mot->f.y += y;
    mot->f.z += z;
    if (mot->frozen) mot_unfreeze(mot);
}

static int collision_world_check(collcheck_t* ct)
{
    int x, y, x1, y1, x2, y2;
    x1 = ct->aabb.min.x/CELLSIZE;
    y1 = ct->aabb.min.z/CELLSIZE;
    x2 = ct->aabb.max.x/CELLSIZE;
    y2 = ct->aabb.max.z/CELLSIZE;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= map_width) x2 = map_width - 1;
    if (y2 >= map_height) y2 = map_height - 1;
    for (y=y1; y<=y2; y++) for (x=x1; x<=x2; x++) {
        cell_t* c = cell + y*map_width + x;
        if (c->flags & CF_OCCLUDER) {
            ct->type = CT_WORLD;
            return 1;
        }
        if (ct->aabb.min.y < c->floorz || ct->aabb.max.y > c->ceilz) {
            ct->type = CT_WORLD;
            return 1;
        }
    }
    return 0;
}

static int collision_entity_in_cluster_check(collcheck_t* ct, int cc, int cr)
{
    cluster_t* cls;
    listitem_t* li;
    if (cc < 0 || cr < 0 || cc >= cluster_width || cr >= cluster_height) return 0;
    cls = cluster + cr*cluster_width + cc;
    for (li=cls->ents->first; li; li=li->next) {
        entity_t* other = li->ptr;
        if (other == ct->ent) continue;
        if (aabb_aabb_collision(&ct->aabb, &other->aabb)) {
            ct->type = CT_ENTITY;
            ct->other_ent = other;
            if (other->mot && ct->bounce) {
                motion_t* omot = other->mot;
                float ff = omot->frf > 0.0001f ? omot->frf + ct->mot->xff : 0.0f;
                mot_force(omot,
                    (ct->nf.x + ct->mot->c.x)*ff,
                    (ct->nf.y + ct->mot->c.y)*ff,
                    (ct->nf.z + ct->mot->c.z)*ff);
            }
            return 1;
        }
    }
    return 0;
}

static int collision_entity_check(collcheck_t* ct)
{
    int cc = ((int)ct->np.x)/(CLUSTERSIZE*CELLSIZE);
    int cr = ((int)ct->np.z)/(CLUSTERSIZE*CELLSIZE);
    int x, y;
    /* TODO: use the following code once entity_t has a list of touching clusters
    int x1 = ((int)ct->aabb.min.x)/(CLUSTERSIZE*CELLSIZE);
    int y1 = ((int)ct->aabb.min.z)/(CLUSTERSIZE*CELLSIZE);
    int x2 = ((int)ct->aabb.max.x)/(CLUSTERSIZE*CELLSIZE);
    int y2 = ((int)ct->aabb.max.z)/(CLUSTERSIZE*CELLSIZE);
    for (x=x1; x<=x2; x++)
        for (y=y1; y<=y2; y++)
            if (collision_entity_in_cluster_check(ct, x, y)) {
                ct->type = CT_ENTITY;
                return 1;
            }*/
#define check(xx,xy) \
    if (collision_entity_in_cluster_check(ct, xx, xy)) { \
        ct->type = CT_ENTITY; \
        return 1; \
    } \

    check(cc, cr)
    check(cc - 1, cr)
    check(cc + 1, cr)
    check(cc, cr - 1)
    check(cc, cr + 1)
    check(cc - 1, cr - 1)
    check(cc + 1, cr - 1)
    check(cc - 1, cr + 1)
    check(cc + 1, cr + 1)

#undef check
    return 0;
}

static int collision_check(collcheck_t* ct)
{
    ct->aabb.min.x = ct->np.x + (ct->ent->aabb.min.x - ct->ent->p.x);
    ct->aabb.min.y = ct->np.y + (ct->ent->aabb.min.y - ct->ent->p.y);
    ct->aabb.min.z = ct->np.z + (ct->ent->aabb.min.z - ct->ent->p.z);
    ct->aabb.max.x = ct->np.x + (ct->ent->aabb.max.x - ct->ent->p.x);
    ct->aabb.max.y = ct->np.y + (ct->ent->aabb.max.y - ct->ent->p.y);
    ct->aabb.max.z = ct->np.z + (ct->ent->aabb.max.z - ct->ent->p.z);
    ct->type = CT_AIR;
    if (collision_world_check(ct)) return ct->type;
    if (collision_entity_check(ct)) return ct->type;
    return 0;
}

static void update_motion(float ms, motion_t* mot)
{
    collcheck_t ct;
    float factor = 1.0f, s = ms/100.0f, frv, save;
    entity_t* ent = mot->ent;
    int i, j, db;
    vector_t op;
    if (mot->frozen) return;

    ct.ent = ent;
    ct.mot = mot;
    ct.other_ent = NULL;
    ct.nf = mot->f;

    ct.np = ent->p;

    for (i=0; i<3; i++,factor *= 0.5f) {
        ct.np.x = ent->p.x + (mot->c.x + mot->f.x)*s*factor;
        ct.bounce = i == 0;
        if (!collision_check(&ct)) break;
        ct.bounce = 0;
        if (!i) {
            db = 0;
            save = ct.np.y;
            for (j=0; j<mot->slideupc; j++) {
                ct.np.y += mot->slideup;
                if (!collision_check(&ct)) {
                    db = 1;
                    mot->sliding = 1;
                    break;
                }
            }
            ct.np.y = save;
            if (db) break;
        }
        ct.np.x = ent->p.x;
        if (!i) {
            ct.nf.x = -mot->bncf*ct.nf.x;
            ct.nf.y *= mot->frif;
            ct.nf.z *= mot->frif;
            if (ct.type == CT_ENTITY) ct.nf.x *= mot->fdf;
        }
    }
    factor = 1.0;
    for (i=0; i<3; i++,factor *= 0.5f) {
        ct.np.y = ent->p.y + (mot->c.y + mot->f.y)*s*factor;
        ct.bounce = i == 0;
        if (!collision_check(&ct)) {
            mot->sliding = 0;
            break;
        }
        mot->onground = 1;
        if (mot->sliding) {
            ct.np.y += mot->slideup;
            break;
        }
        ct.np.y = ent->p.y;
        if (!i) {
            ct.nf.y = -mot->bncf*ct.nf.y;
            ct.nf.x *= mot->frif;
            ct.nf.z *= mot->frif;
            if (ct.type == CT_ENTITY) ct.nf.y *= mot->fdf;
        }
    }
    factor = 1.0;
    for (i=0; i<3; i++,factor *= 0.5f) {
        ct.np.z = ent->p.z + (mot->c.z + mot->f.z)*s*factor;
        ct.bounce = i == 0;
        if (!collision_check(&ct)) break;
        ct.bounce = 0;
        if (!i) {
            db = 0;
            save = ct.np.y;
            for (j=0; j<mot->slideupc; j++) {
                ct.np.y += mot->slideup;
                if (!collision_check(&ct)) {
                    db = 1;
                    mot->sliding = 1;
                    break;
                }
            }
            ct.np.y = save;
            if (db) break;
        }
        ct.np.z = ent->p.z;
        if (!i) {
            ct.nf.z = -mot->bncf*ct.nf.z;
            ct.nf.x *= mot->frif;
            ct.nf.y *= mot->frif;
            if (ct.type == CT_ENTITY) ct.nf.z *= mot->fdf;
        }
    }

    op = ent->p;

    ent_move(ent, ct.np.x, ct.np.y, ct.np.z);

    frv = mot->airf + (1.0-mot->airf)*s;
    mot->f.x = ct.nf.x*frv + mot->g.x*s;
    mot->f.y = ct.nf.y*frv + (mot->sliding ? 0 : (mot->g.y*s));
    mot->f.z = ct.nf.z*frv + mot->g.z*s;

    if (vec_distsq(&ent->p, &op) < 0.1f) {
        if ((--mot->frozctr) <= 0)
            mot_freeze(mot);
    } else mot->frozctr = 4;
}

void mot_const(motion_t* mot, float x, float y, float z)
{
    mot->c.x = x;
    mot->c.y = y;
    mot->c.z = z;
    if (mot->frozen) mot_unfreeze(mot);
}

void mot_freeze(motion_t* mot)
{
    vec_set(&mot->f, 0.0f, 0.0f, 0.0f);
    vec_set(&mot->c, 0.0f, 0.0f, 0.0f);
    mot->frozen = 1;
    mot->frozctr = 0;
}

void mot_unfreeze(motion_t* mot)
{
    mot->frozen = 0;
    mot->frozctr = 4;
}

void mot_update(float ms)
{
    listitem_t* li;
    for (li=motions->first; li; li=li->next)
        update_motion(ms, li->ptr);
}
