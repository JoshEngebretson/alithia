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

texbankitem_t** texbank_item;
size_t texbank_items;

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

static texture_t* tex_from_bgra(const uint8_t* bgra, size_t width, size_t height, int colors, int format)
{
    texture_t* tex = new(texture_t);
    glGenTextures(1, &tex->name);
    glBindTexture(GL_TEXTURE_2D, tex->name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, colors, width, height, format,
        GL_UNSIGNED_BYTE, bgra);
    tex->bucket = -1;

    return tex;
}

static texture_t* tex_from_surface_part(SDL_Surface* s, size_t left, size_t top, size_t width, size_t height, int colors, int format)
{
    size_t x2 = left + width - 1;
    size_t y2 = top + height - 1;
    size_t x, y, i=0;
    uint8_t* src = s->pixels;
    uint8_t* pixels = malloc(width*height*colors);
    texture_t* tex;
    for (y=top; y<=y2; y++) {
        for (x=left; x<=x2; x++) {
            for (i=0; i<colors; i++)
                pixels[((y - top)*width + x - left)*colors + i] = src[(y*s->w + x)*colors + i];
        }
    }
    tex = tex_from_bgra(pixels, width, height, colors, format);
    free(pixels);
    return tex;
}

void tex_load_skybox(const char* filename, texture_t** left, texture_t** back, texture_t** right, texture_t** bottom, texture_t** top, texture_t** front)
{
    SDL_Surface* bmp = SDL_LoadBMP(filename);
    GLint colors;
    GLenum format;
    if (!bmp) {
        fprintf(stderr, "failed to load '%s'\n", filename);
        return;
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
    }

    *left = tex_from_surface_part(bmp, 0, 0, 256, 256, colors, format);
    *back = tex_from_surface_part(bmp, 256, 0, 256, 256, colors, format);
    *right = tex_from_surface_part(bmp, 512, 0, 256, 256, colors, format);
    *bottom = tex_from_surface_part(bmp, 0, 256, 256, 256, colors, format);
    *top = tex_from_surface_part(bmp, 256, 256, 256, 256, colors, format);
    *front = tex_from_surface_part(bmp, 512, 256, 256, 256, colors, format);

    SDL_FreeSurface(bmp);
}

void tex_free(texture_t* tex)
{
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glDeleteTextures(1, &tex->name);
    free(tex);
}

void texbank_init(void)
{
    struct dirent* de;
    DIR* dir;

    texbank_item = NULL;
    texbank_items = 0;

    dir = opendir("data/textures");
    while ((de = readdir(dir))) {
        if (strstr(de->d_name, ".bmp")) {
            char* tmp = strdup(de->d_name);
            char* tmpfilename = malloc(strlen(de->d_name) + 16);
            int i;
            for (i=strlen(tmp)-1; i>=0; i--)
                if (tmp[i] == '.') {
                    tmp[i] = 0;
                    break;
                }
            sprintf(tmpfilename, "data/textures/%s", de->d_name);
            texbank_add(tmp, tmpfilename);
            free(tmpfilename);
            free(tmp);
        }
    }
    closedir(dir);
}

void texbank_shutdown(void)
{
    size_t i;
    for (i=0; i<texbank_items; i++) {
        tex_free(texbank_item[i]->tex);
        free(texbank_item[i]->name);
        free(texbank_item[i]);
    }
    free(texbank_item);
    texbank_item = NULL;
    texbank_items = 0;
}

texture_t* texbank_add(const char* name, const char* filename)
{
    texture_t* tex = tex_load(filename);
    if (!tex) return NULL;
    texbank_item = realloc(texbank_item, sizeof(texbankitem_t*)*(texbank_items + 1));
    texbank_item[texbank_items] = new(texbankitem_t);
    texbank_item[texbank_items]->name = strdup(name);
    texbank_item[texbank_items]->tex = tex;
    texbank_items++;
    return tex;
}

texture_t* texbank_get(const char* name)
{
    size_t i;
    for (i=0; i<texbank_items; i++)
        if (!strcmp(texbank_item[i]->name, name))
            return texbank_item[i]->tex;
    return NULL;
}
