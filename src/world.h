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

#define PICK_NOTHING 0
#define PICK_WORLD 1
#define PICK_ENTITY 2
#define PICK_LIGHT 3

#define PICKFLAG_NOFLAGS 0
#define PICKFLAG_PICK_LIGHTS 0x0001

#define EVMASK_NOTHING 0
#define EVMASK_MOTION_TOUCH 0x0001
#define EVMASK_MOTION_TOUCHED 0x0002
#define EVMASK_MOTION_HIT_GROUND 0x0004
#define EVMASK_MOVE_TARGET_REACHED 0x0008

typedef struct _light_t
{
    vector_t p;
    float r, g, b;
    float rad;
} light_t;

typedef struct _entity_attr_t
{
    lil_value_t name;
    lil_value_t value;
} entity_attr_t;

typedef struct _entity_t
{
    char* class;
    model_t* mdl;
    vector_t p;
    float xoff, yoff, zoff;
    float yrot;
    float mtx[16];
    struct _cluster_t* clus;
    list_t* attrs;
    aabb_t aabb;
    aabb_t laabb; /* aabb for modelless entities */
    void* mot;
    void* ai;
    uint32_t frame;
    uint32_t frames;
    uint32_t start_frame;
    uint32_t end_frame;
    float durstate;
    float framedur;
    uint32_t event_mask;
} entity_t;

typedef struct _cell_t
{
    texture_t* toptex;
    texture_t* bottomtex;
    texture_t* uppertex;
    texture_t* lowetex;
    int32_t floorz;
    int32_t ceilz;
    int32_t flags;
    int8_t zfoffs[4];
    int8_t zcoffs[4];
} cell_t;

typedef struct _decal_t
{
    vector_t p[4];
    texture_t* tex;
} decal_t;

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
    triangle_t* tri;
    size_t tris;
    list_t* ents;
    decal_t* decal;
    size_t decals;
    aabb_t aabb;
} cluster_t;

typedef struct _lmap_texel_t
{
    unsigned char r, g, b, x;
} lmap_texel_t;

typedef struct _pickdata_t
{
    entity_t* ignore_ent; /* entity to ignore */
    vector_t ip; /* intersection point */
    int result; /* one of PICK_xxx */
    entity_t* entity; /* picked entity for PICK_ENTITY */
    int cluster_x, cluster_y; /* cluster coords where picking was done */
    cluster_t* cluster; /* cluster at the above coords */
    cell_t* cell; /* cell for PICK_WORLD */
    int cellx, celly; /* coords for above */
    triangle_t tri; /* triangle for PICK_WORLD */
    vector_t* a; /* passed in pick */
    vector_t* b; /* passed in pick */
    int nopick;
    cluster_t* lastcluster;
    float ipad; /* ip-to-a distance (squared) */
    int pickflags; /* picking flags */
    light_t* light; /* picked light for PICK_LIGHT */
} pickdata_t;

extern cell_t* cell;
extern char* ocmap;
extern cluster_t* cluster;
extern list_t* ents;
extern list_t* lights;
extern int map_width;
extern int map_height;
extern int cluster_width;
extern int cluster_height;
extern lmap_texel_t* lightmap;
extern GLuint lmaptex;
extern int lmap_quality;
extern entity_t* player_ent;
extern int world_init_frames;

void map_init(int width, int height);
void map_free(void);
void map_update_cell(int x, int y);

void ray_march(int x1, int y1, int x2, int y2, int (*func)(int x, int y, cell_t* cell, void* data), void* data);
int pick(vector_t* a, vector_t* b, pickdata_t* pd, int flags);

void lmap_update(void);

light_t* light_new(float x, float y, float z, float r, float g, float b, float rad);
void light_free(light_t* light);

decal_t* decal_new(cluster_t* clus, vector_t* p, texture_t* tex);

entity_t* ent_new(void);
void ent_free(entity_t* ent);
entity_t* ent_clone(entity_t* ent);
void ent_update(entity_t* ent);
void ent_set_model(entity_t* ent, model_t* mdl);
void ent_move(entity_t* ent, float x, float y, float z);
void ent_set_attr(entity_t* ent, lil_value_t name, lil_value_t value);
lil_value_t ent_get_attr(entity_t* ent, lil_value_t name);
lil_value_t ent_call_attr(entity_t* ent, const char* name);

int world_save(const char* filename);
int world_load(const char* filename);
void world_animate(float ms);

#endif
