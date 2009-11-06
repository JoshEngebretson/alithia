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
