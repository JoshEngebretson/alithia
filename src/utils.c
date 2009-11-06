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

uint64_t total=0;

void* malloc0(size_t size)
{
    void* ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);

    total += size;
    return ptr;
}

list_t* list_new(void)
{
    list_t* list = new(list_t);
    return list;
}

void list_free(list_t* list)
{
    if (!list) return;
    while (list->first) {
        list->last = list->first->next;
        if (list->item_free) list->item_free(list->first->ptr);
        free(list->first);
        list->first = list->last;
    }
    free(list);
}

void list_add(list_t* list, void* ptr)
{
    listitem_t* item = new(listitem_t);
    item->list = list;
    item->prev = list->last;
    item->ptr = ptr;
    if (list->last) list->last->next = item;
    else list->first = item;
    list->last = item;
    list->count++;
}

void list_remove(list_t* list, void* ptr, int release)
{
    listitem_t* item = list_find(list, ptr);
    if (!item) return;
    if (item->prev) item->prev->next = item->next;
    else list->first = item->next;
    if (item->next) item->next->prev = item->prev;
    else list->last = item->prev;
    if (release && list->item_free) list->item_free(item->ptr);
    free(item);
    list->count--;
}

listitem_t* list_find(list_t* list, void* ptr)
{
    listitem_t* item;
    for (item=list->first; item; item=item->next)
        if (item->ptr == ptr) return item;
    return NULL;
}
