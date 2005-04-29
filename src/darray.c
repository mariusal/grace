/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2005 Grace Development Team
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
 * DArray utilities
 *
 */
 
#include "numerics.h"

DArray *darray_new(unsigned int size)
{
    DArray *da = xmalloc(sizeof(DArray));
    if (da) {
        da->x = xmalloc(size*SIZEOF_DOUBLE);
        if (!da->x) {
            XCFREE(da);
        } else {
            da->size = size;
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
