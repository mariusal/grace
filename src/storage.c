/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
 * 
 * 
 *                           All Rights Reserved
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *
 * DLL (Double Linked List) data storage
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef STORAGE_TEST
# define xmalloc malloc
# define xrealloc realloc
# define xfree free
#endif

#include "defines.h"
#include "utils.h"

#include "storage.h"

#define STORAGE_SAFETY_CHECK(sto, retaction)                            \
    if (!sto) {                                                         \
        _exception_handler(STORAGE_ETYPE_ERROR, "NULL pointer passed"); \
        retaction;                                                      \
    } else if (sto->ierrno == STORAGE_EFATAL) {                         \
        sto->exception_handler(STORAGE_ETYPE_FATAL, "Fatal error");     \
        retaction;                                                      \
    }

static void _data_free(void *data)
{
}

static void *_data_copy(void *data)
{
    return data;
}

static void _exception_handler(int type, const char *msg)
{
    const char *s;
    if (msg) {
        s = msg;
    } else {
        s = "unknown error";
    }
    switch (type) {
    case STORAGE_ETYPE_DEBUG:
    case STORAGE_ETYPE_INFO:
        break;
    case STORAGE_ETYPE_WARN:
        fprintf(stderr, "Storage warning: %s\n", s);
        break;
    case STORAGE_ETYPE_ERROR:
        fprintf(stderr, "Storage error: %s\n", s);
        break;
    case STORAGE_ETYPE_FATAL:
    default:
        fprintf(stderr, "Storage fatal error: %s\n", s);
        abort();
        break;
    }
}

Storage *storage_new(Storage_data_free data_free, Storage_data_copy data_copy,
                     Storage_exception_handler exception_handler)
{
    Storage *sto;
    
    sto = xmalloc(sizeof(Storage));
    if (sto == NULL) {
        return NULL;
    }
    
    sto->start = NULL;
    sto->cp    = NULL;
    sto->count = 0;
    sto->ierrno = 0;
    if (data_free == NULL) {
        sto->data_free = _data_free;
    } else {
        sto->data_free = data_free;
    }
    if (data_copy == NULL) {
        sto->data_copy = _data_copy;
    } else {
        sto->data_copy = data_copy;
    }
    if (exception_handler == NULL) {
        sto->exception_handler = _exception_handler;
    } else {
        sto->exception_handler = exception_handler;
    }
    
    return sto;
}

int storage_next(Storage *sto)
{
    LLNode *cllnode;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    cllnode = sto->cp;
    if (cllnode == NULL || cllnode->next == NULL) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else {
        sto->cp = cllnode->next;
        return RETURN_SUCCESS;
    }
}

int storage_prev(Storage *sto)
{
    LLNode *cllnode;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    cllnode = sto->cp;
    if (cllnode == NULL || cllnode->prev == NULL) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else {
        sto->cp = cllnode->prev;
        return RETURN_SUCCESS;
    }
}

int storage_rewind(Storage *sto)
{
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    sto->cp = sto->start;
    return RETURN_SUCCESS;
}

int storage_eod(Storage *sto)
{
    LLNode *cllnode;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    cllnode = sto->cp;
    while (cllnode) {
        if (!cllnode->next) {
            sto->cp = cllnode;
        }
        cllnode = cllnode->next;
    }
    
    return RETURN_SUCCESS;
}

int storage_count(Storage *sto)
{
    STORAGE_SAFETY_CHECK(sto, return 0)
    
    return sto->count;
}

static LLNode *storage_get_node_by_id(Storage *sto, int id)
{
    LLNode *cllnode;
    int i;
    
    STORAGE_SAFETY_CHECK(sto, return NULL)
    
    if (id < 0 || id >= sto->count) {
        sto->ierrno = STORAGE_ENOENT;
        return NULL;
    }
    
    cllnode = sto->start;
    for (i = 0; i < id; i++) {
        if (!cllnode) {
            sto->ierrno = STORAGE_EFATAL;
            return NULL;
        }
        cllnode = cllnode->next;
    }

    return cllnode;
}

static LLNode *storage_get_node_by_data(Storage *sto, const void *data)
{
    LLNode *cllnode;
    
    STORAGE_SAFETY_CHECK(sto, return NULL)
    
    cllnode = sto->cp;
    while (cllnode) {
        if (cllnode->data == data) {
            return cllnode;
        } else {
            cllnode = cllnode->next;
        }
    }

    cllnode = sto->start;
    while (cllnode && cllnode != sto->cp) {
        if (cllnode->data == data) {
            return cllnode;
        } else {
            cllnode = cllnode->next;
        }
    }

    sto->ierrno = STORAGE_ENOENT;
    return NULL;
}

