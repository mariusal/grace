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

void ssd_data_free(AMem *amem, ss_data *ssd)
{
    if (ssd) {
        int i, j;
        char **sp;

        for (i = 0; i < ssd->ncols; i++) {
            if (ssd->formats[i] == FFORMAT_STRING) {
                sp = (char **) ssd->data[i];
                for (j = 0; j < ssd->nrows; j++) {
                    amem_free(amem, sp[j]);
                }
            }
            amem_free(amem, ssd->data[i]);
        }
        amem_free(amem, ssd->data);
        amem_free(amem, ssd->formats);
        amem_free(amem, ssd->label);
        
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
    
    ssd_new->label = amem_strdup(amem, ssd->label);
    
    ssd_new->data = amem_calloc(amem, ssd->ncols, SIZEOF_VOID_P);
    ssd_new->formats = amem_malloc(amem, ssd->ncols*SIZEOF_INT);

    memcpy(ssd_new->formats, ssd->formats, ssd->ncols*SIZEOF_INT);
    
    for (i = 0; i < ssd->ncols; i++) {
        if (ssd->formats[i] == FFORMAT_STRING) {
            ssd_new->data[i] =
                copy_string_column(amem, ssd->data[i], ssd->nrows);
        } else {
            ssd_new->data[i] =
                copy_data_column(amem, ssd->data[i], ssd->nrows);
        }
        if (!ssd_new->data[i]) {
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

int *ssd_get_formats(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->formats;
    } else {
        return NULL;
    }
}

int ssd_set_nrows(Quark *q, unsigned int nrows)
{
    unsigned int i, j;
    char  **sp;
    ss_data *ssd = ssd_get_data(q);
    AMem *amem = quark_get_amem(q);
    
    if (!ssd) {
        return RETURN_FAILURE;
    }
    
    if (ssd->nrows == nrows) {
        /* nothing to do */
        return RETURN_SUCCESS;
    }
    
    for (i = 0; i < ssd->ncols; i++) {
        if (ssd->formats[i] == FFORMAT_STRING) {
            sp = (char **) ssd->data[i];
            for (j = nrows; j < ssd->nrows; j++) {
                AMEM_CFREE(amem, sp[j]);
            }
            ssd->data[i] =
                amem_realloc(amem, ssd->data[i], nrows*SIZEOF_VOID_P);
            sp = (char **) ssd->data[i];
            for (j = ssd->nrows; j < nrows; j++) {
                sp[j] = NULL;
            }
        } else {
            ssd->data[i] =
                amem_realloc(amem, ssd->data[i], nrows*SIZEOF_DOUBLE);
        }
    }
    ssd->nrows = nrows;

    quark_dirtystate_set(q, TRUE);
    
    return RETURN_SUCCESS;
}

int ssd_set_ncols(Quark *q, unsigned int ncols, const int *formats)
{
    ss_data *ssd = ssd_get_data(q);
    AMem *amem = quark_get_amem(q);
    
    if (!ssd) {
        return RETURN_FAILURE;
    }
    
    ssd->data = amem_calloc(amem, ncols, SIZEOF_VOID_P);

    ssd->formats = amem_malloc(amem, ncols*SIZEOF_INT);
    memcpy(ssd->formats, formats, ncols*SIZEOF_INT);
    ssd->ncols = ncols;
    ssd->nrows = 0;
    
    quark_dirtystate_set(q, TRUE);

    return RETURN_SUCCESS;
}

int ssd_set_label(Quark *q, const char *label)
{
    ss_data *ssd = ssd_get_data(q);

    if (!ssd) {
        return RETURN_FAILURE;
    } else {
        AMem *amem = quark_get_amem(q);

        ssd->label = amem_strcpy(amem, ssd->label, label);

        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    }
}

void *ssd_get_col(const Quark *q, int col, int *format)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd && col >= 0 && col < ssd->ncols) {
        *format = ssd->formats[col];
        return ssd->data[col];
    } else {
        return NULL;
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
