/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
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
 * Base stuff - public declarations
 */

#ifndef __BASE_H_
#define __BASE_H_

#include <config.h>

/* for FILE */
#include <stdio.h>

/* for size_t */
#include <sys/types.h>

/* boolean values */
#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

/* function return codes */
#define RETURN_SUCCESS (0)
#define RETURN_FAILURE (1)

#define NICE_FLOOR   0
#define NICE_CEIL    1
#define NICE_ROUND   2


/* FIXME! */
extern void errmsg(const char *msg);

#ifndef ADVANCED_MEMORY_HANDLERS
/* generic memory allocation & friends */
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
void xfree(void *ptr);
# define XCFREE(ptr) xfree(ptr); ptr = NULL

/* string (re)allocation */
char *copy_string(char *dest, const char *src);
char *concat_strings(char *dest, const char *src);
#else
# define xmalloc bad_malloc
# define xcalloc bad_calloc
# define xrealloc bad_realloc
# define xfree bad_free
# define XCFREE BAD_CFREE
# define copy_string bad_copy_string
# define concat_strings bad_concat_strings
#endif

#define AMEM_MODEL_SIMPLE   0
#define AMEM_MODEL_LIBUNDO  1

/* advanced memory allocation & friends */
typedef struct _AMem AMem;
AMem *amem_amem_new(int model);
void amem_amem_free(AMem *amem);
void *amem_malloc(AMem *amem, size_t size);
void *amem_calloc(AMem *amem, size_t nmemb, size_t size);
void *amem_realloc(AMem *amem, void *ptr, size_t size);
void amem_free(AMem *amem, void *ptr);
# define AMEM_CFREE(amem, ptr) amem_free(amem, ptr); ptr = NULL

int amem_set_undo_limit(AMem *amem, size_t max_memory);

int amem_snapshot(AMem *amem);
int amem_undo(AMem *amem);
int amem_redo(AMem *amem);
unsigned int amem_get_undo_count(AMem *amem);
unsigned int amem_get_redo_count(AMem *amem);

char *amem_strdup(AMem *amem, const char *s);
char *amem_strcpy(AMem *amem, char *dest, const char *src);


/* string comparison etc */
int strings_are_equal(const char *s1, const char *s2);
int string_is_empty(const char *s);

/* bit manipulations */
#define PADBITS(bits, pad)  (((bits)+(pad)-1)&-(pad))
int bin_dump(char *value, int i, int pad);
unsigned char reversebits(unsigned char inword);

/* file i/o */
char *grace_fgets(char *s, int size, FILE *stream);

/* dates */

/* rounding types for dates */
#define ROUND_SECOND 1
#define ROUND_MINUTE 2
#define ROUND_HOUR   3
#define ROUND_DAY    4
#define ROUND_MONTH  5

long cal_to_jul(int y, int m, int d);
void jul_to_cal(long n, int *y, int *m, int *d);
double jul_and_time_to_jul(long jul, int hour, int min, double sec);
double cal_and_time_to_jul(int y, int m, int d,
                           int hour, int min, double sec);
void jul_to_cal_and_time(double jday, int rounding,
                         int *y, int *m, int *d,
                         int *hour, int *min, int *sec);

/* misc numerics */
int sign(double a);
double nicenum(double x, int nrange, int round);
void fswap(double *x, double *y);
void iswap(int *x, int *y);
void uswap(unsigned int *x, unsigned int *y);
void sswap(char **x, char **y);
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax);

/* locale */
int init_locale(void);
void set_locale_num(int flag);

/* dict3 stuff */
typedef struct {
    int key;     /* key */
    char *name;  /* name */
    char *descr; /* textual description */
} DictEntry;

typedef struct {
    DictEntry defaults;
    unsigned int asize; /* allocated size */
    unsigned int size;  /* dictionary size */
    DictEntry *entries; /* dictionary entries */
} Dictionary;

Dictionary *dict_new(void);
void dict_free(Dictionary *dict);
Dictionary *dict_new_from_array(unsigned int nentries, const DictEntry *entries,
    const DictEntry *defaults);
int dict_resize(Dictionary *dict, unsigned int size);
int dict_entry_copy(DictEntry *dest, const DictEntry *src);

