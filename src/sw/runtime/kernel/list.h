/*
 * list.h
 *
 *  Created on: Nov 26, 2010
 *      Author: wallento
 */

#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>

typedef struct list_elem_t {
	struct list_elem_t *next;
	struct list_elem_t *prev;
	void               *data;
} list_elem_t;

typedef struct list_t {
	list_elem_t *head;
	list_elem_t *tail;
} list_t;

void list_append_node(list_t* list, list_elem_t *node);
void list_insert_node(list_t* list, list_elem_t *node, list_elem_t *after);
void list_remove_node(list_t *list, list_elem_t *node);
list_elem_t *list_find(list_t *list,void *data);
size_t list_length(list_t* list);

#endif /* LIST_H_ */
