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

#ifndef __WORLD_H_INCLUDED__
#define __WORLD_H_INCLUDED__

#define CELLSIZE 16
#define CLUSTERSIZE 16

#define CF_NOFLAGS 0
#define CF_OCCLUDER 0x00000001
#define CF_HEIGHTMAP 0x00000002

typedef struct _entity_t
{
    model_t* mdl;
    int x, y, z;
    float xoff, yoff, zoff;
    int yrot;
    float mtx[16];
    struct _cluster_t* clus;
} entity_t;

typedef struct _cell_t
{
    texture_t* toptex;
    texture_t* bottomtex;
    texture_t* uppertex;
    texture_t* lowetex;
    int floorz;
    int ceilz;
    int flags;
    int height;
} cell_t;

typedef struct _clusterpart_t
{
    texture_t* tex;
    GLuint dl;
} clusterpart_t;

typedef struct _cluster_t
{
    int visible;
    clusterpart_t* part;
    int parts;
    list_t* ents;
    float x1, y1, z1, x2, y2, z2;
} cluster_t;

extern cell_t* cell;
extern cluster_t* cluster;
extern list_t* ents;
extern int map_width;
extern int map_height;
extern int cluster_width;
extern int cluster_height;

void map_init(int width, int height);
void map_free(void);

entity_t* ent_new(void);
void ent_free(entity_t* ent);
void ent_update(entity_t* ent);
void ent_set_model(entity_t* ent, model_t* mdl);
void ent_move(entity_t* ent, int x, int y, int z);

#endif
