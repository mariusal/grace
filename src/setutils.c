/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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
 * information about sets.
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "utils.h"
#include "graphs.h"
#include "protos.h"

extern graph *g;

static char buf[256];

/*
 * return the string version of the set type
 */
char *set_types(int it)
{
    char *s = "xy";

    switch (it) {
    case SET_XY:
	s = "xy";
	break;
    case SET_BAR:
	s = "bar";
	break;
    case SET_BARDY:
	s = "bardy";
	break;
    case SET_BARDYDY:
	s = "bardydy";
	break;
    case SET_XYZ:
	s = "xyz";
	break;
    case SET_XYDX:
	s = "xydx";
	break;
    case SET_XYDY:
	s = "xydy";
	break;
    case SET_XYDXDX:
	s = "xydxdx";
	break;
    case SET_XYDYDY:
	s = "xydydy";
	break;
    case SET_XYDXDY:
	s = "xydxdy";
	break;
    case SET_XYHILO:
	s = "xyhilo";
	break;
    case SET_XYR:
	s = "xyr";
	break;
    case SET_XYSTRING:
	s = "xystring";
	break;
    }
    return s;
}

int get_settype_by_name(char *s)
{
    int i;
    
    for (i = 0; i < NUMBER_OF_SETTYPES; i++) {
        if (strcmp(set_types(i), s) == 0) {
            return i;
        }
    }
    return SET_BAD;
}

int settype_cols(int type)
{
    int ncols;
    
    switch (type) {
    case SET_XY:
    case SET_BAR:
    case SET_XYSTRING:
	ncols = 2;
	break;
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
    case SET_BARDY:
    case SET_XYR:
	ncols = 3;
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_BARDYDY:
	ncols = 4;
	break;
    case SET_XYHILO:
	ncols = 5;
	break;
    default:
        ncols = 0;
        break;
    }
    
    return ncols;
}

/*
 * allocate arrays for a set of length len.
 */
void allocxy(plotarr *p, int len)
{
    int i, ncols;

    if (len == p->len) {
	return;
    }

    ncols = settype_cols(p->type);
    
    if (ncols == 0) {
	errmsg("Set type not found in setutils.c:allocxy()!!");
	return;
    }
    
    if (p->type == SET_XYSTRING) {
        p->s = xrealloc(p->s, len*sizeof(char *));
    }
    
    for (i = 0; i < ncols; i++) {
	if ((p->ex[i] = xrealloc(p->ex[i], len*SIZEOF_DOUBLE)) == NULL) {
	    errmsg("Insufficient memory to allocate for plots");
	    return;
	}
    }
    
    p->len = len;
    set_dirtystate();
    set_lists_dirty(TRUE);
}

int init_array(double **a, int n)
{
    *a = (double *) xrealloc(*a, n * SIZEOF_DOUBLE);
    
    return *a == NULL ? 1 : 0;
}

int init_scratch_arrays(int n)
{
    if (!init_array(&ax, n)) {
	if (!init_array(&bx, n)) {
	    if (!init_array(&cx, n)) {
		if (!init_array(&dx, n)) {
		    maxarr = n;
		    return 0;
		}
		free(cx);
	    }
	    free(bx);
	}
	free(ax);
    }
    return 1;
}

/*
 * get the min/max fields of a set
 */
