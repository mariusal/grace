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

#ifndef __STORAGE_H_
#define __STORAGE_H_

/* errno codes */
#define STORAGE_ENOENT  1
#define STORAGE_ENOMEM  2
#define STORAGE_ENULLP  3
#define STORAGE_EPARAM  4
#define STORAGE_EFATAL  5

/* error types */
#define STORAGE_ETYPE_DEBUG 0
#define STORAGE_ETYPE_INFO  1
#define STORAGE_ETYPE_WARN  2
#define STORAGE_ETYPE_ERROR 3
#define STORAGE_ETYPE_FATAL 4

typedef void (*Storage_data_free)(void *data); 
typedef void *(*Storage_data_copy)(void *data); 
typedef void (*Storage_exception_handler)(int type, const char *msg); 

typedef struct _LLNode {
    struct _LLNode *next;
    struct _LLNode *prev;
    void *data;
} LLNode;

typedef struct _Storage {
    LLNode *start;
    LLNode *cp;
    int count;
    int ierrno;
    Storage_data_free data_free;
    Storage_data_copy data_copy;
    Storage_exception_handler exception_handler;
} Storage;

typedef int (*Storage_traverse_hook)(unsigned int step, void *data, void *udata); 

Storage *storage_new(Storage_data_free data_free, Storage_data_copy data_copy,
                     Storage_exception_handler exception_handler);
void storage_free(Storage *sto);

Storage *storage_copy(Storage *sto);

void storage_empty(Storage *sto);

int storage_count(Storage *sto);

int storage_next(Storage *sto);
int storage_prev(Storage *sto);
int storage_rewind(Storage *sto);
int storage_eod(Storage *sto);

int storage_scroll(Storage *sto, int skip, int loop);
int storage_scroll_to_id(Storage *sto, int id);
int storage_scroll_to_data(Storage *sto, const void *data);

int storage_id_exists(Storage *sto, int id);

int storage_add(Storage *sto, void *data);
int storage_delete(Storage *sto);

void *storage_duplicate(Storage *sto, int id);

int storage_get_data(Storage *sto, void **datap);

int storage_push(Storage *sto, int forward);
int storage_push_id(Storage *sto, int id, int forward);

int storage_move(Storage *sto, int forward);

int storage_get_id(Storage *sto);

int storage_get_data_next(Storage *sto, void **datap);
int storage_get_data_by_id(Storage *sto, int id, void **datap);
int storage_delete_by_id(Storage *sto, int id);

int storage_data_copy_by_id(Storage *sto, int id1, int id2);
int storage_data_move_by_id(Storage *sto, int id1, int id2);
int storage_data_swap(Storage *sto, int id1, int id2);

void storage_traverse(Storage *sto, Storage_traverse_hook hook, void *udata);

int storage2_data_copy_by_id(Storage *sto1, int id1, Storage *sto2, int id2);
int storage2_data_move_by_id(Storage *sto1, int id1, Storage *sto2, int id2);
int storage2_data_swap(Storage *sto1, int id1, Storage *sto2, int id2);

int storage2_data_copy(Storage *sto1, Storage *sto2);
int storage2_data_move(Storage *sto1, Storage *sto2);
int storage2_data_flush(Storage *sto1, Storage *sto2);

#endif /* __STORAGE_H_ */
