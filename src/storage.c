/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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
    } else if (sto->ierrno == STORAGE_EFATAL) {                          \
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
    sto->ids   = NULL;
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
        sto->cp = cllnode;
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
    
    STORAGE_SAFETY_CHECK(sto, return NULL)

    cllnode = sto->cp;
    while (cllnode) {
        if (cllnode->id == id) {
            return cllnode;
        }
        cllnode = cllnode->next;
    }

    cllnode = sto->cp;
    while (cllnode) {
        if (cllnode->id == id) {
            return cllnode;
        }
        cllnode = cllnode->prev;
    }
    
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
    
    if (sto->count == 0) {
        return FALSE;
    } else if (id < sto->ids[0] || id > sto->ids[sto->count - 1]) {
        return FALSE;
    } else {
        int i;
        for (i = 0; i < sto->count; i++) {
            if (id == sto->ids[i]) {
                return TRUE;
            } else if (id < sto->ids[i]) {
                break;
            }
        }
    }
    
    return FALSE;
}

static int storage_id_cmp(const void *p1, const void *p2)
{
    if (*(const int *)p1 > *(const int *)p2) {
        return 1;
    } else {
        return -1;
    }
}

static LLNode *storage_allocate_node(Storage *sto, int id, void *data)
{
    LLNode *new;
    
    if (storage_id_exists(sto, id)) {
        sto->ierrno = STORAGE_EEXIST;
        return NULL;
    }
    new = xmalloc(sizeof(LLNode));
    if (new != NULL) {
        int *ids;
        new->id = id;
        new->data = data;
        ids = xrealloc(sto->ids, (sto->count + 1)*SIZEOF_INT);
        if (ids) {
            ids[sto->count] = id;
            sto->count++;
            qsort(ids, sto->count, SIZEOF_INT, storage_id_cmp);
            sto->ids = ids;
            sto->cp = new;
        } else {
            sto->ierrno = STORAGE_ENOMEM;
            xfree(new);
            new = NULL;
        }
    } else {
        sto->ierrno = STORAGE_ENOMEM;
    }
    return new;
}

static void storage_deallocate_node(Storage *sto, LLNode *llnode)
{
    int i, id = llnode->id;
    
    sto->data_free(llnode->data);
    xfree(llnode);
    
    for (i = 0; i < sto->count; i++) {
        if (id == sto->ids[i]) {
            if (i < sto->count - 1) {
                memmove(sto->ids + i, sto->ids + i + 1,
                    (sto->count - i - 1)*SIZEOF_INT);
            }
            sto->count--;
            return;
        }
    }
    
    /* should never come here */
    sto->ierrno = STORAGE_EFATAL;
}

int storage_add(Storage *sto, int id, void *data)
{
    LLNode *new, *prev, *next;

    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)

    prev = sto->cp;
    new = storage_allocate_node(sto, id, data);
    if (new == NULL) {
        return RETURN_FAILURE;
    } else {
        if (prev) {
            next = prev->next;
            prev->next = new;
        } else {
            sto->start = new;
            next = NULL;
        }

        new->prev = prev;
        new->next = next;

        if (next) {
            next->prev = new;
        }

        return RETURN_SUCCESS;
    }
}

int storage_insert(Storage *sto, int id, void *data)
{
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    if (sto->count == 0 || storage_prev(sto) == RETURN_SUCCESS) {
        return storage_add(sto, id, data);
    } else {
        /* non-empty rewound storage */
        LLNode *new;
        new = storage_allocate_node(sto, id, data);
        if (new == NULL) {
            return RETURN_FAILURE;
        } else {
            new->prev = NULL;
            new->next = sto->start;
            sto->start->prev = new;
            sto->start = new;
            
            return RETURN_SUCCESS;
        }
    }
}

int storage_delete(Storage *sto)
{
    LLNode *llnode, *prev, *next;
    
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    llnode = sto->cp;
    if (llnode == NULL) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else {
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
        sto->data_free(llnode->data);
        xfree(llnode);
        llnode = next;
    }
    xfree(sto->ids);
    sto->ids = NULL; 
    sto->count = 0;
    sto->start = NULL;
    sto->cp    = NULL;
}

void storage_free(Storage *sto)
{
    storage_empty(sto);
    xfree(sto);
}