int getsetminmax(int gno, int setno, 
                    double *xmin, double *xmax, double *ymin, double *ymax)
{
    double x1, x2, y1, y2;
    int i, first = TRUE;
    int imin, imax; /* dummy */

    if (setno == ALL_SETS) {
        for (i = 0; i < g[gno].maxplot; i++) {
            if (is_set_active(gno, i)) {
                minmax(g[gno].p[i].ex[0], g[gno].p[i].len, &x1, &x2, &imin, &imax);
                minmax(g[gno].p[i].ex[1], g[gno].p[i].len, &y1, &y2, &imin, &imax);
                if (first) {
                    *xmin = x1;
                    *xmax = x2;
                    *ymin = y1;
                    *ymax = y2;
                    first = FALSE;
                } else {
                    *xmin = (x1 < *xmin) ? x1 : *xmin;
                    *xmax = (x2 > *xmax) ? x2 : *xmax;
                    *ymin = (y1 < *ymin) ? y1 : *ymin;
                    *ymax = (y2 > *ymax) ? y2 : *ymax;
                }
            }
        }
    } else if (setno >= 0) {
        minmax(g[gno].p[setno].ex[0], g[gno].p[setno].len, xmin, xmax, &imin, &imax);
        minmax(g[gno].p[setno].ex[1], g[gno].p[setno].len, ymin, ymax, &imin, &imax);
        first = FALSE;
    }
    
    if (first == FALSE) {
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

/*
 * get the min/max fields of a set with fixed x/y range
 */
int getsetminmax_c(int gno, int setno, 
            double *xmin, double *xmax, double *ymin, double *ymax, int ivec)
{
    double vmin_t, vmax_t, *vmin, *vmax, bvmin, bvmax, *vec, *bvec;
    int i, start, stop, n;
    int first = TRUE, hits;

    if (ivec == 1) {    
        bvmin = *xmin;
        bvmax = *xmax;
        vmin  = ymin; 
        vmax  = ymax; 
    } else {
        bvmin = *ymin;
        bvmax = *ymax;
        vmin  = xmin;
        vmax  = xmax;
    }
    if (setno == ALL_SETS) {
        start = 0;
        stop  = number_of_sets(gno) - 1;
    } else if (setno >= 0) {
        start = setno;
        stop  = setno;
    } else {
        return GRACE_EXIT_FAILURE;
    }
    
    for (i = start; i <= stop; i++) {
        if (is_set_active(gno, i)) {
            
            if (ivec == 1) {
                bvec = getx(gno, i);
                vec  = gety(gno, i);
            } else {
                bvec = gety(gno, i);
                vec  = getx(gno, i);
            }
            
            n = getsetlength(gno, i);
            hits = minmaxrange(bvec, vec, n, bvmin, bvmax, &vmin_t, &vmax_t);
            if (hits == GRACE_EXIT_SUCCESS) {
                if (first) {
                    *vmin = vmin_t;
                    *vmax = vmax_t;
                    first = FALSE;
                } else {
                    *vmin = MIN2(vmin_t, *vmin);
                    *vmax = MAX2(vmax_t, *vmax);
                }
            }
        }
    }
    
    if (first == FALSE) {
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}


/*
 * compute the mins and maxes of a vector x
 */
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax)
{
    int i;
    
    if (x == NULL) {
        return;
    }
    
    *xmin = x[0];
    *xmax = x[0];
    *imin = 1;
    *imax = 1;
    for (i = 1; i < n; i++) {
	if (x[i] < *xmin) {
	    *xmin = x[i];
	    *imin = i + 1;
	}
	if (x[i] > *xmax) {
	    *xmax = x[i];
	    *imax = i + 1;
	}
    }
}


/*
 * compute the min and max of vector vec calculated for indices such that
 * bvec values lie within [bmin, bmax] range
 * returns GRACE_EXIT_FAILURE if none found
 */
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax)
{
    int i, first = TRUE;
    
    if ((vec == NULL) || (bvec == NULL)) {
        return GRACE_EXIT_FAILURE;
    }
    
    for (i = 0; i < n; i++) {
        if ((bvec[i] >= bvmin) && (bvec[i] <= bvmax)) {
	    if (first == TRUE) {
                *vmin = vec[i];
                *vmax = vec[i];
                first = FALSE;
            } else {
                if (vec[i] < *vmin) {
                    *vmin = vec[i];
  	        } else if (vec[i] > *vmax) {
                    *vmax = vec[i];
       	        }
            }
        }
    }
    
    if (first == FALSE) {
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}


/*
 * compute the mins and maxes of a vector x
 */
double vmin(double *x, int n)
{
    int i;
    double xmin;
    if (n <= 0) {
	return 0.0;
    }
    xmin = x[0];
    for (i = 1; i < n; i++) {
	if (x[i] < xmin) {
	    xmin = x[i];
	}
    }
    return xmin;
}

double vmax(double *x, int n)
{
    int i;
    double xmax;
    if (n <= 0) {
	return 0.0;
    }
    xmax = x[0];
    for (i = 1; i < n; i++) {
	if (x[i] > xmax) {
	    xmax = x[i];
	}
    }
    return xmax;
}

void set_point(int gno, int setn, int seti, double wx, double wy)
{
    g[gno].p[setn].ex[0][seti] = wx;
    g[gno].p[setn].ex[1][seti] = wy;
    set_dirtystate();
}

void get_point(int gno, int setn, int seti, double *wx, double *wy)
{
    *wx = g[gno].p[setn].ex[0][seti];
    *wy = g[gno].p[setn].ex[1][seti];
}

void setcol(int gno, double *x, int setno, int len, int col)
{
    g[gno].p[setno].ex[col] = x;
    g[gno].p[setno].len = len;
    set_dirtystate();
}

int getncols(int gno, int setno)
{
    int i = 0;

    while (g[gno].p[setno].ex[i]) {
	i++;
    }
    return i;
}

void setxy(int gno, double **ex, int setno, int len, int ncols)
{
    int i;

    for (i = 0; i < ncols; i++) {
	g[gno].p[setno].ex[i] = ex[i];
    }
    g[gno].p[setno].len = len;
    set_dirtystate();
}

void setlength(int gno, int i, int length)
{
    allocxy(&g[gno].p[i], length);
}

void copycol2(int gfrom, int setfrom, int gto, int setto, int col)
{
    int i, n;
    double *x1, *x2;

    n = g[gfrom].p[setfrom].len;
    x1 = getcol(gfrom, setfrom, col);
    x2 = getcol(gto, setto, col);
    for (i = 0; i < n; i++) {
	x2[i] = x1[i];
    }
    set_dirtystate();
}


/*
 * moveset 
 */
int moveset(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;

    if (gnoto == gnofrom && setfrom == setto) {
	return GRACE_EXIT_FAILURE;
    }

    if (is_valid_setno(gnofrom, setfrom) != TRUE ||
        is_valid_setno(gnoto, setto) != TRUE ) {
        return GRACE_EXIT_FAILURE;
    }

    killset(gnoto, setto);

/*
 *     setlength(gnoto, setto, g[gnofrom].p[setfrom].len);
 */
    memcpy(&g[gnoto].p[setto], &g[gnofrom].p[setfrom], sizeof(plotarr));

    g[gnofrom].p[setfrom].len = 0;
    for (k = 0; k < MAX_SET_COLS; k++) {
	g[gnofrom].p[setfrom].ex[k] = NULL;
    }
    g[gnofrom].p[setfrom].s = NULL;
    
    g[gnofrom].p[setfrom].hidden = TRUE;
    
    set_dirtystate();
    return GRACE_EXIT_SUCCESS;
}

/*
 * copyset assumes both sets exist, have their length
 * properly set, and that they are both active
 */
void copyset(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;
    double *savec[MAX_SET_COLS];
    int len = getsetlength(gnofrom, setfrom);

    for (k = 0; k < MAX_SET_COLS; k++) {
	savec[k] = g[gnoto].p[setto].ex[k];
    }
    memcpy(&g[gnoto].p[setto], &g[gnofrom].p[setfrom], sizeof(plotarr));
    for (k = 0; k < MAX_SET_COLS; k++) {
	g[gnoto].p[setto].ex[k] = savec[k];
	if (g[gnofrom].p[setfrom].ex[k] != NULL && g[gnoto].p[setto].ex[k] != NULL) {
	    memcpy(g[gnoto].p[setto].ex[k], g[gnofrom].p[setfrom].ex[k], len * SIZEOF_DOUBLE);
	}
    }
    set_dirtystate();
}

/*
 * copy everything but the data
 */
void copysetprops(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;
    double *savec[MAX_SET_COLS];

    for (k = 0; k < MAX_SET_COLS; k++) {
	savec[k] = g[gnoto].p[setto].ex[k];
    }
    memcpy(&g[gnoto].p[setto], &g[gnofrom].p[setfrom], sizeof(plotarr));
    for (k = 0; k < MAX_SET_COLS; k++) {
	g[gnoto].p[setto].ex[k] = savec[k];
    }
    set_dirtystate();
}

/*
 * copy data only
 */
void copysetdata(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;
    int len = getsetlength(gnofrom, setfrom);

    for (k = 0; k < MAX_SET_COLS; k++) {
	if (g[gnofrom].p[setfrom].ex[k] != NULL && g[gnoto].p[setto].ex[k] != NULL) {
	    memcpy(g[gnoto].p[setto].ex[k], g[gnofrom].p[setfrom].ex[k], len * SIZEOF_DOUBLE);
	}
    }
    set_dirtystate();
}

/*
 * pack all sets leaving no gaps in the set structure
 */
void packsets(int gno)
{
    int i, j, k;

    i = 0;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (is_set_active(gno, i)) {
	    j = 0;
	    while (j < i) {
		if (!is_set_active(gno, j) && !g[gno].p[j].hidden) {
		    memcpy(&g[gno].p[j], &g[gno].p[i], sizeof(plotarr));
		    for (k = 0; k < MAX_SET_COLS; k++) {
			g[gno].p[i].ex[k] = NULL;
		    }
		    killset(gno, i);
		    set_dirtystate();
#ifndef NONE_GUI
		    updatesymbols(gno, j);
		    updatesymbols(gno, i);
		    update_set_status(gno, j);
		    update_set_status(gno, i);
#endif
		}
		j++;
	    }
	}
    }
}

/*
 * action proc for menu item
 */
void do_packsets(void)
{
    packsets(get_cg());
}

int activateset(int gno, int setno)
{
    int retval;
    
    if (is_valid_gno(gno) != TRUE) {
        return GRACE_EXIT_FAILURE;
    } else if (is_valid_setno(gno, setno)) {
        set_set_hidden(gno, setno, FALSE);
        return GRACE_EXIT_SUCCESS;
    } else {
        retval = realloc_graph_plots(gno, setno + 1);
        if (retval == GRACE_EXIT_SUCCESS) {
            set_set_hidden(gno, setno, FALSE);
        }
        return retval;
    }
}

/*
 * return the next available set in graph gno
 * ignoring deactivated sets.
 * If target is inactive and not deactivated, choose it (used for loading sets
 * from project files when sets aren't packed)
 */
int nextset(int gno)
{
    int i;
    int maxplot;

    if (is_valid_gno(gno) != TRUE) {
        return (-1);
    }
    
    if ( (target_set.gno == gno) &&
         (target_set.setno >= 0) &&
         (target_set.setno < g[gno].maxplot) &&
         !is_set_active(gno, target_set.setno)) {
	i = target_set.setno;
	target_set.gno = -1;
	target_set.setno = -1;
	return (i);
    }
    i = 0;
    maxplot = g[gno].maxplot;
    for (i = 0; i < maxplot; i++) {
	if (!is_set_active(gno, i)) {
	    return (i);
	}
    }
    /* Allocating new set */
    if (realloc_graph_plots(gno, maxplot + 1) == GRACE_EXIT_SUCCESS) {
        return maxplot;
    } else {
        return (-1);
    }
}

/*
 * free set data, but preserve the parameter settings
 */
void killsetdata(int gno, int setno)
{
    int i;

    if (is_set_active(gno, setno)) {
	for (i = 0; i < MAX_SET_COLS; i++) {
	    if (g[gno].p[setno].ex[i] != NULL) {
		free(g[gno].p[setno].ex[i]);
	    }
	    g[gno].p[setno].ex[i] = NULL;
	}
	if (dataset_type(gno, setno) == SET_XYSTRING && g[gno].p[setno].s != NULL) {
	    for (i = 0; i < getsetlength(gno, setno); i++) {
		cxfree(g[gno].p[setno].s[i]);
	    }
	    cxfree(g[gno].p[setno].s[i]);
	    g[gno].p[setno].s = NULL;
	}
	g[gno].p[setno].len = 0;
	g[gno].p[setno].hidden = TRUE;
	set_lists_dirty(TRUE);
	set_dirtystate();
    }
}

/*
 * kill a set
 */
void killset(int gno, int setno)
{
    if (is_valid_setno(gno, setno)) {
	killsetdata(gno, setno);
	set_default_plotarr(&g[gno].p[setno]);
    }
}

int is_set_active(int gno, int setno)
{
    if (is_valid_setno(gno, setno) && g[gno].p[setno].len > 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * return TRUE if there are active set(s) in the gno graph
 */
int activeset(int gno)
{
    int i;

    for (i = 0; i < g[gno].maxplot; i++) {
	if (is_set_active(gno, i) == TRUE) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * drop points from a set
 */
void droppoints(int gno, int setno, int startno, int endno, int dist)
{
    double *x;
    int i, j, len, ncols;

    len = getsetlength(gno, setno);
    ncols = getncols(gno, setno);
    for (j = 0; j < ncols; j++) {
	x = getcol(gno, setno, j);
	for (i = endno + 1; i < len; i++) {
	    x[i - dist] = x[i];
	}
    }
    setlength(gno, setno, len - dist);
}

/*
 * join 2 sets together
 */
void joinsets(int g1, int j1, int g2, int j2)
{
    int i, j, len1, len2, ncols1, ncols2, ncols;
    double *x1, *x2;

    len1 = getsetlength(g1, j1);
    len2 = getsetlength(g2, j2);
    setlength(g2, j2, len1 + len2);
    ncols1 = getncols(g1, j1);
    ncols2 = getncols(g2, j2);
    ncols = (ncols2 < ncols1) ? ncols2 : ncols1;
    for (j = 0; j < ncols; j++) {
	x1 = getcol(g1, j1, j);
	x2 = getcol(g2, j2, j);
	for (i = len2; i < len2 + len1; i++) {
	    x2[i] = x1[i - len2];
	}
    }
}

/*
 * sort a set
 */
static double *vptr;

/*
 * for ascending and descending sorts
 */
 
int compare_points1(const void *p1, const void *p2)
{
    const double *a, *b;
    a = (const double *)p1;
    b = (const double *)p2;
    if (*a < *b) {
	return -1;
    }
    if (*a > *b) {
	return 1;
    }
    return 0;
}

int compare_points2(const void *p1, const void *p2)
{
    const double *a, *b;
    a = (const double *)p1;
    b = (const double *)p2;
    if (*a > *b) {
	return -1;
    }
    if (*a < *b) {
	return 1;
    }
    return 0;
}


void sortset(int gno, int setno, int sorton, int stype)
{
    int i, j, nc, len, *ind;
    double *dtmp, *stmp;

/*
 * get the vector to sort on
 */
    vptr = getvptr(gno, setno, sorton);
    if (vptr == NULL) {
	errmsg("NULL vector in sort, operation cancelled, check set type");
	return;
    }
    len = getsetlength(gno, setno);
    if (len <= 1) {
	errmsg("Setlength <= 1, nothing to do!");
	return;
    }
/*
 * allocate memory for permuted indices
 */
    ind = (int *) calloc(len, sizeof(int));
    if (ind == NULL) {
	errmsg("Unable to allocate memory for sort");
	return;
    }
/*
 * allocate memory for temporary array
 */
    dtmp = (double *) calloc(len, SIZEOF_DOUBLE);
    if (dtmp == NULL) {
	free(ind);
	errmsg("Unable to allocate memory for sort");
	return;
    }
/*
 * initialize indices
 */
    for (i = 0; i < len; i++) {
	ind[i] = i;
    }

/*
 * sort
 */
    qsort(ind, len, sizeof(int),  stype ? compare_points2 : compare_points1);

/*
 * straighten things out - done one vector at a time for storage.
 */
    nc = getncols(gno, setno);
/* loop over the number of columns */
    for (j = 0; j < nc; j++) {
/* get this vector and put into the temporary vector in the right order */
	stmp = getcol(gno, setno, j);
	for (i = 0; i < len; i++) {
	    dtmp[i] = stmp[ind[i]];
	}
/* load it back to the set */
	for (i = 0; i < len; i++) {
	    stmp[i] = dtmp[i];
	}
    }
    set_dirtystate();
}

/*
 * sort a set - only does type SET_XY
 */
void sort_xy(double *tmp1, double *tmp2, int up, int sorton, int stype)
{

    int d, i, j;
    int lo = 0;
    double t1, t2;

    if (sorton == 1) {
	double *ttmp;

	ttmp = tmp1;
	tmp1 = tmp2;
	tmp2 = ttmp;
    }
    up--;

    for (d = up - lo + 1; d > 1;) {
	if (d < 5)
	    d = 1;
	else
	    d = (5 * d - 1) / 11;
	for (i = up - d; i >= lo; i--) {
	    t1 = tmp1[i];
	    t2 = tmp2[i];
	    if (!stype) {
		for (j = i + d; j <= up && (t1 > tmp1[j]); j += d) {
		    tmp1[j - d] = tmp1[j];
		    tmp2[j - d] = tmp2[j];
		}
		tmp1[j - d] = t1;
		tmp2[j - d] = t2;
	    } else {
		for (j = i + d; j <= up && (t1 < tmp1[j]); j += d) {
		    tmp1[j - d] = tmp1[j];
		    tmp2[j - d] = tmp2[j];
		}
		tmp1[j - d] = t1;
		tmp2[j - d] = t2;
	    }
	}
    }
    set_dirtystate();
}

/*
 * locate a point and the set the point is in
 */
void findpoint(int gno, double x, double y, double *xs, double *ys, int *setno, int *loc)
{
    double dx = g[gno].w.xg2 - g[gno].w.xg1, dy = g[gno].w.yg2 - g[gno].w.yg1,
    *xtmp, *ytmp, tmp, tmin = MAXNUM;
    int i, j, len;

    *setno = -1;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (is_set_active(gno, i)) {
	    xtmp = getx(gno, i);
	    ytmp = gety(gno, i);
	    len = getsetlength(gno, i);
	    for (j = 0; j < len; j++) {
		if ((tmp = hypot((x - xtmp[j]) / dx, (y - ytmp[j]) / dy)) < tmin) {
		    *setno = i;
		    *loc = j + 1;
		    *xs = xtmp[j];
		    *ys = ytmp[j];
		    tmin = tmp;
		}
	    }
	}
    }
}

/*
 * locate a point in setno nearest (x, y)
 */
void findpoint_inset(int gno, int setno, double x, double y, int *loc)
{
    double dx = g[gno].w.xg2 - g[gno].w.xg1, dy = g[gno].w.yg2 - g[gno].w.yg1,
    *xtmp, *ytmp, tmp, tmin = MAXNUM;
    int j, len;

    if (is_set_active(gno, setno)) {
	xtmp = getx(gno, setno);
	ytmp = gety(gno, setno);
	len = getsetlength(gno, setno);
	for (j = 0; j < len; j++) {
	    if ((tmp = hypot((x - xtmp[j]) / dx, (y - ytmp[j]) / dy)) < tmin) {
		*loc = j + 1;
		tmin = tmp;
	    }
	}
    } else {
	*loc = -1;
    }
}

/*
 * delete the point pt in setno
 */
void del_point(int gno, int setno, int pt)
{
    int i, j, len, ncols;
    double *tmp;

    ncols = getncols(gno, setno);
    len = getsetlength(gno, setno);
    if (pt > len) {
	return;
    }
    if (pt != len) {
	for (i = pt - 1; i < len - 1; i++) {
	    for (j = 0; j < ncols; j++) {
		tmp = g[gno].p[setno].ex[j];
		tmp[i] = tmp[i + 1];
	    }
	}
    }
    if (len > 1) {
	setlength(gno, setno, len - 1);
    } else {
	killsetdata(gno, setno);
    }
}

/*
 * add a point to setno
 */
void add_point(int gno, int setno, double px, double py, double tx, double ty, int type)
{
    int len;
    double *x, *y;

    if (is_valid_setno(gno, setno)) {
	 len = getsetlength(gno, setno);
	 setlength(gno, setno, len + 1);
	 x = getx(gno, setno);
	 y = gety(gno, setno);
	 x[len] = px;
	 y[len] = py;
    }
}

/*
 * add a point to setno after or before ind
 * where: 1 = after, 0 = before
 */
void add_point_at(int gno, int setno, int ind, int where,
                        double px, double py, double tx, double ty, int type)
{
    int i, len, ncols;
    double *x, *y, *dx = NULL, *dy = NULL;

    if (is_valid_setno(gno, setno)) {
        len = getsetlength(gno, setno);
        setlength(gno, setno, len + 1);
        ncols = getncols( gno, setno );
        x = getx(gno, setno);
        y = gety(gno, setno);
        if ( ncols >=3 ) {
            dx = getcol( gno, setno, 2 );
            if ( ncols >= 4 ) {
                dy = getcol( gno, setno, 3 );
            }
        }
	for (i = len - 1; i > ind; i--) {
	    x[i + 1] = x[i];
	    y[i + 1] = y[i];
	    if( ncols >=3 ) {
	        dx[i+1] = dx[i];
	        if( ncols >= 4 ) {
	            dy[i+1] = dy[i];
                }
	    }
        }
	if (where) {		/* add after ind */
	    x[ind + 1] = px;
	    y[ind + 1] = py;
	    if ( ncols >=3 ) {
	        dx[ind+1] = tx;
	        if( ncols >= 4 ) {
	            dy[ind+1] = ty;
                }
	    }
	} else {		/* add point before ind (at ind) */
	    i--;
	    x[i + 1] = x[i];
	    y[i + 1] = y[i];
	    if ( ncols >=3 ) {
	        dx[i+1] = dx[i];
	        if ( ncols >= 4 ) {
	            dy[i+1] = dy[i];
                }
	    }
	    x[ind] = px;
	    y[ind] = py;
	    if( ncols >=3 ) {
	        dx[ind] = tx;
	        if ( ncols >= 4 ) {
	            dy[ind] = ty;
                }
	    }
	}
    } 
}

void delete_byindex(int gno, int setno, int *ind)
{
    int i, j, cnt = 0;
    int ncols = getncols(gno, setno);
    for (i = 0; i < getsetlength(gno, setno); i++) {
	if (ind[i]) {
	    cnt++;
	}
    }
    if (cnt == getsetlength(gno, setno)) {
	killset(gno, setno);
	return;
    }
    cnt = 0;
    for (i = 0; i < getsetlength(gno, setno); i++) {
	if (ind[i] == 0) {
	    for (j = 0; j < ncols; j++) {
		g[gno].p[setno].ex[j][cnt] = g[gno].p[setno].ex[j][i];
	    }
	    cnt++;
	}
    }
    setlength(gno, setno, cnt);
}


/*
 * copy a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
void do_copyset(int gfrom, int j1, int gto, int j2)
{
    if (!is_graph_active(gto)) {
	set_graph_active(gto, TRUE);
    }
    if (!is_set_active(gfrom, j1)) {
	return;
    }
    if (j1 == j2 && gfrom == gto) {
	return;
    }
    if (is_set_active(gto, j2)) {
	killset(gto, j2);
    }
    activateset(gto, j2);
    set_dataset_type(gto, j2, dataset_type(gfrom, j1));
    setlength(gto, j2, getsetlength(gfrom, j1));
    copyset(gfrom, j1, gto, j2);
    sprintf(buf, "copy of set %d", j1);
    setcomment(gto, j2, buf);
    log_results(buf);
}

/*
 * move a set to another set, in possibly another graph
 */
void do_moveset(int gfrom, int j1, int gto, int j2)
{
    moveset(gfrom, j1, gto, j2);
}

/*
 * swap a set with another set
 */
int swapset(int gno1, int setno1, int gno2, int setno2)
{
    plotarr p;

    if (setno1 == setno2 && gno1 == gno2) {
	errmsg("Set from and set to are the same");
	return GRACE_EXIT_FAILURE;
    }
    memcpy(&p, &g[gno2].p[setno2], sizeof(plotarr));
    memcpy(&g[gno2].p[setno2], &g[gno1].p[setno1], sizeof(plotarr));
    memcpy(&g[gno1].p[setno1], &p, sizeof(plotarr));
    set_lists_dirty(TRUE);
    set_dirtystate();
    
    return GRACE_EXIT_SUCCESS;
}

/*
 * split a set into lpart length sets
 */
void do_splitsets(int gno, int setno, int lpart)
{
    int i, j, k, nsets, ncols, len, nleft, tmpset, psets, stype;
    char s[256];
    double *x[MAX_SET_COLS], *xtmp[MAX_SET_COLS], *xt[MAX_SET_COLS];
    plotarr p;

    if (!activeset(gno)) {
	errmsg("No active sets");
	return;
    }
    if (!is_set_active(gno, setno)) {
	sprintf(s, "Set %d not active", setno);
	errmsg(s);
	return;
    }
    if ((len = getsetlength(gno, setno)) < 3) {
	errmsg("Set length < 3");
	return;
    }
    if (lpart >= len) {
	errmsg("Split length >= set length");
	return;
    }
    if (lpart == 0) {
	errmsg("Split length = 0");
	return;
    }
    psets = len / lpart;
    nleft = len % lpart;
    if (nleft) {
	psets++;
    }
    nsets = 0;

    for (i = 0; i < g[gno].maxplot; i++) {
	if (is_set_active(gno, i)) {
	    nsets++;
	}
    }
    if (psets > (g[gno].maxplot - nsets + 1)) {
	errmsg("Not enough sets for split");
	return;
    }
    /* get number of columns in this set */
    ncols = getncols(gno, setno);

    /* copy the contents to a temporary buffer */
    for (j = 0; j < ncols; j++) {
	x[j] = getcol(gno, setno, j);
	xtmp[j] = (double *) calloc(len, SIZEOF_DOUBLE);
	if (xtmp[j] == NULL) {
	    errmsg("Not enough memory for split");
	    for (k = 0; k < j; k++) {
		cxfree(xtmp[k]);
	    }
	    return;
	}
    }
    for (j = 0; j < ncols; j++) {
	for (i = 0; i < len; i++) {
	    xtmp[j][i] = x[j][i];
	}
    }

    /* save the set type */
    stype = dataset_type(gno, setno);
    /*
     * load the props for this set into a temporary set, set the columns to
     * NULL
     */
    p = g[gno].p[setno];
    p.len = 0;
    for (k = 0; k < MAX_SET_COLS; k++) {
	p.ex[k] = NULL;
    }

    /* return the set to the heap */
    killset(gno, setno);
    /* now load each set */

    for (i = 0; i < psets - 1; i++) {
	tmpset = nextset(gno);
	/* set the plot parameters includes the set type */
	g[gno].p[tmpset] = p;
	activateset(gno, tmpset);
	set_dataset_type(gno, tmpset, stype);
	setlength(gno, tmpset, lpart);
	/* load the data into each column */
	for (k = 0; k < ncols; k++) {
	    xt[k] = getcol(gno, tmpset, k);
	    for (j = 0; j < lpart; j++) {
		xt[k][j] = xtmp[k][i * lpart + j];
	    }
	}
	sprintf(s, "partition %d of set %d", i + 1, setno);
	setcomment(gno, tmpset, s);
	log_results(buf);
#ifndef NONE_GUI
	update_set_status(gno, tmpset);
#endif
    }
    if (nleft == 0) {
	nleft = lpart;
    }
    tmpset = nextset(gno);
    memcpy(&g[gno].p[tmpset], &p, sizeof(plotarr));
    activateset(gno, tmpset);
    set_dataset_type(gno, tmpset, stype);
    setlength(gno, tmpset, nleft);

    /* load the data into each column */
    for (k = 0; k < ncols; k++) {
	xt[k] = getcol(gno, tmpset, k);
	for (j = 0; j < nleft; j++) {
	    xt[k][j] = xtmp[k][i * lpart + j];
	}
    }

    sprintf(s, "partition %d of set %d", i + 1, setno);
    setcomment(gno, tmpset, s);
    log_results(buf);
#ifndef NONE_GUI
    update_set_status(gno, tmpset);
#endif
    for (k = 0; k < ncols; k++) {
	free(xtmp[k]);
    }
}

/*
 * break a set at a point
 */
void do_breakset(int gno, int setno, int ind)
{
    int j, k, ncols, len, tmpset, stype;
    int n1, n2;
    char s[256];
    double *e1, *e2;

    if (!activeset(gno)) {
	errmsg("No active sets");
	return;
    }
    if (!is_set_active(gno, setno)) {
	sprintf(s, "Set %d not active", setno);
	errmsg(s);
	return;
    }
    if ((len = getsetlength(gno, setno)) < ind + 1) {
	errmsg("Set length less than point index");
	return;
    }
    /* get number of columns in this set */
    ncols = getncols(gno, setno);
    stype = dataset_type(gno, setno);

    n2 = len - ind;		/* upper part of new set */
    n1 = len - n2;		/* lower part of old set */
    if (n1 <= 0 || n2 <= 0) {
	errmsg("Break set length <= 0");
	return;
    }
    tmpset = nextset(gno);
    if (tmpset == -1) {
	return;
    }
    activateset(gno, tmpset);
    set_dataset_type(gno, tmpset, stype);
    setlength(gno, tmpset, n2);

    /* load the data into each column */
    for (k = 0; k < ncols; k++) {
	e1 = getcol(gno, setno, k);
	e2 = getcol(gno, tmpset, k);
	for (j = ind; j < len; j++) {
	    e2[j - ind] = e1[j];
	}
    }

    setlength(gno, setno, n1);

    sprintf(s, "Break S%d at point %d", setno, ind);
    setcomment(gno, tmpset, s);
    log_results(buf);
#ifndef NONE_GUI
    update_set_status(gno, setno);
    update_set_status(gno, tmpset);
#endif
}


/*
 * activate a set and set its length
 */
void do_activate(int setno, int type, int len)
{
    if (is_set_active(get_cg(), setno)) {
	sprintf(buf, "Set %d already active", setno);
	errmsg(buf);
	return;
    }
    if (len <= 0) {
	sprintf(buf, "Improper set length = %d", len);
	errmsg(buf);
	return;
    }
    activateset(get_cg(), setno);
    set_dataset_type(get_cg(), setno, type);
    setlength(get_cg(), setno, len);
#ifndef NONE_GUI
    update_set_status(get_cg(), setno);
#endif
}

/*
 * hide a set
 */
void do_hideset(int gno, int setno)
{
    g[gno].p[setno].hidden = TRUE;
    set_lists_dirty(TRUE);
#ifndef NONE_GUI
    update_set_status(gno, setno);
#endif
}

/*
 * show a hidden set
 */
void do_showset(int gno, int setno)
{
    if (is_set_active(gno, setno)) {
        g[gno].p[setno].hidden = FALSE;
        set_lists_dirty(TRUE);
#ifndef NONE_GUI
        update_set_status(gno, setno);
#endif
    }
}

/*
 * copy a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
void do_copy(int j1, int gfrom, int j2, int gto)
{
    if (!is_set_active(gfrom, j1)) {
	sprintf(buf, "Set %d not active", j1);
	errmsg(buf);
	return;
    }
    gto--;
    if (gto == -1) {
	gto = get_cg();
    }
    if (!is_graph_active(gto)) {
	set_graph_active(gto, TRUE);
    }
    if (j1 == j2 && gfrom == gto) {
	errmsg("Set from and set to are the same");
	return;
    }
    /* select next set */
    if (j2 == SET_SELECT_NEXT) {
	if ((j2 = nextset(gto)) != -1) {
	    activateset(gto, j2);
	    set_dataset_type(gto, j2, dataset_type(gfrom, j1));
	    setlength(gto, j2, getsetlength(gfrom, j1));
	} else {
	    return;
	}
    }
    /* use user selected set */
    else {
	if (is_set_active(gto, j2)) {
	    sprintf(buf, "Set %d active, overwrite?", j2);
	    if (!yesno(buf, NULL, NULL, NULL)) {
		return;
	    }
	    killset(gto, j2);
	}
	activateset(gto, j2);
	set_dataset_type(gto, j2, dataset_type(gfrom, j1));
	setlength(gto, j2, getsetlength(gfrom, j1));
    }
    copyset(gfrom, j1, gto, j2);
    sprintf(buf, "copy of set %d", j1);
    setcomment(gto, j2, buf);
    log_results(buf);
#ifndef NONE_GUI
    update_set_status(gto, j2);
#endif
}

/*
 * move a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
void do_move(int j1, int gfrom, int j2, int gto)
{
    if (!is_set_active(gfrom, j1)) {
	sprintf(buf, "Set %d not active", j1);
	errmsg(buf);
	return;
    }
    gto--;
    if (gto == -1) {
	gto = get_cg();
    }
    if (!is_graph_active(gto)) {
	set_graph_active(gto, TRUE);
    }
    if (j2 == SET_SELECT_NEXT) {
	if ((j2 = nextset(gto)) == -1) {
	    return;
	}
    }
    if (j1 == j2 && gto == gfrom) {
	errmsg("Set from and set to are the same");
	return;
    }
    if (is_set_active(gto, j2)) {
	sprintf(buf, "Set %d active, overwrite?", j2);
	if (!yesno(buf, NULL, NULL, NULL)) {
	    return;
	}
    }
    moveset(gfrom, j1, gto, j2);
#ifndef NONE_GUI
    updatesymbols(gto, j2);
    updatesymbols(gfrom, j1);
    update_set_status(gto, j2);
    update_set_status(gfrom, j1);
#endif
}

/*
 * drop points from an active set
 */
void do_drop_points(int setno, int startno, int endno)
{
    int dist;
    int setlength;

    if (!is_set_active(get_cg(), setno)) {
	sprintf(buf, "Set %d not active", setno);
	errmsg(buf);
	return;
    }

    setlength = getsetlength(get_cg(), setno);
    if (startno < 0) startno = setlength + 1 + startno;
    if (endno   < 0) endno   = setlength + 1 + endno;

    if(startno > endno) {
      dist=startno; startno=endno; endno=dist;
    }

    dist = endno - startno + 1;

    if (startno < 0) {
	errmsg("Start # < 1");
	return;
    }
    if (endno >= setlength) {
	errmsg("Ending # > set length");
	return;
    }

    if (dist == setlength) {
	errmsg("# of points to drop = set length, use kill");
	return;
    }
    droppoints(get_cg(), setno, startno, endno, dist);
#ifndef NONE_GUI
    update_set_status(get_cg(), setno);
#endif
}

/*
 * append one set to another
 */
void do_join_sets(int gfrom, int j1, int gto, int j2)
{
    int i;

    if (j1 == -1) {
	if (!is_set_active(gfrom, j2)) {
	    activateset(gfrom, j2);
	    setlength(gfrom, j2, 0);
	}
	for (i = 0; i < g[gfrom].maxplot; i++) {
	    if (is_set_active(gfrom, i) && i != j2) {
		joinsets(gfrom, i, gfrom, j2);
		killset(gfrom, i);
#ifndef NONE_GUI
		update_set_status(gfrom, i);
#endif
	    }
	}
    } else {
	if (!is_set_active(gfrom, j1)) {
	    sprintf(buf, "Set %d not active", j1);
	    errmsg(buf);
	    return;
	}
	if (!is_set_active(gto, j2)) {
	    sprintf(buf, "Set %d not active", j2);
	    errmsg(buf);
	    return;
	}
	joinsets(gfrom, j1, gto, j2);
	killset(gfrom, j1);
#ifndef NONE_GUI
	update_set_status(gfrom, j1);
#endif
    }
#ifndef NONE_GUI
    update_set_status(gto, j2);
#endif
}

/*
 * reverse the order of a set
 */
void do_reverse_sets(int setno)
{
    int n, i, j, k, ncols;
    double *x;

    if (!is_set_active(get_cg(), setno)) {
	sprintf(buf, "Set %d not active", setno);
	errmsg(buf);
	return;
    }
    n = getsetlength(get_cg(), setno);
    ncols = getncols(get_cg(), setno);
    for (k = 0; k < ncols; k++) {
	x = getcol(get_cg(), setno, k);
	for (i = 0; i < n / 2; i++) {
	    j = (n - 1) - i;
	    fswap(&x[i], &x[j]);
	}
    }
    set_dirtystate();
#ifndef NONE_GUI
    update_set_status(get_cg(), setno);
#endif
}

/*
 * kill a set
 */
void do_kill(int gno, int setno, int soft)
{
    int i;

    if (setno == g[gno].maxplot || setno == -1) {
	for (i = 0; i < g[gno].maxplot; i++) {
	    if (is_set_active(gno, i)) {
		if (soft) {
		    killsetdata(gno, i);
		} else {
		    killset(gno, i);
		}
#ifndef NONE_GUI
		set_lists_dirty(TRUE);
		update_set_status(gno, i);
#endif
	    }
	}
    } else {
	if (!is_set_active(gno, setno)) {
	    sprintf(buf, "Set %d already dead", setno);
	    errmsg(buf);
	    return;
	} else {
	    if (soft) {
		killsetdata(gno, setno);
	    } else {
		killset(gno, setno);
	    }
#ifndef NONE_GUI
	    set_lists_dirty(TRUE);
	    update_set_status(gno, setno);
#endif
	}
    }
}

/*
 * sort sets, only works on sets of type XY
 */
void do_sort(int setno, int sorton, int stype)
{
    int i;

    if (setno == -1) {
	for (i = 0; i < g[get_cg()].maxplot; i++) {
	    if (is_set_active(get_cg(), i)) {
		sort_set(i, sorton, stype);
	    }
	}
    } else {
	if (!is_set_active(get_cg(), setno)) {
	    sprintf(buf, "Set %d not active", setno);
	    errmsg(buf);
	    return;
	} else {
	    sort_set(setno, sorton, stype);
	}
    }
}

void sort_set(int setno, int sorton, int stype)
{
    int up;

    up = getsetlength(get_cg(), setno);
    if (up < 2) {
	return;
    }
    sortset(get_cg(), setno, sorton, stype);
}

void set_hotlink(int gno, int setno, int onoroff, char *fname, int src)
{
    g[gno].p[setno].hotlink = onoroff;
    if (onoroff && fname != NULL) {
	strcpy(g[gno].p[setno].hotfile, fname);
	g[gno].p[setno].hotsrc = src;
    }
    set_dirtystate();
}

int is_hotlinked(int gno, int setno)
{
    if ( g[gno].p[setno].hotlink && strlen(g[gno].p[setno].hotfile) )
   		return g[gno].p[setno].hotlink;
	else 
		return 0;
}

char *get_hotlink_file(int gno, int setno)
{
    return g[gno].p[setno].hotfile;
}

int get_hotlink_src(int gno, int setno)
{
    return g[gno].p[setno].hotsrc;
}

void do_update_hotlink(int gno, int setno)
{
    read_set_fromfile(gno, setno, g[gno].p[setno].hotfile, 
			g[gno].p[setno].hotsrc,g[gno].p[setno].hotlink );
}

static int wp = 0;

void set_work_pending(int d)
{
    wp = d;
}

int work_pending(void)
{
    return wp;
}

/*
 * return a pointer to the array given by v
 */
double *getvptr(int gno, int setno, int v)
{
    switch (v) {
    case DATA_X:
	return g[gno].p[setno].ex[0];
	break;
    case DATA_Y:
	return g[gno].p[setno].ex[1];
	break;
    case DATA_Y1:
	return g[gno].p[setno].ex[2];
	break;
    case DATA_Y2:
	return g[gno].p[setno].ex[3];
	break;
    case DATA_Y3:
	return g[gno].p[setno].ex[4];
	break;
    case DATA_Y4:
	return g[gno].p[setno].ex[5];
	break;
    default:
	errmsg ("Internal error in function getvptr()");
	break;
    }
    return NULL;
}

double setybase(int gno, int setno)
{
    double ybase = 0.0;
    double xmin, xmax, ymin, ymax;
    
    getsetminmax(gno, setno, &xmin, &xmax, &ymin, &ymax);
    switch (g[gno].p[setno].baseline_type) {
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
        ybase = g[gno].w.yg1;
        break;
    case BASELINE_TYPE_GMAX:
        ybase = g[gno].w.yg2;
        break;
    default:
        errmsg("Wrong type of baseline");
    }
    
    return(ybase);
}

double *getcol(int gno, int setno, int col)
{
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].ex[col];
    } else {
        return NULL;
    }
}

char **get_set_strings(int gno, int setno)
{
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].s;
    } else {
        return NULL;
    }
}

