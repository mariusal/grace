/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2007 Grace Development Team
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
 * spreadsheet data stuff
 *
 */

#include <string.h>

#define ADVANCED_MEMORY_HANDLERS
#include "grace/coreP.h"

ss_data *ssd_data_new(AMem *amem)
{
    ss_data *ssd;
    
    ssd = amem_malloc(amem, sizeof(ss_data));
    if (ssd) {
        memset(ssd, 0, sizeof(ss_data));
    }
    
    return ssd;
}

static void ss_column_free(AMem *amem, unsigned int nrows, ss_column *col)
{
    unsigned int j;
    if (col->format == FFORMAT_STRING) {
        char **sp = (char **) col->data;
        for (j = 0; j < nrows; j++) {
            amem_free(amem, sp[j]);
        }
    }
    amem_free(amem, col->data);
    amem_free(amem, col->label);
}

void ssd_data_free(AMem *amem, ss_data *ssd)
{
    if (ssd) {
        unsigned int i;

        for (i = 0; i < ssd->ncols; i++) {
            ss_column *col = &ssd->cols[i];
            
            ss_column_free(amem, ssd->nrows, col);
        }

        amem_free(amem, ssd->cols);

        amem_free(amem, ssd->hotfile);
        
        amem_free(amem, ssd);
    }
}

ss_data *ssd_data_copy(AMem *amem, ss_data *ssd)
{
    ss_data *ssd_new;
    unsigned int i;
    
    ssd_new = amem_malloc(amem, sizeof(ss_data));
    if (!ssd_new) {
        return NULL;
    }

    ssd_new->ncols = ssd->ncols;
    ssd_new->nrows = ssd->nrows;
    
    ssd_new->hotfile = amem_strdup(amem, ssd->hotfile);
    
    ssd_new->cols = amem_calloc(amem, ssd->ncols, sizeof(ss_column));

    for (i = 0; i < ssd->ncols; i++) {
        ss_column *col     = &ssd->cols[i];
        ss_column *col_new = &ssd_new->cols[i];
        col_new->format = col->format;
        col_new->label  = amem_strdup(amem, col->label);
        if (col->format == FFORMAT_STRING) {
            col_new->data = copy_string_column(amem, col->data, ssd->nrows);
        } else {
            col_new->data = copy_data_column(amem, col->data, ssd->nrows);
        }
        if (!col_new->data) {
            ssd_data_free(amem, ssd_new);
            return NULL;
        }
    }
    
    return ssd_new;
}

ss_data *ssd_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorSSD) {
        return (ss_data *) q->data;
    } else {
        return NULL;
    }
}

Quark *ssd_new(Quark *q)
{
    Quark *ss; 
    ss = quark_new(q, QFlavorSSD);
    return ss;
}

int ssd_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorSSD,
        (Quark_data_new) ssd_data_new,
        (Quark_data_free) ssd_data_free,
        (Quark_data_copy) ssd_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
}

unsigned int ssd_get_ncols(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->ncols;
    } else {
        return 0;
    }
}

unsigned int ssd_get_nrows(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->nrows;
    } else {
        return 0;
    }
}

int ssd_get_col_format(const Quark *q, int col)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && col >= 0 && col < ssd->ncols) {
        return ssd->cols[col].format;
    } else {
        return FFORMAT_UNKNOWN;
    }
}

char *ssd_get_col_label(const Quark *q, int col)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && col >= 0 && col < ssd->ncols) {
        return ssd->cols[col].label;
    } else {
        return NULL;
    }
}

