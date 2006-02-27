#include <string.h>

#include "grace/graal.h"

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
        xfree(g);
    }
}

void graal_parse_line(Graal *g, const char *s)
{
    if (g && s) {
        char *buf = copy_string(NULL, s);
        buf = concat_strings(buf, "\n");
        graal_parse(g, buf);
        xfree(buf);
    }
}

static void gvar_free(GVar *var)
{
    if (var) {
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

GVar *graal_get_var(Graal *g, const char *name)
{
    unsigned int i;
    for (i = 0; i < g->nvars; i++) {
        GVar *var = g->vars[i];
        if (strings_are_equal(var->name, name)) {
            return var;
        }
    }
    
    return gvar_new(g, name);
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
        var->data.arr = da;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

