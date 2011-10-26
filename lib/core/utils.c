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
    unsigned int i;
    if (set_get_length(pset) > 0) {
        /* FIXME: this is a hack */
        for (i = 0; i < 2; i++) {
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
        for (i = 0; i < nrows; i++) {
            dest[i] = amem_strdup(amem, src[i]);
        }
    }
    return dest;
}


/*
 * drop rows
 */
int ssd_delete_rows(Quark *q, unsigned int startno, unsigned int endno)
{
    ss_data *ssd = ssd_get_data(q);

    unsigned int i, j, dist;

    if (!ssd || startno >= ssd->nrows || endno >= ssd->nrows) {
        return RETURN_FAILURE;
    }
    
    if (endno < startno) {
        uswap(&endno, &startno);
    }

    if (endno == ssd->nrows - 1) {
        /* trivial case */
        return ssd_set_nrows(q, startno);
    }
    
    dist = endno - startno + 1;
    
    for (j = 0; j < ssd->ncols; j++) {
        ss_column *col = &ssd->cols[j];
        if (col->format == FFORMAT_STRING) {
            char **s = col->data;
            for (i = startno; i <= endno; i++) {
                amem_free(q->amem, s[i]);
            }
            memmove(s + startno, s + endno + 1,
                (ssd->nrows - endno - 1)*SIZEOF_VOID_P);
        } else {
            double *x = col->data;
            memmove(x + startno, x + endno + 1,
                (ssd->nrows - endno - 1)*SIZEOF_DOUBLE);
        }
    }
    
    ssd->nrows -= dist;
    
    quark_dirtystate_set(q, TRUE);
    
    return RETURN_SUCCESS;
}

int ssd_reverse(Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    int nrows, ncols, i, j, k;

    if (!ssd) {
        return RETURN_FAILURE;
    }
    
    nrows = ssd_get_nrows(q);
    ncols = ssd_get_ncols(q);

    for (k = 0; k < ncols; k++) {
        ss_column *col = &ssd->cols[k];
        if (col->format == FFORMAT_STRING) {
            char **s = col->data;
            for (i = 0; i < nrows/2; i++) {
                j = (nrows - 1) - i;
                sswap(&s[i], &s[j]);
            }
        } else {
            double *x = col->data;
            for (i = 0; i < nrows/2; i++) {
                j = (nrows - 1) - i;
                fswap(&x[i], &x[j]);
            }
        }
    }
    quark_dirtystate_set(q, TRUE);
    
    return RETURN_SUCCESS;
}

int ssd_is_numeric(const Quark *q)
{
    unsigned int i, ncols = ssd_get_ncols(q);
    for (i = 0; i < ncols; i++) {
        int fformat = ssd_get_col_format(q, i);
        if (fformat == FFORMAT_STRING || fformat == FFORMAT_UNKNOWN) {
            return FALSE;
        }
    }
    
    return TRUE;
}

int ssd_transpose(Quark *q)
{
    ss_data *ssd_new, *ssd_old;
    unsigned int ncols, nrows, i, j;
    
    if (!ssd_is_numeric(q)) {
        return RETURN_FAILURE;
    }
    
    ncols = ssd_get_ncols(q);
    nrows = ssd_get_nrows(q);
    if (!ncols || !nrows) {
        return RETURN_FAILURE;
    }
    
    ssd_new = ssd_data_new(q->amem);
    if (!ssd_new) {
        return RETURN_FAILURE;
    }
    
    /* swap the data */
    ssd_old = q->data;
    q->data = ssd_new;
    
    if (ssd_set_ncols(q, nrows, NULL) != RETURN_SUCCESS ||
        ssd_set_nrows(q, ncols)       != RETURN_SUCCESS) {
        
        ssd_data_free(q->amem, ssd_new);
        q->data = ssd_old;
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < ncols; i++) {
        for (j = 0; j < nrows; j++) {
            ssd_set_value(q, i, j, ((double *)ssd_old->cols[i].data)[j]);
        }
    }

    quark_dirtystate_set(q, TRUE);

    ssd_data_free(q->amem, ssd_old);
    
    return RETURN_SUCCESS;
}

typedef struct {
    unsigned int nshift;
    Quark *toq;
} coalesce_hook_t;

static int coalesce_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    coalesce_hook_t *p = (coalesce_hook_t *) udata;
    
    if (quark_fid_get(q) == QFlavorSet) {
        Dataset *dsp = set_get_dataset(q);
        unsigned int k;
        for (k = 0; k < MAX_SET_COLS; k++) {
            if (dsp->cols[k] != COL_NONE) {
                dsp->cols[k] += p->nshift;
            }
        }
        if (dsp->acol != COL_NONE) {
            dsp->acol += p->nshift;
        }
    }
    if (closure->depth == 1) {
        quark_reparent(q, p->toq);
    }
    
    return TRUE;
}

/* 
 * Coalesce two SSDs. fromq will be deleted, preceded by transferring of all
 * its children to toq
 */
int ssd_coalesce(Quark *toq, Quark *fromq)
{
    unsigned int nrows, ncols, i, j;
    ss_data *ssd;
    AMem *amem  = toq->amem;
    coalesce_hook_t p;
       
    nrows = ssd_get_nrows(fromq);
    if (nrows > ssd_get_nrows(toq)) {
        if (ssd_set_nrows(toq, nrows) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }

    /* original number of columns */
    ncols = ssd_get_ncols(toq);
    
    ssd = ssd_get_data(fromq);
    for (i = 0; i < ssd->ncols; i++) {
        ss_column *col = &ssd->cols[i];
        
        ss_column *col_new = ssd_add_col(toq, col->format);
        if (!col_new) {
            return RETURN_FAILURE;
        }
        
        col_new->label = amem_strdup(amem, col->label);
        
        if (col->format == FFORMAT_STRING) {
            for (j = 0; j < nrows; j++) {
                ((char **) col_new->data)[j] =
                    amem_strdup(amem, ((char **) col->data)[j]);
            }
        } else {
            memcpy(col_new->data, col->data, nrows*SIZEOF_DOUBLE);
        }
    }
    
    p.toq = toq;
    p.nshift = ncols;
    quark_traverse(fromq, coalesce_hook, &p);
    
    quark_free(fromq);
    
    return RETURN_SUCCESS;
}


/* Misc (de)allocation utilities */

Symbol *symbol_new(void)
{
    Symbol *retval;
    retval = xmalloc(sizeof(Symbol));
    if (retval) {
        memset(retval, 0, sizeof(Symbol));
    }
    return retval;
}

void symbol_free(Symbol *sym)
{
    xfree(sym);
}

SetLine *setline_new(void)
{
    SetLine *retval;
    retval = xmalloc(sizeof(SetLine));
    if (retval) {
        memset(retval, 0, sizeof(SetLine));
    }
    return retval;
}

void setline_free(SetLine *sl)
{
    xfree(sl);
}

BarLine *barline_new(void)
{
    BarLine *retval;
    retval = xmalloc(sizeof(BarLine));
    if (retval) {
        memset(retval, 0, sizeof(BarLine));
    }
    return retval;
}

RiserLine *riserline_new(void)
{
    RiserLine *retval;
    retval = xmalloc(sizeof(RiserLine));
    if (retval) {
        memset(retval, 0, sizeof(RiserLine));
    }
    return retval;
}


Format *format_new(void)
{
    Format *retval;
    retval = xmalloc(sizeof(Format));
    if (retval) {
        memset(retval, 0, sizeof(Format));
    }
    return retval;
}

void format_free(Format *f)
{
    if (f) {
        xfree(f->fstring);
        xfree(f);
    }
}
