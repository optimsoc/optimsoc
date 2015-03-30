/* Copyright (c) 2012-2013 by the author(s)
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

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <string.h>

struct optimsoc_list_entry_t {
    void* data;
    struct optimsoc_list_entry_t* prev;
    struct optimsoc_list_entry_t* next;
};

struct optimsoc_list_t {
    struct optimsoc_list_entry_t* head;
    struct optimsoc_list_entry_t* tail;
};

typedef struct optimsoc_list_entry_t* optimsoc_list_iterator_t;

struct optimsoc_list_t* optimsoc_list_init(void* data);
void optimsoc_list_add_tail(struct optimsoc_list_t* l, void* data);
void optimsoc_list_add_head(struct optimsoc_list_t* l, void* data);
void* optimsoc_list_remove_tail(struct optimsoc_list_t* l);
void* optimsoc_list_remove_head(struct optimsoc_list_t* l);
int optimsoc_list_remove(struct optimsoc_list_t* l, void* data);
int optimsoc_list_contains(struct optimsoc_list_t *l, void* data);
size_t optimsoc_list_length(struct optimsoc_list_t* l);

void* optimsoc_list_first_element(struct optimsoc_list_t* l, optimsoc_list_iterator_t *list_iter);
void* optimsoc_list_next_element(struct optimsoc_list_t* l, optimsoc_list_iterator_t *list_iter);

#endif