#define DICT_NEW_STATIC(arr, defs) \
    dict_new_from_array(sizeof(arr)/sizeof(DictEntry), arr, defs)

int dict_get_key_by_name(const Dictionary *dict, const char *name, int *key);
int dict_get_name_by_key(const Dictionary *dict, int key, char **name);
int dict_get_descr_by_key(const Dictionary *dict, int key, char **descr);


/* Double precision arrays */
typedef struct {
    unsigned int size;
    unsigned int asize; /* allocated size */
    double *x;
    int allocated;
} DArray;

DArray *darray_new(unsigned int size);
void darray_free(DArray *da);

int darray_set_val(DArray *da, unsigned int i, double val);
int darray_set_const(DArray *da, double val);
int darray_append_val(DArray *da, double val);
int darray_get_val(const DArray *da, unsigned int i, double *val);

DArray *darray_copy(const DArray *da);
int darray_add_val(DArray *da, double val);
int darray_mul_val(DArray *da, double val);
int darray_pow(DArray *da, double y);
int darray_add(DArray *da, const DArray *da2);
int darray_sub(DArray *da, const DArray *da2);
int darray_mul(DArray *da, const DArray *da2);
int darray_div(DArray *da, const DArray *da2);
DArray *darray_slice(const DArray *da, unsigned int from, unsigned int to);
DArray *darray_concat(const DArray *da1, const DArray *da2);

int darray_has_zero(const DArray *da);
int darray_min(const DArray *da, double *val);
int darray_max(const DArray *da, double *val);
int darray_avg(const DArray *da, double *val);
int darray_std(const DArray *da, double *val);

/*
 *
 * DLL (Double Linked List) data storage
 *
 */

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

typedef void (*Storage_data_free)(AMem *amem, void *data); 
typedef void *(*Storage_data_copy)(AMem *amem, void *data); 
typedef void (*Storage_exception_handler)(int type, const char *msg); 
typedef int (*Storage_comp_proc)(const void *d1, const void *d2, void *udata); 


typedef struct _LLNode {
    struct _LLNode *next;
    struct _LLNode *prev;
    void *data;
} LLNode;

typedef struct _Storage {
    AMem *amem;
    LLNode *start;
    LLNode *cp;
    int count;
    int ierrno;
    Storage_data_free data_free;
    Storage_data_copy data_copy;
    Storage_exception_handler exception_handler;
} Storage;

typedef int (*Storage_traverse_hook)(unsigned int step, void *data, void *udata); 

Storage *storage_new(AMem *amem,
    Storage_data_free data_free, Storage_data_copy data_copy,
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
int storage_data_exists(Storage *sto, const void *data);

int storage_add(Storage *sto, void *data);
int storage_delete(Storage *sto);

void *storage_duplicate(Storage *sto);

int storage_get_data(Storage *sto, void **datap);

int storage_push(Storage *sto, int forward);
int storage_push_id(Storage *sto, int id, int forward);

int storage_move(Storage *sto, int forward);

int storage_get_id(Storage *sto);

int storage_get_data_next(Storage *sto, void **datap);
int storage_get_data_by_id(Storage *sto, int id, void **datap);
int storage_delete_by_id(Storage *sto, int id);
int storage_delete_by_data(Storage *sto, void *data);

int storage_data_copy_by_id(Storage *sto, int id1, int id2);
int storage_data_move_by_id(Storage *sto, int id1, int id2);
int storage_data_swap(Storage *sto, int id1, int id2);

void storage_traverse(Storage *sto, Storage_traverse_hook hook, void *udata);

int storage_extract_data(Storage *sto, void *data);

int storage_sort(Storage *sto, Storage_comp_proc fcomp, void *udata);

int storage2_data_copy_by_id(Storage *sto1, int id1, Storage *sto2, int id2);
int storage2_data_move_by_id(Storage *sto1, int id1, Storage *sto2, int id2);
int storage2_data_swap(Storage *sto1, int id1, Storage *sto2, int id2);

int storage2_data_copy(Storage *sto1, Storage *sto2);
int storage2_data_move(Storage *sto1, Storage *sto2);
int storage2_data_flush(Storage *sto1, Storage *sto2);

#endif /* __BASE_H_ */
