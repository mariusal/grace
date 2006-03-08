/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2006 Grace Development Team
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
 * Graal interpreter - private API
 */

#ifndef __GRAALP_H_
#define __GRAALP_H_

#include "grace/graal.h"

typedef union {
    double  num;
    DArray *arr;
    char   *str;
} GVarData;

struct _GVar {
    char *name;
    GVarType type;
    int allocated;
    GVarData data;
};

struct _Graal {
    void *scanner;
    
    DArray **darrs;
    unsigned int ndarrs;
    
    GVar **vars;
    unsigned int nvars;
    
    void *udata;
};


void graal_scanner_init(Graal *g);
void graal_scanner_delete(Graal *g);

int graal_register_darr(Graal *g, DArray *da);
void graal_free_darrs(Graal *g);

#endif /* __GRAALP_H_ */
