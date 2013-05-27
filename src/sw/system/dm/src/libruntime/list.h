#ifndef LIST_H
#define LIST_H

#include "stdlib.h"
#include "string.h"

typedef struct list_entry_t {
    void* data;
	struct list_entry_t* prev;
	struct list_entry_t* next;
} list_entry_t;

typedef struct list_t {
	struct list_entry_t* head;
	struct list_entry_t* tail;
} list_t;

struct list_t* list_init(void* data);
void list_add_tail(struct list_t* l, void* data);
void list_add_head(struct list_t* l, void* data);
void* list_remove_tail(struct list_t* l);
void* list_remove_head(struct list_t* l);
int list_remove(struct list_t* l, void* data);
int list_contains(struct list_t *l, void* data);

#endif
