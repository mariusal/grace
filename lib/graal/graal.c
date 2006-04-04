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

#include <string.h>

#include "grace/graalP.h"

Graal *graal_new(void)
{
    Graal *g = xmalloc(sizeof(Graal));
    if (g) {
        memset(g, 0, sizeof(Graal));
        graal_scanner_init(g);
    }
    
    return g;
}

void graal_free(Graal *g)
{
    if (g) {
        graal_scanner_delete(g);
        graal_free_vars(g);
        graal_free_darrs(g);
        xfree(g);
    }
}

int graal_parse_line(Graal *g, const char *s, void *context)
{
    if (g && s) {
        int retval;
        char *buf = copy_string(NULL, s);
        buf = concat_strings(buf, "\n");
        retval = graal_parse(g, buf, context);
        xfree(buf);
        return retval;
    } else {
        return RETURN_FAILURE;
    }
}

static void gvar_free(GVar *var)
{
    if (var) {
        switch (var->type) {
        case GVarStr:
            xfree(var->data.str);
            break;
        case GVarArr:
            darray_free(var->data.arr);
            break;
        default:
            break;
        }
        xfree(var->name);
        xfree(var);
    }
}

static GVar *gvar_new(Graal *g, const char *name)
{
    GVar *var = xmalloc(sizeof(GVar));
    if (var) {
        void *p;
        
        memset(var, 0, sizeof(GVar));
        var->name = copy_string(NULL, name);
        
        p = xrealloc(g->vars, (g->nvars + 1)*SIZEOF_VOID_P);
        if (!p) {
            gvar_free(var);
            return NULL;
        } else {
            g->vars = p;
            g->vars[g->nvars] = var;
            g->nvars++;
        }
    }
    
    return var;
}

GVar *graal_get_var(Graal *g, const char *name, int allocate)
{
    unsigned int i;
    for (i = 0; i < g->nvars; i++) {
        GVar *var = g->vars[i];
        if (strings_are_equal(var->name, name)) {
            return var;
        }
    }
    
    if (allocate) {
        return gvar_new(g, name);
    } else {
        return NULL;
    }
}

int gvar_get_num(GVar *var, double *value)
{
    if (var && var->type == GVarNum) {
        *value = var->data.num;
        return RETURN_SUCCESS;
    } else {
        *value = 0;
        return RETURN_FAILURE;
    }
}

int gvar_set_num(GVar *var, double value)
{
    if (var && (var->type == GVarNil || var->type == GVarNum)) {
        var->type = GVarNum;
        var->data.num = value;
        return RETURN_SUCCESS;
    } else
    if (var->type == GVarArr) {
        return darray_set_const(var->data.arr, value);
    } else {
        return RETURN_FAILURE;
    }
}

int gvar_get_str(GVar *var, char **s)
{
    if (var && var->type == GVarStr) {
        *s = var->data.str;
        return RETURN_SUCCESS;
    } else {
        *s = NULL;
        return RETURN_FAILURE;
    }
}

