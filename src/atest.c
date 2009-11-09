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

#define CELLVSIZE 2

#define MAX_POINTS 16384

typedef struct _bucket_t
{
    texture_t* tex;
    GLuint* dl;
    GLfloat** dlmtx;
    int dls;
    int dlc;
} bucket_t;

static bucket_t bucket[4096];
static int buckets = 0;

typedef struct _geoquad_t
{
    float u[4], v[4];
    float x[4], y[4], z[4];
    int ignore;
} geoquad_t;

typedef struct _partbuild_t
{
    texture_t* tex;
    geoquad_t* q;
    int qc;
} partbuild_t;

static partbuild_t* pb;
static int pbc;

int running = 1;
int argc;
char** argv;
char status[256];
int statlen = 0;
int millis = 0;
uint32_t mark;
uint32_t frames = 0;
uint32_t calls = 0;
uint32_t vertices = 0;
int key[1024];
int pntx[MAX_POINTS], pnty[MAX_POINTS];
int pntc = 0;
int camx = 0, camy = 0;
int down = 0;
int draw_map = 0;
int draw_wire = 0;
float plx, ply, plz, pla, pll;
texture_t* tex_bricks;
texture_t* tex_floor;
texture_t* tex_stuff;
texture_t* tex_wtf;
float frustum[6][4];
float proj[16];
float modl[16];
float clip[16];
font_t* normal;


static void process_events(void)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_MOUSEMOTION:
            pla -= ev.motion.xrel/10.0;
            pll -= ev.motion.yrel/10.0;
            if (pll > 60) pll = 60;
            if (pll < -60) pll = -60;
            break;
        case SDL_MOUSEBUTTONDOWN:
            down = 1;
            break;
        case SDL_MOUSEBUTTONUP:
            down = 0;
            break;
        case SDL_KEYDOWN:
            if (ev.key.keysym.sym == SDLK_F10) running = 0;
            if (ev.key.keysym.sym == SDLK_m) draw_map = !draw_map;
            if (ev.key.keysym.sym == SDLK_q) draw_wire = !draw_wire;
            key[ev.key.keysym.sym] = 1;
            break;
        case SDL_KEYUP:
            key[ev.key.keysym.sym] = 0;
            break;
        case SDL_QUIT:
            running = 0;
            break;
        }
    }
}

static void calc_campoints(float r)
{
    int i;
    pntc = 0;
    for (i = 0; i < 3600; i++) {
        int px = (int) (cos(i * PI / 1800.0) * r);
        int py = (int) (sin(i * PI / 1800.0) * r);
        if (i == 0 || pntx[pntc - 1] != px || pnty[pntc - 1] != py) {
            pntx[pntc] = px;
            pnty[pntc] = py;
            pntc++;
        }
    }
    for (i=1; i<pntc-1; i++) {
        int px = pntx[i];
        int py = pnty[i];
        int ox = pntx[i - 1];
        int oy = pnty[i - 1];
        int nx = pntx[i + 1];
        int ny = pnty[i + 1];
        int remove = 0;

        if (px == ox - 1 && py == oy && px == nx && py == ny - 1) {
            remove = 1;
        } else if (px == ox + 1 && py == oy && px == nx && py == ny - 1) {
            remove = 1;
        } else if (px == ox - 1 && py == oy && px == nx && py == ny + 1) {
            remove = 1;
        } else if (px == ox + 1 && py == oy && px == nx && py == ny + 1) {
            remove = 1;
        } else if (px == ox && py == oy - 1 && px == nx + 1 && py == ny) {
            remove = 1;
        } else if (px == ox && py == oy + 1 && px == nx + 1 && py == ny) {
            remove = 1;
        } else if (px == ox && py == oy - 1 && px == nx - 1 && py == ny) {
            remove = 1;
        } else if (px == ox && py == oy + 1 && px == nx - 1 && py == ny) {
            remove = 1;
        }

        if (remove) {
            for (; i < pntc - 1; i++) {
                pntx[i] = pntx[i + 1];
                pnty[i] = pnty[i + 1];
            }
            pntc--;
            i = 0;
        }
    }
}

