/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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

#ifndef __PARSER_H_
#define __PARSER_H_

#define GRARR_TMP   0
#define GRARR_VEC   1
#define GRARR_SET   2

/* symbol table entry type */
typedef struct {
    char *s;
    int type;
    void *data;
} symtab_entry;

/* array variable */
typedef struct _grarr {
    int type;
    int length;
    double *data;
} grarr;

int   scanner(const char *s);
int s_scanner(const char *s, double *res);
int v_scanner(const char *s, int *reslen, double **vres);

grarr *get_parser_arr_by_name(char * const name);

#endif /* __PARSER_H_ */
