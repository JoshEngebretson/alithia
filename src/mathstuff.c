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

void vec_normalize(vector_t* v)
{
    float len = vec_len(v);
    if (len > 0.0) {
        v->x /= len;
        v->y /= len;
        v->z /= len;
    }
}

void vec_sub(vector_t* a, vector_t* b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
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
