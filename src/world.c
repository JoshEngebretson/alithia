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

cell_t* cell;
char* ocmap;
cluster_t* cluster;
list_t* ents;
list_t* lights;
int map_width;
int map_height;
int cluster_width;
int cluster_height;
lmap_texel_t* lightmap;
GLuint lmaptex;
int lmap_quality = 2;
entity_t* player_ent;
int world_init_frames = 10;

model_t* mdl_vytio;
model_t* mdl_armor;

void map_init(int width, int height)
{
    int i, x, y;
    texture_t* tex_bricks;
    texture_t* tex_floor;

    map_width = width;
    map_height = height;
    cluster_width = width/CLUSTERSIZE;
    cluster_height = height/CLUSTERSIZE;

    cell = calloc(width*height, sizeof(cell_t));
    ocmap = calloc(1, width*height);
    cluster = calloc(cluster_width*cluster_height, sizeof(cluster_t));

    mot_reset();

    ents = list_new();
    ents->item_free = (void*)ent_free;

    lights = list_new();
    lights->item_free = (void*)light_free;

    i = 0;
    for (y=0; y<cluster_height; y++) {
        for (x=0; x<cluster_width; x++) {
            cluster[i].ents = list_new();
            cluster[i].aabb.min.x = x*CELLSIZE*CLUSTERSIZE;
            cluster[i].aabb.min.z = y*CELLSIZE*CLUSTERSIZE;
            cluster[i].aabb.max.x = (x + 1)*CELLSIZE*CLUSTERSIZE;
            cluster[i].aabb.max.z = (y + 1)*CELLSIZE*CLUSTERSIZE;
            i++;
        }
    }

    tex_bricks = texbank_get("redbricks");
    tex_floor = texbank_get("floorbrick");
    for (i=0; i<width*height; i++) {
        cell[i].floorz = -128;
        cell[i].ceilz = 128;
        cell[i].uppertex = tex_bricks;
        cell[i].lowetex = tex_bricks;
        cell[i].bottomtex = tex_floor;
        cell[i].toptex = tex_bricks;
    }

    for (i=0; i<width; i++) {
        cell[i].flags = cell[(height-1)*width + i].flags = CF_OCCLUDER;
        cell[i].ceilz = cell[(height-1)*width + i].ceilz = 0;
        cell[i].floorz = cell[(height-1)*width + i].floorz = 0;
    }
    for (i=0; i<height; i++) {
        cell[i*width].flags = cell[i*width + height - 1].flags = CF_OCCLUDER;
        cell[i*width].ceilz = cell[i*width + height - 1].ceilz = 0;
        cell[i*width].floorz = cell[i*width + height - 1].floorz = 0;
    }

    lightmap = calloc(map_width*map_height, sizeof(lmap_texel_t));
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &lmaptex);
    glBindTexture(GL_TEXTURE_2D, lmaptex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map_width, map_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightmap);

    player_ent = ent_new();
    camera_ent = player_ent;
    ent_move(player_ent, map_width*CELLSIZE*0.5, -128+48, map_height*CELLSIZE*0.5);
    {
        motion_t* mot = mot_new(player_ent);
        mot_for_char(mot);
        vec_set(&player_ent->laabb.min, -16, -48, -16);
        vec_set(&player_ent->laabb.max, 16, 16, 16);
    }

    for (y=0; y<map_height; y++)
        for (x=0; x<map_width; x++)
            map_update_cell(x, y);

    world_init_frames = 10;

    plfov = arg_intval("-fov", 66);
}

