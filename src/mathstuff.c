/*
 * Alithia Engine
 * Copyright (C) 2009-2010 Kostas Michalopoulos
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

void vec_makenormal(vector_t* n, vector_t* v)
{
    vector_t a;
    vector_t b;
    vec_set(&a, v[1].x - v[0].x, v[1].y - v[0].y, v[1].z - v[0].z);
    vec_set(&b, v[2].x - v[0].x, v[2].y - v[0].y, v[2].z - v[0].z);
    vec_cross(n, &a, &b);
    vec_normalize(n);
}

void vec_normalize(vector_t* v)
{
    float len = vec_len(v);
    if (len > 0.0) {
        v->x /= len;
        v->y /= len;
        v->z /= len;
    }
}

void vec_add(vector_t* a, vector_t* b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

void vec_sub(vector_t* a, vector_t* b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

void vec_cross(vector_t* r, vector_t* a, vector_t* b)
{
    r->x = a->y*b->z - a->z*b->y;
    r->y = a->z*b->x - a->x*b->z;
    r->z = a->x*b->y - a->y*b->x;
}

void vec_makedir(vector_t* d, vector_t* a, vector_t* b)
{
    *d = *b;
    vec_sub(d, a);
    vec_normalize(d);
}

float vec_distsq(vector_t* a, vector_t* b)
{
    return SQR(b->x - a->x) + SQR(b->y - a->y) + SQR(b->z - a->z);
}

float vec_dist(vector_t* a, vector_t* b)
{
    return sqrtf(SQR(b->x - a->x) + SQR(b->y - a->y) + SQR(b->z - a->z));
}

void mat_identity(matrix_t mat)
{
    mat[0] = 1;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;
    mat[4] = 0;
    mat[5] = 1;
    mat[6] = 0;
    mat[7] = 0;
    mat[8] = 0;
    mat[9] = 1;
    mat[10] = 0;
    mat[11] = 0;
    mat[12] = 0;
    mat[13] = 0;
    mat[14] = 0;
    mat[15] = 1;
}

void mat_transform_vector(matrix_t mat, vector_t* r, vector_t* v)
{
    r->x = mat[0]*v->x + mat[1]*v->y + mat[2]*v->z + mat[12];
    r->y = mat[4]*v->x + mat[5]*v->y + mat[6]*v->z + mat[13];
    r->z = mat[8]*v->x + mat[9]*v->y + mat[10]*v->z + mat[14];
}

int plane_from_three_points(plane_t* p, vector_t* a, vector_t* b, vector_t* c)
{
    float abx = b->x - a->x;
    float aby = b->y - a->y;
    float abz = b->z - a->z;
    float acx = c->x - a->x;
    float acy = c->y - a->y;
    float acz = c->z - a->z;
    float len;
    p->nx = aby*acz - acy*abz;
    p->ny = abz*acx - acz*abx;
    p->nz = abx*acy - acx*aby;
    len = sqrtf(p->nx*p->nx + p->ny*p->ny + p->nz*p->nz);
    if (len > 0.0) {
        p->nx /= len;
        p->ny /= len;
        p->nz /= len;
    } else return 0;
    p->d = a->x*p->nx + a->y*p->ny + a->z*p->nz;
    return 1;
}

int plane_from_three_points_xyz(plane_t* p, float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz)
{
    float abx = bx - ax;
    float aby = by - ay;
    float abz = bz - az;
    float acx = cx - ax;
    float acy = cy - ay;
    float acz = cz - az;
    float len;
    p->nx = aby*acz - acy*abz;
    p->ny = abz*acx - acz*abx;
    p->nz = abx*acy - acx*aby;
    len = sqrtf(p->nx*p->nx + p->ny*p->ny + p->nz*p->nz);
    if (len > 0.0) {
        p->nx /= len;
        p->ny /= len;
        p->nz /= len;
    } else return 0;
    p->d = ax*p->nx + ay*p->ny + az*p->nz;
    return 1;
}

void plane_from_point_and_normal(plane_t* p, vector_t* v, vector_t* n)
{
    p->nx = n->x;
    p->ny = n->y;
    p->nz = n->z;
    p->d = v->x*n->x + v->y*n->y + v->z*n->z;
}

void plane_project(plane_t* p, vector_t* v)
{
    double sd = (v->x - p->nx*p->d)*p->nx + (v->y - p->ny*p->d)*p->ny + (v->z - p->nz*p->d)*p->nz;
    v->x -= p->nx*sd;
    v->y -= p->ny*sd;
    v->z -= p->nz*sd;
}

float plane_signdist(plane_t* p, vector_t* v)
{
    return (v->x - p->nx*p->d)*p->nx + (v->y - p->ny*p->d)*p->ny + (v->z - p->nz*p->d)*p->nz;
}

void aabb_zero(aabb_t* aabb)
{
    aabb->min.x = aabb->min.y = aabb->min.z = aabb->max.x = aabb->max.y = aabb->max.z = 0.0f;
}

void aabb_consider(aabb_t* aabb, vector_t* v)
{
    if (v->x < aabb->min.x) aabb->min.x = v->x;
    if (v->y < aabb->min.y) aabb->min.y = v->y;
    if (v->z < aabb->min.z) aabb->min.z = v->z;
    if (v->x > aabb->max.x) aabb->max.x = v->x;
    if (v->y > aabb->max.y) aabb->max.y = v->y;
    if (v->z > aabb->max.z) aabb->max.z = v->z;
}

void aabb_consider_xyz(aabb_t* aabb, float x, float y, float z)
{
    if (x < aabb->min.x) aabb->min.x = x;
    if (y < aabb->min.y) aabb->min.y = y;
    if (z < aabb->min.z) aabb->min.z = z;
    if (x > aabb->max.x) aabb->max.x = x;
    if (y > aabb->max.y) aabb->max.y = y;
    if (z > aabb->max.z) aabb->max.z = z;
}

void aabb_copytrans(aabb_t* tgt, aabb_t* src, float tx, float ty, float tz)
{
    tgt->min.x = src->min.x + tx;
    tgt->min.y = src->min.y + ty;
    tgt->min.z = src->min.z + tz;
    tgt->max.x = src->max.x + tx;
    tgt->max.y = src->max.y + ty;
    tgt->max.z = src->max.z + tz;
}

int ray_plane_intersection(plane_t* p, vector_t* a, vector_t* b, vector_t* ip)
{
    float pax = p->nx*p->d - a->x;
    float pay = p->ny*p->d - a->y;
    float paz = p->nz*p->d - a->z;
    float pbx = b->x - a->x;
    float pby = b->y - a->y;
    float pbz = b->z - a->z;
    float da = p->nx*pax + p->ny*pay + p->nz*paz;
    float db = p->nx*pbx + p->ny*pby + p->nz*pbz;
    float u;
    if (db > -0.00001 && db < 0.00001) return 0;
    u = da/db;
    if (u < 0) return 0;
    if (ip) {
        ip->x = a->x + pbx*u;
        ip->y = a->y + pby*u;
        ip->z = a->z + pbz*u;
    }
    return 1;
}

int ray_tri_intersection(triangle_t* t, vector_t* a, vector_t* b, vector_t* ip, int doublesided)
{
    plane_t p;
    float abx = t->v[1].x - t->v[0].x;
    float aby = t->v[1].y - t->v[0].y;
    float abz = t->v[1].z - t->v[0].z;
    float acx = t->v[2].x - t->v[0].x;
    float acy = t->v[2].y - t->v[0].y;
    float acz = t->v[2].z - t->v[0].z;
    float len, uu, uv, vv, wu, wv, wx, wy, wz, d, sv, tv;
    p.nx = aby*acz - acy*abz;
    p.ny = abz*acx - acz*abx;
    p.nz = abx*acy - acx*aby;
    len = sqrtf(p.nx*p.nx + p.ny*p.ny + p.nz*p.nz);
    if (len > 0.00001) {
        p.nx /= len;
        p.ny /= len;
        p.nz /= len;
    } else return 0;
    p.d = t->v[0].x*p.nx + t->v[0].y*p.ny + t->v[0].z*p.nz;
    if (!ray_plane_intersection(&p, a, b, ip)) return 0;
    if (!doublesided) {
        vector_t ab;
        vec_makedir(&ab, b, a);
        if ((p.nx*ab.x + p.ny*ab.y + p.nz*ab.z) < 0) return 0;
    }
    uu = abx*abx + aby*aby + abz*abz;
    uv = abx*acx + aby*acy + abz*acz;
    vv = acx*acx + acy*acy + acz*acz;
    wx = ip->x - t->v[0].x;
    wy = ip->y - t->v[0].y;
    wz = ip->z - t->v[0].z;
    wu = wx*abx + wy*aby + wz*abz;
    wv = wx*acx + wy*acy + wz*acz;
    d = uv*uv - uu*vv;
    if (d > -0.00001 && d < 0.00001) return 0;
    sv = (uv*wv - vv*wu)/d;
    if (sv < 0.0 || sv > 1.0) return 0;
    tv = (uv*wu - uu*wv)/d;
    if (tv < 0.0 || (sv + tv) > 1.0) return 0;
    return 1;
}

int ray_sphere_intersection(vector_t* sc, float sr, vector_t* a, vector_t* b, vector_t* ip)
{
    vector_t d, dst;
    float bb, cc, dd, u;
    vec_makedir(&d, a, b);
    dst = *a;
    vec_sub(&dst, sc);
    bb = vec_dot(&dst, &d);
    cc = vec_dot(&dst, &dst) - sr*sr;
    dd = bb*bb - cc;
    if (dd < 0.000001) return 0;
    u = -bb - sqrt(dd);
    ip->x = a->x + u*d.x;
    ip->y = a->y + u*d.y;
    ip->z = a->z + u*d.z;
    return 1;
}

int ray_aabb_intersection(aabb_t* aabb, vector_t* a, vector_t* b, vector_t* ip)
{
    /* TODO: find some faster way */
    plane_t planes[6];
    int i, j, ipfound = 0;
    vector_t lip;
    float ipad = 0;
    plane_from_three_points_xyz(planes + 0, aabb->min.x, aabb->max.y, aabb->min.z, aabb->min.x, aabb->max.y, aabb->max.z, aabb->max.x, aabb->max.y, aabb->min.z);
    plane_from_three_points_xyz(planes + 1, aabb->min.x, aabb->max.y, aabb->max.z, aabb->min.x, aabb->min.y, aabb->max.z, aabb->max.x, aabb->max.y, aabb->max.z);
    plane_from_three_points_xyz(planes + 2, aabb->min.x, aabb->min.y, aabb->max.z, aabb->min.x, aabb->min.y, aabb->min.z, aabb->max.x, aabb->min.y, aabb->max.z);
    plane_from_three_points_xyz(planes + 3, aabb->max.x, aabb->max.y, aabb->min.z, aabb->max.x, aabb->min.y, aabb->min.z, aabb->min.x, aabb->max.y, aabb->min.z);
    plane_from_three_points_xyz(planes + 4, aabb->min.x, aabb->max.y, aabb->min.z, aabb->min.x, aabb->min.y, aabb->min.z, aabb->min.x, aabb->min.y, aabb->max.z);
    plane_from_three_points_xyz(planes + 5, aabb->max.x, aabb->max.y, aabb->min.z, aabb->max.x, aabb->max.y, aabb->max.z, aabb->max.x, aabb->min.y, aabb->min.z);
    for (i=0; i<6; i++) {
        if (ray_plane_intersection(planes + i, a, b, &lip)) {
            int inside = 1;
            for (j=0; j<6; j++) {
                if (i != j && plane_signdist(planes + j, &lip) > 0.0) {
                    inside = 0;
                    break;
                }
            }
            if (inside) {
                float pipad = vec_distsq(&lip, a);
                if (!ipfound || pipad < ipad) {
                    ipad = pipad;
                    ipfound = 1;
                    *ip = lip;
                }
            }
        }
    }
    return ipfound;
}

int aabb_aabb_collision(aabb_t* a, aabb_t* b)
{
    return fabsf((b->min.y + b->max.y) - (a->min.y + a->max.y)) < ((a->max.y - a->min.y) + (b->max.y - b->min.y)) &&
           fabsf((b->min.x + b->max.x) - (a->min.x + a->max.x)) < ((a->max.x - a->min.x) + (b->max.x - b->min.x)) &&
           fabsf((b->min.z + b->max.z) - (a->min.z + a->max.z)) < ((a->max.z - a->min.z) + (b->max.z - b->min.z));
}

















