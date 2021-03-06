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

#ifndef __MODELS_H_INCLUDED__
#define __MODELS_H_INCLUDED__

typedef struct _model_t
{
    float* v;
    int* f;
    int vc;
    int fc;
    int frames;
    texture_t* tex;
    GLuint dl;
    GLuint dlshadow;
    aabb_t aabb;
} model_t;

model_t* mdl_load(const char* geofile, const char* texfile);
void mdl_free(model_t* mdl);

model_t* modelcache_get(const char* name);
void modelcache_clear(void);

#endif
