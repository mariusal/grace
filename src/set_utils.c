/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 * routines to allocate, manipulate, and return
 * information about sets.
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "core_utils.h"
#include "ssdata.h"
#include "utils.h"
#include "files.h"
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

/*
 * free set data, but preserve the parameter settings
 */
void killsetdata(Quark *pset)
{
    set *p = set_get_data(pset);
    if (p) {
        dataset_empty(&p->ds);
        quark_dirtystate_set(pset, TRUE);
    }
}

/*
 * same as copyset(), but doesn't alter the to set appearance
 */
int copysetdata(Quark *psrc, Quark *pdest)
{
    Dataset *dsp = set_get_dataset(psrc);
    
    return set_set_dataset(pdest, dsp);
}

/*
 * kill a set
 */
void killset(Quark *pset)
{
    quark_free(pset);
}


void do_update_hotlink(Quark *pset)
{
    // update_set_from_file(pset);
}


/*
 * get the min/max fields of a set
 */
int getsetminmax(Quark **sets, int nsets, 
                    double *xmin, double *xmax, double *ymin, double *ymax)
{
    int i, first = TRUE;

    if (nsets < 1 || !sets) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nsets; i++) {
        Quark *pset = sets[i];
        if (set_is_drawable(pset)) {
            double x1, x2, y1, y2;
            set_get_minmax(pset, &x1, &x2, &y1, &y2);
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
        if (set_is_drawable(pset)) {
            
            if (ivec == 1) {
                bvec = getx(pset);
                vec  = gety(pset);
            } else {
                bvec = gety(pset);
                vec  = getx(pset);
            }
            
            n = set_get_length(pset);
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


int set_point(Quark *pset, int seti, const WPoint *wp)
{
    if (!pset) {
        return RETURN_FAILURE;
    }
    if (seti >= set_get_length(pset) || seti < 0) {
        return RETURN_FAILURE;
    }
    (set_get_col(pset, DATA_X))[seti] = wp->x;
    (set_get_col(pset, DATA_Y))[seti] = wp->y;
    quark_dirtystate_set(pset, TRUE);
    return RETURN_SUCCESS;
}

int get_point(Quark *pset, int seti, WPoint *wp)
{
    if (!pset) {
        return RETURN_FAILURE;
    }
    if (seti >= set_get_length(pset) || seti < 0) {
        return RETURN_FAILURE;
    }
    wp->x = (set_get_col(pset, DATA_X))[seti];
    wp->y = (set_get_col(pset, DATA_Y))[seti];
    return RETURN_SUCCESS;
}

int set_point_shift(Quark *pset, int seti, const VVector *vshift)
{
    WPoint wp;
    VPoint vp;
    
    if (get_point(pset, seti, &wp) == RETURN_SUCCESS &&
        Wpoint2Vpoint(pset, &wp, &vp) == RETURN_SUCCESS) {
        vp.x += vshift->x;
        vp.y += vshift->y;
        Vpoint2Wpoint(pset, &vp, &wp);
        return set_point(pset, seti, &wp);
    } else {
        return RETURN_FAILURE;
    }
}

void copycol2(Quark *psrc, Quark *pdest, int col)
{
    int i, n1, n2;
    double *x1, *x2;

    if (!psrc || !pdest) {
        return;
    }
    n1 = set_get_length(psrc);
    n2 = set_get_length(pdest);
    if (n1 != n2) {
        return;
    }
    x1 = set_get_col(psrc, col);
    x2 = set_get_col(pdest, col);
    for (i = 0; i < n1; i++) {
	x2[i] = x1[i];
    }
    quark_dirtystate_set(pdest, TRUE);
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
    
    len = set_get_length(pset);
    
    if (dist == len) {
        killsetdata(pset);
        return;
    }
    
    ncols = set_get_ncols(pset);
    for (j = 0; j < ncols; j++) {
	x = set_get_col(pset, j);
	for (i = endno + 1; i < len; i++) {
	    x[i - dist] = x[i];
	}
    }
    if ((s = set_get_strings(pset)) != NULL) {
	for (i = endno + 1; i < len; i++) {
	    s[i - dist] = copy_string(s[i - dist], s[i]);
	}
    }
    set_set_length(pset, len - dist);
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
    ncols = set_get_ncols(pset_final);
    for (i = 0; i < nsets; i++) {
        pset = sets[i];
        if (!pset) {
            errmsg("Invalid pset in the list");
            return RETURN_FAILURE;
        }
        if (set_get_ncols(pset) != ncols) {
            errmsg("Can't join datasets with different number of cols");
            return RETURN_FAILURE;
        }
    }
    
    new_length = set_get_length(pset_final);
    for (i = 1; i < nsets; i++) {
        pset = sets[i];
        old_length = new_length;
        new_length += set_get_length(pset);
        if (set_set_length(pset_final, new_length) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
        for (j = 0; j < ncols; j++) {
            x1 = set_get_col(pset_final, j);
            x2 = set_get_col(pset, j);
            for (n = old_length; n < new_length; n++) {
                x1[n] = x2[n - old_length];
            }
        }
        s1 = set_get_strings(pset_final);
        s2 = set_get_strings(pset);
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
    n = set_get_length(pset);
    ncols = set_get_ncols(pset);
    for (k = 0; k < ncols; k++) {
	x = set_get_col(pset, k);
	for (i = 0; i < n / 2; i++) {
	    j = (n - 1) - i;
	    fswap(&x[i], &x[j]);
	}
    }
    if ((s = set_get_strings(pset)) != NULL) {
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
    vptr = set_get_col(pset, sorton);
    if (vptr == NULL) {
	errmsg("NULL vector in sort, operation cancelled, check set type");
	return;
    }

    len = set_get_length(pset);
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
    
    s = set_get_strings(pset);
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
    
    nc = set_get_ncols(pset);
    /* loop over the number of columns */
    for (j = 0; j < nc; j++) {
        /* get this vector and put into the temporary vector in the right order */
	x = set_get_col(pset, j);
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
	 len = set_get_length(pset);
	 set_set_length(pset, len + 1);
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

/*
 * add a point to setno at ind
 */
int add_point_at(Quark *pset, int ind, const Datapoint *dpoint)
{
    int len, col, ncols;
    double *ex;
    char **s;

    if (pset) {
        len = set_get_length(pset);
        if (ind < 0 || ind > len) {
            return RETURN_FAILURE;
        }
        len++;
        set_set_length(pset, len);
        ncols = set_get_ncols(pset);
        for (col = 0; col < ncols; col++) {
            ex = set_get_col(pset, col);
            if (ind < len - 1) {
                memmove(ex + ind + 1, ex + ind, (len - ind - 1)*SIZEOF_DOUBLE);
            }
            ex[ind] = dpoint->ex[col];
        }
        s = set_get_strings(pset);
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
    
    n = set_get_length(pset);
    if (ind < 0 || ind >= n) {
        return RETURN_FAILURE;
    } else {
        *ncols = set_get_ncols(pset);
        for (col = 0; col < *ncols; col++) {
            ex = set_get_col(pset, col);
            dpoint->ex[col] = ex[ind];
        }
        s = set_get_strings(pset);
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
    int ncols = set_get_ncols(pset);

    if (!pset) {
        return;
    }
    
    for (i = 0; i < set_get_length(pset); i++) {
	if (ind[i]) {
	    cnt++;
	}
    }
    if (cnt == set_get_length(pset)) {
	killset(pset);
	return;
    }
    cnt = 0;
    for (i = 0; i < set_get_length(pset); i++) {
	if (ind[i] == 0) {
	    for (j = 0; j < ncols; j++) {
                (set_get_col(pset, j))[cnt] = (set_get_col(pset, j))[i];
	    }
	    cnt++;
	}
    }
    set_set_length(pset, cnt);
}

/*
 * split a set into lpart length sets
 */
int do_splitsets(Quark *pset, int lpart)
{
#if 0
    int i, j, k, ncols, len, plen, npsets;
    double *x;
    char s[256];
    Quark *gr, *ptmp;
    Dataset *dsp, *dsptmp;

    if ((len = set_get_length(pset)) < 2) {
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
    ncols = set_get_ncols(pset);

    gr = get_parent_graph(pset);
    dsp = set_get_dataset(pset);

    /* now load each set */
    for (i = 0; i < npsets; i++) {
	plen = MIN2(lpart, len - i*lpart); 
        ptmp = grace_set_new(gr);
        if (!ptmp) {
            errmsg("Can't create new set");
            return RETURN_FAILURE;
        }

        dsptmp = set_get_dataset(ptmp);
        
        /* set the plot parameters */
        copy_set_params(pset, ptmp);

	if (set_set_length(ptmp, plen) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
        if (dsp->s) {
            dataset_enable_scol(dsptmp, TRUE);
        }
        
        /* load the data into each column */
	for (k = 0; k < ncols; k++) {
	    x = set_get_col(ptmp, k);
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
	set_set_comment(ptmp, s);
    }
    
    /* kill the original set */
    killset(pset);
#endif    
    return RETURN_SUCCESS;
}

/*
 * drop points from an active set
 */
void do_drop_points(Quark *pset, int startno, int endno)
{
    int set_set_length;

    if (!pset) {
	errmsg("Set not active");
	return;
    }

    set_set_length = set_get_length(pset);
    if (startno < 0) {
        startno = set_set_length + 1 + startno;
    }
    if (endno < 0) {
        endno = set_set_length + 1 + endno;
    }

    if (startno > endno) {
        iswap(&startno, &endno);
    }

    if (startno < 0) {
	errmsg("Start # < 0");
	return;
    }
    if (endno >= set_set_length) {
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
    if (set_is_dataless(pset)) {
	errmsg("Set not active");
	return;
    } else {
	sortset(pset, sorton, stype);
    }
}


int load_comments_to_legend(Quark *pset)
{
    return set_set_legstr(pset, set_get_comment(pset));
}

int set_set_colors(Quark *pset, unsigned int color)
{
    set *p = set_get_data(pset);
    RunTime *rt = rt_from_quark(pset);
    if (!p || !rt) {
        return RETURN_FAILURE;
    }
    
    if (color < number_of_colors(rt->canvas)) {
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

static int set_delete_cb(Quark *q, int etype, void *data)
{
#ifndef NONE_GUI
    if (etype == QUARK_ETYPE_DELETE) {
        close_ss_editors(q);
    }
#endif
    return RETURN_SUCCESS;
}


Quark *grace_set_new(Quark *gr)
{
    Quark *pset = set_new(gr);
    RunTime *rt = rt_from_quark(pset);
    if (!pset || !rt) {
        return NULL;
    }
    
    rt->setcolor++;
    rt->setcolor %= number_of_colors(rt->canvas);
    if (rt->setcolor == 0) {
        rt->setcolor = 1;
    }
    set_set_colors(pset, rt->setcolor);

    quark_cb_set(pset, set_delete_cb, NULL);
    
    return pset;
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