static void bar(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static int proc_map(int x, int y)
{
    int addr;
    cluster[(y / CLUSTERSIZE) * cluster_width + (x / CLUSTERSIZE)].visible = 1;
    if (x / CLUSTERSIZE < cluster_width - 1) cluster[(y / CLUSTERSIZE)
        * cluster_width + (x / CLUSTERSIZE) + 1].visible = 1;

    addr = y*map_width + x;
    return !(ocmap[(addr>>3)] & (1 << (addr&7)));
}

void ray_march(int x1, int y1, int x2, int y2)
{
    int x, y, dx, dy, steep = abs(y2 - y1) > abs(x2 - x1), ystep = 1, error;
    if (steep) {
        x = x1;
        x1 = y1;
        y1 = x;
        x = x2;
        x2 = y2;
        y2 = x;
    }
    if (x1 > x2) {
        dx = x1 - x2;
        dy = abs(y2 - y1);
        error = dx >> 1;
        if (y1 < y2) ystep = -1;
        y = y1;
        if (steep) {
            for (x = x1; x > x2; x--) {
                if (!proc_map(y, x)) return;
                error -= dy;
                if (error < 0) {
                    y += ystep;
                    error += dx;
                }
            }
        } else {
            for (x = x1; x > x2; x--) {
                if (!proc_map(x, y)) return;
                error -= dy;
                if (error < 0) {
                    y += ystep;
                    error += dx;
                }
            }
        }
    } else {
        dx = x2 - x1;
        dy = abs(y2 - y1);
        error = dx >> 1;
        if (y2 < y1) ystep = -1;
        y = y1;
        if (steep) {
            for (x = x1; x < x2; x++) {
                if (!proc_map(y, x)) return;
                error -= dy;
                if (error < 0) {
                    y += ystep;
                    error += dx;
                }
            }
        } else {
            for (x = x1; x < x2; x++) {
                if (!proc_map(x, y)) return;
                error -= dy;
                if (error < 0) {
                    y += ystep;
                    error += dx;
                }
            }
        }
    }
}

static void cell_vertices(cell_t* c, int x, int y, float* vx, float* vy,
    float* vz, int floor)
{
    vx[0] = x * CELLSIZE;
    vy[0] = floor ? c->floorz : c->ceilz;
    vz[0] = y * CELLSIZE;
    vx[1] = x * CELLSIZE;
    vy[1] = floor ? c->floorz : c->ceilz;
    vz[1] = y * CELLSIZE + CELLSIZE;
    vx[2] = x * CELLSIZE + CELLSIZE;
    vy[2] = floor ? c->floorz : c->ceilz;
    vz[2] = y * CELLSIZE + CELLSIZE;
    vx[3] = x * CELLSIZE + CELLSIZE;
    vy[3] = floor ? c->floorz : c->ceilz;
    vz[3] = y * CELLSIZE;
}

static void do_quad(texture_t* tex, float x1, float y1, float z1, float x2,
    float y2, float z2, float x3, float y3, float z3, float x4, float y4,
    float z4, float ux, float uy, float uz, float vx, float vy, float vz)
{
    int i;
    partbuild_t* part = NULL;
    for (i = 0; i < pbc; i++)
        if (pb[i].tex == tex) {
            part = pb + i;
            break;
        }
    if (!part) {
        pb = realloc(pb, sizeof(partbuild_t) * (pbc + 1));
        pb[pbc].q = NULL;
        pb[pbc].qc = 0;
        pb[pbc].tex = tex;
        part = pb + pbc;
        pbc++;
    }

    part->q = realloc(part->q, sizeof(geoquad_t)*(part->qc + 1));
    part->q[part->qc].u[0] = (x1 * ux + y1 * uy + z1 * uz) / 256.0f;
    part->q[part->qc].v[0] = (x1 * vx + y1 * vy + z1 * vz) / 256.0f;
    part->q[part->qc].u[1] = (x2 * ux + y2 * uy + z2 * uz) / 256.0f;
    part->q[part->qc].v[1] = (x2 * vx + y2 * vy + z2 * vz) / 256.0f;
    part->q[part->qc].u[2] = (x3 * ux + y3 * uy + z3 * uz) / 256.0f;
    part->q[part->qc].v[2] = (x3 * vx + y3 * vy + z3 * vz) / 256.0f;
    part->q[part->qc].u[3] = (x4 * ux + y4 * uy + z4 * uz) / 256.0f;
    part->q[part->qc].v[3] = (x4 * vx + y4 * vy + z4 * vz) / 256.0f;

    part->q[part->qc].x[0] = x1;
    part->q[part->qc].y[0] = y1;
    part->q[part->qc].z[0] = z1;
    part->q[part->qc].x[1] = x2;
    part->q[part->qc].y[1] = y2;
    part->q[part->qc].z[1] = z2;
    part->q[part->qc].x[2] = x3;
    part->q[part->qc].y[2] = y3;
    part->q[part->qc].z[2] = z3;
    part->q[part->qc].x[3] = x4;
    part->q[part->qc].y[3] = y4;
    part->q[part->qc].z[3] = z4;

    part->q[part->qc].ignore = 0;

    part->qc++;
}

static void draw_cell(cell_t* c, int cx, int cy)
{
    cell_t* cu = cell + (cy - 1) * map_width + cx;
    cell_t* cd = cell + (cy + 1) * map_width + cx;
    cell_t* cl = cell + cy * map_width + cx - 1;
    cell_t* cr = cell + cy * map_width + cx + 1;
    float vx[4], vy[4], vz[4];
    float avx[4], avy[4], avz[4];

    /**** FLOOR ****/
    cell_vertices(c, cx, cy, vx, vy, vz, 1);
    /* floor quad */
    if (c->floorz != c->ceilz) {
        do_quad(tex_floor, vx[0], vy[0], vz[0], vx[1], vy[1], vz[1], vx[2],
            vy[2], vz[2], vx[3], vy[3], vz[3], 1, 0, 0, 0, 0, 1);
    }

    /* up cap */
    if (cy > 0 && cu->floorz < c->floorz) {
        cell_vertices(cu, cx, cy - 1, avx, avy, avz, 1);
        do_quad(tex_bricks, vx[3], vy[3], vz[3], avx[2], avy[2], avz[2],
            avx[1], avy[1], avz[1], vx[0], vy[0], vz[0], -1, 0, 0, 0, -1, 0);
    }
    /* down cap */
    if (cy < map_height - 1 && cd->floorz < c->floorz) {
        cell_vertices(cd, cx, cy + 1, avx, avy, avz, 1);
        do_quad(tex_bricks, vx[1], vy[1], vz[1], avx[0], avy[0], avz[0],
            avx[3], avy[3], avz[3], vx[2], vy[2], vz[2], 1, 0, 0, 0, -1, 0);
    }
    /* left cap */
    if (cx > 0 && cl->floorz < c->floorz) {
        cell_vertices(cl, cx - 1, cy, avx, avy, avz, 1);
        do_quad(tex_bricks, vx[0], vy[0], vz[0], avx[3], avy[3], avz[3],
            avx[2], avy[2], avz[2], vx[1], vy[1], vz[1], 0, 0, 1, 0, -1, 0);
    }
    /* right cap */
    if (cx < map_width - 1 && cr->floorz < c->floorz) {
        cell_vertices(cr, cx + 1, cy, avx, avy, avz, 1);
        do_quad(tex_bricks, vx[2], vy[2], vz[2], avx[1], avy[1], avz[1],
            avx[0], avy[0], avz[0], vx[3], vy[3], vz[3], 0, 0, -1, 0, -1, 0);
    }

    /**** CEILING ****/
    cell_vertices(c, cx, cy, vx, vy, vz, 0);
    /* ceiling quad */
    if (c->floorz != c->ceilz) {
        do_quad(tex_wtf, vx[0], vy[0], vz[0], vx[3], vy[3], vz[3], vx[2],
            vy[2], vz[2], vx[1], vy[1], vz[1], 1, 0, 0, 0, 0, -1);
    }

    /* up cap */
    if (cy > 0 && cu->ceilz > c->ceilz) {
        cell_vertices(cu, cx, cy - 1, avx, avy, avz, 0);
        do_quad(tex_stuff, avx[2], avy[2], avz[2], vx[3], vy[3], vz[3], vx[0],
            vy[0], vz[0], avx[1], avy[1], avz[1], -1, 0, 0, 0, -1, 0);
    }
    /* down cap */
    if (cy < map_height - 1 && cd->ceilz > c->ceilz) {
        cell_vertices(cd, cx, cy + 1, avx, avy, avz, 0);
        do_quad(tex_stuff, avx[0], avy[0], avz[0], vx[1], vy[1], vz[1], vx[2],
            vy[2], vz[2], avx[3], avy[3], avz[3], 1, 0, 0, 0, -1, 0);
    }
    /* left cap */
    if (cx > 0 && cl->ceilz > c->ceilz) {
        cell_vertices(cl, cx - 1, cy, avx, avy, avz, 0);
        do_quad(tex_stuff, avx[3], avy[3], avz[3], vx[0], vy[0], vz[0], vx[1],
            vy[1], vz[1], avx[2], avy[2], avz[2], 0, 0, 1, 0, -1, 0);
    }

    /* right cap */
    if (cx < map_width - 1 && cr->ceilz > c->ceilz) {
        cell_vertices(cr, cx + 1, cy, avx, avy, avz, 0);
        do_quad(tex_stuff, avx[1], avy[1], avz[1], vx[2], vy[2], vz[2], vx[3],
            vy[3], vz[3], avx[0], avy[0], avz[0], 0, 0, -1, 0, -1, 0);
    }
}

static void plane_from_three_points(float x1, float y1, float z1, float x2,
    float y2, float z2, float x3, float y3, float z3, float* nx, float* ny,
    float* nz, float* d)
{
    float abx = x2 - x1;
    float aby = y2 - y1;
    float abz = z2 - z1;
    float acx = x3 - x1;
    float acy = y3 - y1;
    float acz = z3 - z1;
    float len;
    *nx = aby*acz - acy*abz;
    *ny = abz*acx - acz*abx;
    *nz = abx*acy - acx*aby;
    len = sqrt(SQR(*nx) + SQR(*ny) + SQR(*nz));
    if (len > 0.0f) {
        *nx /= len;
        *ny /= len;
        *nz /= len;
    }
    *d = x1*(*nx) + y1*(*ny) + z1*(*nz);
}

static int same_plane(geoquad_t* a, geoquad_t* b)
{
    float nx1, ny1, nz1, d1;
    float nx2, ny2, nz2, d2;
    plane_from_three_points(a->x[0], a->y[0], a->z[0], a->x[1], a->y[1], a->z[1], a->x[2], a->y[2], a->z[2], &nx1, &ny1, &nz1, &d1);
    plane_from_three_points(b->x[0], b->y[0], b->z[0], b->x[1], b->y[1], b->z[1], b->x[2], b->y[2], b->z[2], &nx2, &ny2, &nz2, &d2);
    return (nx1 == nx2) && (ny1 == ny2) && (nz1 == nz2) && (d1 == d2);
}

static int optimize_part(partbuild_t* pb)
{
    int i, j, k, l, m, cmn_vtx;
    int optimizations = 0;
    for (i=0; i<pb->qc; i++) {
        geoquad_t* a = pb->q + i;
        if (a->ignore) continue;
        for (j=0; j<pb->qc; j++) {
            geoquad_t* b = pb->q + j;
            float x[4], y[4], z[4], u[4], v[4];
            int coma[4], comb[4];
            if (i == j) continue;
            if (b->ignore) continue;
            if (!same_plane(a, b)) continue;

            memset(coma, 0, sizeof(coma));
            memset(comb, 0, sizeof(comb));
            cmn_vtx = 0;
            for (k=0; k<4; k++) {
                for (l=0; l<4; l++) {
                    if (a->x[k] == b->x[l] && a->y[k] == b->y[l] && a->z[k] == b->z[l]) {
                        cmn_vtx++;
                        coma[k] = 1;
                        comb[l] = 1;
                    }
                }
            }

            if (cmn_vtx != 2) continue;

            m = 0;
            k = 0;
            l = 0;
            while (m < 4) {
                while (1) {
                    if (coma[k]) {
                        k=(k + 1)&3;
                        break;
                    }
                    x[m] = a->x[k];
                    y[m] = a->y[k];
                    z[m] = a->z[k];
                    u[m] = a->u[k];
                    v[m] = a->v[k];
                    coma[k] = 1;
                    m++;
                    k=(k + 1)&3;
                }
                while (1) {
                    if (comb[l]) {
                        l=(l + 1)&3;
                        break;
                    }
                    x[m] = b->x[l];
                    y[m] = b->y[l];
                    z[m] = b->z[l];
                    u[m] = b->u[l];
                    v[m] = b->v[l];
                    comb[l] = 1;
                    m++;
                    l=(l + 1)&3;
                }
            }

            for (k=0; k<4; k++) {
                a->x[k] = x[k];
                a->y[k] = y[k];
                a->z[k] = z[k];
                a->u[k] = u[k];
                a->v[k] = v[k];
                b->ignore = 1;
            }

            optimizations++;
        }
    }

    return optimizations;
}

static void optimize_parts(void)
{
    int i;
    for (i=0; i<pbc; i++)
        while (optimize_part(pb + i));
}

static void add_list_to_bucket(texture_t* tex, GLuint dl, GLfloat* mtx)
{
    bucket_t* buck;
    if (tex->bucket == -1) {
        buck = bucket + buckets;
        buck->dls = 16;
        buck->dlc = 0;
        buck->dl = malloc(sizeof(GLuint) * 16);
        buck->dlmtx = malloc(sizeof(GLfloat*) * 16);
        buck->tex = tex;
        tex->bucket = buckets++;
    } else buck = bucket + tex->bucket;

    if (buck->dls == buck->dlc) {
        buck->dls += 16;
        buck->dl = realloc(buck->dl, sizeof(GLuint) * buck->dls);
        buck->dlmtx = realloc(buck->dlmtx, sizeof(GLfloat*) * buck->dls);
    }
    buck->dl[buck->dlc] = dl;
    buck->dlmtx[buck->dlc] = mtx;
    buck->dlc++;
}

/* function by Mark Morley */
static void calc_frustum_planes(void)
{
    float proj[16];
    float modl[16];
    float clip[16];
    float t;

    /* Get the current PROJECTION matrix from OpenGL */
    glGetFloatv(GL_PROJECTION_MATRIX, proj);

    /* Get the current MODELVIEW matrix from OpenGL */
    glGetFloatv(GL_MODELVIEW_MATRIX, modl);

    /* Combine the two matrices (multiply projection by modelview)    */
    clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8]
        + modl[3] * proj[12];
    clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9]
        + modl[3] * proj[13];
    clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10]
        + modl[3] * proj[14];
    clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11]
        + modl[3] * proj[15];

    clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8]
        + modl[7] * proj[12];
    clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9]
        + modl[7] * proj[13];
    clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10]
        + modl[7] * proj[14];
    clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11]
        + modl[7] * proj[15];

    clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8]
        + modl[11] * proj[12];
    clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9]
        + modl[11] * proj[13];
    clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10]
        + modl[11] * proj[14];
    clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11]
        + modl[11] * proj[15];

    clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8]
        + modl[15] * proj[12];
    clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9]
        + modl[15] * proj[13];
    clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10]
        + modl[15] * proj[14];
    clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11]
        + modl[15] * proj[15];

    /* Extract the numbers for the RIGHT plane */
    frustum[0][0] = clip[3] - clip[0];
    frustum[0][1] = clip[7] - clip[4];
    frustum[0][2] = clip[11] - clip[8];
    frustum[0][3] = clip[15] - clip[12];

    /* Normalize the result */
    t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1]
        + frustum[0][2] * frustum[0][2]);
    frustum[0][0] /= t;
    frustum[0][1] /= t;
    frustum[0][2] /= t;
    frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    frustum[1][0] = clip[3] + clip[0];
    frustum[1][1] = clip[7] + clip[4];
    frustum[1][2] = clip[11] + clip[8];
    frustum[1][3] = clip[15] + clip[12];

    /* Normalize the result */
    t = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1]
        + frustum[1][2] * frustum[1][2]);
    frustum[1][0] /= t;
    frustum[1][1] /= t;
    frustum[1][2] /= t;
    frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    frustum[2][0] = clip[3] + clip[1];
    frustum[2][1] = clip[7] + clip[5];
    frustum[2][2] = clip[11] + clip[9];
    frustum[2][3] = clip[15] + clip[13];

    /* Normalize the result */
    t = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1]
        + frustum[2][2] * frustum[2][2]);
    frustum[2][0] /= t;
    frustum[2][1] /= t;
    frustum[2][2] /= t;
    frustum[2][3] /= t;

    /* Extract the TOP plane */
    frustum[3][0] = clip[3] - clip[1];
    frustum[3][1] = clip[7] - clip[5];
    frustum[3][2] = clip[11] - clip[9];
    frustum[3][3] = clip[15] - clip[13];

    /* Normalize the result */
    t = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1]
        + frustum[3][2] * frustum[3][2]);
    frustum[3][0] /= t;
    frustum[3][1] /= t;
    frustum[3][2] /= t;
    frustum[3][3] /= t;

    /* Extract the FAR plane */
    frustum[4][0] = clip[3] - clip[2];
    frustum[4][1] = clip[7] - clip[6];
    frustum[4][2] = clip[11] - clip[10];
    frustum[4][3] = clip[15] - clip[14];

    /* Normalize the result */
    t = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1]
        + frustum[4][2] * frustum[4][2]);
    frustum[4][0] /= t;
    frustum[4][1] /= t;
    frustum[4][2] /= t;
    frustum[4][3] /= t;

    /* Extract the NEAR plane */
    frustum[5][0] = clip[3] + clip[2];
    frustum[5][1] = clip[7] + clip[6];
    frustum[5][2] = clip[11] + clip[10];
    frustum[5][3] = clip[15] + clip[14];

    /* Normalize the result */
    t = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1]
        + frustum[5][2] * frustum[5][2]);
    frustum[5][0] /= t;
    frustum[5][1] /= t;
    frustum[5][2] /= t;
    frustum[5][3] /= t;
}

