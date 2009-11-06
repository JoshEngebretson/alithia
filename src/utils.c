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