void map_free(void)
{
    int i;
    glBindTexture(GL_TEXTURE_2D, lmaptex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glDeleteTextures(1, &lmaptex);
    list_free(lights);
    list_free(ents);
    for (i=0; i<cluster_width*cluster_height; i++) {
        int j;
        for (j=0; j<cluster[i].parts; j++)
            glDeleteLists(cluster[i].part[j].dl, 1);
        free(cluster[i].decal);
        free(cluster[i].part);
        free(cluster[i].tri);
        list_free(cluster[i].ents);
    }
    free(cluster);
    free(ocmap);
    free(cell);
    player_ent = NULL;
}

static void invalidate_cluster(int cx, int cy)
{
    cluster_t* clus = cluster + cy*cluster_width + cx;
    int i;
    if (clus->part) {
        for (i=0; i<clus->parts; i++)
            glDeleteLists(clus->part[i].dl, 1);
        free(clus->part);
        clus->part = NULL;
        clus->parts = 0;
    }
    if (clus->tris) {
        free(clus->tri);
        clus->tri = NULL;
        clus->tris = 0;
    }
    clus->visible = 0;
}

void map_update_cell(int x, int y)
{
    int cx = x/CLUSTERSIZE;
    int cy = y/CLUSTERSIZE;
    int addr;

    invalidate_cluster(cx, cy);

    if (cell[y*map_width + x].floorz == cell[y*map_width +x ].ceilz)
        cell[y*map_width + x].flags |= CF_OCCLUDER;
    else
        cell[y*map_width + x].flags &= ~CF_OCCLUDER;

    addr = y*map_width + x;
    if (cell[y*map_width + x].flags & CF_OCCLUDER)
        ocmap[(addr>>3)] |= 1 << (addr&7);
    else
        ocmap[(addr>>3)] &= ~(1 << (addr&7));

    /* check for edges */
    if (x > 0 && (x - 1)/CLUSTERSIZE != cx) invalidate_cluster(cx - 1, cy);
    if (y > 0 && (y - 1)/CLUSTERSIZE != cy) invalidate_cluster(cx, cy - 1);
    if (x < map_width - 1 && (x + 1)/CLUSTERSIZE != cx) invalidate_cluster(cx + 1, cy);
    if (y < map_height - 1 && (y + 1)/CLUSTERSIZE != cy) invalidate_cluster(cx, cy + 1);
}

void ray_march(int x1, int y1, int x2, int y2, int (*func)(int x, int y, cell_t* cell, void* data), void* data)
{
    int x, y, dx, dy, steep = abs(y2 - y1) > abs(x2 - x1), ystep = 1, error;
    if (steep) {
        x = x1;
        x1 = y1;
        y1 = x;
        x = x2;
        x2 = y2;
        y2 = x;
    }
    if (x1 > x2) {
        dx = x1 - x2;
        dy = abs(y2 - y1);
        error = dx >> 1;
        if (y1 < y2) ystep = -1;
        y = y1;
        if (steep) {
            for (x = x1; x > x2; x--) {
                if (!func(y, x, cell + (x*map_width + y), data)) return;
                error -= dy;
                if (error < 0) {
                    y -= ystep;
                    error += dx;
                }
            }
        } else {
            for (x = x1; x > x2; x--) {
                if (!func(x, y, cell + (y*map_width + x), data)) return;
                error -= dy;
                if (error < 0) {
                    y -= ystep;
                    error += dx;
                }
            }
        }
    } else {
        dx = x2 - x1;
        dy = abs(y2 - y1);
        error = dx >> 1;
        if (y2 < y1) ystep = -1;
        y = y1;
        if (steep) {
            for (x = x1; x < x2; x++) {
                if (!func(y, x, cell + (x*map_width + y), data)) return;
                error -= dy;
                if (error < 0) {
                    y += ystep;
                    error += dx;
                }
            }
        } else {
            for (x = x1; x < x2; x++) {
                if (!func(x, y, cell + (y*map_width + x), data)) return;
                error -= dy;
                if (error < 0) {
                    y += ystep;
                    error += dx;
                }
            }
        }
    }
}

static int pick_rm_func(int x, int y, cell_t* cell, void* data)
{
    pickdata_t* pd = data;
    size_t i;
    int cx, cy;
    cluster_t* cls;
    vector_t ip;
    listitem_t* li;
    float ipad;
    if (x < 0 || y < 0 || x >= map_width || y >= map_height) return 0;
    cx = x/CLUSTERSIZE;
    cy = y/CLUSTERSIZE;
    cls = &cluster[cy*cluster_width + cx];
#ifdef DEBUG_DRAW_PICK
    glColor3f(0, 0, 1);
    glBegin(GL_QUADS);
    glVertex3f(x*CELLSIZE, -126, y*CELLSIZE);
    glVertex3f(x*CELLSIZE + CELLSIZE, -126, y*CELLSIZE);
    glVertex3f(x*CELLSIZE + CELLSIZE, -126, y*CELLSIZE + CELLSIZE);
    glVertex3f(x*CELLSIZE, -126, y*CELLSIZE + CELLSIZE);
    glEnd();
#endif
    if (cls == pd->lastcluster) return 1;
    pd->lastcluster = cls;
    /* check world triangles */
    for (i=0; i<cls->tris; i++) {
        if (ray_tri_intersection(cls->tri + i, pd->a, pd->b, &ip, 0)) {
            ipad = vec_distsq(pd->a, &ip);
            if (pd->nopick || ipad < pd->ipad) {
                pd->nopick = 0;
                pd->ip = ip;
                pd->ipad = ipad;
                pd->cluster_x = cx;
                pd->cluster_y = cy;
                pd->cluster = cls;
                pd->cell = &cell[y*map_width + x];
                pd->cellx = x;
                pd->celly = y;
                pd->result = PICK_WORLD;
                pd->tri = cls->tri[i];
            }
        }
    }
    /* check entities */
    for (li=cls->ents->first; li; li=li->next) {
        entity_t* ent = li->ptr;
        if (ent == pd->ignore_ent) continue;
        if (ray_aabb_intersection(&ent->aabb, pd->a, pd->b, &ip)) {
            ipad = vec_distsq(pd->a, &ip);
            if (pd->nopick || ipad < pd->ipad) {
                pd->nopick = 0;
                pd->ip = ip;
                pd->ipad = ipad;
                pd->cluster_x = cx;
                pd->cluster_y = cy;
                pd->cluster = cls;
                pd->cell = &cell[y*map_width + x];
                pd->cellx = x;
                pd->celly = y;
                pd->result = PICK_ENTITY;
                pd->entity = ent;
            }
        }
    }
    return 1;
}

int pick(vector_t* a, vector_t* b, pickdata_t* pd, int flags)
{
    pd->a = a;
    pd->b = b;
    pd->nopick = 1;
    pd->lastcluster = NULL;
    pd->pickflags = flags;
    /*{
        int x, y;
        for (y=0; y<map_height; y++)
            for (x=0; x<map_width; x++)
                pick_rm_func(x, y, &cell[y*map_width + x], pd);
    }*/
    /* check world and entities */
    ray_march(a->x/CELLSIZE, a->z/CELLSIZE, b->x/CELLSIZE, b->z/CELLSIZE, pick_rm_func, pd);
    /* check lights */
    if (flags & PICKFLAG_PICK_LIGHTS) {
        listitem_t* li;
        for (li=lights->first; li; li=li->next) {
            light_t* l = li->ptr;
            vector_t ip;
            if (ray_sphere_intersection(&l->p, 16, a, b, &ip)) {
                float ipad = vec_distsq(a, &ip);
                if (ipad < pd->ipad) {
                    pd->result = PICK_LIGHT;
                    pd->light = l;
                    pd->ip = ip;
                    pd->ipad = vec_distsq(a, &ip);
                }
            }
        }
    }
    if (pd->result) return pd->result;
    pd->result = 0;
    return pd->result;
}

static int light_vfc_proc(int x, int y, cell_t* cell, void* data)
{
    int* flag = data;
    if (cell->flags & CF_OCCLUDER) {
        *flag = 0;
        return 0;
    }
    return 1;
}

static int light_visible_from_cell(light_t* l, int cx, int cy)
{
    int r = 1;
    ray_march(((int)l->p.x)/CELLSIZE, ((int)l->p.z)/CELLSIZE, cx, cy, light_vfc_proc, &r);
    return r;
}

static void calc_lightmap_for_cell_at(float* or, float* og, float* ob, int mx, int my)
{
    cell_t* c = cell + my*map_width + mx;
    float r = 0.05, g = 0.05, b = 0.05;
    float cx = mx*CELLSIZE + (CELLSIZE/2.0);
    float cz = my*CELLSIZE + (CELLSIZE/2.0);
    int x, y, x1, y1, x2, y2;
    listitem_t* le;

    if (c->toptex == NULL) { /* open sky */
        r += 0.5;
        g += 0.5;
        b += 0.5;
    } else { /* near open sky */
        if ((mx > 0 && !cell[my*map_width + mx - 1].toptex) ||
            (mx < map_width - 1 && !cell[my*map_width + mx + 1].toptex) ||
            (my > 0 && !cell[(my - 1)*map_width + mx].toptex) ||
            (my < map_height - 1 && !cell[(my + 1)*map_width + mx].toptex)) {
            r += 0.25;
            g += 0.25;
            b += 0.25;
        }
    }

    for (le=lights->first; le; le=le->next) {
        light_t* l = le->ptr;
        float dist;
        dist = sqrt(SQR(cx - l->p.x) + SQR(cz - l->p.z));
        if (dist < l->rad) {
            if (!light_visible_from_cell(l, mx, my)) continue;
            if (dist < l->rad*0.65f) {
                r += l->r;
                g += l->g;
                b += l->b;
            } else {
                float v = 1.0 - (dist - l->rad*0.65f)/(l->rad*0.35f);
                r += l->r*v;
                g += l->g*v;
                b += l->b*v;
            }
        }
    }

    if (lmap_quality == 2) {
        x1 = mx - 2; if (x1 < 0) x1 = 0;
        y1 = my - 2; if (y1 < 0) y1 = 0;
        x2 = mx + 2; if (x2 >= map_width) x2 = map_width - 1;
        y2 = my + 2; if (y2 >= map_height) y2 = map_height - 1;
        for (y=y1; y<=y2; y++) {
            for (x=x1; x<=x2; x++) {
                cell_t* nc = cell + y*map_width + x;
                if (nc->floorz > c->floorz + CELLSIZE || nc->ceilz < c->ceilz - CELLSIZE || (nc->flags & CF_OCCLUDER)) {
//                    r -= 0.003;
//                    g -= 0.003;
//                    b -= 0.003;
                }
            }
        }
    }

    if (r < 0.0) r = 0.0; else if (r > 1.0) r = 1.0;
    if (g < 0.0) g = 0.0; else if (g > 1.0) g = 1.0;
    if (b < 0.0) b = 0.0; else if (b > 1.0) b = 1.0;

    *or = r;
    *og = g;
    *ob = b;
}

static void calc_lightmap_for_cell(lmap_texel_t* lm, int mx, int my)
{
    int x, y, x1, y1, x2, y2, n = 0;
    float r, g, b, nr, ng, nb;

    if (lmap_quality > 0) {
        x1 = mx - 1; if (x1 < 0) x1 = 0;
        y1 = my - 1; if (y1 < 0) y1 = 0;
        x2 = mx + 1; if (x2 >= map_width) x2 = map_width - 1;
        y2 = my + 1; if (y2 >= map_height) y2 = map_height - 1;

        r = g = b = 0.0f;

        for (y=y1; y<=y2; y++) {
            for (x=x1; x<=x2; x++) {
                calc_lightmap_for_cell_at(&nr, &ng, &nb, x, y);
                r += nr;
                g += ng;
                b += nb;
                n++;
            }
        }

        r /= (float)n;
        g /= (float)n;
        b /= (float)n;
    } else {
        calc_lightmap_for_cell_at(&r, &g, &b, mx, my);
    }

    lm->r = (int)(255.0*r);
    lm->g = (int)(255.0*g);
    lm->b = (int)(255.0*b);
    lm->x = 255;
}

void lmap_update(void)
{
    int mx, my;
    for (my=0; my<map_height; my++) {
        for (mx=0; mx<map_width; mx++) {
            calc_lightmap_for_cell(lightmap + my*map_width + mx, mx, my);
        }
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lmaptex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map_width, map_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightmap);
}

light_t* light_new(float x, float y, float z, float r, float g, float b, float rad)
{
    light_t* light = new(light_t);
    light->p.x = x;
    light->p.y = y;
    light->p.z = z;
    light->r = r;
    light->g = g;
    light->b = b;
    light->rad = rad;
    list_add(lights, light);
    return light;
}

void light_free(light_t* light)
{
    or_deleted(light);
    list_remove(lights, light, 0);
    free(light);
}

decal_t* decal_new(cluster_t* clus, vector_t* p, texture_t* tex)
{
    clus->decal = realloc(clus->decal, sizeof(decal_t)*(clus->decals + 1));
    memcpy(clus->decal[clus->decals].p, p, sizeof(vector_t)*4);
    clus->decal[clus->decals++].tex = tex;
    return &clus->decal[clus->decals - 1];
}

static void entity_attr_free(void* ptr)
{
    entity_attr_t* attr = ptr;
    lil_free_value(attr->name);
    lil_free_value(attr->value);
    free(attr);
}

entity_t* ent_new(void)
{
    entity_t* ent = new(entity_t);
    list_add(ents, ent);
    ent->attrs = list_new();
    ent->attrs->item_free = entity_attr_free;
    return ent;
}

void ent_free(entity_t* ent)
{
    or_deleted(ent);
    editor_entity_deleted(ent);
    free(ent->class);
    if (ent->ai) ai_release(ent->ai);
    if (ent->mot) mot_free(ent->mot);
    list_free(ent->attrs);
    if (ent->clus) list_remove(ent->clus->ents, ent, 0);
    list_remove(ents, ent, 0);
    free(ent);
}

entity_t* ent_clone(entity_t* ent)
{
    entity_t* clone;
    list_t* save_attrs;
    motion_t* save_mot;
    aidata_t* save_ai;
    listitem_t* li;
    if (ent->class)
        clone = ent_new_by_class(ent->class);
    else
        clone = ent_new();
    save_mot = clone->mot;
    save_ai = clone->ai;
    save_attrs = clone->attrs;
    *clone = *ent;
    if (clone->class)
        clone->class = strdup(clone->class);
    clone->clus = NULL;
    clone->attrs = save_attrs;
    for (li=ent->attrs->first; li; li=li->next) {
        entity_attr_t* attr = li->ptr;
        ent_set_attr(clone, attr->name, attr->value);
    }
    clone->mot = save_mot;
    clone->ai = save_ai;
    ent_update(clone);
    return clone;
}

void ent_update(entity_t* ent)
{
    int cc = ((int)ent->p.x)/(CLUSTERSIZE*CELLSIZE);
    int cr = ((int)ent->p.z)/(CLUSTERSIZE*CELLSIZE);
    cluster_t* clus = cluster + cr*cluster_width + cc;
    float* mtx = ent->mtx;
    float c = cosf(ent->yrot*PI/180.0);
    float s = sinf(ent->yrot*PI/180.0);

    if (ent->clus != clus) {
        if (ent->clus) list_remove(ent->clus->ents, ent, 0);
        list_add(clus->ents, ent);
        ent->clus = clus;
    }

    if (ent->mdl)
        aabb_copytrans(&ent->aabb, &ent->mdl->aabb, ent->p.x, ent->p.y, ent->p.z);
    else
        aabb_copytrans(&ent->aabb, &ent->laabb, ent->p.x, ent->p.y, ent->p.z);

    if (ent->mot)
        mot_unfreeze(ent->mot);

    mtx[0] = c; mtx[4] = 0; mtx[8] = -s;mtx[12] = ent->p.x;
    mtx[1] = 0; mtx[5] = 1; mtx[9] = 0; mtx[13] = ent->p.y;
    mtx[2] = s; mtx[6] = 0; mtx[10]= c; mtx[14] = ent->p.z;
    mtx[3] = 0; mtx[7] = 0; mtx[11]= 0; mtx[15] = 1;
}

void ent_set_model(entity_t* ent, model_t* mdl)
{
    ent->mdl = mdl;
    ent->frame = 0;
    ent->frames = ent->mdl->frames;
    ent->framedur = 50;
    ent->start_frame = 0;
    ent->end_frame = ent->frames - 1;
    ent->durstate = 0;
    ent_update(ent);
}

static int aabb_below_aabb(aabb_t* a, aabb_t* b)
{
    return fabsf(a->min.y < b->min.y) &&
           fabsf((b->min.x + b->max.x) - (a->min.x + a->max.x)) < ((a->max.x - a->min.x) + (b->max.x - b->min.x)) &&
           fabsf((b->min.z + b->max.z) - (a->min.z + a->max.z)) < ((a->max.z - a->min.z) + (b->max.z - b->min.z));
}

void ent_move(entity_t* ent, float x, float y, float z)
{
    listitem_t* li;
    int cc, cr;
    cluster_t* clus;
    if (ent->p.x == x && ent->p.y == y && ent->p.z == z) return;
    if (x < 0) x = 0; else if (x > (map_width - 1)*CELLSIZE) x = (map_width - 1)*CELLSIZE;
    if (z < 0) z = 0; else if (z > (map_height - 1)*CELLSIZE) z = (map_height - 1)*CELLSIZE;
    ent->p.x = x;
    ent->p.y = y;
    ent->p.z = z;
    ent_update(ent);
    cc = ((int)x)/(CLUSTERSIZE*CELLSIZE);
    cr = ((int)z)/(CLUSTERSIZE*CELLSIZE);
    clus = cluster + cr*cluster_width + cc;
    for (li=clus->ents->first; li; li=li->next) {
        entity_t*e = li->ptr;
        if (aabb_below_aabb(&ent->aabb, &e->aabb) && e->mot)
            mot_unfreeze(e->mot);
    }
}

void ent_set_attr(entity_t* ent, lil_value_t name, lil_value_t value)
{
    listitem_t* li;
    entity_attr_t* attr;
    const char* cname = lil_to_string(name);
    for (li=ent->attrs->first; li; li=li->next) {
        attr = li->ptr;
        if (!strcmp(lil_to_string(attr->name), cname)) {
            lil_free_value(attr->value);
            attr->value = lil_clone_value(value);
            return;
        }
    }
#define __EVMASK_UPDATE(mv) ent->event_mask = lil_to_string(value)[0] ? (ent->event_mask | (mv)) : (ent->event_mask & (~(mv)))
    /* check event names */
    if (!strcmp(cname, "motion-touch"))
        __EVMASK_UPDATE(EVMASK_MOTION_TOUCH);
    else if (!strcmp(cname, "motion-touched"))
        __EVMASK_UPDATE(EVMASK_MOTION_TOUCHED);
    else if (!strcmp(cname, "motion-hit-ground"))
        __EVMASK_UPDATE(EVMASK_MOTION_HIT_GROUND);
    else if (!strcmp(cname, "move-target-reached"))
        __EVMASK_UPDATE(EVMASK_MOVE_TARGET_REACHED);
#undef __EVMASK_UPDATE
    else if (!strcmp(cname, "draw-shadow"))
        ent->draw_shadow = lil_to_boolean(value);
    attr = new(entity_attr_t);
    attr->name = lil_clone_value(name);
    attr->value = lil_clone_value(value);
    list_add(ent->attrs, attr);
}

lil_value_t ent_get_attr(entity_t* ent, lil_value_t name)
{
    listitem_t* li;
    entity_attr_t* attr;
    const char* cname = lil_to_string(name);
    for (li=ent->attrs->first; li; li=li->next) {
        attr = li->ptr;
        if (!strcmp(lil_to_string(attr->name), cname))
            return attr->value;
    }
    return NULL;
}

lil_value_t ent_call_attr(entity_t* ent, const char* name)
{
    lil_value_t lilname = lil_alloc_string(name);
    lil_value_t code = ent_get_attr(ent, lilname);
    lil_value_t retval = code ? lil_parse_value(lil, code, 0) : NULL;
    lil_free_value(lilname);
    return retval;
}

static void animate_entities(float ms)
{
    listitem_t* li = ents->first;
    for (li=ents->first; li; li=li->next) {
        entity_t* ent = li->ptr;
        if (ent->frames < 2) continue;
        ent->durstate += ms;
        while (ent->durstate >= ent->framedur) {
            ent->frame++;
            if (ent->frame > ent->end_frame)
                ent->frame = ent->start_frame;
            ent->durstate -= ent->framedur;
        }
    }
}

int world_save(const char* filename)
{
    const char id[4] = "ALW";
    uint8_t subversion = 0;
    uint16_t width = (uint16_t)map_width, height = (uint16_t)map_height;
    uint32_t entcount = (uint32_t)ents->count;
    uint32_t lightcount = (uint32_t)lights->count;
    uint8_t reserved[256];
    uint32_t foffset = 1;
    listitem_t* litem;
    texture_t** usv_texptr = NULL;
    uint32_t usv_count = 0;
    uint32_t i, player_entidx = 0;
    int x, y;
    FILE* f = rio_create(filename);
    memset(reserved, 0, 256);
    if (!f) return 0;
    fwrite(id, 4, 1, f);
    fwrite(&subversion, 1, 1, f);
    fwrite(&width, 2, 1, f);
    fwrite(&height, 2, 1, f);
    fwrite(&entcount, 4, 1, f);
    fwrite(&lightcount, 4, 1, f);
    for (litem=ents->first; litem; litem=litem->next) {
        if (litem->ptr == player_ent) break;
        player_entidx++;
    }
    fwrite(&player_entidx, 4, 1, f);
    fwrite(&pla, 4, 1, f);
    fwrite(&pll, 4, 1, f);
    fwrite(reserved, 1, 256, f);

#define world_save_add_texture_offset(tex) { \
    if ((tex) && (tex)->bankname) { \
        if (!(tex)->foffset) { \
            usv_texptr = realloc(usv_texptr, sizeof(texture_t*)*(usv_count + 1)); \
            usv_texptr[usv_count++] = tex; \
            (tex)->foffset = foffset; \
            foffset += strlen((tex)->bankname) + 1; \
        } \
        fwrite(&(tex)->foffset, 4, 1, f); \
    } else fwrite(reserved, 4, 1, f); }

    for (y=0; y<map_height; y++) {
        for (x=0; x<map_width; x++) {
            cell_t* c = cell + y*map_width + x;
            fwrite(&c->floorz, 4, 1, f);
            fwrite(&c->ceilz, 4, 1, f);
            fwrite(&c->flags, 4, 1, f);
            fwrite(c->zfoffs, 4, 1, f);
            fwrite(c->zcoffs, 4, 1, f);
            world_save_add_texture_offset(c->toptex);
            world_save_add_texture_offset(c->bottomtex);
            world_save_add_texture_offset(c->uppertex);
            world_save_add_texture_offset(c->lowetex);
            world_save_add_texture_offset(c->uppertrim);
            world_save_add_texture_offset(c->lowertrim);
        }
    }
#undef world_save_add_texture_offset

    for (litem=lights->first; litem; litem=litem->next) {
        light_t* light = litem->ptr;
        fwrite(&light->p.x, 4, 1, f);
        fwrite(&light->p.y, 4, 1, f);
        fwrite(&light->p.z, 4, 1, f);
        fwrite(&light->r, 4, 1, f);
        fwrite(&light->g, 4, 1, f);
        fwrite(&light->b, 4, 1, f);
        fwrite(&light->rad, 4, 1, f);
    }

    for (litem=ents->first; litem; litem=litem->next) {
        entity_t* ent = litem->ptr;
        listitem_t* aitem;
        uint32_t attrcount = (uint32_t)ent->attrs->count;
        uint8_t clslen = ent->class ? ((uint8_t)strlen(ent->class)) : 0;
        fwrite(&clslen, 1, 1, f);
        if (clslen) fwrite(ent->class, clslen, 1, f);
        fwrite(&ent->p.x, 4, 1, f);
        fwrite(&ent->p.y, 4, 1, f);
        fwrite(&ent->p.z, 4, 1, f);
        fwrite(&ent->xoff, 4, 1, f);
        fwrite(&ent->yoff, 4, 1, f);
        fwrite(&ent->zoff, 4, 1, f);
        fwrite(&ent->yrot, 4, 1, f);
        fwrite(ent->mtx, 4, 16, f);
        fwrite(&ent->laabb.min.x, 4, 1, f);
        fwrite(&ent->laabb.min.y, 4, 1, f);
        fwrite(&ent->laabb.min.z, 4, 1, f);
        fwrite(&ent->laabb.max.x, 4, 1, f);
        fwrite(&ent->laabb.max.y, 4, 1, f);
        fwrite(&ent->laabb.max.z, 4, 1, f);
        fwrite(&ent->frame, 4, 1, f);
        fwrite(&ent->durstate, 4, 1, f);
        fwrite(&ent->event_mask, 4, 1, f);
        fwrite(reserved, 1, 32, f);
        fwrite(&attrcount, 4, 1, f);
        for (aitem=ent->attrs->first; aitem; aitem=aitem->next) {
            entity_attr_t* attr = aitem->ptr;
            const char* name = lil_to_string(attr->name);
            const char* value = lil_to_string(attr->value);
            uint32_t len = strlen(name);
            fwrite(&len, 4, 1, f);
            fwrite(name, len, 1, f);
            len = strlen(value);
            fwrite(&len, 4, 1, f);
            fwrite(value, len, 1, f);
        }
    }

    fwrite(reserved, 1, 1, f);
    for (i=0; i<usv_count; i++) {
        uint8_t len = strlen(usv_texptr[i]->bankname);
        fwrite(&len, 1, 1, f);
        fwrite(usv_texptr[i]->bankname, len, 1, f);
        usv_texptr[i]->foffset = 0;
    }
    free(usv_texptr);

    fclose(f);

    return 1;
}