int storage_get_id(Storage *sto, int *id)
{
    STORAGE_SAFETY_CHECK(sto, return RETURN_FAILURE)
    
    if (sto->cp) {
        *id = sto->cp->id;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
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

int storage_get_unique_id(Storage *sto)
{
    STORAGE_SAFETY_CHECK(sto, return -1)
    
    if (sto->ids) {
        int idmin, idmax, count;
        count = sto->count;
        idmin = sto->ids[0];
        idmax = sto->ids[count - 1];
        if (idmin > 0) {
            return (idmin - 1); 
        } else if (idmax - idmin < count) {
            return (idmax + 1);
        } else {
            int i;
            for (i = 0; i < count - 1; i++) {
                if (sto->ids[i + 1] - sto->ids[i] > 1) {
                    return (sto->ids[i] + 1);
                }
            }
            
            /* should never come here */
            sto->ierrno = STORAGE_EFATAL;
            return -1;
        }
    } else {
        return 0;
    }
}

int storage_get_all_ids(Storage *sto, int **ids)
{
    STORAGE_SAFETY_CHECK(sto, return 0)
    
    if (sto->count) {
        *ids = sto->ids;
        return sto->count;
    } else {
        return 0;
    }
}

int storage_duplicate(Storage *sto, int id)
{
    int id_new;
    void *data, *data_new;
    
    if (storage_get_data_by_id(sto, id, &data) != RETURN_SUCCESS) {
        return -1;
    } else {
        data_new = sto->data_copy(data);
        if (data && !data_new) {
            sto->ierrno = STORAGE_ENOMEM;
            return -1;
        } else {
            id_new = storage_get_unique_id(sto);
            if (id_new < 0) {
                sto->data_free(data_new);
                return -1;
            } else {
                storage_eod(sto);
                if (storage_add(sto, id_new, data_new) != RETURN_SUCCESS) {
                    sto->data_free(data_new);
                    return -1;
                } else {
                    return id_new;
                }
            }
        }
    }
}

int storage_data_copy(Storage *sto, int id1, int id2, int create)
{
    LLNode *llnode1, *llnode2;
    void *data;
    
    if (id1 == id2) {
        sto->ierrno = STORAGE_EPARAM;
        return RETURN_FAILURE;
    }
    
    llnode1 = storage_get_node_by_id(sto, id1);
    if (!llnode1) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
    llnode2 = storage_get_node_by_id(sto, id2);
    if (!llnode2) {
        if (create) {
            data = sto->data_copy(llnode1->data);
            if (llnode1->data && !data) {
                sto->ierrno = STORAGE_ENOMEM;
                return RETURN_FAILURE;
            } else {
                return storage_add(sto, id2, data);
            }
        } else {
            sto->ierrno = STORAGE_ENOENT;
        }
        return RETURN_FAILURE;
    } else {
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
}

int storage_data_move(Storage *sto, int id1, int id2, int create)
{
    /* FIXME */
    if (storage_data_copy(sto, id1, id2, create) == RETURN_SUCCESS) {
        storage_delete_by_id(sto, id1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int storage_data_swap(Storage *sto, int id1, int id2, int create)
{
    LLNode *llnode1, *llnode2;
    void *data;
    
    if (id1 == id2) {
        sto->ierrno = STORAGE_EPARAM;
        return RETURN_FAILURE;
    }
    
    llnode1 = storage_get_node_by_id(sto, id1);
    llnode2 = storage_get_node_by_id(sto, id2);
    if (!llnode1 && !llnode2) {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else if (!llnode2 && create) {
        return storage_data_move(sto, id1, id2, TRUE);
    } else if (!llnode1 && create) {
        return storage_data_move(sto, id2, id1, TRUE);
    } else if (llnode1 && llnode2) {
        data = llnode1->data;
        llnode1->data = llnode2->data;
        llnode2->data = data;
        return RETURN_SUCCESS;
    } else {
        sto->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
}

void storage_traverse(Storage *sto, Storage_traverse_hook hook, void *udata)
{
    LLNode *cllnode;
    StorageTData stdata;
    
    STORAGE_SAFETY_CHECK(sto, return)

    cllnode = sto->start;
    stdata.step = 0;
    while (cllnode) {
        stdata.id   = cllnode->id;
        stdata.data = cllnode->data;
        if (hook(&stdata, udata) != TRUE) {
            return;
        }
        stdata.step++;
        cllnode = cllnode->next;
    }
}

int storage2_data_copy(Storage *sto1, int id1, Storage *sto2, int id2, int create)
{
    LLNode *llnode1, *llnode2;
    void *data;
    
    if (sto1 == sto2) {
        return storage_data_copy(sto1, id1, id2, create);
    }
    
    llnode1 = storage_get_node_by_id(sto1, id1);
    if (!llnode1) {
        sto1->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
    llnode2 = storage_get_node_by_id(sto2, id2);
    if (!llnode2) {
        if (create) {
            data = sto1->data_copy(llnode1->data);
            if (llnode1->data && !data) {
                sto1->ierrno = STORAGE_ENOMEM;
                return RETURN_FAILURE;
            } else {
                return storage_add(sto2, id2, data);
            }
        } else {
            sto2->ierrno = STORAGE_ENOENT;
        }
        return RETURN_FAILURE;
    } else {
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
}

int storage2_data_move(Storage *sto1, int id1, Storage *sto2, int id2, int create)
{
    if (sto1 == sto2) {
        return storage_data_move(sto1, id1, id2, create);
    }
    
    /* FIXME */
    if (storage2_data_copy(sto1, id1, sto2, id2, create) == RETURN_SUCCESS) {
        storage_delete_by_id(sto1, id1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int storage2_data_swap(Storage *sto1, int id1, Storage *sto2, int id2, int create)
{
    LLNode *llnode1, *llnode2;
    void *data;

    if (sto1 == sto2) {
        return storage_data_swap(sto1, id1, id2, create);
    }
    
    llnode1 = storage_get_node_by_id(sto1, id1);
    llnode2 = storage_get_node_by_id(sto2, id2);
    if (!llnode1 && !llnode2) {
        sto1->ierrno = STORAGE_ENOENT;
        sto2->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    } else if (!llnode2 && create) {
        return storage2_data_move(sto1, id1, sto2, id2, TRUE);
    } else if (!llnode1 && create) {
        return storage2_data_move(sto2, id2, sto1, id1, TRUE);
    } else if (llnode1 && llnode2) {
        data = llnode1->data;
        llnode1->data = llnode2->data;
        llnode2->data = data;
        return RETURN_SUCCESS;
    } else {
        sto1->ierrno = STORAGE_ENOENT;
        return RETURN_FAILURE;
    }
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
                if (storage_add(sto_new, llnode->id, data_new) !=
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
        storage_add(sto, i, (void *) i);
    }
    storage_rewind(sto);
    for (i = TEST_LEN/2; i < TEST_LEN; i++) {
        storage_insert(sto, i, (void *) i);
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
