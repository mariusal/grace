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
 * Dictionaries
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "utils.h"
#include "dict3.h"

Dictionary *dict_new(void)
{
    Dictionary *dict;
    
    dict = xmalloc(sizeof(Dictionary));
    if (dict) {
        memset(dict, 0, sizeof(Dictionary));
    }
    
    return dict;
}

void dict_free(Dictionary *dict)
{
    if (dict) {
        unsigned int i;
        DictEntry *e;
        for (i = 0; i < dict->size; i++) {
            e = &dict->entries[i];
            xfree(e->name);
            xfree(e->descr);
        }
        xfree(dict->entries);
        
        e = &dict->defaults;
        xfree(e->name);
        xfree(e->descr);
    }
}

#define DICT_INCR 16

int dict_resize(Dictionary *dict, unsigned int size)
{
    if (size > dict->asize) {
        unsigned int new_asize = DICT_INCR*(1 + (size - 1)/DICT_INCR);
        DictEntry *p = xrealloc(dict->entries, new_asize*sizeof(DictEntry));
        if (p) {
            dict->entries = p;
            dict->asize = new_asize;
        } else {
            return RETURN_FAILURE;
        }
    }
    
    dict->size = size;
    
    return RETURN_SUCCESS;
}

int dict_entry_copy(DictEntry *dest, const DictEntry *src)
{
    if (!dest || !src) {
        return RETURN_FAILURE;
    } else {
        dest->key   = src->key;
        dest->name  = copy_string(NULL, src->name);
        dest->descr = copy_string(NULL, src->descr);
        return RETURN_SUCCESS;
    }
}

Dictionary *dict_new_from_array(unsigned int nentries, const DictEntry *entries,
    const DictEntry *defaults)
{
    Dictionary *dict;
    
    dict = dict_new();
    
    if (dict_resize(dict, nentries) == RETURN_SUCCESS) {
        unsigned int i;
        dict_entry_copy(&dict->defaults, defaults);
        for (i = 0; i < nentries; i++) {
            DictEntry *e = &dict->entries[i];
            dict_entry_copy(e, &entries[i]);
        }
    }
    
    return dict;
}

int dict_get_key_by_name(const Dictionary *dict, const char *name, int *key)
{
    unsigned int i;
    
    if (!dict || !name) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < dict->size; i++) {
        DictEntry *e = &dict->entries[i];
        if (strcmp(e->name, name) == 0) {
            *key = e->key;
            return RETURN_SUCCESS;
        }
    }
    
    *key = dict->defaults.key;
    
    return RETURN_FAILURE;
}

int dict_get_name_by_key(const Dictionary *dict, int key, char **name)
{
    unsigned int i;
    
    if (!dict) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < dict->size; i++) {
        DictEntry *e = &dict->entries[i];
        if (e->key == key) {
            *name = e->name;
            return RETURN_SUCCESS;
        }
    }
    
    *name = dict->defaults.name;
    
    return RETURN_FAILURE;
}

