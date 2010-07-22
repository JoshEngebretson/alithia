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

#ifndef __EDITOR_H_INCLUDED__
#define __EDITOR_H_INCLUDED__

typedef void (*modifier_proc_t)(int cx, int cy, cell_t* cell, void* data);

typedef struct _setcelltexture_modifier_data_t
{
    int part;
    texture_t* tex;
} setcelltexture_modifier_data_t;

extern screen_t* editorscreen;

void editor_init(void);
void editor_shutdown(void);

void editor_apply_cell_modifier(modifier_proc_t proc, void* data);
void editor_set_selection(int x1, int y1, int x2, int y2);
void editor_get_selection(int* x1, int* y1, int* x2, int* y2);
void editor_get_cursor(int* x, int* y);
pickdata_t* editor_get_pickdata(void);

void editor_scriptmenu_add(const char* name, lil_value_t code);

#endif