int storage_scroll_to_id(Storage *sto, int id)
{
    LLNode *cllnode;
    
    cllnode = storage_get_node_by_id(sto, id);
    if (cllnode) {
        sto->cp = cllnode;
        return RETURN_SUCCESS;
    } else {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
}

int storage_scroll_to_data(Storage *sto, const void *data)
{
    LLNode *cllnode;
    
    cllnode = storage_get_node_by_data(sto, data);
    if (cllnode) {
        sto->cp = cllnode;
        return RETURN_SUCCESS;
    } else {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
}

int storage_scroll(Storage *sto, int skip, int loop)
{
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    if (abs(skip) >= sto->count) {
        if (loop) {
            skip %= sto->count;
        } else {
            sto->ierrno = STORAGE_EPARAM;
            return RETURN_FAILURE;
        }
    }
    
    if (skip == 0) {
        return RETURN_SUCCESS;
    } else if (skip > 0) {
        while (sto->cp) {
            sto->cp = sto->cp->next ? sto->cp->next : sto->start;
            skip--;
            if (skip == 0) {
                return RETURN_SUCCESS;
            }
        }
        /* should never happen */
        sto->ierrno = STORAGE_EFATAL;
        return RETURN_FAILURE;
    } else {
        /* skip < 0 */
        while (sto->cp) {
            sto->cp = sto->cp->prev ? sto->cp->prev : sto->start;
            skip++;
            if (skip == 0) {
                return RETURN_SUCCESS;
            }
        }
        /* should never happen */
        sto->ierrno = STORAGE_EFATAL;
        return RETURN_FAILURE;
    }
}

int storage_id_exists(Storage *sto, int id)
{
    STORAGE_SAFETY_CHECK(sto, return FALSE)
    
    if (id >= 0 && id < sto->count) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static LLNode *storage_allocate_node(Storage *sto, void *data)
{
    LLNode *new;
    
    new = xmalloc(sizeof(LLNode));
    if (new != NULL) {
        new->data = data;
    } else {
        sto->ierrno = STORAGE_ENOMEM;
    }
    return new;
}

static void storage_deallocate_node(Storage *sto, LLNode *llnode)
{
    sto->data_free(llnode->data);
    xfree(llnode);
}

static void storage_add_node(Storage *sto, LLNode *llnode, int forward)
{
    if (forward) {
        LLNode *prev;

        storage_eod(sto);

        prev = sto->cp;

        if (prev) {
            prev->next = llnode;
        } else {
            sto->start = llnode;
        }

        llnode->prev = prev;
        llnode->next = NULL;
    } else {
        LLNode *next;
        
        next = sto->start;
        
        if (next) {
            next->prev = llnode;
        }
        sto->start = llnode;

        llnode->next = next;
        llnode->prev = NULL;
    }

    sto->cp = llnode;
    sto->count++;
}

int storage_add(Storage *sto, void *data)
{
    LLNode *new;

    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)

    new = storage_allocate_node(sto, data);
    if (new == NULL) {
        return RETURN_FAILURE;
    } else {
        storage_add_node(sto, new, TRUE);
        
        return RETURN_SUCCESS;
    }
}

static void storage_extract_node(Storage *sto, LLNode *llnode)
{
    LLNode *prev, *next;
    
    next = llnode->next;
    prev = llnode->prev;
    if (next) {
        next->prev = prev;
    }
    if (prev) {
        prev->next = next;
    }
    if (sto->start == sto->cp) {
        if (next) {
            sto->start = next;
        } else if (prev) {
            sto->start = prev;
        } else {
            /* empty storage */
            sto->start = NULL;
        }
    }
    if (next) {
        sto->cp = next;
    } else if (prev) {
        sto->cp = prev;
    } else {
        /* empty storage */
        sto->cp = sto->start;
    }

    sto->count--;
    
    if (sto->cp == llnode) {
        sto->cp = llnode->prev;
    }
}

int storage_delete(Storage *sto)
{
    LLNode *llnode;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    llnode = sto->cp;
    if (llnode == NULL) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else {
        storage_extract_node(sto, llnode);
        storage_deallocate_node(sto, llnode);
        
        return RETURN_SUCCESS;
    }
}

void storage_empty(Storage *sto)
{
    LLNode *llnode, *next;
    
    STORAGE_SAFETY_CHECK(sto, return)
    
    llnode = sto->start;
    while (llnode) {
        next = llnode->next;
        storage_deallocate_node(sto, llnode);
        llnode = next;
    }
    sto->count = 0;
    sto->start = NULL;
    sto->cp    = NULL;
}

void storage_free(Storage *sto)
{
    storage_empty(sto);
    xfree(sto);
}

int storage_get_data(Storage *sto, void **datap)
{
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    if (sto->cp) {
        *datap = sto->cp->data;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int storage_push(Storage *sto, int forward)
{
    LLNode *cllnode;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    cllnode = sto->cp;
    if (!cllnode) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
    
    storage_extract_node(sto, cllnode);
    
    storage_add_node(sto, cllnode, forward);
    
    return RETURN_SUCCESS;
}

int storage_push_id(Storage *sto, int id, int forward)
{
    if (storage_scroll_to_id(sto, id) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    } else {
        return storage_push(sto, forward);
    }
}

int storage_move(Storage *sto, int forward)
{
    LLNode *llnode1, *llnode2;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    llnode1 = sto->cp;
    if (!llnode1) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
    
    if (forward) {
        llnode2 = llnode1->next;
    } else {
        llnode2 = llnode1->prev;
    }
    
    if (!llnode2) {
        sto->ierrno = STORAGE_ENOENT;
    
        return RETURN_FAILURE;
    } else {
        void *databuf;
        
        databuf = llnode1->data;
        llnode1->data = llnode2->data;
        llnode2->data = databuf;
    
        return RETURN_SUCCESS;
    }
}

void *storage_duplicate(Storage *sto, int id)
{
    void *data, *data_new;
    
    if (storage_get_data_by_id(sto, id, &data) != RETURN_SUCCESS) {
        return NULL;
    } else {
        data_new = sto->data_copy(data);
        if (data && !data_new) {
            sto->ierrno = STORAGE_ENOMEM;
            return NULL;
        } else {
            storage_eod(sto);
            if (storage_add(sto, data_new) != RETURN_SUCCESS) {
                sto->data_free(data_new);
                return NULL;
            } else {
                return data_new;
            }
        }
    }
}

int storage_data_copy_by_id(Storage *sto, int id1, int id2)
{
    LLNode *llnode1, *llnode2;
    void *data;
    
    llnode1 = storage_get_node_by_id(sto, id1);
    if (!llnode1) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
    llnode2 = storage_get_node_by_id(sto, id2);
    if (!llnode2) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
    
    data = sto->data_copy(llnode1->data);
    if (llnode1->data && !data) {
        sto->ierrno = STORAGE_ENOMEM;
        return RETURN_FAILURE;
    } else {
        sto->data_free(llnode2->data);
        llnode2->data = data;
        return RETURN_SUCCESS;
    }
}

int storage_data_move_by_id(Storage *sto, int id1, int id2)
{
    /* FIXME */
    if (storage_data_copy_by_id(sto, id1, id2) == RETURN_SUCCESS) {
        storage_delete_by_id(sto, id1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int storage_data_swap(Storage *sto, int id1, int id2)
{
    LLNode *llnode1, *llnode2;
    
    if (id1 == id2) {
        sto->ierrno = STORAGE_EPARAM;
        return RETURN_FAILURE;
    }
    
    llnode1 = storage_get_node_by_id(sto, id1);
    llnode2 = storage_get_node_by_id(sto, id2);
    if (!llnode1 || !llnode2) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else {
        void *data;
        data = llnode1->data;
        llnode1->data = llnode2->data;
        llnode2->data = data;
        return RETURN_SUCCESS;
    }
}

void storage_traverse(Storage *sto, Storage_traverse_hook hook, void *udata)
{
    LLNode *cllnode;
    unsigned int step;
    
    STORAGE_SAFETY_CHECK(sto, return)

    cllnode = sto->start;
    step = 0;
    while (cllnode) {
        if (hook(step, cllnode->data, udata) != TRUE) {
            return;
        }
        step++;
        cllnode = cllnode->next;
    }
}

int storage2_data_copy_by_id(Storage *sto1, int id1, Storage *sto2, int id2)
{
    LLNode *llnode1, *llnode2;
    void *data;
    
    if (sto1 == sto2) {
        return storage_data_copy_by_id(sto1, id1, id2);
    }
    
    llnode1 = storage_get_node_by_id(sto1, id1);
    llnode2 = storage_get_node_by_id(sto2, id2);
    if (!llnode1 || !llnode2) {
        return RETURN_FAILURE;
    }
    
    data = sto1->data_copy(llnode1->data);
    if (llnode1->data && !data) {
        sto1->ierrno = STORAGE_ENOMEM;
        return RETURN_FAILURE;
    } else {
        sto2->data_free(llnode2->data);
        llnode2->data = data;
        return RETURN_SUCCESS;
    }
}

int storage2_data_move_by_id(Storage *sto1, int id1, Storage *sto2, int id2)
{
    if (sto1 == sto2) {
        return storage_data_move_by_id(sto1, id1, id2);
    }
    
    /* FIXME */
    if (storage2_data_copy_by_id(sto1, id1, sto2, id2) == RETURN_SUCCESS) {
        storage_delete_by_id(sto1, id1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int storage2_data_swap(Storage *sto1, int id1, Storage *sto2, int id2)
{
    LLNode *llnode1, *llnode2;
    void *data;

    if (sto1 == sto2) {
        return storage_data_swap(sto1, id1, id2);
    }
    
    llnode1 = storage_get_node_by_id(sto1, id1);
    llnode2 = storage_get_node_by_id(sto2, id2);
    if (!llnode1 || !llnode2) {
        return RETURN_FAILURE;
    }
    
    data = llnode1->data;
    llnode1->data = llnode2->data;
    llnode2->data = data;
    return RETURN_SUCCESS;
}

int storage2_data_copy(Storage *sto1, Storage *sto2)
{
    void *data, *data_new;
    
    STORAGE_SAFETY_CHECK(sto1, return RETURN_FAILURE)
    STORAGE_SAFETY_CHECK(sto2, return RETURN_FAILURE)
    
    if (sto1 == sto2) {
        return RETURN_FAILURE;
    }
    
    if (storage_get_data(sto1, &data) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    data_new = sto1->data_copy(data);
    if (data_new == NULL) {
        sto1->ierrno = STORAGE_ENOMEM;
        return RETURN_FAILURE;
    }
    
    return storage_add(sto2, data_new);
}

int storage2_data_move(Storage *sto1, Storage *sto2)
{
    LLNode *cllnode;
    
    STORAGE_SAFETY_CHECK(sto1, return RETURN_FAILURE)
    STORAGE_SAFETY_CHECK(sto2, return RETURN_FAILURE)
    
    if (sto1 == sto2) {
        return RETURN_FAILURE;
    }
    
    cllnode = sto1->cp;
    if (!cllnode) {
        return RETURN_FAILURE;
    }
    
    storage_extract_node(sto1, cllnode);
    
    storage_add_node(sto2, cllnode, TRUE);
    
    return RETURN_SUCCESS;
}

int storage2_data_flush(Storage *sto1, Storage *sto2)
{
    storage_rewind(sto1);
    
    while (sto1->count) {
        if (storage2_data_move(sto1, sto2) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }
    
    return RETURN_SUCCESS;
}

Storage *storage_copy(Storage *sto)
{
    Storage *sto_new;
    
    STORAGE_SAFETY_CHECK(sto, return NULL)
    
    sto_new = storage_new(sto->data_free,
                          sto->data_copy,
                          sto->exception_handler);
    
    if (sto_new) {
        LLNode *llnode = sto->start;
        while (llnode) {
            void *data_new;
            data_new = sto->data_copy(llnode->data);
            if (llnode->data && !data_new) {
                storage_free(sto_new);
                return NULL;
            } else {
                if (storage_add(sto_new, data_new) !=
                    RETURN_SUCCESS) {
                    sto->data_free(data_new);
                    storage_free(sto_new);
                    return NULL;
                }
            }
            llnode = llnode->next;
        }
    }
    
    return sto_new;
}

int storage_get_id(Storage *sto)
{
    int id = 0;
    LLNode *llnode;
       
    STORAGE_SAFETY_CHECK(sto, return -1)
    
    if (!sto->count) {
        return -1;
    }
    
    llnode = sto->start;
    while (llnode && llnode != sto->cp) {
        llnode = llnode->next;
        id++;
    }
    
    return id;
}

/**** convenience functions ****/
int storage_get_data_next(Storage *sto, void **datap)
{
    if (storage_next(sto) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    } else {
        return storage_get_data(sto, datap);
    }
}

int storage_get_data_by_id(Storage *sto, int id, void **datap)
{
    if (storage_scroll_to_id(sto, id) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    } else {
        return storage_get_data(sto, datap);
    }
}

int storage_delete_by_id(Storage *sto, int id)
{
    if (storage_scroll_to_id(sto, id) == RETURN_SUCCESS) {
        return storage_delete(sto);
    } else {
        return RETURN_FAILURE;
    }
}

#ifdef STORAGE_TEST

#define TEST_LEN    10

int main(void)
{
    Storage *sto;
    int i, j;
    
    sto = storage_new(NULL, NULL, NULL);
    for (i = 0; i < TEST_LEN/2; i++) {
        storage_add(sto, (void *) i);
    }
    storage_rewind(sto);
    while (storage_get_data_next(sto, (void **) &j) == RETURN_SUCCESS) {
        printf("%d\n", j);
    }
    for (i = 0; i < TEST_LEN; i++) {
        storage_get_data_by_id(sto, i, (void **) &j);
        printf("%d\n", j);
    }
    storage_rewind(sto);
    while (storage_delete(sto) == RETURN_SUCCESS) {
        i = storage_count(sto);
        printf("count: %d\n", i);
    }
    storage_free(sto);
    exit(0);
}

#endif
