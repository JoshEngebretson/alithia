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

typedef struct _cache_entry_t
{
    char* name;
    model_t* model;
} cache_entry_t;

static list_t* cache;

model_t* mdl_load(const char* geofile, const char* texfile)
{
    model_t* mdl = new(model_t);
    FILE* f = fopen(geofile, "rb");
    char id[4];
    int flags;
    int frames;
    int i;
    int16_t* fidx = NULL;

    if (!f) goto fail;

    if (fread(id, 1, 4, f) != 4) goto fail;
    if (strncmp(id, "ALM1", 4)) goto fail;
    if (fread(&flags, 1, 4, f) != 4) goto fail;
    if (fread(&mdl->vc, 1, 4, f) != 4) goto fail;
    if (fread(&mdl->fc, 1, 4, f) != 4) goto fail;
    if (fread(&frames, 1, 4, f) != 4) goto fail;

    mdl->frames = frames;
    mdl->v = malloc0(sizeof(float)*mdl->vc*8*frames);
    mdl->f = malloc0(sizeof(int)*mdl->fc*3);
    fidx = malloc0(2*mdl->fc*3);

    if (fread(mdl->v, sizeof(float)*8, mdl->vc*frames, f) != mdl->vc*frames) goto fail;
    if (fread(fidx, 6, mdl->fc, f) != mdl->fc) goto fail;
    for (i=0; i<mdl->fc*3; i++) mdl->f[i] = fidx[i];
    free(fidx);
    fidx = NULL;

    fclose(f);
    f = NULL;

    mdl->tex = tex_load(texfile);
    if (!mdl->tex) goto fail;

    for (i=0; i<mdl->vc; i++)
        aabb_consider_xyz(&mdl->aabb, mdl->v[i*8], mdl->v[i*8 + 1], mdl->v[i*8 + 2]);

    return mdl;

fail:
    if (f) fclose(f);
    if (fidx) free(fidx);
    if (mdl) {
        if (mdl->tex) tex_free(mdl->tex);
        if (mdl->v) free(mdl->v);
        if (mdl->f) free(mdl->f);
        free(mdl);
    }

    return NULL;
}

void mdl_free(model_t* mdl)
{
    if (!mdl) return;
    or_deleted(mdl);
    if (mdl->dl) glDeleteLists(mdl->dl, mdl->frames);
    if (mdl->tex) tex_free(mdl->tex);
    if (mdl->v) free(mdl->v);
    if (mdl->f) free(mdl->f);
    free(mdl);
}

model_t* modelcache_get(const char* name)
{
    cache_entry_t* e;
    char mdlname[256];
    char texname[256];
    model_t* mdl;

    if (strlen(name) > 200) return NULL;
    if (cache) {
        listitem_t* li;
        for (li=cache->first; li; li=li->next) {
            e = li->ptr;
            if (!strcmp(name, e->name))
                return e->model;
        }
    }
    sprintf(mdlname, "data/models/%s.alm", name);
    sprintf(texname, "data/models/%s.bmp", name);
    mdl = mdl_load(mdlname, texname);
    if (!mdl) return NULL;

    e = new(cache_entry_t);
    e->name = strdup(name);
    e->model = mdl;
    if (!cache) cache = list_new();
    list_add(cache, e);

    return mdl;
}

void modelcache_clear(void)
{
    listitem_t* li;
    if (!cache) return;
    for (li=cache->first; li; li=li->next) {
        cache_entry_t* e = li->ptr;
        free(e->name);
        mdl_free(e->model);
        free(e);
    }
    list_free(cache);
    cache = NULL;
}
