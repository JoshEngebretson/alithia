#include "atest.h"

int vid_init(void)
{
    int r = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE);
    int width = arg_intval("-width", 800);
    int height = arg_intval("-height", 600);
    int fullscreen = arg_intval("-fullscreen", 0);
    int bpp = arg_intval("-bpp", 32);
    int flags = SDL_OPENGL|(fullscreen?SDL_FULLSCREEN:0);

    if (r < 0) return FALSE;

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

    SDL_WM_SetCaption("ATest", "ATest");

    return TRUE;
}

void vid_shutdown(void)
{
    SDL_Quit();
}