int world_load(const char* filename)
{
    FILE* f = rio_open(filename, NULL);
    char id[4];
    uint8_t subversion = 0;
    uint16_t width, height;
    uint32_t entcount;
    uint32_t lightcount;
    uint8_t reserved[256];
    listitem_t* litem;
    texture_t*** usv_texptr = NULL;
    uint32_t* usv_foffset = NULL;
    uint32_t usv_count = 0;
    uint32_t i, player_entidx;
    size_t base;
    int x, y;
    if (!f) return 0;

    fread(id, 4, 1, f);
    if (id[0] != 'A' || id[1] != 'L' || id[2] != 'W' || id[3] != 0) goto cleanup;
    fread(&subversion, 1, 1, f);
    if (subversion != 0) goto cleanup;
    fread(&width, 2, 1, f);
    fread(&height, 2, 1, f);
    fread(&entcount, 4, 1, f);
    fread(&lightcount, 4, 1, f);
    fread(&player_entidx, 4, 1, f);
    fread(&pla, 4, 1, f);
    fread(&pll, 4, 1, f);
    fread(reserved, 1, 256, f);

    map_free();
    map_init(width, height);

#define world_load_solve_texture_later(tex,foffs) { \
    if (foffs) { \
        usv_texptr = realloc(usv_texptr, sizeof(texture_t**)*(usv_count + 1)); \
        usv_foffset = realloc(usv_foffset, 4*(usv_count + 1)); \
        usv_texptr[usv_count] = tex; \
        usv_foffset[usv_count++] = foffs; \
    } else *(tex) = NULL; }\

    for (y=0; y<map_height; y++) {
        for (x=0; x<map_width; x++) {
            cell_t* c = cell + y*map_width + x;
            uint32_t foffset;
            fread(&c->floorz, 4, 1, f);
            fread(&c->ceilz, 4, 1, f);
            fread(&c->flags, 4, 1, f);
            fread(c->zfoffs, 4, 1, f);
            fread(c->zcoffs, 4, 1, f);
            fread(&foffset, 4, 1, f);
            world_load_solve_texture_later(&c->toptex, foffset);
            fread(&foffset, 4, 1, f);
            world_load_solve_texture_later(&c->bottomtex, foffset);
            fread(&foffset, 4, 1, f);
            world_load_solve_texture_later(&c->uppertex, foffset);
            fread(&foffset, 4, 1, f);
            world_load_solve_texture_later(&c->lowetex, foffset);
            fread(&foffset, 4, 1, f);
            world_load_solve_texture_later(&c->uppertrim, foffset);
            fread(&foffset, 4, 1, f);
            world_load_solve_texture_later(&c->lowertrim, foffset);
        }
    }
