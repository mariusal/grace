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
 *
 * Misc core utils
 *
 */

#include <string.h>

#include "grace/coreP.h"

/* convenience function */
int graph_get_viewport(Quark *gr, view *v)
{
    if (gr) {
        Quark *fr = get_parent_frame(gr);
        
        frame_get_view(fr, v);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


static int set_count_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    int *nsets = (int *) udata;
    
    if (quark_fid_get(q) == QFlavorSet) {
        (*nsets)++;
    }
    
    return TRUE;
}

int quark_get_number_of_descendant_sets(Quark *q)
{
    int nsets = 0;
    
    quark_traverse(q, set_count_hook, &nsets);
    
    return nsets;
}

typedef struct {
    int nsets;
    Quark **sets;
} set_hook_t;

static int set_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    set_hook_t *p = (set_hook_t *) udata;
    
    if (quark_fid_get(q) == QFlavorSet) {
        p->sets[p->nsets] = q;
        p->nsets++;
    }
    
    return TRUE;
}

int quark_get_descendant_sets(Quark *q, Quark ***sets)
{
    set_hook_t p;
    
    if (q) {
        p.nsets = 0;
        p.sets  = xmalloc(quark_get_number_of_descendant_sets(q)*SIZEOF_VOID_P);

        if (p.sets) {
            quark_traverse(q, set_hook, &p);
        }
        
        *sets = p.sets;

        return p.nsets;
    } else {
        return 0;
    }
}

int set_is_dataless(Quark *pset)
{
    int i, ncols = set_get_ncols(pset);
    if (set_get_length(pset) > 0) {
        for (i = 0; i < ncols; i++) {
            if (set_get_col(pset, i) == NULL) {
                return TRUE;
            }
        }
        
        return FALSE;
    } else {
        return TRUE;
    }
}

/*
 * get the min/max values of a set
 */
int set_get_minmax(Quark *pset,
    double *xmin, double *xmax, double *ymin, double *ymax)
{
    double *x, *y;
    int len;
    double x1, x2, y1, y2;
    int imin, imax; /* dummy */

    if (!pset) {
        return RETURN_FAILURE;
    }
    
    x = set_get_col(pset, DATA_X);
    y = set_get_col(pset, DATA_Y);
    len = set_get_length(pset);
    minmax(x, len, &x1, &x2, &imin, &imax);
    minmax(y, len, &y1, &y2, &imin, &imax);
    *xmin = x1;
    *xmax = x2;
    *ymin = y1;
    *ymax = y2;
    
    return RETURN_SUCCESS;
}

double set_get_ybase(Quark *pset)
{
    double ybase = 0.0;
    double xmin, xmax, ymin, ymax;
    Quark *gr;
    set *p;
    world w;
    
    if (!pset) {
        return 0.0;
    }
    
    gr = get_parent_graph(pset);
    p = set_get_data(pset);
    graph_get_world(gr, &w);
    
    set_get_minmax(pset, &xmin, &xmax, &ymin, &ymax);

    switch (p->line.baseline_type) {
    case BASELINE_TYPE_0:
        ybase = 0.0;
        break;
    case BASELINE_TYPE_SMIN:
        ybase = ymin;
        break;
    case BASELINE_TYPE_SMAX:
        ybase = ymax;
        break;
    case BASELINE_TYPE_GMIN:
        ybase = w.yg1;
        break;
    case BASELINE_TYPE_GMAX:
        ybase = w.yg2;
        break;
    default:
        errmsg("Wrong type of baseline");
    }
    
    return ybase;
}

double *copy_data_column(AMem *amem, double *src, int nrows)
{
    double *dest;
    
    if (!src) {
        return NULL;
    }
    
    dest = amem_malloc(amem, nrows*SIZEOF_DOUBLE);
    if (dest != NULL) {
        memcpy(dest, src, nrows*SIZEOF_DOUBLE);
    }
    return dest;
}

char **copy_string_column(AMem *amem, char **src, int nrows)
{
    char **dest;
    unsigned int i;

    if (!src) {
        return NULL;
    }
    
    dest = amem_malloc(amem, nrows*sizeof(char *));
    if (dest != NULL) {
        for (i = 0; i < nrows; i++)
            dest[i] = amem_strdup(amem, src[i]);
    }
    return dest;
}
