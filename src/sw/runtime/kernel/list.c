/*
 * list.c
 *
 *  Created on: Nov 26, 2010
 *      Author: wallento
 */

#include "list.h"

void list_append_node(list_t* list, list_elem_t *node) {
	if( list->head == NULL ) {
		list->head = node;
		node->prev = NULL;
	} else {
		list->tail->next = node;
		node->prev       = list->tail;
	}
	list->tail = node;
	node->next = NULL;
}

void list_insert_node(list_t* list, list_elem_t *node, list_elem_t *after) {
	node->next = after->next;
	node->prev = after;

	if(after->next != NULL)
		after->next->prev = node;
	else
		list->tail = node;

	after->next = node;
}

void list_remove_node(list_t *list, list_elem_t *node) {
	if(node->prev == NULL)
		list->head = node->next;
	else
		node->prev->next = node->next;

	if(node->next == NULL)
		list->tail = node->prev;
	else
		node->next->prev = node->prev;
}

list_elem_t *list_find(list_t *list,void *data) {
	list_elem_t *node = NULL;
	list_elem_t *it   = list->head;
	while (it) {
		if ( it->data == data ) {
			node = it;
			break;
		}
		it = it->next;
	}
	return node;
}

size_t list_length(list_t* list) {
	size_t l = 0;
	list_elem_t *it = list->head;
	while(it) {
		l++;
		it = it->next;
	}
	return l;
}