#undef world_load_solve_texture_later

    for (i=0; i<lightcount; i++) {
        light_t* light = light_new(0, 0, 0, 0, 0, 0, 16);
        fread(&light->p.x, 4, 1, f);
        fread(&light->p.y, 4, 1, f);
        fread(&light->p.z, 4, 1, f);
        fread(&light->r, 4, 1, f);
        fread(&light->g, 4, 1, f);
        fread(&light->b, 4, 1, f);
        fread(&light->rad, 4, 1, f);
    }

    for (i=0; i<entcount; i++) {
        entity_t* ent;
        uint32_t a, attrcount;
        uint8_t clslen;
        char* class;
        fread(&clslen, 1, 1, f);
        if (clslen) {
            class = calloc(1, clslen + 1);
            fread(class, clslen, 1, f);
            ent = ent_new_by_class(class);
            free(class);
        } else ent = ent_new();
        if (i == player_entidx) {
            motion_t* mot;
            if (player_ent) ent_free(player_ent);
            player_ent = ent;
            camera_ent = ent;
            mot = mot_new(player_ent);
            mot_for_char(mot);
            vec_set(&player_ent->laabb.min, -16, -48, -16);
            vec_set(&player_ent->laabb.max, 16, 16, 16);
        } else {
            ent->texture = texbank_get("sp_squares1");
        }
        fread(&ent->p.x, 4, 1, f);
        fread(&ent->p.y, 4, 1, f);
        fread(&ent->p.z, 4, 1, f);
        fread(&ent->xoff, 4, 1, f);
        fread(&ent->yoff, 4, 1, f);
        fread(&ent->zoff, 4, 1, f);
        fread(&ent->yrot, 4, 1, f);
        fread(ent->mtx, 4, 16, f);
        fread(&ent->laabb.min.x, 4, 1, f);
        fread(&ent->laabb.min.y, 4, 1, f);
        fread(&ent->laabb.min.z, 4, 1, f);
        fread(&ent->laabb.max.x, 4, 1, f);
        fread(&ent->laabb.max.y, 4, 1, f);
        fread(&ent->laabb.max.z, 4, 1, f);
        fread(&ent->frame, 4, 1, f);
        fread(&ent->durstate, 4, 1, f);
        fread(&ent->event_mask, 4, 1, f);
        fread(reserved, 1, 32, f);
        fread(&attrcount, 4, 1, f);
        for (a=0; a<attrcount; a++) {
            char* name;
            char* value;
            lil_value_t vname, vvalue;
            uint32_t len;
            fread(&len, 4, 1, f);
            name = calloc(1, len + 1);
            fread(name, len, 1, f);
            fread(&len, 4, 1, f);
            value = calloc(1, len + 1);
            fread(value, len, 1, f);
            vname = lil_alloc_string(name);
            vvalue = lil_alloc_string(value);
            free(value);
            free(name);
            ent_set_attr(ent, vname, vvalue);
            lil_free_value(vname);
            lil_free_value(vvalue);
        }
    }

    base = ftell(f);
    for (i=0; i<usv_count; i++) {
        uint8_t len;
        char* name;
        fseek(f, base + usv_foffset[i], SEEK_SET);
        fread(&len, 1, 1, f);
        name = calloc(1, len + 1);
        fread(name, len, 1, f);
        *usv_texptr[i] = texbank_get(name);
        free(name);
    }
    free(usv_foffset);
    free(usv_texptr);

    for (y=0; y<cluster_height; y++)
        for (x=0; x<cluster_width; x++)
            invalidate_cluster(x, y);

    for (y=0; y<map_height; y++)
        for (x=0; x<map_width; x++)
            map_update_cell(x, y);

    lmap_update();

    for (litem=ents->first; litem; litem=litem->next)
        ent_update(litem->ptr);

    return 1;
cleanup:
    if (f) fclose(f);
    return 0;
}

void world_animate(float ms)
{
    animate_entities(ms);
}
