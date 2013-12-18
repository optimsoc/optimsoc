/* Copyright (c) 2013 by the author(s)
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
 */

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
