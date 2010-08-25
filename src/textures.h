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

#ifndef __TEXTURES_H_INCLUDED__
#define __TEXTURES_H_INCLUDED__

typedef struct _texture_t
{
    const char* bankname;
    GLuint name;
    int bucket;
    uint32_t foffset;
} texture_t;

typedef struct _texbankitem_t
{
    char* name;
    texture_t* tex;
} texbankitem_t;

extern texbankitem_t** texbank_item;
extern size_t texbank_items;

texture_t* tex_load(const char* filename);
void tex_load_skybox(const char* filename, texture_t** left, texture_t** back, texture_t** right, texture_t** bottom, texture_t** top, texture_t** front);
void tex_free(texture_t* tex);

void texbank_init(void);
void texbank_shutdown(void);
texture_t* texbank_add(const char* name, const char* filename);
texture_t* texbank_get(const char* name);

#endif
