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

model_t* mdl_vytio;
model_t* mdl_armor;

void map_init(int width, int height)
{
    int i, x, y;
    entity_t* ent;

    map_width = width;
    map_height = height;
    cluster_width = width/CLUSTERSIZE;
    cluster_height = height/CLUSTERSIZE;

    cell = malloc0(width*height*sizeof(cell_t));
    ocmap = malloc0(width*height);
    cluster = malloc0(cluster_width*cluster_height*sizeof(cluster_t));

    ents = list_new();
    ents->item_free = (void*)ent_free;

    lights = list_new();
    lights->item_free = (void*)light_free;

    i = 0;
    for (y=0; y<cluster_height; y++) {
        for (x=0; x<cluster_width; x++) {
            cluster[i].ents = list_new();
            cluster[i].x1 = x*CELLSIZE*CLUSTERSIZE;
            cluster[i].z1 = y*CELLSIZE*CLUSTERSIZE;
            cluster[i].x2 = (x + 1)*CELLSIZE*CLUSTERSIZE;
            cluster[i].z2 = (y + 1)*CELLSIZE*CLUSTERSIZE;
            i++;
        }
    }

    for (i=0; i<width*height; i++) {
        cell[i].floorz = -128;
        cell[i].ceilz = 128;
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

    lightmap = malloc0(sizeof(lmap_texel_t)*map_width*map_height);
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &lmaptex);
    glBindTexture(GL_TEXTURE_2D, lmaptex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map_width, map_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    mdl_vytio = mdl_load("data/models/vytio.alm", "data/models/vytio.bmp");
    mdl_armor = mdl_load("data/models/armor.alm", "data/models/armor.bmp");

    ent = ent_new();
    ent_set_model(ent, mdl_vytio);
    ent_move(ent, 10*CELLSIZE*32.0, -128*32.0, 10*CELLSIZE*32.0);

    ent = ent_new();
    ent_set_model(ent, mdl_armor);
    ent_move(ent, 18*CELLSIZE*32.0, -128*32.0, 10*CELLSIZE*32.0);

    light_new(14*CELLSIZE, 0, 10*CELLSIZE, 0.7, 0.2, 0.1, 150);
    light_new(14*CELLSIZE, 0, 10*CELLSIZE, 0.1, 0.1, 0.2, 150000);
    light_new(114*CELLSIZE, 0, 34*CELLSIZE, 0.1, 0.3, 1.0, 1500);
    light_new(38*CELLSIZE, 0, 78*CELLSIZE, 0.2, 0.8, 0.3, 1000);
    light_new(138*CELLSIZE, 0, 208*CELLSIZE, 0.8, 0.2, 0.3, 1000);
/*
    for (i=0; i<100; i++) {
        for (y=7+i; y<map_height-(7+i); y++) {
            for (x=7+i; x<=map_width-(7+i); x++) {
                cell[y*map_width + x].floorz -= 8 + (i/40);
            }
        }
    }
*/
    for (y=0; y<map_height; y++)
        for (x=0; x<map_width; x++)
            map_update_cell(x, y);
}

void map_free(void)
{
    int i;
    glBindTexture(GL_TEXTURE_2D, lmaptex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glDeleteTextures(1, &lmaptex);
    for (i=0; i<cluster_width*cluster_height; i++)
        list_free(cluster[i].ents);
    list_free(lights);
    list_free(ents);
    free(cluster);
    free(ocmap);
    free(cell);
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
    ray_march(a->x/CELLSIZE, a->z/CELLSIZE, b->x/CELLSIZE, b->z/CELLSIZE, pick_rm_func, pd);
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
                    r -= 0.02;
                    g -= 0.02;
                    b -= 0.02;
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
    list_remove(lights, light, 0);
    free(light);
}

entity_t* ent_new(void)
{
    entity_t* ent = new(entity_t);
    list_add(ents, ent);
    return ent;
}

void ent_free(entity_t* ent)
{
    list_remove(ents, ent, 0);
    free(ent);
}

void ent_update(entity_t* ent)
{
    int cc = (ent->x >> 5)/(CLUSTERSIZE*CELLSIZE);
    int cr = (ent->z >> 5)/(CLUSTERSIZE*CELLSIZE);
    cluster_t* clus = cluster + cr*cluster_width + cc;
    float* mtx = ent->mtx;

    if (ent->clus != clus) {
        if (ent->clus) list_remove(ent->clus->ents, ent, 0);
        list_add(clus->ents, ent);
        ent->clus = clus;
    }

    mtx[0] = 1; mtx[4] = 0; mtx[8] = 0; mtx[12] = ((float)ent->x)/32.0;
    mtx[1] = 0; mtx[5] = 1; mtx[9] = 0; mtx[13] = ((float)ent->y)/32.0;
    mtx[2] = 0; mtx[6] = 0; mtx[10]= 1; mtx[14] = ((float)ent->z)/32.0;
    mtx[3] = 0; mtx[7] = 0; mtx[11]= 0; mtx[15] = 1;
}

void ent_set_model(entity_t* ent, model_t* mdl)
{
    ent->mdl = mdl;
    ent_update(ent);
}

void ent_move(entity_t* ent, int x, int y, int z)
{
    ent->x = x;
    ent->y = y;
    ent->z = z;
    ent_update(ent);
}
