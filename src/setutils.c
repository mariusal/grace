/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
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
 * information about sets.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ssdata.h"
#include "utils.h"
#include "grace/canvas.h"
#include "files.h"
#include "core_utils.h"
#include "protos.h"

/*
 * return the string version of the dataset column
 */
char *dataset_colname(int col)
{
    char *s;

    switch (col) {
    case 0:
	s = "X";
	break;
    case 1:
	s = "Y";
	break;
    case 2:
	s = "Y1";
	break;
    case 3:
	s = "Y2";
	break;
    case 4:
	s = "Y3";
	break;
    case 5:
	s = "Y4";
	break;
    default:
	s = "?";
	errmsg("Internal error in dataset_colname()");
        break;
    }
    return s;
}

int dataset_set_nrows(Dataset *data, int len)
{
    int i, j, oldlen;
    
    if (!data || len < 0) {
	return RETURN_FAILURE;
    }
    
    oldlen = data->len;
    if (len == oldlen) {
	return RETURN_SUCCESS;
    }
    
    for (i = 0; i < data->ncols; i++) {
	if ((data->ex[i] = xrealloc(data->ex[i], len*SIZEOF_DOUBLE)) == NULL
            && len != 0) {
	    return RETURN_FAILURE;
	}
        for (j = oldlen; j < len; j++) {
            data->ex[i][j] = 0.0;
        }
    }
    
    if (data->s != NULL) {
        for (i = len; i < oldlen; i++) {
            xfree(data->s[i]);
        }
        data->s = xrealloc(data->s, len*sizeof(char *));
        for (j = oldlen; j < len; j++) {
            data->s[j] = copy_string(NULL, "");
        }
    }
    
    data->len = len;

    return RETURN_SUCCESS;
}

int dataset_set_ncols(Dataset *data, int ncols)
{
    if (ncols < 0 || ncols > MAX_SET_COLS) {
        return RETURN_FAILURE;
    }
    
    if (data->ncols == ncols) {
        /* nothing changed */
        return RETURN_SUCCESS;
    } else {
        int i, ncols_old = data->ncols;
        
        for (i = ncols_old; i < ncols; i++) {
            data->ex[i] = xcalloc(data->len, SIZEOF_DOUBLE);
        }
        for (i = ncols; i < ncols_old; i++) {
            XCFREE(data->ex[i]);
        }

        data->ncols = ncols;
        
        return RETURN_SUCCESS;
    }
}

int set_dataset_scol(Dataset *data, int yesno)
{
    if (yesno) {
        if (data->s) {
            return RETURN_SUCCESS;
        } else {
            data->s = xcalloc(data->len, sizeof(char *));
            if (data->len && !data->s) {
                return RETURN_FAILURE;
            } else {
                return RETURN_SUCCESS;
            }
        }
    } else {
        if (data->s) {
            int i;
            for (i = 0; i < data->len; i++) {
                xfree(data->s[i]);
            }
            xfree(data->s);
        }
        return RETURN_SUCCESS;
    }
}


