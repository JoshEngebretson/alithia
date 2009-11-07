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

    printf("vc=%i fc=%i\n", mdl->vc, mdl->fc);

    mdl->v = malloc0(sizeof(float)*mdl->vc*8);
    mdl->f = malloc0(sizeof(int)*mdl->fc*3);
    fidx = malloc0(2*mdl->fc*3);

    if (fread(mdl->v, sizeof(float)*8, mdl->vc, f) != mdl->vc) goto fail;
    if (fread(fidx, 6, mdl->fc, f) != mdl->fc) goto fail;
    for (i=0; i<mdl->fc*3; i++) mdl->f[i] = fidx[i];
    free(fidx);
    fidx = NULL;

    fclose(f);
    f = NULL;

    mdl->tex = tex_load(texfile);
    if (!mdl->tex) goto fail;

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
    if (mdl->dl) glDeleteLists(mdl->dl, 1);
    if (mdl->tex) tex_free(mdl->tex);
    if (mdl->v) free(mdl->v);
    if (mdl->f) free(mdl->f);
    free(mdl);
}
