/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 * routines to allocate, manipulate, and return
 * information about regions.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "grace/canvas.h"
#include "graphs.h"
#include "utils.h"
#include "protos.h"

/*
 * see if (x,y) lies inside the plot
 */
int inbounds(Quark *gr, double x, double y)
{
    WPoint wp;
    
    wp.x = x;
    wp.y = y;
    return is_validWPoint(wp);
}

/*
 * routines to determine if a point lies in a polygon
*/
int intersect_to_left(double x, double y, double x1, double y1, double x2, double y2)
{
    double xtmp, m, b;

    /* ignore horizontal lines */
    if (y1 == y2) {
	return 0;
    }
    /* not contained vertically */
    if (((y < y1) && (y < y2)) || ((y > y1) && (y > y2))) {
	return 0;
    }
    /* none of the above, compute the intersection */
    if ((xtmp = x2 - x1) != 0.0) {
	m = (y2 - y1) / xtmp;
	b = y1 - m * x1;
	xtmp = (y - b) / m;
    } else {
	xtmp = x1;
    }
    if (xtmp <= x) {
	/* check for intersections at a vertex */
	/* if this is the max ordinate then accept */
	if (y == y1) {
	    if (y1 > y2) {
		return 1;
	    } else {
		return 0;
	    }
	}
	/* check for intersections at a vertex */
	if (y == y2) {
	    if (y2 > y1) {
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
int inbound(double x, double y, double *xlist, double *ylist, int n)
{
    int i, l = 0;

    for (i = 0; i < n; i++) {
	l += intersect_to_left(x, y, xlist[i], ylist[i], xlist[(i + 1) % n], ylist[(i + 1) % n]);
    }
    return l % 2;
}

/*
 * routines to determine if a point lies to the left of an infinite line
*/
int isleft(double x, double y, double x1, double y1, double x2, double y2)
{
    double xtmp, m, b;

    /* horizontal lines */
    if (y1 == y2) {
	return 0;
    }
    /* none of the above, compute the intersection */
    if ((xtmp = x2 - x1) != 0.0) {
	m = (y2 - y1) / xtmp;
	b = y1 - m * x1;
	xtmp = (y - b) / m;
    } else {
	xtmp = x1;
    }
    if (xtmp >= x) {
	return 1;
    }
    return 0;
}

/*
 * routines to determine if a point lies to the left of an infinite line
*/
int isright(double x, double y, double x1, double y1, double x2, double y2)
{
    double xtmp, m, b;

    /* horizontal lines */
    if (y1 == y2) {
	return 0;
    }
    if ((xtmp = x2 - x1) != 0.0) {
	m = (y2 - y1) / xtmp;
	b = y1 - m * x1;
	xtmp = (y - b) / m;
    } else {
	xtmp = x1;
    }
    if (xtmp <= x) {
	return 1;
    }
    return 0;
}

/*
 * routines to determine if a point lies above an infinite line
*/
int isabove(double x, double y, double x1, double y1, double x2, double y2)
{
    double ytmp, m, b;

    /* vertical lines */
    if (x1 == x2) {
	return 0;
    }
    if ((ytmp = y2 - y1) != 0.0) {
	m = ytmp / (x2 - x1);
	b = y1 - m * x1;
	ytmp = m * x + b;
    } else {
	ytmp = y1;
    }
    if (ytmp <= y) {
	return 1;
    }
    return 0;
}

/*
 * routines to determine if a point lies below an infinite line
*/
int isbelow(double x, double y, double x1, double y1, double x2, double y2)
{
    double ytmp, m, b;

    /* vertical lines */
    if (x1 == x2) {
	return 0;
    }
    if ((ytmp = y2 - y1) != 0.0) {
	m = ytmp / (x2 - x1);
	b = y1 - m * x1;
	ytmp = m * x + b;
    } else {
	ytmp = y1;
    }
    if (ytmp >= y) {
	return 1;
    }
    return 0;
}

int inregion(Quark *gr, int regno, double x, double y)
{
#if 0
    if (regno == MAXREGION) {
	return (inbounds(gr , x, y));
    }
    if (regno == MAXREGION + 1) {
	return (!inbounds(gr , x, y));
    }
    if (rg[regno].active == TRUE) {
	switch (rg[regno].type) {
	case REGION_POLYI:
	    if (inbound(x, y, rg[regno].x, rg[regno].y, rg[regno].n)) {
		return 1;
	    }
	    break;
	case REGION_POLYO:
	    if (!inbound(x, y, rg[regno].x, rg[regno].y, rg[regno].n)) {
		return 1;
	    }
	    break;
	case REGION_TORIGHT:
	    if (isright(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
		return 1;
	    }
	    break;
	case REGION_TOLEFT:
	    if (isleft(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
		return 1;
	    }
	    break;
	case REGION_ABOVE:
	    if (isabove(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
		return 1;
	    }
	    break;
	case REGION_BELOW:
	    if (isbelow(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
		return 1;
	    }
	    break;
	case REGION_HORIZI:
	  return (x >= rg[regno].x1) && ( x <= rg[regno].x2);
	  break;
	case REGION_VERTI:
	  return (y >= rg[regno].y1) && ( y <= rg[regno].y2);
	  break;
	case REGION_HORIZO:
	  return !( (x >= rg[regno].x1) && ( x <= rg[regno].x2) );
	  break;
	case REGION_VERTO:
	  return !( (y >= rg[regno].y1) && ( y <= rg[regno].y2) );
	  break;

	}
    }
#endif
    return FALSE;
}

char *region_types(int it, int which)
{
    return "";
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

int region_add_point(Quark *q, double x, double y)
{
    region *r = region_get_data(q);
    if (r) {
        r->x = xrealloc(r->x, (r->n + 1)*SIZEOF_DOUBLE);
        r->y = xrealloc(r->y, (r->n + 1)*SIZEOF_DOUBLE);
        r->x[r->n] = x;
        r->y[r->n] = y;
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