int zero_set_data(Dataset *dsp)
{
    int k;
    
    if (dsp) {
        dsp->len = 0;
        for (k = 0; k < dsp->ncols; k++) {
	    dsp->ex[k] = NULL;
        }
        dsp->s = NULL;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int copy_set_params(Quark *src, Quark *dest)
{
    if (!src || !dest) {
        return RETURN_FAILURE;
    } else {
        Dataset *data;
        int type;
        char *legstr;
        set *p1, *p2;
        
        p1 = set_get_data(src);
        p2 = set_get_data(dest);
        
        /* preserve allocatables and related stuff */
        type    = p2->type;
        data    = p2->data;
        legstr  = p2->legstr;
        
        memcpy(p2, p1, sizeof(set));
        
        p2->type    = type;
        p2->data    = data;
        p2->legstr  = legstr;
        
        return RETURN_SUCCESS;
    }
}

/*
 * free set data, but preserve the parameter settings
 */
void killsetdata(Quark *pset)
{
    set *p = set_get_data(pset);
    if (p) {
        dataset_empty(p->data);
        quark_dirtystate_set(pset, TRUE);
    }
}

/*
 * (re)allocate data arrays for a set of length len.
 */
int setlength(Quark *pset, int len)
{
    set *p = set_get_data(pset);

    if (!p) {
        return RETURN_FAILURE;
    }
    
    quark_dirtystate_set(pset, TRUE);
    
    return dataset_set_nrows(p->data, len);
}

/*
 * same as copyset(), but doesn't alter the to set appearance
 */
int copysetdata(Quark *psrc, Quark *pdest)
{
    set *p1, *p2;
    
    p1 = set_get_data(psrc);
    p2 = set_get_data(pdest);
    
    if (!p1 || !p2 || p1 == p2) {
	return RETURN_FAILURE;
    }
    
    dataset_free(p2->data);
    p2->data = dataset_copy(p1->data);
    
    if (p2->data) {
        if (set_get_dataset_ncols(pdest) != set_get_dataset_ncols(psrc)) {
            p2->type = p1->type;
        }
        quark_dirtystate_set(pdest, TRUE);
	return RETURN_SUCCESS;
    } else {
	return RETURN_FAILURE;
    }
}

/*
 * kill a set
 */
void killset(Quark *pset)
{
    quark_free(pset);
}

double *getcol(Quark *pset, int col)
{
    if (pset) {
        set *p = set_get_data(pset);
        return p->data->ex[col];
    } else {
        return NULL;
    }
}

void setcol(Quark *pset, int col, double *x, int len)
{
    if (pset) {
        set *p = set_get_data(pset);
        p->data->ex[col] = x;
        p->data->len = len;
        quark_dirtystate_set(pset, TRUE);
    }
}

char **get_set_strings(Quark *pset)
{
    if (pset) {
        set *p = set_get_data(pset);
        return p->data->s;
    } else {
        return NULL;
    }
}

int set_set_strings(Quark *pset, int len, char **s)
{
    if (pset && len > 0 && s!= NULL) {
        set *p = set_get_data(pset);
        p->data->s = s;
        p->data->len = len;
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int getsetlength(Quark *pset)
{
    if (pset) {
        set *p = set_get_data(pset);
        return p->data->len;
    } else {
        return -1;
    }
}

int setcomment(Quark *pset, char *s)
{ 
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        dsp->comment = copy_string(dsp->comment, s);
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *getcomment(Quark *pset)
{ 
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        return dsp->comment;
    } else {
        return NULL;
    }
}

int set_dataset_type(Quark *pset, int type)
{ 
    int ncols_new = settype_cols(type);
    set *p;
    
    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    if (dataset_set_ncols(p->data, ncols_new) == RETURN_SUCCESS) {
        p->type = type;
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int dataset_type(Quark *pset)
{ 
    if (pset) {
        set *p = set_get_data(pset);
        return p->type;
    } else {
        return -1;
    }
}


void do_update_hotlink(Quark *pset)
{
    update_set_from_file(pset);
}


/*
 * get the min/max fields of a set
 */
int getsetminmax(Quark **sets, int nsets, 
                    double *xmin, double *xmax, double *ymin, double *ymax)
{
    double *x, *y;
    int len;
    double x1, x2, y1, y2;
    int i, first = TRUE;
    int imin, imax; /* dummy */

    if (nsets < 1 || !sets) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nsets; i++) {
        Quark *pset = sets[i];
        if (is_set_drawable(pset)) {
            x = getcol(pset, DATA_X);
            y = getcol(pset, DATA_Y);
            len = getsetlength(pset);
            minmax(x, len, &x1, &x2, &imin, &imax);
            minmax(y, len, &y1, &y2, &imin, &imax);
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
    
    if (first == FALSE) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

/*
 * get the min/max fields of a set with fixed x/y range
 */
int getsetminmax_c(Quark **sets, int nsets, 
            double *xmin, double *xmax, double *ymin, double *ymax, int ivec)
{
    double vmin_t, vmax_t, *vmin, *vmax, bvmin, bvmax, *vec, *bvec;
    int i, n;
    int first = TRUE, hits;

    if (nsets < 1 || !sets) {
        return RETURN_FAILURE;
    }

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
    
    for (i = 0; i < nsets; i++) {
        Quark *pset = sets[i];
        if (is_set_drawable(pset)) {
            
            if (ivec == 1) {
                bvec = getx(pset);
                vec  = gety(pset);
            } else {
                bvec = gety(pset);
                vec  = getx(pset);
            }
            
            n = getsetlength(pset);
            hits = minmaxrange(bvec, vec, n, bvmin, bvmax, &vmin_t, &vmax_t);
            if (hits == RETURN_SUCCESS) {
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
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


/*
 * compute the mins and maxes of a vector x
 */
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax)
{
    int i;
    
    *imin = 0;
    *imax = 0;

    if (x == NULL) {
        *xmin = 0.0;
        *xmax = 0.0;
        return;
    }
    
    *xmin = x[0];
    *xmax = x[0];
    
    for (i = 1; i < n; i++) {
	if (x[i] < *xmin) {
	    *xmin = x[i];
	    *imin = i;
	}
	if (x[i] > *xmax) {
	    *xmax = x[i];
	    *imax = i;
	}
    }
}


/*
 * compute the min and max of vector vec calculated for indices such that
 * bvec values lie within [bmin, bmax] range
 * returns RETURN_FAILURE if none found
 */
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax)
{
    int i, first = TRUE;
    
    if ((vec == NULL) || (bvec == NULL)) {
        return RETURN_FAILURE;
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
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
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

static int dbl_comp(const void *a, const void *b)
{
    if (*(double *)a < *(double *)b) {
        return -1;
    } else if (*(double *)a > *(double *)b) {
        return 1;
    } else {
        return 0;
    }
}

/* get the median of a vector */
int vmedian(double *x, int n, double *med)
{
    double *d;

    if (n < 1) {
        return RETURN_FAILURE;
    } else if (n == 1) {
        *med = x[0];
    } else {
        if (monotonicity(x, n, FALSE) == 0) {
            d = copy_data_column(x, n);
            if (!d) {
                return RETURN_FAILURE;
            }
            qsort(d, n, SIZEOF_DOUBLE, dbl_comp);
        } else {
            d = x;
        }
        if (n % 2) {
            /* odd length */
            *med = d[(n + 1)/2 - 1];
        } else {
            *med = (d[n/2 - 1] + d[n/2])/2;
        }

        if (d != x) {
            xfree(d);
        }
    }
    
    return RETURN_SUCCESS;
}


int set_point(Quark *pset, int seti, const WPoint *wp)
{
    if (!pset) {
        return RETURN_FAILURE;
    }
    if (seti >= getsetlength(pset) || seti < 0) {
        return RETURN_FAILURE;
    }
    (getcol(pset, DATA_X))[seti] = wp->x;
    (getcol(pset, DATA_Y))[seti] = wp->y;
    quark_dirtystate_set(pset, TRUE);
    return RETURN_SUCCESS;
}

int get_point(Quark *pset, int seti, WPoint *wp)
{
    if (!pset) {
        return RETURN_FAILURE;
    }
    if (seti >= getsetlength(pset) || seti < 0) {
        return RETURN_FAILURE;
    }
    wp->x = (getcol(pset, DATA_X))[seti];
    wp->y = (getcol(pset, DATA_Y))[seti];
    return RETURN_SUCCESS;
}

void copycol2(Quark *psrc, Quark *pdest, int col)
{
    int i, n1, n2;
    double *x1, *x2;

    if (!psrc || !pdest) {
        return;
    }
    n1 = getsetlength(psrc);
    n2 = getsetlength(pdest);
    if (n1 != n2) {
        return;
    }
    x1 = getcol(psrc, col);
    x2 = getcol(pdest, col);
    for (i = 0; i < n1; i++) {
	x2[i] = x1[i];
    }
    quark_dirtystate_set(pdest, TRUE);
}

int is_set_dataless(Quark *pset)
{
    if (getsetlength(pset) > 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*
 * return number of active set(s) in gno
 */
int number_of_active_sets(Quark *gr)
{
    int i, nsets, na = 0;
    Quark **psets;

    nsets = get_descendant_sets(gr, &psets);
    for (i = 0; i < nsets; i++) {
        Quark *pset = psets[i];
        if (is_set_drawable(pset) == TRUE) {
	    na++;
	}
    }
    xfree(psets);
    return na;
}

/*
 * drop points from a set
 */
void droppoints(Quark *pset, int startno, int endno)
{
    double *x;
    char **s;
    int i, j, len, ncols, dist;

    if (!pset) {
        return;
    }

    dist = endno - startno + 1;
    if (dist <= 0) {
        return;
    }
    
    len = getsetlength(pset);
    
    if (dist == len) {
        killsetdata(pset);
        return;
    }
    
    ncols = set_get_dataset_ncols(pset);
    for (j = 0; j < ncols; j++) {
	x = getcol(pset, j);
	for (i = endno + 1; i < len; i++) {
	    x[i - dist] = x[i];
	}
    }
    if ((s = get_set_strings(pset)) != NULL) {
	for (i = endno + 1; i < len; i++) {
	    s[i - dist] = copy_string(s[i - dist], s[i]);
	}
    }
    setlength(pset, len - dist);
}

/*
 * join several sets together; all but the first set in the list will be killed 
 */
int join_sets(Quark **sets, int nsets)
{
    int i, j, n, ncols, old_length, new_length;
    Quark *pset, *pset_final;
    double *x1, *x2;
    char **s1, **s2;

    if (nsets < 2) {
        errmsg("nsets < 2");
        return RETURN_FAILURE;
    }
    
    pset_final = sets[0];
    ncols = set_get_dataset_ncols(pset_final);
    for (i = 0; i < nsets; i++) {
        pset = sets[i];
        if (!pset) {
            errmsg("Invalid pset in the list");
            return RETURN_FAILURE;
        }
        if (set_get_dataset_ncols(pset) != ncols) {
            errmsg("Can't join datasets with different number of cols");
            return RETURN_FAILURE;
        }
    }
    
    new_length = getsetlength(pset_final);
    for (i = 1; i < nsets; i++) {
        pset = sets[i];
        old_length = new_length;
        new_length += getsetlength(pset);
        if (setlength(pset_final, new_length) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
        for (j = 0; j < ncols; j++) {
            x1 = getcol(pset_final, j);
            x2 = getcol(pset, j);
            for (n = old_length; n < new_length; n++) {
                x1[n] = x2[n - old_length];
            }
        }
        s1 = get_set_strings(pset_final);
        s2 = get_set_strings(pset);
        if (s1 != NULL && s2 != NULL) {
            for (n = old_length; n < new_length; n++) {
                s1[n] = copy_string(s1[n], s2[n - old_length]);
            }
        }
        killset(pset);
    }
    
    return RETURN_SUCCESS;
}

void reverse_set(Quark *pset)
{
    int n, i, j, k, ncols;
    double *x;
    char **s;

    if (!pset) {
	return;
    }
    n = getsetlength(pset);
    ncols = set_get_dataset_ncols(pset);
    for (k = 0; k < ncols; k++) {
	x = getcol(pset, k);
	for (i = 0; i < n / 2; i++) {
	    j = (n - 1) - i;
	    fswap(&x[i], &x[j]);
	}
    }
    if ((s = get_set_strings(pset)) != NULL) {
	char *stmp;
        for (i = 0; i < n / 2; i++) {
	    j = (n - 1) - i;
	    stmp = s[i];
            s[i] = s[j];
            s[j] = stmp;
	}
    }
    quark_dirtystate_set(pset, TRUE);
}
/*
 * sort a set
 */
static double *vptr;

/*
 * for ascending and descending sorts
 */
 
static int compare_points1(const void *p1, const void *p2)
{
    const int *i1, *i2;
    double a, b;
    i1 = (const int *)p1;
    i2 = (const int *)p2;
    a = vptr[*i1];
    b = vptr[*i2];
    if (a < b) {
	return -1;
    }
    if (a > b) {
	return 1;
    }
    return 0;
}

static int compare_points2(const void *p1, const void *p2)
{
    const int *i1, *i2;
    double a, b;
    i1 = (const int *)p1;
    i2 = (const int *)p2;
    a = vptr[*i1];
    b = vptr[*i2];
    if (a > b) {
	return -1;
    }
    if (a < b) {
	return 1;
    }
    return 0;
}

void sortset(Quark *pset, int sorton, int stype)
{
    int i, j, nc, len, *ind;
    double *x, *xtmp;
    char **s, **stmp;

    /* get the vector to sort on */
    vptr = getcol(pset, sorton);
    if (vptr == NULL) {
	errmsg("NULL vector in sort, operation cancelled, check set type");
	return;
    }

    len = getsetlength(pset);
    if (len <= 1) {
	return;
    }
    
    /* allocate memory for permuted indices */
    ind = xmalloc(len*SIZEOF_INT);
    if (ind == NULL) {
	return;
    }
    /* allocate memory for temporary array */
    xtmp = xmalloc(len*SIZEOF_DOUBLE);
    if (xtmp == NULL) {
	xfree(ind);
	return;
    }
    
    s = get_set_strings(pset);
    if (s != NULL) {
        stmp = xmalloc(len*sizeof(char *));
        if (stmp == NULL) {
	    xfree(xtmp);
	    xfree(ind);
        }
    } else {
        stmp = NULL;
    }
    
    /* initialize indices */
    for (i = 0; i < len; i++) {
	ind[i] = i;
    }

    /* sort */
    qsort(ind, len, SIZEOF_INT, stype ? compare_points2 : compare_points1);

    /* straighten things out - done one vector at a time for storage */
    
    nc = set_get_dataset_ncols(pset);
    /* loop over the number of columns */
    for (j = 0; j < nc; j++) {
        /* get this vector and put into the temporary vector in the right order */
	x = getcol(pset, j);
	for (i = 0; i < len; i++) {
	    xtmp[i] = x[ind[i]];
	}
        
        /* load it back to the set */
	for (i = 0; i < len; i++) {
	    x[i] = xtmp[i];
	}
    }
    
    /* same with strings, if any */
    if (s != NULL) {
	for (i = 0; i < len; i++) {
	    stmp[i] = s[ind[i]];
	}

	for (i = 0; i < len; i++) {
	    s[i] = stmp[i];
	}
    }
    
    /* free allocated temporary arrays */
    xfree(stmp);
    xfree(xtmp);
    xfree(ind);

    quark_dirtystate_set(pset, TRUE);
}

/*
 * delete the point pt in setno
 */
void del_point(Quark *pset, int pt)
{
    droppoints(pset, pt, pt);
}

/*
 * add a point to setno
 */
void add_point(Quark *pset, double px, double py)
{
    int len;
    double *x, *y;

    if (pset) {
	 len = getsetlength(pset);
	 setlength(pset, len + 1);
	 x = getx(pset);
	 y = gety(pset);
	 x[len] = px;
	 y[len] = py;
    }
}

void zero_datapoint(Datapoint *dpoint)
{
    int k;
    
    for (k = 0; k < MAX_SET_COLS; k++) {
        dpoint->ex[k] = 0.0;
    }
    dpoint->s = NULL;
}

Datapoint *datapoint_new(void)
{
    Datapoint *dpoint;
    dpoint = xmalloc(sizeof(Datapoint));
    zero_datapoint(dpoint);
    
    return dpoint;
}

void datapoint_free(Datapoint *dpoint)
{
    xfree(dpoint->s);
    xfree(dpoint);
}

int dataset_set_datapoint(Dataset *dsp, const Datapoint *dpoint, int ind)
{
    int col;
    
    if (!dsp) {
        return RETURN_FAILURE;
    }
    
    if (ind < 0 || ind >= dsp->len) {
        return RETURN_FAILURE;
    }
    for (col = 0; col < dsp->ncols; col++) {
        dsp->ex[col][ind] = dpoint->ex[col];
    }
    if (dpoint->s) {
        set_dataset_scol(dsp, TRUE);
    }
    if (dsp->s) {
        dsp->s[ind] = copy_string(dsp->s[ind], dpoint->s);
    }
    return RETURN_SUCCESS;
}

/*
 * add a point to setno at ind
 */
int add_point_at(Quark *pset, int ind, const Datapoint *dpoint)
{
    int len, col, ncols;
    double *ex;
    char **s;

    if (pset) {
        len = getsetlength(pset);
        if (ind < 0 || ind > len) {
            return RETURN_FAILURE;
        }
        len++;
        setlength(pset, len);
        ncols = set_get_dataset_ncols(pset);
        for (col = 0; col < ncols; col++) {
            ex = getcol(pset, col);
            if (ind < len - 1) {
                memmove(ex + ind + 1, ex + ind, (len - ind - 1)*SIZEOF_DOUBLE);
            }
            ex[ind] = dpoint->ex[col];
        }
        s = get_set_strings(pset);
        if (s != NULL) {
            if (ind < len - 1) {
                memmove(s + ind + 1, s + ind, (len - ind - 1)*sizeof(char *));
            }
            s[ind] = copy_string(NULL, dpoint->s);
        }
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_datapoint(Quark *pset, int ind, int *ncols, Datapoint *dpoint)
{
    int n, col;
    double *ex;
    char **s;
    
    n = getsetlength(pset);
    if (ind < 0 || ind >= n) {
        return RETURN_FAILURE;
    } else {
        *ncols = set_get_dataset_ncols(pset);
        for (col = 0; col < *ncols; col++) {
            ex = getcol(pset, col);
            dpoint->ex[col] = ex[ind];
        }
        s = get_set_strings(pset);
        if (s != NULL) {
            dpoint->s = s[ind];
        } else {
            dpoint->s = NULL;
        }
        return RETURN_SUCCESS;
    }
}

void delete_byindex(Quark *pset, int *ind)
{
    int i, j, cnt = 0;
    int ncols = set_get_dataset_ncols(pset);

    if (!pset) {
        return;
    }
    
    for (i = 0; i < getsetlength(pset); i++) {
	if (ind[i]) {
	    cnt++;
	}
    }
    if (cnt == getsetlength(pset)) {
	killset(pset);
	return;
    }
    cnt = 0;
    for (i = 0; i < getsetlength(pset); i++) {
	if (ind[i] == 0) {
	    for (j = 0; j < ncols; j++) {
                (getcol(pset, j))[cnt] = (getcol(pset, j))[i];
	    }
	    cnt++;
	}
    }
    setlength(pset, cnt);
}

/*
 * split a set into lpart length sets
 */
int do_splitsets(Quark *pset, int lpart)
{
    int i, j, k, ncols, len, plen, npsets;
    double *x;
    char s[256];
    Quark *gr, *ptmp;
    Dataset *dsp, *dsptmp;

    if ((len = getsetlength(pset)) < 2) {
	errmsg("Set length < 2");
	return RETURN_FAILURE;
    }
    if (lpart >= len) {
	errmsg("Split length >= set length");
	return RETURN_FAILURE;
    }
    if (lpart <= 0) {
	errmsg("Split length <= 0");
	return RETURN_FAILURE;
    }

    npsets = (len - 1)/lpart + 1;

    /* get number of columns in this set */
    ncols = set_get_dataset_ncols(pset);

    gr = get_parent_graph(pset);
    dsp = set_get_dataset(pset);

    /* now load each set */
    for (i = 0; i < npsets; i++) {
	plen = MIN2(lpart, len - i*lpart); 
        ptmp = set_new(gr);
        if (!ptmp) {
            errmsg("Can't create new set");
            return RETURN_FAILURE;
        }

        dsptmp = set_get_dataset(ptmp);
        
        /* set the plot parameters */
        copy_set_params(pset, ptmp);

	if (setlength(ptmp, plen) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
        if (dsp->s) {
            dsptmp->s = xmalloc(plen*sizeof(char *));
        }
        
        /* load the data into each column */
	for (k = 0; k < ncols; k++) {
	    x = getcol(ptmp, k);
	    for (j = 0; j < plen; j++) {
		x[j] = dsp->ex[k][i*lpart + j];
	    }
	}
        if (dsp->s) {
	    for (j = 0; j < plen; j++) {
		dsptmp->s[j] =
                    copy_string(NULL, dsp->s[i*lpart + j]);
	    }
        }
	
        sprintf(s, "partition %d of set %s", i + 1, quark_idstr_get(pset));
	setcomment(ptmp, s);
    }
    
    /* kill the original set */
    killset(pset);
    
    return RETURN_SUCCESS;
}

/*
 * drop points from an active set
 */
void do_drop_points(Quark *pset, int startno, int endno)
{
    int setlength;

    if (!pset) {
	errmsg("Set not active");
	return;
    }

    setlength = getsetlength(pset);
    if (startno < 0) {
        startno = setlength + 1 + startno;
    }
    if (endno < 0) {
        endno = setlength + 1 + endno;
    }

    if (startno > endno) {
        iswap(&startno, &endno);
    }

    if (startno < 0) {
	errmsg("Start # < 0");
	return;
    }
    if (endno >= setlength) {
	errmsg("Ending # >= set length");
	return;
    }

    droppoints(pset, startno, endno);
}


/*
 * sort sets
 */
void do_sort(Quark *pset, int sorton, int stype)
{
    if (is_set_dataless(pset)) {
	errmsg("Set not active");
	return;
    } else {
	sortset(pset, sorton, stype);
    }
}


double setybase(Quark *pset)
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
    
    getsetminmax(&pset, 1, &xmin, &xmax, &ymin, &ymax);

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


int load_comments_to_legend(Quark *pset)
{
    return set_set_legstr(pset, getcomment(pset));
}

int filter_set(Quark *pset, char *rarray)
{
    int i, ip, j, ncols;
    Dataset *dsp;
    
    if (!pset) {
        return RETURN_FAILURE;
    }
    if (rarray == NULL) {
        return RETURN_SUCCESS;
    }
    ncols = set_get_dataset_ncols(pset);
    dsp = set_get_dataset(pset);
    ip = 0;
    for (i = 0; i < dsp->len; i++) {
        if (rarray[i]) {
            for (j = 0; j < ncols; j++) {
                dsp->ex[j][ip] = dsp->ex[j][i];
            }
            if (dsp->s != NULL) {
                dsp->s[ip] = copy_string(dsp->s[ip], dsp->s[i]);
            }
            ip++;
        }
    }
    setlength(pset, ip);
    return RETURN_SUCCESS;
}

int set_set_colors(Quark *pset, int color)
{
    set *p = set_get_data(pset);
    RunTime *rt = rt_from_quark(pset);
    if (!p || !rt) {
        return RETURN_FAILURE;
    }
    
    if (color < number_of_colors(rt->canvas) && color >= 0) {
        p->line.line.pen.color    = color;
        p->sym.line.pen.color = color;
        p->sym.fillpen.color  = color;
        p->errbar.pen.color  = color;

        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

Symbol *symbol_new()
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

SetLine *setline_new()
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
