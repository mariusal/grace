/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
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
 * Region stuff
 *
 */

#include <string.h>

#include "grace/core.h"

static void set_region_defaults(region *r)
{
    if (!r) {
        return;
    }
    
    r->active = FALSE;
    r->type = 0;
    r->color = 1;
    r->n = 0;
    r->wps = NULL;
}

Quark *region_new(Quark *gr)
{
    Quark *r; 
    r = quark_new(gr, QFlavorRegion);
    return r;
}

region *region_data_new(void)
{
    region *r;
    
    r = xmalloc(sizeof(region));
    if (r) {
        memset(r, 0, sizeof(region));
        set_region_defaults(r);
    }
    return r;
}

void region_data_free(region *r)
{
    if (r) {
        xfree(r->wps);
        xfree(r);
    }
}

region *region_data_copy(region *r)
{
    region *r_new;
    
    if (!r) {
        return NULL;
    }
    
    r_new = xmalloc(sizeof(region));
    if (!r_new) {
        return NULL;
    }
    
    memcpy(r_new, r, sizeof(region));

    /* duplicate allocatable storage */
    
    return r_new;
}

region *region_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorRegion) {
        return (region *) q->data;
    } else {
        return NULL;
    }
}

int region_set_active(Quark *q, int flag)
{
    region *r = region_get_data(q);
    if (r) {
        r->active = flag;
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int region_set_type(Quark *q, int type)
{
    region *r = region_get_data(q);
    if (r) {
        r->type = type;
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int region_add_point(Quark *q, const WPoint *wp)
{
    region *r = region_get_data(q);
    if (r) {
        r->wps = xrealloc(r->wps, (r->n + 1)*sizeof(WPoint));
        r->wps[r->n] = *wp;
        r->n++;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int region_set_color(Quark *q, int color)
{
    region *r = region_get_data(q);
    if (r) {
        r->color = color;
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
