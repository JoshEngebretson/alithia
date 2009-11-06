#ifndef __MODELS_H_INCLUDED__
#define __MODELS_H_INCLUDED__

typedef struct _model_t
{
    float* v;
    int* f;
    int vc;
    int fc;
    texture_t* tex;
    GLuint dl;
    GLuint dlshadow;
} model_t;

model_t* mdl_load(const char* geofile, const char* texfile);
void mdl_free(model_t* mdl);
void mdl_render(model_t* mdl);

#endif
