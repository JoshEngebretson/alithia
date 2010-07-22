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

#ifndef __SCRIPT_H_INCLUDED__
#define __SCRIPT_H_INCLUDED__

#define OT_NOTHING 0
#define OT_ENTITY 1
#define OT_LIGHT 2
#define OT_MODEL 3
#define OT_TEXTURE 4

extern lil_t lil;

size_t or_new(int type, void* obj);
void* or_get(int type, size_t idx);
void or_release(size_t idx);
void or_deleted(void* obj);

void script_init(void);
void script_shutdown(void);
void script_eval(const char* code);
void script_run(const char* name);
void script_run_execats(const char* event);

entity_t* ent_new_by_class(const char* clsname);

#endif
