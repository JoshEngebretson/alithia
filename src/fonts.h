#ifndef __FONTS_H_INCLUDED__
#define __FONTS_H_INCLUDED__

typedef struct _fontcharinfo_t
{
    float u1, v1;
    float u2, v2;
    float w;
} fontcharinfo_t;

typedef struct _font_t
{
    GLuint tex;
    fontcharinfo_t ci[127];
} font_t;

extern font_t* font_normal;

void font_init(void);
void font_shutdown(void);

font_t* font_load(const char* filename);
void font_free(font_t* font);
void font_render(font_t* font, float x, float y, const char* str, int len, float size);
float font_width(font_t* font, const char* str, int len, float size);

#endif
