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

void mdl_render(model_t* mdl)
{
    int i;
    static float pos[] = {0, 1, 0, 0};
    static float amb[] = {0.9, 0.9, 0.9, 1.0};
    glBindTexture(GL_TEXTURE_2D, mdl->tex->name);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glBegin(GL_TRIANGLES);
    for (i=0; i<mdl->fc; i++) {
        float* v = mdl->v + (mdl->f[i*3]*8);
        glTexCoord2f(v[6], v[7]);
        glNormal3f(v[3], v[4], v[5]);
        glVertex3f(v[0], v[1], v[2]);
        v = mdl->v + (mdl->f[i*3 + 1]*8);
        glTexCoord2f(v[6], v[7]);
        glNormal3f(v[3], v[4], v[5]);
        glVertex3f(v[0], v[1], v[2]);
        v = mdl->v + (mdl->f[i*3 + 2]*8);
        glTexCoord2f(v[6], v[7]);
        glNormal3f(v[3], v[4], v[5]);
        glVertex3f(v[0], v[1], v[2]);
    }
    glEnd();
    glDisable(GL_LIGHTING);

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glBegin(GL_TRIANGLES);
    for (i=0; i<mdl->fc; i++) {
        float* v = mdl->v + (mdl->f[i*3]*8);
        glVertex3f(v[0], 0, v[2]);
        v = mdl->v + (mdl->f[i*3 + 1]*8);
        glVertex3f(v[0], 0, v[2]);
        v = mdl->v + (mdl->f[i*3 + 2]*8);
        glVertex3f(v[0], 0, v[2]);
    }
    glEnd();
    glColorMask(1, 1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

    glColor4f(0, 0, 0, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glVertex2d(-1, -1);
    glVertex2d(1, -1);
    glVertex2d(1, 1);
    glVertex2d(-1, 1);
    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDepthMask(1);

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glColor3f(1, 1, 1);
}