int ssd_set_col_label(Quark *q, int col, const char *s)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && col >= 0 && col < ssd->ncols) {
        ssd->cols[col].label = amem_strcpy(q->amem, ssd->cols[col].label, s);
        
        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int ssd_set_nrows(Quark *q, unsigned int nrows)
{
    unsigned int i, j;
    double *dp;
    char  **sp;
    ss_data *ssd = ssd_get_data(q);
    
    if (!ssd) {
        return RETURN_FAILURE;
    }
    
    if (ssd->nrows == nrows) {
        /* nothing to do */
        return RETURN_SUCCESS;
    }
    
    for (i = 0; i < ssd->ncols; i++) {
        ss_column *col = &ssd->cols[i];
        if (col->format == FFORMAT_STRING) {
            sp = (char **) col->data;
            for (j = nrows; j < ssd->nrows; j++) {
                AMEM_CFREE(q->amem, sp[j]);
            }
            col->data = amem_realloc(q->amem, col->data, nrows*SIZEOF_VOID_P);
            sp = (char **) col->data;
            if (nrows > ssd->nrows) {
                memset(sp + ssd->nrows, 0, (nrows - ssd->nrows)*SIZEOF_VOID_P);
            }
        } else {
            col->data = amem_realloc(q->amem, col->data, nrows*SIZEOF_DOUBLE);
            dp = (double *) col->data;
            if (nrows > ssd->nrows) {
                memset(dp + ssd->nrows, 0, (nrows - ssd->nrows)*SIZEOF_DOUBLE);
            }
        }
    }
    ssd->nrows = nrows;

    quark_dirtystate_set(q, TRUE);
    
    return RETURN_SUCCESS;
}

int ssd_set_ncols(Quark *q, unsigned int ncols, const int *formats)
{
    ss_data *ssd = ssd_get_data(q);
    unsigned int i;
    
    if (!ssd) {
        return RETURN_FAILURE;
    }
    
    ssd->cols = amem_calloc(q->amem, ncols, sizeof(ss_column));
    if (!ssd->cols) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < ncols; i++) {
        ss_column *col = &ssd->cols[i];
        if (formats) {
            col->format = formats[i];
        } else {
            col->format = FFORMAT_NUMBER;
        }
    }

    ssd->ncols = ncols;
    ssd->nrows = 0;
    
    quark_dirtystate_set(q, TRUE);

    return RETURN_SUCCESS;
}

ss_column *ssd_get_col(const Quark *q, int col)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && col >= 0 && col < ssd->ncols) {
        return &ssd->cols[col];
    } else {
        return NULL;
    }
}

int ssd_get_column_by_name(const Quark *q, const char *name)
{
    unsigned int i;
    ss_data *ssd = ssd_get_data(q);
    if (!ssd || !name) {
        return -1;
    }
    
    for (i = 0; i < ssd->ncols; i++) {
        if (strings_are_equal(ssd->cols[i].label, name)) {
            return i;
        }
    }
    return -1;
}

/* assign given column to DArray without actually allocating the data */
DArray *ssd_get_darray(const Quark *q, int column)
{
    ss_column *col = ssd_get_col(q, column);
    if (col && col->format != FFORMAT_STRING) {
        DArray *da = darray_new(0);
        
        da->allocated = FALSE;
        da->asize = 0;
        da->size = ssd_get_nrows(q);
        da->x = col->data;
        
        return da;
    } else {
        return NULL;
    }
}

int ssd_set_darray(Quark *q, int column, const DArray *da)
{
    ss_column *col = ssd_get_col(q, column);
    if (da && col && col->format != FFORMAT_STRING &&
        ssd_get_nrows(q) == da->size) {
        memcpy(col->data, da->x, da->size*SIZEOF_DOUBLE);
        
        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

ss_column *ssd_add_col(Quark *q, int format)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        void *p1, *p2;
        p1 = amem_realloc(q->amem, ssd->cols, (ssd->ncols + 1)*sizeof(ss_column));
        if (format == FFORMAT_STRING) {
            p2 = amem_calloc(q->amem, ssd->nrows, SIZEOF_VOID_P);
        } else {
            p2 = amem_calloc(q->amem, ssd->nrows, SIZEOF_DOUBLE);
        }

        if (!p1 || !p2) {
            amem_free(q->amem, p1);
            amem_free(q->amem, p2);
            return NULL;
        } else {
            ss_column *col;
            ssd->cols = p1;
            col = &ssd->cols[ssd->ncols];
            col->data = p2;
            col->format = format;
            col->label = NULL;
            ssd->ncols++;

            quark_dirtystate_set(q, TRUE);

            return col;
        }
    } else {
        return NULL;
    }
}

