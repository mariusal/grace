/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003,2004 Grace Development Team
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
 * Region stuff
 *
 */

#include <string.h>

#define ADVANCED_MEMORY_HANDLERS
#include "grace/coreP.h"

static void set_region_defaults(region *r)
{
    if (!r) {
        return;
    }
    
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

region *region_data_new(AMem *amem)
{
    region *r;
    
    r = amem_malloc(amem, sizeof(region));
    if (r) {
        memset(r, 0, sizeof(region));
        set_region_defaults(r);
    }
    return r;
}

void region_data_free(AMem *amem, region *r)
{
    if (r) {
        amem_free(amem, r->wps);
        amem_free(amem, r);
    }
}

region *region_data_copy(AMem *amem, region *r)
{
    region *r_new;
    
    if (!r) {
        return NULL;
    }
    
    r_new = amem_malloc(amem, sizeof(region));
    if (!r_new) {
        return NULL;
    }
    
    memcpy(r_new, r, sizeof(region));

    /* duplicate allocatable storage */
    r_new->wps = amem_malloc(amem, r->n*sizeof(WPoint));
    memcpy(r_new->wps, r->wps, r->n*sizeof(WPoint));
    
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

int region_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorRegion,
        (Quark_data_new) region_data_new,
        (Quark_data_free) region_data_free,
        (Quark_data_copy) region_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
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
        r->wps = amem_realloc(q->amem, r->wps, (r->n + 1)*sizeof(WPoint));
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



/*
 * routines to determine if a point lies in a polygon
*/
static int intersect_to_left(const WPoint *wp,
    const WPoint *wp1, const WPoint *wp2)
{
    double xtmp, m, b;

    /* ignore horizontal lines */
    if (wp1->y == wp2->y) {
	return 0;
    }
    /* not contained vertically */
    if (((wp->y < wp1->y) && (wp->y < wp2->y)) ||
        ((wp->y > wp1->y) && (wp->y > wp2->y))) {
	return 0;
    }
    /* none of the above, compute the intersection */
    if ((xtmp = wp2->x - wp1->x) != 0.0) {
	m = (wp2->y - wp1->y) / xtmp;
	b = wp1->y - m * wp1->x;
	xtmp = (wp->y - b) / m;
    } else {
	xtmp = wp1->x;
    }
    if (xtmp <= wp->x) {
	/* check for intersections at a vertex */
	/* if this is the max ordinate then accept */
	if (wp->y == wp1->y) {
	    if (wp1->y > wp2->y) {
		return 1;
	    } else {
		return 0;
	    }
	}
	/* check for intersections at a vertex */
	if (wp->y == wp2->y) {
	    if (wp2->y > wp1->y) {
		return 1;
	    } else {
		return 0;
	    }
	}
	/* no vertices intersected */
	return 1;
    }
    return 0;
}

/*
 * determine if (x,y) is in the polygon xlist[], ylist[]
 */
static int inbound(const WPoint *wp, const WPoint *wps, int n)
{
    int i, l = 0;

    for (i = 0; i < n; i++) {
	l += intersect_to_left(wp, &wps[i], &wps[(i + 1) % n]);
    }
    return l % 2;
}

static int isleft(const WPoint *wp, const WPoint *wp1, const WPoint *wp2)
{
    if ((wp2->x - wp1->x)*(wp->y - wp2->y) -
        (wp2->y - wp1->y)*(wp->x - wp2->x) >= 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int inband(const WPoint *wp, const WPoint *wp1, const WPoint *wp2)
{
    double s, d2;
    
    s  = (wp->x - wp1->x)*(wp2->x - wp1->x) +
         (wp->y - wp1->y)*(wp2->y - wp1->y);
    d2 = (wp2->x - wp1->x)*(wp2->x - wp1->x) +
         (wp2->y - wp1->y)*(wp2->y - wp1->y);
    if (s < 0 || s > d2) {
        return FALSE;
    } else {
        return TRUE;
    }
}

int region_contains(const Quark *q, const WPoint *wp)
{
    region *r = region_get_data(q);
    
    if (!r) {
        return FALSE;
    }
        
    switch (r->type) {
    case REGION_POLYGON:
	if (r->n > 2) {
            return inbound(wp, r->wps, r->n);
        } else
	if (r->n == 2) {
            return isleft(wp, &r->wps[0], &r->wps[1]);
        } else {
            return FALSE;
        }
	break;
    case REGION_BAND:
	if (r->n == 2) {
	    return inband(wp, &r->wps[0], &r->wps[1]);
	} else {
            return FALSE;
        }
        break;
    }
    
    return FALSE;
}
