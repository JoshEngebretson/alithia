#include "atest.h"

cell_t* cell;
cluster_t* cluster;
list_t* ents;
int map_width;
int map_height;
int cluster_width;
int cluster_height;

model_t* mdl_vytio;
model_t* mdl_armor;

void map_init(int width, int height)
{
    int i, x, y;
    entity_t* ent;

    map_width = width;
    map_height = height;
    cluster_width = width/CLUSTERSIZE;
    cluster_height = height/CLUSTERSIZE;

    cell = malloc0(width*height*sizeof(cell_t));
    cluster = malloc0(cluster_width*cluster_height*sizeof(cluster_t));

    ents = list_new();
    ents->item_free = (void*)ent_free;

    i = 0;
    for (y=0; y<cluster_height; y++) {
        for (x=0; x<cluster_width; x++) {
            cluster[i].ents = list_new();
            cluster[i].x1 = x*CELLSIZE*CLUSTERSIZE;
            cluster[i].z1 = y*CELLSIZE*CLUSTERSIZE;
            cluster[i].x2 = (x + 1)*CELLSIZE*CLUSTERSIZE;
            cluster[i].z2 = (y + 1)*CELLSIZE*CLUSTERSIZE;
            i++;
        }
    }

    for (i=0; i<width*height; i++) {
        cell[i].floorz = -128;
        cell[i].ceilz = 128;
    }

    for (i=0; i<width; i++) {
        cell[i].flags = cell[(height-1)*width + i].flags = CF_OCCLUDER;
        cell[i].ceilz = cell[(height-1)*width + i].ceilz = 0;
        cell[i].floorz = cell[(height-1)*width + i].floorz = 0;
    }
    for (i=0; i<height; i++) {
        cell[i*width].flags = cell[i*width + height - 1].flags = CF_OCCLUDER;
        cell[i*width].ceilz = cell[i*width + height - 1].ceilz = 0;
        cell[i*width].floorz = cell[i*width + height - 1].floorz = 0;
    }

    mdl_vytio = mdl_load("data/models/vytio.alm", "data/models/vytio.bmp");
    mdl_armor = mdl_load("data/models/armor.alm", "data/models/armor.bmp");

    ent = ent_new();
    ent_set_model(ent, mdl_vytio);
    ent_move(ent, 10*CELLSIZE*32.0, -128*32.0, 10*CELLSIZE*32.0);

    ent = ent_new();
    ent_set_model(ent, mdl_armor);
    ent_move(ent, 18*CELLSIZE*32.0, -128*32.0, 10*CELLSIZE*32.0);
}

void map_free(void)
{
    int i;
    for (i=0; i<cluster_width*cluster_height; i++)
        list_free(cluster[i].ents);
    list_free(ents);
    free(cluster);
    free(cell);
}

entity_t* ent_new(void)
{
    entity_t* ent = new(entity_t);
    list_add(ents, ent);
    return ent;
}

void ent_free(entity_t* ent)
{
    list_remove(ents, ent, 0);
    free(ent);
}

void ent_update(entity_t* ent)
{
    int cc = (ent->x >> 5)/(CLUSTERSIZE*CELLSIZE);
    int cr = (ent->z >> 5)/(CLUSTERSIZE*CELLSIZE);
    cluster_t* clus = cluster + cr*cluster_width + cc;
    float* mtx = ent->mtx;

    if (ent->clus != clus) {
        if (ent->clus) list_remove(ent->clus->ents, ent, 0);
        list_add(clus->ents, ent);
        ent->clus = clus;
    }

    mtx[0] = 1; mtx[4] = 0; mtx[8] = 0; mtx[12] = ((float)ent->x)/32.0;
    mtx[1] = 0; mtx[5] = 1; mtx[9] = 0; mtx[13] = ((float)ent->y)/32.0;
    mtx[2] = 0; mtx[6] = 0; mtx[10]= 1; mtx[14] = ((float)ent->z)/32.0;
    mtx[3] = 0; mtx[7] = 0; mtx[11]= 0; mtx[15] = 1;
}

void ent_set_model(entity_t* ent, model_t* mdl)
{
    ent->mdl = mdl;
    ent_update(ent);
}

void ent_move(entity_t* ent, int x, int y, int z)
{
    ent->x = x;
    ent->y = y;
    ent->z = z;
    ent_update(ent);
}
