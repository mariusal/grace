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
 * Graal interpreter
 */

#ifndef __GRAAL_H_
#define __GRAAL_H_

#include "grace/base.h"

typedef struct _Graal Graal;
typedef struct _GVar GVar;

typedef enum {
    GVarNil,
    GVarNum,
    GVarArr,
    GVarStr
} GVarType;

typedef union {
    double  num;
    DArray *arr;
    char   *str;
} GVarData;

typedef void (*GEvalProc)(GVarType type, GVarData vardata, void *udata);

typedef void *(*GLookupObjProc)(void *context, const char *name, void *udata);
typedef GVarType (*GGetPropProc)(const void *obj, const char *name,
    GVarData *prop, void *udata);
typedef int (*GSetPropProc)(void *obj, const char *name,
    GVarType type, GVarData prop, void *udata);

Graal *graal_new(void);
void graal_free(Graal *g);

int graal_parse(Graal *g, const char *s, void *context);
int graal_parse_line(Graal *g, const char *s, void *context);

int graal_eval_expr(Graal *g, const char *formula, double *val, void *context);
int graal_transform_arr(Graal *g,
    const char *formula, const char *varname, DArray *da, void *context);

GVar *graal_get_var(Graal *g, const char *name, int allocate);

int gvar_get_num(GVar *var, double *value);
int gvar_set_num(GVar *var, double value);
int gvar_get_arr(GVar *var, DArray **da);
int gvar_set_arr(GVar *var, DArray *da);
int gvar_get_str(GVar *var, char **s);
int gvar_set_str(GVar *var, const char *s);

int graal_set_udata(Graal *g, void *udata);
int graal_set_lookup_procs(Graal *g,
    GLookupObjProc obj_proc, GGetPropProc get_proc, GSetPropProc set_proc);

int graal_set_eval_proc(Graal *g, GEvalProc eval_proc);

#endif /* __GRAAL_H_ */
