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

#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#define new(o) (malloc0(sizeof(o)))

typedef struct _listitem_t
{
    struct _list_t* list;
    struct _listitem_t* prev;
    struct _listitem_t* next;
    void* ptr;
} listitem_t;

typedef struct _list_t
{
    listitem_t* first;
    listitem_t* last;
    size_t count;
    void (*item_free)(void* ptr);
} list_t;

void* malloc0(size_t size);

list_t* list_new(void);
void list_free(list_t* list);
void list_add(list_t* list, void* ptr);
void list_remove(list_t* list, void* ptr, int release);
listitem_t* list_find(list_t* list, void* ptr);
#define list_has(list,ptr) (list_find(list, ptr) != NULL)

#endif