/*static int point_in_frustum(float x, float y, float z)
{
    int i;
    for (i = 0; i < 6; i++)
        if (frustum[i][0] * x + frustum[i][1] * y + frustum[i][2] * z
            + frustum[i][3] <= 0.0f) return 0;
    return 1;
}*/

static int box_in_frustum(float x1, float y1, float z1, float x2, float y2,
    float z2)
{
    int i;
    for (i = 0; i < 6; i++) {
        if (frustum[i][0] * x1 + frustum[i][1] * y1 + frustum[i][2] * z1
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x2 + frustum[i][1] * y1 + frustum[i][2] * z1
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x1 + frustum[i][1] * y1 + frustum[i][2] * z2
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x2 + frustum[i][1] * y1 + frustum[i][2] * z2
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x1 + frustum[i][1] * y2 + frustum[i][2] * z1
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x2 + frustum[i][1] * y2 + frustum[i][2] * z1
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x1 + frustum[i][1] * y2 + frustum[i][2] * z2
            + frustum[i][3] > 0.0f) continue;
        if (frustum[i][0] * x2 + frustum[i][1] * y2 + frustum[i][2] * z2
            + frustum[i][3] > 0.0f) continue;
        return 0;
    }
    return 1;
}

static void update(void)
{
    static int init_frames = 10;
    int x, y, i;
    entity_t* e[1024];
    int ec = 0;
    float pmtx[16];
    float vmtx[16];
    float pos[] = { 0, 1, 0, 0 };
    float amb[] = { 0.9, 0.9, 0.9, 1.0 };
    float col[] = { 0.9, 0.9, 0.9, 1.0 };

    /* prepare scene for rendering */
    glClearColor(0.1, 0.12, 0.2, 1.0);
    glClearStencil(0);
    if (draw_wire)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    else
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (init_frames)
        init_frames--;

    /* prepare for rendering the perspective from camera */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)vid_width/(double)vid_height, 1.0, 65536.0);
    glGetFloatv(GL_PROJECTION_MATRIX, pmtx);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-pll, 1, 0, 0);
    glRotatef(-pla, 0, 1, 0);
    glTranslatef(-plx, -ply, -plz);
    glGetFloatv(GL_MODELVIEW_MATRIX, vmtx);
    glColor3f(1, 1, 1);

    camx = plx/CELLSIZE;
    camy = plz/CELLSIZE;

    /* calculate frustum planes */
    calc_frustum_planes();

    /* find visible clusters */
    for (x = 0; x < pntc; x++)
        ray_march(camx, camy, camx + pntx[x], camy + pnty[x]);

    /* enable texturing and depth tests */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);

    if (draw_wire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    calls += 10;

    /* fill the buckets with the world geometry */
    for (y = 0; y < cluster_height; y++) {
        for (x = 0; x < cluster_width; x++) {
            cluster_t* cls = cluster + y * cluster_width + x;
            /* test if the cluster is inside the frustum */
            if (cls->visible) {
                cls->visible = box_in_frustum(cls->x1, cls->y1, cls->z1,
                    cls->x2, cls->y2, cls->z2);
            }
            /* inside */
            if (cls->visible || init_frames > 0) {
                int i;
                if (!cls->part) {
                    int x1 = x * CLUSTERSIZE;
                    int y1 = y * CLUSTERSIZE;
                    int x2 = x * CLUSTERSIZE + CLUSTERSIZE;
                    int y2 = y * CLUSTERSIZE + CLUSTERSIZE;
                    int cx, cy;
                    pb = NULL;
                    pbc = 0;
                    for (cy = y1; cy < y2; cy++) {
                        for (cx = x1; cx < x2; cx++) {
                            cell_t* c = cell + cy * map_width + cx;
                            if ((cy == y1 && cx == x1) || (cls->y1 > c->floorz)) cls->y1
                                = c->floorz;
                            if ((cy == y1 && cx == x1) || (cls->y2 < c->ceilz)) cls->y2
                                = c->ceilz;
                            draw_cell(c, cx, cy);
                        }
                    }
                    optimize_parts();
                    if (pb) {
                        cls->part = malloc0(sizeof(clusterpart_t) * pbc);
                        cls->parts = pbc;
                        cls->vertices = 0;
                        for (i = 0; i < pbc; i++) {
                            int j;
                            cls->part[i].dl = glGenLists(1);
                            cls->part[i].tex = pb[i].tex;
                            glNewList(cls->part[i].dl, GL_COMPILE);
                            glBegin(GL_QUADS);
                            for (j = 0; j < pb[i].qc; j++) {
                                geoquad_t* q = pb[i].q + j;
                                int v;
                                if (q->ignore) continue;

                                cls->vertices += 4;

                                for (v=0; v<4; v++) {
                                    glMultiTexCoord2f(GL_TEXTURE0, q->u[v], q->v[v]);
                                    glMultiTexCoord2f(GL_TEXTURE1, q->x[v]/(float)map_width/CELLSIZE, q->z[v]/(float)map_height/CELLSIZE);
                                    glVertex3f(q->x[v], q->y[v], q->z[v]);
                                    calls += 3;
                                }
                            }
                            glEnd();
                            glEndList();
                            calls += 4;
                            free(pb[i].q);
                        }
                        free(pb);
                    }
                }
                for (i = 0; i < cls->parts; i++)
                    add_list_to_bucket(cls->part[i].tex, cls->part[i].dl, NULL);
                vertices += cls->vertices;

                if (cls->ents->first) {
                    listitem_t* item;
                    for (item = cls->ents->first; item; item = item->next) {
                        entity_t* en = item->ptr;
                        if (en->mdl) { /* TODO: use a separate list for models only */
                            e[ec++] = en;
                            vertices += en->mdl->fc*3*2;
                        }
                    }
                }
                if (!draw_map) cls->visible = 0;
            }
        }
    }

    /* fill the buckets with the entity geometry */
    for (i = 0; i < ec; i++) {
        entity_t* ent = e[i];
        if (!ent->mdl->dl) {
            /* normal display list */
            ent->mdl->dl = glGenLists(1);
            glNewList(ent->mdl->dl, GL_COMPILE);
            glBegin(GL_TRIANGLES);
            for (i = 0; i < ent->mdl->fc; i++) {
                float* v = ent->mdl->v + (ent->mdl->f[i * 3] * 8);
                glMultiTexCoord2f(GL_TEXTURE0, v[6], v[7]);
                glMultiTexCoord2f(GL_TEXTURE1, v[0] / (float) map_width
                    / CELLSIZE, v[2] / (float) map_height / CELLSIZE);
                glNormal3f(v[3], v[4], v[5]);
                glVertex3f(v[0], v[1], v[2]);
                v = ent->mdl->v + (ent->mdl->f[i * 3 + 1] * 8);
                glMultiTexCoord2f(GL_TEXTURE0, v[6], v[7]);
                glMultiTexCoord2f(GL_TEXTURE1, v[0] / (float) map_width
                    / CELLSIZE, v[2] / (float) map_height / CELLSIZE);
                glNormal3f(v[3], v[4], v[5]);
                glVertex3f(v[0], v[1], v[2]);
                v = ent->mdl->v + (ent->mdl->f[i * 3 + 2] * 8);
                glMultiTexCoord2f(GL_TEXTURE0, v[6], v[7]);
                glMultiTexCoord2f(GL_TEXTURE1, v[0] / (float) map_width
                    / CELLSIZE, v[2] / (float) map_height / CELLSIZE);
                glNormal3f(v[3], v[4], v[5]);
                glVertex3f(v[0], v[1], v[2]);
            }
            glEnd();
            glEndList();
            /* shadow display list */
            ent->mdl->dlshadow = glGenLists(1);
            glNewList(ent->mdl->dlshadow, GL_COMPILE);
            glBegin(GL_TRIANGLES);
            for (i = 0; i < ent->mdl->fc; i++) {
                float* v = ent->mdl->v + (ent->mdl->f[i * 3] * 8);
                glVertex3f(v[0], 0, v[2]);
                v = ent->mdl->v + (ent->mdl->f[i * 3 + 1] * 8);
                glVertex3f(v[0], 0, v[2]);
                v = ent->mdl->v + (ent->mdl->f[i * 3 + 2] * 8);
                glVertex3f(v[0], 0, v[2]);
            }
            glEnd();
            glEndList();
        }
        add_list_to_bucket(ent->mdl->tex, ent->mdl->dl, ent->mtx);
    }

    glEnable(GL_LIGHT0);
    calls++;
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    calls++;

    /* render buckets */
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    calls++;
    glBindTexture(GL_TEXTURE_2D, lmaptex);
    calls++;
    glActiveTexture(GL_TEXTURE0);
    calls++;
    for (i = 0; i < buckets; i++)
        if (bucket[i].dlc) {
            int j;
            calls++;
            glBindTexture(GL_TEXTURE_2D, bucket[i].tex->name);
            for (j = 0; j < bucket[i].dlc; j++) {
                if (bucket[i].dlmtx[j]) {
                    int bx = bucket[i].dlmtx[j][12] / CELLSIZE;
                    int by = bucket[i].dlmtx[j][14] / CELLSIZE;
                    lmap_texel_t* tex = lightmap + by * map_width + bx;
                    col[0] = (float) tex->r / 255.0f;
                    col[1] = (float) tex->g / 255.0f;
                    col[2] = (float) tex->b / 255.0f;
                    amb[0] = (float) tex->r / 767.0f;
                    amb[1] = (float) tex->g / 767.0f;
                    amb[2] = (float) tex->b / 767.0f;
                    glActiveTexture(GL_TEXTURE1);
                    glDisable(GL_TEXTURE_2D);
                    glActiveTexture(GL_TEXTURE0);
                    glEnable(GL_LIGHTING);
                    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
                    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
                    glPushMatrix();
                    glMultMatrixf(bucket[i].dlmtx[j]);
                    glCallList(bucket[i].dl[j]);
                    glPopMatrix();
                    glDisable(GL_LIGHTING);
                    glActiveTexture(GL_TEXTURE1);
                    glEnable(GL_TEXTURE_2D);
                    glActiveTexture(GL_TEXTURE0);
                    calls += 14;
                } else {
                    glCallList(bucket[i].dl[j]);
                    calls++;
                }
            }
            bucket[i].dlc = 0;
        }
    glActiveTexture(GL_TEXTURE0);
    calls++;

    if (draw_wire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* render shadows in the stencil buffer */
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    calls += 9;

    for (i = 0; i < ec; i++) {
        entity_t* ent = e[i];
        int shadow_y = cell[((ent->z >> 5) / CELLSIZE) * map_width + ((ent->x
            >> 5) / CELLSIZE)].floorz;
        glPushMatrix();
        glTranslatef((float) ent->x / 32.0, shadow_y, (float) ent->z / 32.0);
        glCallList(ent->mdl->dlshadow);
        glPopMatrix();
        calls += 4;
    }

    /* draw fullscreen quad using the stencil buffer to affect only shadows */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColorMask(1, 1, 1, 1);

    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

    glColor4f(0, 0, 0, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glVertex2d(-1, -1);
    glVertex2d(1, -1);
    glVertex2d(1, 1);
    glVertex2d(-1, 1);
    glEnd();
    vertices += 4;

    glDisable(GL_BLEND);
    glDepthMask(1);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_POLYGON_OFFSET_FILL);

    calls += 20;

    /* hud */
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glColor4f(1, 1, 1, 1);
    font_render(normal, -1, 1 - 0.06, status, statlen, 0.06);

    /* overlay map */
    glDisable(GL_TEXTURE_2D);

    calls += 3;

    if (draw_map) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glTranslatef(10, 10, 0);

        for (y = 0; y < map_height; y++) {
            for (x = 0; x < map_width; x++) {
                float v = (cell[y * map_width + x].flags & CF_OCCLUDER) ? 1.0
                    : 0.0;
                glColor3f(v * 0.4, v * 0.7, v * 0.8);
                bar(x * CELLVSIZE, y * CELLVSIZE, x * CELLVSIZE + CELLVSIZE, y
                    * CELLVSIZE + CELLVSIZE);
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        i = 0;
        for (y = 0; y < cluster_height; y++) {
            for (x = 0; x < cluster_width; x++) {
                int v = cluster[y * cluster_width + x].visible;
                if (v) {
                    glColor4f(0.8, 0.5, 0.3, 0.3);
                    bar(x * CELLVSIZE * CLUSTERSIZE, y * CELLVSIZE
                        * CLUSTERSIZE, x * CELLVSIZE * CLUSTERSIZE + CELLVSIZE
                        * CLUSTERSIZE, y * CELLVSIZE * CLUSTERSIZE + CELLVSIZE
                        * CLUSTERSIZE);
                    i++;
                }
                cluster[y * cluster_width + x].visible = 0;
            }
        }
        glDisable(GL_BLEND);
    }

    /* present the backbuffer to the screen */
    SDL_GL_SwapBuffers();
    calls++;

    /* FPS calculation */
    if (++frames == 10) {
        uint32_t ticks = SDL_GetTicks();
        double diff = ((double) (ticks - mark)) / 10.0;
        sprintf(status, "%0.2f FPS (%0.2f ms/frame)\n%i calls %i vertices", (float) (1000.0
            / diff), diff, calls / 10, vertices / 10);
        statlen = strlen(status);
        mark = SDL_GetTicks();
        frames = 0;
        calls = 0;
        vertices = 0;
    }

    ((entity_t*) ents->last->ptr)->yrot = SDL_GetTicks() * 15;
}

static int collides(void)
{
    int x, y, x1, y1, x2, y2;
    x1 = plx/CELLSIZE - 2;
    y1 = plz/CELLSIZE - 2;
    x2 = plx/CELLSIZE + 2;
    y2 = plz/CELLSIZE + 2;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= map_width) x2 = map_width - 1;
    if (y2 >= map_height) y2 = map_height - 1;
    for (y=y1; y<=y2; y++) for (x=x1; x<=x2; x++)
        if (cell[y*map_width + x].flags & CF_OCCLUDER) return 1;
    return 0;
}

static void move_towards(float ix, float iy, float iz)
{
    plx += ix;
    if (collides()) plx -= ix;
    ply += iy;
    if (collides()) ply -= iy;
    plz += iz;
    if (collides()) plz -= iz;
}

static void update_game(float ms)
{
    float floorz = cell[camy*map_width + camx].floorz;
    if (key[SDLK_w]) {
        move_towards(-sin(pla*PI/180.0)*ms*0.55, 0, -cos(pla*PI/180.0)*ms*0.55);
    }
    if (key[SDLK_s]) {
        move_towards(sin(pla*PI/180.0)*ms*0.55, 0, cos(pla*PI/180.0)*ms*0.55);
    }
    if (key[SDLK_a]) {
        move_towards(sin((pla-90)*PI/180.0)*ms*0.55, 0, cos((pla-90)*PI/180.0)*ms*0.55);
    }
    if (key[SDLK_d]) {
        move_towards(sin((pla+90)*PI/180.0)*ms*0.55, 0, cos((pla+90)*PI/180.0)*ms*0.55);
    }

    if (floorz > ply + 48) {
        ply += ms*4;
        if (ply > floorz + 48) ply = floorz + 48;
    }
    if (floorz < ply + 48) {
        ply -= ms;
        if (ply < floorz + 48) ply = floorz + 48;
    }
}

static void run(void)
{
    tex_bricks = tex_load("data/textures/bricks.bmp");
    tex_floor = tex_load("data/textures/floor.bmp");
    tex_stuff = tex_load("data/textures/stuff.bmp");
    tex_wtf = tex_load("data/textures/wtf.bmp");
    normal = font_load("data/fonts/normal.bifo");
    if (!normal) printf("normal failed\n");

    map_init(256, 256);
    calc_campoints(sqrt(256*256 + 256*256));
    lmap_update();
    plx = (map_width*CELLSIZE)*0.5;
    ply = -128+48;
    plz = (map_width*CELLSIZE)*0.5;

    mark = SDL_GetTicks();
    millis = mark;
    while (running) {
        int error = glGetError();
        int nowmillis = SDL_GetTicks();
        if (nowmillis - millis > 15) {
            while (nowmillis - millis > 15) {
                update_game(15);
                millis += 15;
            }
            if (nowmillis - millis > 0)
                update_game(nowmillis - millis);
            millis = nowmillis;
        }
        if (error != GL_NONE) printf("GL error: %s\n", gluErrorString(error));
        process_events();
        update();
    }
}

extern uint64_t total;
int main(int _argc, char **_argv)
{
    argc = _argc;
    argv = _argv;
    if (!vid_init()) return 1;

    run();

    vid_shutdown();
    return 0;
}
