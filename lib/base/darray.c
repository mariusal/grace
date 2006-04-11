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

int darray_add(DArray *da, const DArray *da2)
{
    unsigned int i;
    
    if (!da || !da2 || da->size != da2->size) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < da->size; i++) {
        da->x[i] += da2->x[i];
    }
    
    return RETURN_SUCCESS;
}

int darray_sub(DArray *da, const DArray *da2)
{
    unsigned int i;
    
    if (!da || !da2 || da->size != da2->size) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < da->size; i++) {
        da->x[i] -= da2->x[i];
    }
    
    return RETURN_SUCCESS;
}

int darray_mul(DArray *da, const DArray *da2)
{
    unsigned int i;
    
    if (!da || !da2 || da->size != da2->size) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < da->size; i++) {
        da->x[i] *= da2->x[i];
    }
    
    return RETURN_SUCCESS;
}

int darray_div(DArray *da, const DArray *da2)
{
    unsigned int i;
    
    if (!da || !da2 || da->size != da2->size) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < da->size; i++) {
        if (da2->x[i] == 0.0) {
            return RETURN_FAILURE;
        } else {
            da->x[i] /= da2->x[i];
        }
    }
    
    return RETURN_SUCCESS;
}

int darray_pow(DArray *da, double y)
{
    unsigned int i;
    int ny = (int) rint(y);
    int intpow;
    
    if (!da) {
        return RETURN_FAILURE;
    }
    
    if (y == (double) ny) {
        intpow = TRUE;
    } else {
        intpow = FALSE;
    }
    
    for (i = 0; i < da->size; i++) {
        if (da->x[i] < 0.0 && !intpow) {
            return RETURN_FAILURE;
        } else
        if (da->x[i] == 0.0 && y < 0.0) {
            return RETURN_FAILURE;
        } else {
            da->x[i] = pow(da->x[i], y);
        }
    }
    
    return RETURN_SUCCESS;
}

DArray *darray_slice(const DArray *da, unsigned int from, unsigned int to)
{
    DArray *da_new;
    
    if (da && from <= to && to < da->size) {
        unsigned int new_size = to - from + 1;
        da_new = darray_new(new_size);
        if (da_new) {
            memcpy(da_new->x, da->x + from, new_size*SIZEOF_DOUBLE);
        }
    } else {
        da_new = NULL;
    }
    
    return da_new;
}

DArray *darray_concat(const DArray *da1, const DArray *da2)
{
    DArray *da_new;
    
    if (da1 && da2) {
        da_new = darray_new(da1->size + da2->size);
        if (da_new) {
            memcpy(da_new->x, da1->x, da1->size*SIZEOF_DOUBLE);
            memcpy(da_new->x + da1->size, da2->x, da2->size*SIZEOF_DOUBLE);
        }
    } else
    if (da1) {
        da_new = darray_copy(da1);
    } else
    if (da2) {
        da_new = darray_copy(da2);
    } else {
        da_new = NULL;
    }
    
    return da_new;
}

int darray_min(const DArray *da, double *val)
{
    unsigned int i;

    *val = 0.0;

    if (!da || !da->size) {
        return RETURN_FAILURE;
    }
    
    *val = da->x[0];
    for (i = 1; i < da->size; i++) {
        if (da->x[i] < *val) {
            *val = da->x[i];
        }
    }
    
    return RETURN_SUCCESS;
}

int darray_max(const DArray *da, double *val)
{
    unsigned int i;

    *val = 0.0;

    if (!da || !da->size) {
        return RETURN_FAILURE;
    }
    
    *val = da->x[0];
    for (i = 1; i < da->size; i++) {
        if (da->x[i] > *val) {
            *val = da->x[i];
        }
    }
    
    return RETURN_SUCCESS;
}

int darray_has_zero(const DArray *da)
{
    unsigned int i;

    if (!da) {
        return FALSE;
    }
    
    for (i = 1; i < da->size; i++) {
        if (da->x[i] == 0.0) {
            return TRUE;
        }
    }
    
    return FALSE;
}

int darray_avg(const DArray *da, double *val)
{
    unsigned int i;

    *val = 0.0;
    
    if (!da || !da->size) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < da->size; i++) {
        *val += da->x[i];
    }
    *val /= da->size;
    
    return RETURN_SUCCESS;
}

int darray_std(const DArray *da, double *val)
{
    double avg;
    unsigned int i;

    *val = 0.0;
    
    if (!da || da->size < 2) {
        return RETURN_FAILURE;
    }
    
    darray_avg(da, &avg);
    
    for (i = 0; i < da->size; i++) {
        *val += (da->x[i] - avg)*(da->x[i] - avg);
    }
    
    *val = sqrt(*val/(da->size - 1));
    
    return RETURN_SUCCESS;
}
