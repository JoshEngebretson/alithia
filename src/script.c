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

#define OT_NOTHING 0
#define OT_ENTITY 1
#define OT_LIGHT 2

typedef struct _objref_t
{
    void* obj;
    int type;
} objref_t;

static objref_t* ref;
static size_t refs;

lil_t lil;

/* Object references */
static size_t or_new(int type, void* obj)
{
    size_t i;
    for (i=0; i<refs; i++)
        if (!ref[i].type) {
            ref[i].type = type;
            ref[i].obj = obj;
            return i + 1;
        }
    ref = realloc(ref, sizeof(objref_t)*(refs + 1));
    ref[refs].type = type;
    ref[refs].obj = obj;
    return ++refs;
}

static void* or_get(int type, size_t idx)
{
    if (!idx || idx > refs) return NULL;
    if (ref[idx - 1].type == OT_NOTHING) return NULL;
    return ref[idx - 1].obj;
}

static void or_release(size_t idx)
{
    if (!idx || idx > refs) return;
    ref[idx - 1].type = OT_NOTHING;
}

static void or_free(void)
{
    free(ref);
    ref = NULL;
    refs = 0;
}

/* Engine procs */
static lil_value_t nat_exit(lil_t lil, size_t argc, lil_value_t* argv)
{
    running = 0;
    return NULL;
}

static lil_value_t nat_clear(lil_t lil, size_t argc, lil_value_t* argv)
{
    console_clear();
    return NULL;
}

static lil_value_t nat_cellsize(lil_t lil, size_t argc, lil_value_t* argv)
{
    return lil_alloc_integer(CELLSIZE);
}

static void reg_engine_procs(void)
{
    lil_register(lil, "exit", nat_exit);
    lil_register(lil, "clear", nat_clear);
    lil_register(lil, "cellsize", nat_cellsize);
}

/* Editor procs */
static lil_value_t nat_newlight(lil_t lil, size_t argc, lil_value_t* argv)
{
    float x, y, z, r, g, b, rad = 32;
    light_t* light;
    if (argc < 3) {
        int cx, cy;
        editor_get_cursor(&cx, &cy);
        if (cx < 0) cx = 0;
        if (cy < 0) cy = 0;
        if (cx >= map_width) cx = map_width - 1;
        if (cy >= map_height) cy = map_height - 1;
        x = cx*CELLSIZE + CELLSIZE*0.5;
        y = cell[cy*map_width + cx].floorz + 16;
        z = cy*CELLSIZE + CELLSIZE*0.5;
        if (argc > 1) rad = lil_to_double(argv[0]);
    } else {
        x = lil_to_double(argv[0]);
        y = lil_to_double(argv[1]);
        z = lil_to_double(argv[2]);
        if (argc == 4) rad = lil_to_double(argv[3]);
        else if (argc >= 7) rad = lil_to_double(argv[6]);
    }
    if (argc < 6) {
        r = g = b = 0.5;
    } else {
        r = lil_to_double(argv[3]);
        g = lil_to_double(argv[4]);
        b = lil_to_double(argv[5]);
    }
    light = light_new(x, y, z, r, g, b, rad);
    return lil_alloc_integer(or_new(OT_LIGHT, light));
}

static lil_value_t nat_rmlight(lil_t lil, size_t argc, lil_value_t* argv)
{
    light_t* light;
    size_t idx;
    if (!argc) return NULL;
    light = or_get(OT_LIGHT, idx = lil_to_integer(argv[0]));
    light_free(light);
    or_release(idx);
    return NULL;
}

static lil_value_t nat_relight(lil_t lil, size_t argc, lil_value_t* argv)
{
    lmap_update();
    return NULL;
}

static lil_value_t nat_lightmap_quality(lil_t lil, size_t argc, lil_value_t* argv)
{
    int q;
    if (argc < 1) return NULL;
    q = lil_to_integer(argv[0]);
    if (q < 0) q = 0; else if (q > 2) q = 2;
    lmap_quality = q;
    return NULL;
}

static lil_value_t nat_get_cursor(lil_t lil, size_t argc, lil_value_t* argv)
{
    int x, y;
    if (argc < 2) return NULL;
    editor_get_cursor(&x, &y);
    lil_set_var(lil, lil_to_string(argv[0]), lil_alloc_integer(x), LIL_SETVAR_LOCAL);
    lil_set_var(lil, lil_to_string(argv[1]), lil_alloc_integer(y), LIL_SETVAR_LOCAL);
    return NULL;
}

static lil_value_t nat_get_cursor_in_world(lil_t lil, size_t argc, lil_value_t* argv)
{
    int x, y;
    if (argc < 3) return NULL;
    editor_get_cursor(&x, &y);
    lil_set_var(lil, lil_to_string(argv[0]), lil_alloc_double(x*CELLSIZE + CELLSIZE*0.5), LIL_SETVAR_LOCAL);
    lil_set_var(lil, lil_to_string(argv[1]), lil_alloc_double(cell[y*map_width + x].floorz), LIL_SETVAR_LOCAL);
    lil_set_var(lil, lil_to_string(argv[2]), lil_alloc_double(y*CELLSIZE + CELLSIZE*0.5), LIL_SETVAR_LOCAL);
    return NULL;
}

static void reg_editor_procs(void)
{
    lil_register(lil, "newlight", nat_newlight);
    lil_register(lil, "rmlight", nat_rmlight);
    lil_register(lil, "relight", nat_relight);
    lil_register(lil, "lightmap-quality", nat_lightmap_quality);
    lil_register(lil, "get-cursor", nat_get_cursor);
    lil_register(lil, "get-cursor-in-world", nat_get_cursor_in_world);
}

/* Script interface */
static void script_callback(lil_t lil, int cbtype, const void* data)
{
    switch (cbtype) {
    case LIL_CB_ERROR:
        console_write("Script error: ");
        console_write(data);
        console_newline();
        break;
    case LIL_CB_WRITE:
        console_write(data);
        break;
    case LIL_CB_PRINT:
        console_write(data);
        console_newline();
        break;
    }
}

void script_init(void)
{
    lil = lil_new();
    lil_callback(lil, script_callback);
    reg_engine_procs();
    reg_editor_procs();
}

void script_shutdown(void)
{
    lil_free(lil);
    or_free();
}

void script_eval(const char* code)
{
    lil_parse(lil, code, 0, 0);
}
