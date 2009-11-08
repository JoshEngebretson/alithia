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

font_t* font_load(const char* filename)
{
    font_t* font = new(font_t);
    FILE* f = fopen(filename, "rb");
    char id[4];
    int i, bmp_width, bmp_height;
    unsigned char byte, byte2;
    unsigned char* bmp = NULL;
    unsigned char* bmp4 = NULL;
    GLuint gltex;
    if (!f) goto fail;

    if (!fread(id, 4, 1, f)) goto fail;
    if (strncmp(id, "BIFO", 4)) goto fail;
    if (!fread(&byte, 1, 1, f)) goto fail; if (byte != 33) goto fail;
    if (!fread(&byte, 1, 1, f)) goto fail; if (byte != 126) goto fail;
    if (!fread(&byte, 1, 1, f)) goto fail; bmp_width = byte*64;
    if (!fread(&byte, 1, 1, f)) goto fail; bmp_height = byte*64;
    bmp = malloc(bmp_width*bmp_height);
    if (!bmp) goto fail;
    bmp4 = malloc(bmp_width*bmp_height*4);
    if (!bmp4) goto fail;
    if (!fread(bmp, bmp_width*bmp_height, 1, f)) goto fail;
    for (i=bmp_width*bmp_height-1; i>=0; i--) {
        bmp4[i*4] = bmp4[i*4 + 1] = bmp4[i*4 + 2] = bmp4[i*4 + 3] = bmp[i];
    }

    for (i=33; i<127; i++) {
        int x, y, width, height;
        if (!fread(&byte, 1, 1, f)) goto fail;
        if (!fread(&byte2, 1, 1, f)) goto fail;
        x = byte|(byte2 << 8);
        if (!fread(&byte, 1, 1, f)) goto fail;
        if (!fread(&byte2, 1, 1, f)) goto fail;
        y = byte|(byte2 << 8);
        if (!fread(&byte, 1, 1, f)) goto fail;
        width = byte;
        if (!fread(&byte, 1, 1, f)) goto fail;
        height = byte;

        font->ci[i].u1 = (float)x/(float)bmp_width;
        font->ci[i].v1 = (float)y/(float)bmp_height;
        font->ci[i].u2 = (float)(x + width)/(float)bmp_width;
        font->ci[i].v2 = (float)(y + height)/(float)bmp_height;
        font->ci[i].w = (float)width/(float)height;
    }

    fclose(f);
    f = NULL;

    glGenTextures(1, &gltex);
    glBindTexture(GL_TEXTURE_2D, gltex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, bmp_width, bmp_height, GL_RGBA, GL_UNSIGNED_BYTE, bmp4);
    free(bmp4);
    free(bmp);

    font->tex = gltex;

    return font;

fail:
    if (bmp4) free(bmp4);
    if (bmp) free(bmp);
    if (font) free(font);
    if (f) fclose(f);
    return NULL;
}

void font_free(font_t* font)
{
    if (font->tex) {
        glBindTexture(GL_TEXTURE_2D, font->tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, 0, 0, 0, GL_LUMINANCE8, GL_UNSIGNED_BYTE, NULL);
        glDeleteTextures(1, &font->tex);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    free(font);
}
void font_render(font_t* font, float x, float y, const char* str, int len, float size)
{
    float sx = x;
    glPushClientAttrib(GL_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->tex);
    if (len == -1) len = strlen(str);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    while (len > 0) {
        fontcharinfo_t* ci;
        if (*str == '\n') {
            x = sx;
            y -= size;
            len--;
            str++;
            continue;
        }
        if (*str < 33 || *str > 126) {
            x += size/3.0;
            len--;
            str++;
            continue;
        }
        ci = font->ci + *str;

        glTexCoord2f(ci->u1, ci->v2); glVertex2f(x, y);
        glTexCoord2f(ci->u1, ci->v1); glVertex2f(x, y + size);
        glTexCoord2f(ci->u2, ci->v1); glVertex2f(x + ci->w*size, y + size);
        glTexCoord2f(ci->u2, ci->v2); glVertex2f(x + ci->w*size, y);

        x += ci->w*size;

        str++;
        len--;
    }
    glEnd();
    glPopAttrib();
    glPopClientAttrib();
}
