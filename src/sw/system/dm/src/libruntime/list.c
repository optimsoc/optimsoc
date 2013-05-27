#include <list.h>

struct list_t* list_init(void* data)
{
    struct list_t* l = malloc(sizeof(struct list_t));

    if(data == NULL) { /* create empty list */
        l->head = NULL;
        l->tail = NULL;
    } else {
        /* create first element */
        l->head =l->tail = malloc(sizeof(struct list_entry_t));

        /* set data for first element */
        l->head->data = data;
        l->head->next = NULL;
        l->head->prev = NULL;
    }

    return l;
}

void list_add_tail(struct list_t* l, void* data)
{
    /* create new element */
    struct list_entry_t* e = malloc(sizeof(struct list_entry_t));

    /* set data */
    e->data = data;

    /* set next and prev elements */
    e->next = NULL;
    e->prev = l->tail;

    /* insert into list */
    if(l->tail == NULL) { /* empty list */
        l->head = e;
        l->tail = e;
    } else {
        l->tail->next = e;
        l->tail = e;
    }
}


void list_add_head(struct list_t* l, void* data)
{
    /* create new element */
    struct list_entry_t* e = malloc(sizeof(struct list_entry_t));

    /* set data */
    e->data = data;

    /* set next and prev elements */
    e->next = l->head;
    e->prev = NULL;

    /* insert into list */
    if(l->head == NULL) { /* empty list */
        l->head = e;
        l->tail = e;
    } else {
        l->head->prev = e;
        l->head = e;
    }
}

void* list_remove_tail(struct list_t* l)
{
    /* check if list is empty */
    if(l->tail == NULL) {
        return NULL;
    }

    /* save entry and remove from list */
    void* data = l->tail->data;
    l->tail = l->tail->prev;

    /* if list is empty now, set head to NULL */
    if(l->tail == NULL) {
        l->head = NULL;
    }

    return data;
}

void* list_remove_head(struct list_t* l)
{
    /* check if list is empty */
    if(l->head == NULL) {
        return NULL;
    }

    /* save entry and remove from list */
    void* data = l->head->data;
    l->head = l->head->next;

    /* if list is empty now, set tail to NULL */
    if(l->head == NULL) {
        l->tail = NULL;
    }

    return data;
}

/* Remove first element with given data-pointer from list.
 * Returns 1 if element was removed, 0 if element was not found.
 *
 * This does not free the data-pointer.
 */
int list_remove(struct list_t* l, void* data)
{
    struct list_entry_t* entry = l->head;

	while(entry != NULL) {
		if(entry->data == data) {
		    if(entry == l->head && entry == l->tail) { /* check if only one element in list */
                l->head = l->tail = NULL;
            } else if(entry->prev == NULL) { /* check if entry is head */
                l->head = entry->next;
                l->head->prev = NULL;
            } else if(entry->next == NULL) { /* check if entry is tail */
                l->tail = entry->prev;
                l->tail->next = NULL;
            } else { /* element is in the middle, just remove */
			    entry->prev->next = entry->next;
			    entry->next->prev = entry->prev;
		    }

		    return 1;
		}
		entry = entry->next;
	}

	return 0;

}

int list_contains(struct list_t *l, void* data) {
    struct list_entry_t* entry = l->head;

	while(entry != NULL) {
		if(entry->data == data) {
		    return 1;
		}
		entry = entry->next;
	}

	return 0;

}
