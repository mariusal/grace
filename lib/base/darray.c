/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2005,2006 Grace Development Team
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

/* DArrays */

#include <config.h>

#include <string.h>

#include "grace/baseP.h"

DArray *darray_new(unsigned int size)
{
    DArray *da = xmalloc(sizeof(DArray));
    if (da) {
        da->x = xmalloc(size*SIZEOF_DOUBLE);
        if (size && !da->x) {
            XCFREE(da);
        } else {
            da->size = size;
            da->asize = size;
            da->allocated = TRUE;
        }
    }
    
    return da;
}

void darray_free(DArray *da)
{
    if (da) {
        if (da->allocated) {
            xfree(da->x);
        }
        xfree(da);
    }
}

int darray_set_val(DArray *da, unsigned int i, double val)
{
    if (da && i < da->size) {
        da->x[i] = val;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int darray_set_const(DArray *da, double val)
{
    if (da) {
        unsigned int i;
        for (i = 0; i < da->size; i++) {
            da->x[i] = val;
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int darray_get_val(const DArray *da, unsigned int i, double *val)
{
    if (da && i < da->size) {
        *val = da->x[i];
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int darray_append_val(DArray *da, double val)
{
    if (!da || !da->allocated) {
        return RETURN_FAILURE;
    }
    
    if (da->size >= da->asize) {
        unsigned int new_asize = MAX2(16, 2*da->asize);
        void *p = xrealloc(da->x, new_asize*SIZEOF_DOUBLE);
        if (p) {
            da->x = p;
            da->asize = new_asize;
        } else {
            return RETURN_FAILURE;
        }
    }
    
    da->x[da->size] = val;
    da->size++;
    
    return RETURN_SUCCESS;
}

DArray *darray_copy(const DArray *da)
{
    DArray *da_new;
    
    if (da) {
        da_new = darray_new(da->size);
        if (da_new) {
            memcpy(da_new->x, da->x, da->size*SIZEOF_DOUBLE);
        }
    } else {
        da_new = NULL;
    }
    
    return da_new;
}

int darray_add_val(DArray *da, double val)
{
    unsigned int i;
    
    for (i = 0; i < da->size; i++) {
        da->x[i] += val;
    }
    
    return RETURN_SUCCESS;
}

int darray_mul_val(DArray *da, double val)
{
    unsigned int i;
    
    for (i = 0; i < da->size; i++) {
        da->x[i] *= val;
    }
    
    return RETURN_SUCCESS;
}