int set_set_strings(int gno, int setno, int len, char **s)
{
    if (is_valid_setno(gno, setno) && len > 0 && s!= NULL) {
        g[gno].p[setno].s = s;
        g[gno].p[setno].len = len;
        set_dirtystate();
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int getsetlength(int gno, int setno)
{
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].len;
    } else {
        return -1;
    }
}

int setcomment(int gno, int setno, char *s)
{ 
    if (is_valid_setno(gno, setno) && s != NULL) {
        strncpy(g[gno].p[setno].comments, s, MAX_STRING_LENGTH - 1);
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

char *getcomment(int gno, int setno)
{ 
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].comments;
    } else {
        return NULL;
    }
}

int set_legend_string(int gno, int setno, char *s)
{ 
    if (is_valid_setno(gno, setno) && s != NULL) {
        strncpy(g[gno].p[setno].lstr, s, MAX_STRING_LENGTH - 1);
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

char *get_legend_string(int gno, int setno)
{ 
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].lstr;
    } else {
        return NULL;
    }
}

int set_dataset_type(int gno, int setno, int type)
{ 
    int i, len, ncols_old, ncols_new;
    
    if (is_valid_setno(gno, setno)) {
        len = getsetlength(gno, setno);
        ncols_old = dataset_cols(gno, setno);
        ncols_new = settype_cols(type);
        for (i = ncols_old; i < ncols_new; i++) {
            g[gno].p[setno].ex[i] = calloc(len, SIZEOF_DOUBLE);
        }
        for (i = ncols_new; i < ncols_old; i++) {
            cxfree(g[gno].p[setno].ex[i]);
        }
        g[gno].p[setno].type = type;
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

int dataset_type(int gno, int setno)
{ 
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].type;
    } else {
        return -1;
    }
}

int dataset_cols(int gno, int setno)
{
    return settype_cols(dataset_type(gno, setno));
}

int load_comments_to_legend(int gno, int setno)
{
    if (is_valid_setno(gno, setno)) {
        strcpy(g[gno].p[setno].lstr, g[gno].p[setno].comments);
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

void *get_ep_structure(int gno, int setno)
{
    if (is_valid_setno(gno, setno)) {
        return g[gno].p[setno].ep;
    } else {
        return NULL;
    }
}

int set_ep_structure(int gno, int setno, void *ep)
{
    if (is_valid_setno(gno, setno)) {
        g[gno].p[setno].ep = ep;
        return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}



static int dp = 0;

void set_lists_dirty(int d)
{
    dp = d;
}

int lists_dirty(void)
{
    return dp;
}
