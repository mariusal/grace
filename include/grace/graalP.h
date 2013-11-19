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

#define MAX_VARARGS 16

typedef enum {
    GContextNone,
    GContextDot,
    GContextColumn
} GContext;

struct _GVar {
    char *name;
    GVarType type;
    GVarData data;
};

struct _Graal {
    void *scanner;
    
    DArray **darrs;
    unsigned int ndarrs;
    
    GVar **vars;
    unsigned int nvars;
    
    GContext dot_context;
    int      RHS;
    
    void *default_lobj;     /* default LHS object */
    void *default_robj;     /* default RHS object */
    void *current_obj;
    
    GEvalProc eval_proc;
    GFuncProc func_proc;
    
    GLookupObjProc  lookup_obj_proc;
    GGetPropProc    get_prop_proc;
    GSetPropProc    set_prop_proc;
    void *udata;
};


void graal_scanner_init(Graal *g);
void graal_scanner_delete(Graal *g);

int graal_register_darr(Graal *g, DArray *da);
void graal_free_vars(Graal *g);
void graal_free_darrs(Graal *g);

void *graal_get_user_obj(Graal *g, void *obj, const char *name);
GVarType graal_get_user_obj_prop(Graal *g,
    void *obj, const char *name, GVarData *prop);
int graal_set_user_obj_prop(Graal *g,
    void *obj, const char *name, GVarType type, GVarData prop);

void graal_set_context(Graal *g, void *lcontext, void *rcontext);
void *graal_get_context(Graal *g);

void graal_set_dotcontext(Graal *g, GContext dot_context);
GContext graal_get_dotcontext(const Graal *g);

void graal_set_RHS(Graal *g, int onoff);
int graal_get_RHS(const Graal *g);

void graal_call_eval_proc(Graal *g, GVarType type, GVarData vardata);

void gvar_clear(GVar *var);

#endif /* __GRAALP_H_ */
