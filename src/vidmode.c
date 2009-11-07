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

PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2f;
int vid_width;
int vid_height;

#define EXT_LOAD(n) { \
    n = SDL_GL_GetProcAddress(#n); \
    if (!n) return 0; \
}
#define EXT_LOAD2(n,n2) { \
    n = SDL_GL_GetProcAddress(#n); \
    if (!n) n = SDL_GL_GetProcAddress(#n2); \
    if (!n) return 0; \
}
#define EXT_LOAD3(n,n2,n3) { \
    n = SDL_GL_GetProcAddress(#n); \
    if (!n) n = SDL_GL_GetProcAddress(#n2); \
    if (!n) n = SDL_GL_GetProcAddress(#n3); \
    if (!n) return 0; \
}

static int load_extensions(void)
{
    EXT_LOAD2(glActiveTexture, glActiveTextureARB);
    EXT_LOAD2(glMultiTexCoord2f, glMultiTexCoord2fARB);

    return 1;
}

int vid_init(void)
{
    int r = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE);
    int width = arg_intval("-width", 800);
    int height = arg_intval("-height", 600);
    int fullscreen = arg_intval("-fullscreen", 0);
    int bpp = arg_intval("-bpp", 32);
    int flags = SDL_OPENGL|(fullscreen?SDL_FULLSCREEN:0);

    if (r < 0) return FALSE;

    vid_width = width;
    vid_height = height;

    if (bpp == 32 || bpp == 24) {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    } else {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

    if (!SDL_SetVideoMode(width, height, bpp, flags))
        if (!SDL_SetVideoMode(width, height, bpp, flags&(~SDL_FULLSCREEN)))
            return FALSE;

    if (!load_extensions())
        return FALSE;

    SDL_WM_SetCaption("ATest", "ATest");

    return TRUE;
}

void vid_shutdown(void)
{
    SDL_Quit();
}