int ssd_set_value(Quark *q, int row, int column, double value)
{
    ss_column *col = ssd_get_col(q, column);
    if (col && col->format != FFORMAT_STRING &&
        row >= 0 && row < ssd_get_nrows(q)) {
        double *p = (double *) col->data;
        p[row] = value;

        quark_dirtystate_set(q, TRUE);

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int ssd_set_string(Quark *q, int row, int column, const char *s)
{
    ss_column *col = ssd_get_col(q, column);
    if (col && col->format == FFORMAT_STRING &&
        row >= 0 && row < ssd_get_nrows(q)) {
        char **sp = (char **) col->data;
        sp[row] = amem_strcpy(q->amem, sp[row], s);

        quark_dirtystate_set(q, TRUE);

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int ssd_set_index(Quark *q, int column)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        ss_column *col = ssd_get_col(q, column);
        if (col && col->format != FFORMAT_STRING) {
            if (column > 0) {
                ss_column tmpcol = *col;
                memmove(&ssd->cols[1], ssd->cols, column*sizeof(ss_column));
                ssd->cols[0] = tmpcol;
            }
            
            ssd->indexed = TRUE;
            quark_dirtystate_set(q, TRUE);

            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

int ssd_set_indexed(Quark *q, int onoff)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        ss_column *col = ssd_get_col(q, 0);
        if (!col || col->format == FFORMAT_STRING) {
            return RETURN_FAILURE;
        }

        if (ssd->indexed != onoff) {
            ssd->indexed = onoff;
            quark_dirtystate_set(q, TRUE);
        }

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int ssd_is_indexed(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && ssd->indexed) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int delete_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    int *column = (int *) udata;
    
    if (quark_fid_get(q) == QFlavorSet) {
        Dataset *dsp = set_get_dataset(q);
        unsigned int k;
        for (k = 0; k < MAX_SET_COLS; k++) {
            if (dsp->cols[k] > *column) {
                dsp->cols[k]--;
            } else
            if (dsp->cols[k] == *column) {
                dsp->cols[k] = COL_NONE;
            }
        }
        if (dsp->acol > *column) {
            dsp->acol--;
        } else
        if (dsp->acol == *column) {
            dsp->acol = COL_NONE;
        }
    }
    
    return TRUE;
}

int ssd_delete_col(Quark *q, int column)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        ss_column *col = ssd_get_col(q, column);
        if (col) {
            void *p;
            
            ss_column_free(q->amem, ssd->nrows, col);

            if (column < ssd->ncols - 1) {
                memmove(&ssd->cols[column], &ssd->cols[column + 1],
                    (ssd->ncols - 1 - column)*sizeof(ss_column));
            }
            
            p = amem_realloc(q->amem, ssd->cols,
                (ssd->ncols - 1)*sizeof(ss_column));
            ssd->ncols--;

            /* should never happen, but... */
            if (!p) {
                return RETURN_FAILURE;
            }
            ssd->cols = p;
            
            /* index column is removed */
            if (column == 0) {
                ssd->indexed = FALSE;
            }
            
            /* update set indexing */
            quark_traverse(q, delete_hook, &column);
            
            quark_dirtystate_set(q, TRUE);

            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        return RETURN_FAILURE;
    }
}

Quark *get_parent_ssd(const Quark *q)
{
    Quark *p = (Quark *) q;
    
    while (p) {
        if (p->fid == QFlavorSSD) {
            return p;
        }
        p = quark_parent_get(p);
    }
    
    return NULL;
}


int ssd_set_hotlink(Quark *q, int onoroff, const char *fname, int src)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        ssd->hotlink = onoroff;
        ssd->hotfile = amem_strcpy(q->amem, ssd->hotfile, fname);
        ssd->hotsrc = src;
        
        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int ssd_is_hotlinked(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && ssd->hotlink && ssd->hotfile) {
        return TRUE;
    } else { 
        return FALSE;
    }
}

char *ssd_get_hotlink_file(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->hotfile;
    } else {
        return NULL;
    }
}

int ssd_get_hotlink_src(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->hotsrc;
    } else {
        return -1;
    }
}
