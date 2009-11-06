#ifndef __TEXTURES_H_INCLUDED__
#define __TEXTURES_H_INCLUDED__

typedef struct _texture_t
{
    GLuint name;
    int bucket;
} texture_t;

texture_t* tex_load(const char* filename);
void tex_free(texture_t* tex);

#endif
