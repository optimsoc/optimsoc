/* Copyright (c) 2012-2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Stefan Rösch <roe.stefan@gmail.com>
 */

#include <list.h>
#include <assert.h>

// TODO: create non-blocking data structure

struct optimsoc_list_t* optimsoc_list_init(void* data)
{
    struct optimsoc_list_t* l = malloc(sizeof(struct optimsoc_list_t));
    assert(l != NULL);

    //    mutex_init(&l->mutex);

    if(data == NULL) { /* create empty list */
        l->head = NULL;
        l->tail = NULL;
    } else {
        /* create first element */
        l->head =l->tail = malloc(sizeof(struct optimsoc_list_entry_t));
        assert(l->tail != NULL);

        /* set data for first element */
        l->head->data = data;
        l->head->next = NULL;
        l->head->prev = NULL;
    }

    return l;
}

void optimsoc_list_add_tail(struct optimsoc_list_t* l, void* data)
{
    assert(l != NULL);
    /* create new element */
    struct optimsoc_list_entry_t* e;
    e = malloc(sizeof(struct optimsoc_list_entry_t));
    assert(e != NULL);

    //mutex_lock(&l->mutex);
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
    // mutex_unlock(&l->mutex);
}


void optimsoc_list_add_head(struct optimsoc_list_t* l, void* data)
{
    assert(l != NULL);
    /* create new element */
    struct optimsoc_list_entry_t* e;
    e = malloc(sizeof(struct optimsoc_list_entry_t));
    assert(e != NULL);

    //mutex_lock(&l->mutex);
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
    //mutex_unlock(&l->mutex);
}

void* optimsoc_list_remove_tail(struct optimsoc_list_t* l)
{
    assert(l != NULL);
    struct optimsoc_list_entry_t* entry;

    /* check if list is empty */
    if(l->tail == NULL) {
        return NULL;
    }

    //mutex_lock(&l->mutex);
    /* save entry and remove from list */
    entry = l->tail;
    void* data = entry->data;
    l->tail = entry->prev;

    free(entry);
    /* if list is empty now, set head to NULL */
    if(l->tail == NULL) {
        l->head = NULL;
    }

    //mutex_unlock(&l->mutex);
    return data;
}

void* optimsoc_list_remove_head(struct optimsoc_list_t* l)
{
    assert(l != NULL);
    struct optimsoc_list_entry_t* entry;

    /* check if list is empty */
    if(l->head == NULL) {
        return NULL;
    }

    //mutex_lock(&l->mutex);
    /* save entry and remove from list */
    entry = l->head;
    void* data = entry->data;
    l->head = entry->next;

    free(entry);
    /* if list is empty now, set tail to NULL */
    if(l->head == NULL) {
        l->tail = NULL;
    }

    //mutex_unlock(&l->mutex);

    return data;
}

/* Remove first element with given data-pointer from list.
 * Returns 1 if element was removed, 0 if element was not found.
 *
 * This does not free the data-pointer.
 */
int optimsoc_list_remove(struct optimsoc_list_t* l, void* data)
{
    assert(l != NULL);
    //mutex_lock(&l->mutex);
    struct optimsoc_list_entry_t* entry = l->head;

    while(entry != NULL) {
        if(entry->data == data) {
            if(entry == l->head && entry == l->tail) {
                /* check if only one element in list */
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

            //mutex_unlock(&l->mutex);
            free(entry);
            return 1;
        }
        entry = entry->next;
    }
    //mutex_unlock(&l->mutex);

    return 0;

}

int optimsoc_list_contains(struct optimsoc_list_t *l, void* data) {
    assert(l != NULL);
    struct optimsoc_list_entry_t* entry = l->head;

    while(entry != NULL) {
        if(entry->data == data) {
            return 1;
        }
        entry = entry->next;
    }

    return 0;

}

size_t optimsoc_list_length(struct optimsoc_list_t *l) {
    assert(l != NULL);
    unsigned int count = 0;
    struct optimsoc_list_entry_t* entry = l->head;
    while(entry != NULL) {
        count++;
        entry = entry->next;
    }
    return count;
}

void* optimsoc_list_first_element(struct optimsoc_list_t* l,
                                  optimsoc_list_iterator_t *list_iter)
{
    assert(l != NULL);
    if(l->head == NULL) {
        *list_iter = NULL;
        return NULL;
    } else {
        *list_iter = l->head;
        return l->head->data;
    }
}

void* optimsoc_list_next_element(struct optimsoc_list_t* l,
                                 optimsoc_list_iterator_t *list_iter)
{
    assert(l != NULL);
    if((*list_iter)->next == NULL) {
        return NULL;
    } else {
        *list_iter = (*list_iter)->next;
        return (*list_iter)->data;
    }
}
