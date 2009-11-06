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

texture_t* tex_load(const char* filename)
{
    texture_t* tex = new(texture_t);
    SDL_Surface* bmp = SDL_LoadBMP(filename);
    GLuint name;
    GLenum format;
    GLint colors;
    if (!bmp) {
        fprintf(stderr, "failed to load '%s'\n", filename);
        return NULL;
    }

    colors = bmp->format->BytesPerPixel;
    if (colors == 4) {
        if (bmp->format->Rmask == 0x000000FF) format = GL_RGBA;
        else format = GL_BGRA;
    } else if (colors == 3) {
        if (bmp->format->Rmask == 0x000000FF) format = GL_RGB;
        else format = GL_BGR;
    } else {
        SDL_FreeSurface(bmp);
        fprintf(stderr, "unknown format for '%s' (%i bpp)\n", filename, colors);
        return 0;
    }

    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, colors, bmp->w, bmp->h, format,
        GL_UNSIGNED_BYTE, bmp->pixels);

    SDL_FreeSurface(bmp);

    tex->name = name;
    tex->bucket = -1;

    return tex;
}

void tex_free(texture_t* tex)
{
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glDeleteTextures(1, &tex->name);
    free(tex);
}
