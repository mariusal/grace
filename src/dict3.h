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

#ifndef __DICT3_H_
#define __DICT3_H_

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

#define DICT_NEW_STATIC(arr, defs) \
    dict_new_from_array(sizeof(arr)/sizeof(DictEntry), arr, defs)

int dict_get_key_by_name(const Dictionary *dict, const char *name, int *key);
int dict_get_name_by_key(const Dictionary *dict, int key, char **name);

#endif /* __DICT3_H_ */