int gvar_set_str(GVar *var, const char *s)
{
    if (var && (var->type == GVarNil || var->type == GVarStr)) {
        var->type = GVarStr;
        var->data.str = copy_string(var->data.str, s);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int gvar_get_arr(GVar *var, DArray **da)
{
    if (var && var->type == GVarArr) {
        *da = var->data.arr;
        return RETURN_SUCCESS;
    } else {
        *da = NULL;
        return RETURN_FAILURE;
    }
}

int gvar_set_arr(GVar *var, DArray *da)
{
    if (var && (var->type == GVarNil || var->type == GVarArr)) {
        var->type = GVarArr;
        darray_free(var->data.arr);
        var->data.arr = darray_copy(da);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graal_register_darr(Graal *g, DArray *da)
{
    void *p;
    p = xrealloc(g->darrs, (g->ndarrs + 1)*SIZEOF_VOID_P);
    if (!p) {
        darray_free(da);
        
        return RETURN_FAILURE;
    } else {
        g->darrs = p;
        g->darrs[g->ndarrs] = da;
        g->ndarrs++;
        
        return RETURN_SUCCESS;
    }
}

void graal_free_vars(Graal *g)
{
    while (g->nvars) {
        g->nvars--;
        gvar_free(g->vars[g->nvars]);
    }
    g->nvars = 0;
    XCFREE(g->vars);
}

void graal_free_darrs(Graal *g)
{
    while (g->ndarrs) {
        g->ndarrs--;
        darray_free(g->darrs[g->ndarrs]);
    }
    g->ndarrs = 0;
    XCFREE(g->darrs);
}

int graal_transform_arr(Graal *g,
    const char *formula, const char *varname, DArray *da, void *context)
{
    GVar *var = graal_get_var(g, varname, TRUE);
    if (var) {
        DArray *res;
        char *buf;
        
        gvar_set_arr(var, da);
        
        buf = copy_string(NULL, varname);
        buf = concat_strings(buf, " = ");
        buf = concat_strings(buf, formula);
        
        graal_parse_line(g, buf, context);
        
        xfree(buf);
        
        var = graal_get_var(g, varname, FALSE);
        if (var && gvar_get_arr(var, &res) == RETURN_SUCCESS &&
            res->size == da->size) {
            memcpy(da->x, res->x, da->size*SIZEOF_DOUBLE);
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

int graal_eval_expr(Graal *g, const char *formula, double *val, void *context)
{
    char *buf;
    int retval;
    GVar *var;
    
    buf = copy_string(NULL, "$d = ");
    buf = concat_strings(buf, formula);

    retval = graal_parse_line(g, buf, context);

    xfree(buf);
    
    if (retval == RETURN_SUCCESS) {
        var = graal_get_var(g, "$d", FALSE);
        gvar_get_num(var, val);
    }
    
    return retval;
}

int graal_set_udata(Graal *g, void *udata)
{
    if (g) {
        g->udata = udata;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graal_set_lookup_procs(Graal *g,
    GLookupObjProc obj_proc, GGetPropProc get_proc, GSetPropProc set_proc)
{
    if (g) {
        g->lookup_obj_proc = obj_proc;
        g->get_prop_proc   = get_proc;
        g->set_prop_proc   = set_proc;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int graal_set_eval_proc(Graal *g, GEvalProc eval_proc)
{
    if (g && eval_proc) {
        g->eval_proc = eval_proc;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void graal_set_context(Graal *g, void *context)
{
    g->default_obj = context;
}

void *graal_get_user_obj(Graal *g, void *obj, const char *name)
{
    if (g && g->lookup_obj_proc) {
        return g->lookup_obj_proc(obj, name, g->udata);
    } else {
        return NULL;
    }
}

GVarType graal_get_user_obj_prop(Graal *g,
    void *obj, const char *name, GVarData *prop)
{
    if (g && g->get_prop_proc) {
        return g->get_prop_proc(obj, name, prop, g->udata);
    } else {
        return GVarNil;
    }
}

int graal_set_user_obj_prop(Graal *g,
    void *obj, const char *name, GVarType type, GVarData prop)
{
    if (g && g->set_prop_proc) {
        return g->set_prop_proc(obj, name, type, prop, g->udata);
    } else {
        return RETURN_FAILURE;
    }
}

void graal_set_dotcontext(Graal *g, GContext dot_context)
{
    g->dot_context = dot_context;
}

GContext graal_get_dotcontext(const Graal *g)
{
    return g->dot_context;
}

void graal_set_RHS(Graal *g, int onoff)
{
    g->RHS = onoff;
}

int graal_get_RHS(const Graal *g)
{
    return g->RHS;
}

void graal_call_eval_proc(Graal *g, GVarType type, GVarData vardata)
{
    if (g && g->eval_proc) {
        g->eval_proc(type, vardata, g->udata);
    }
}

