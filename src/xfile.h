/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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
 * XML output and related stuff
 */

#ifndef __XFILE_H_
#define __XFILE_H_

typedef struct _XStackEntry {
    char *name;
    void *data;
} XStackEntry;

typedef struct _XStack {
    int size;
    int depth;
    XStackEntry *entries;
} XStack;

typedef struct _ElementAttribute {
    char *name;
    char *value;
} ElementAttribute;

typedef struct _Attributes {
    ElementAttribute *args;
    unsigned int size;
    unsigned int count;
} Attributes;

typedef struct _XFile {
    FILE *fp;
    XStack *tree;
    unsigned int indent;
    char *indstr;
    int curpos;
    int convert;
    char *ns_prefix;
    char *ns_uri;
    int ns_force;
} XFile;

XStack *xstack_new(void);
void xstack_free(XStack *xs);
int xstack_increment(XStack *xs, const char *name, void *data);
int xstack_decrement(XStack *xs, const char *name);
int xstack_get_first(XStack *xs, char **name, void **data);
int xstack_get_last(XStack *xs, char **name, void **data);
int xstack_is_empty(XStack *xs);

Attributes *attributes_new(void);
void attributes_free(Attributes *attrs);
int attributes_reset(Attributes *attrs);

int attributes_set_sval(Attributes *attrs, const char *name, const char *value);
int attributes_set_bval(Attributes *attrs, const char *name, int bval);
int attributes_set_ival(Attributes *attrs, const char *name, int ival);
int attributes_set_ival_formatted(Attributes *attrs, const char *name,
    int ival, char *format);
int attributes_set_dval(Attributes *attrs, const char *name, double dval);
int attributes_set_dval_formatted(Attributes *attrs, const char *name,
    double dval, char *format);

int attributes_set_ns(Attributes *attrs, const char *ns, const char *uri);

XFile *xfile_new(FILE *fp);
void xfile_free(XFile *xf);

int xfile_set_ns(XFile *xf, const char *ns, const char *uri, int force);

int xfile_begin(XFile *xf, int standalone,
    char *dtd_name, char *dtd_uri, char *root, Attributes *root_attrs);
int xfile_end(XFile *xf);

int xfile_begin_element(XFile *xf, char *name, Attributes *attrs);
int xfile_end_element(XFile *xf, char *name);
int xfile_empty_element(XFile *xf, char *name, Attributes *attrs);
int xfile_text_element(XFile *xf,
    char *name, Attributes *attrs, char *text, int cdata);

int xfile_comment(XFile *xf, char *comment);

int xfile_processing_instruction(XFile *xf, Attributes *attrs);

#endif /* __XFILE_H_ */
