/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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

#include "globals.h"
#include "ssdata.h"
#include "storage.h"
#include "utils.h"
#include "grace/canvas.h"
#include "files.h"
#include "graphs.h"
#include "protos.h"

#define graphs grace->project->graphs
#define grdefaults grace->rt->grdefaults

int settype_cols(int type)
{
    int ncols;
    
    switch (type) {
    case SET_XY:
    case SET_BAR:
	ncols = 2;
	break;
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
    case SET_BARDY:
    case SET_XYR:
    case SET_XYCOLOR:
    case SET_XYSIZE:
	ncols = 3;
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_BARDYDY:
    case SET_XYCOLPAT:
    case SET_XYVMAP:
	ncols = 4;
	break;
    case SET_XYHILO:
	ncols = 5;
	break;
    case SET_XYDXDXDYDY:
    case SET_BOXPLOT:
	ncols = 6;
	break;
    default:
        ncols = 0;
        break;
    }
    
    return ncols;
}

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

Dataset *dataset_new(void)
{
    Dataset *dsp;
    int k;
    
    dsp = xmalloc(sizeof(Dataset));
    dsp->len   = 0;
    dsp->ncols = 0;
    for (k = 0; k < MAX_SET_COLS; k++) {
        dsp->ex[k] = NULL;
    }
    dsp->s = NULL;
    
    dsp->comment = NULL;
    dsp->hotlink = FALSE;
    dsp->hotsrc  = SOURCE_DISK;
    dsp->hotfile = NULL;
    
    return dsp;
}

int set_dataset_nrows(Dataset *data, int len)
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

int set_dataset_ncols(Dataset *data, int ncols)
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

/*
 * free dataset columns
 */
int dataset_empty(Dataset *dsp)
{
    int k;
    
    if (dsp) {
        if (dsp->len) {
            for (k = 0; k < dsp->ncols; k++) {
	        XCFREE(dsp->ex[k]);
            }
            if (dsp->s) {
	        for (k = 0; k < dsp->len; k++) {
		    XCFREE(dsp->s[k]);
	        }
                XCFREE(dsp->s);
            }
            dsp->len = 0;
	    set_dirtystate();
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void dataset_free(Dataset *dsp)
{
    if (dsp) {
        dataset_empty(dsp);
        xfree(dsp->hotfile);
        xfree(dsp->comment);
        xfree(dsp);
    }
}

Dataset *dataset_copy(Dataset *data)
{
    Dataset *data_new;
    int k;

    if (!data) {
        return NULL;
    }
    
    data_new = dataset_new();
    if (!data_new) {
        return NULL;
    }
    
    data_new->len   = data->len;
    data_new->ncols = data->ncols;
    
    for (k = 0; k < data->ncols; k++) {
        data_new->ex[k] = copy_data_column(data->ex[k], data->len);
        if (!data_new->ex[k]) {
            dataset_free(data_new);
            return NULL;
        }
    }
    
    if (data->s != NULL) {
        data_new->s = copy_string_column(data->s, data->len);
        if (!data_new->s) {
            dataset_free(data_new);
            return NULL;
        }
    }
    
    data_new->hotlink = data->hotlink;
    data_new->hotsrc  = data->hotsrc;
    data_new->comment = copy_string(NULL, data->comment);
    data_new->hotfile = copy_string(NULL, data->hotfile);
    
    return data_new;
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

static void set_default_set(set *p)
{
    p->hidden = FALSE;                          /* hidden set */
    p->type = SET_XY;                           /* dataset type */

    p->symskip = 0;                             /* How many symbols to skip */

    p->sym.type = 0;                            /* set plot symbol */
    p->sym.size = grdefaults.symsize;            /* size of symbols */
    p->sym.line.pen.color = grdefaults.color;    /* color for symbol line */
    p->sym.line.pen.pattern = grdefaults.pattern;/* pattern */
    p->sym.line.style = grdefaults.lines;        /* set plot sym line style */
    p->sym.line.width = grdefaults.linew;        /* set plot sym line width */
    p->sym.fillpen.color = grdefaults.color;     /* color for symbol fill */
    p->sym.fillpen.pattern = 0;                  /* pattern for symbol fill */
    p->sym.symchar = 'A';
    p->sym.charfont = grdefaults.font;

    p->avalue.active = FALSE;                   /* active or not */
    p->avalue.type = AVALUE_TYPE_Y;             /* type */
    p->avalue.size = 1.0;                       /* char size */
    p->avalue.font = grdefaults.font;           /* font */
    p->avalue.color = grdefaults.color;         /* color */
    p->avalue.angle = 0;                        /* rotation angle */
    p->avalue.format = FORMAT_GENERAL;          /* format */
    p->avalue.prec = 3;                         /* precision */
    p->avalue.prestr[0] = '\0';
    p->avalue.appstr[0] = '\0';
    p->avalue.offset.x = 0.0;
    p->avalue.offset.y = 0.0;

    p->line.type = LINE_TYPE_STRAIGHT;
    p->line.line.style = grdefaults.lines;
    p->line.line.width = grdefaults.linew;
    p->line.line.pen.color = grdefaults.color;
    p->line.line.pen.pattern = grdefaults.pattern;
    
    p->line.baseline_type = BASELINE_TYPE_0;
    p->line.baseline = FALSE;
    p->line.droplines = FALSE;

    p->line.filltype = SETFILL_NONE;                 /* fill type */
    p->line.fillrule = FILLRULE_WINDING;             /* fill type */
    p->line.fillpen.color = grdefaults.color;     /* fill color */
    p->line.fillpen.pattern = grdefaults.pattern; /* fill pattern */

    p->errbar.active = TRUE;                      /* on by default */
    p->errbar.ptype = PLACEMENT_BOTH;             /* error bar placement */
    p->errbar.pen.color = grdefaults.color;       /* color */
    p->errbar.pen.pattern = grdefaults.pattern;   /* pattern */
    p->errbar.lines = grdefaults.lines;           /* error bar line width */
    p->errbar.linew = grdefaults.linew;           /* error bar line style */
    p->errbar.riser_linew = grdefaults.linew;     /* riser line width */
    p->errbar.riser_lines = grdefaults.lines;     /* riser line style */
    p->errbar.barsize = 1.0;                      /* size of error bar */
    p->errbar.arrow_clip = FALSE;                 /* draw arrows if clipped */
    p->errbar.cliplen = 0.1;                      /* max v.p. riser length */

    p->legstr = NULL;                             /* legend string */

    p->data = NULL;
}

Quark *set_new(Quark *gr)
{
    Quark *pset; 
    pset = quark_new(gr, QFlavorSet);
    if (pset) {
        graph *g = (graph *) gr->data;
        if (storage_add(g->sets, pset) != RETURN_SUCCESS) {
            quark_free(pset);
            return NULL;
        }
    }
    return pset;
}

set *set_data_new(void)
{
    set *p;
    
    p = xmalloc(sizeof(set));
    if (!p) {
        return NULL;
    }
    set_default_set(p);
    p->data = dataset_new();
    if (!p->data) {
        xfree(p);
        return NULL;
    }
    p->data->ncols = 2; /* To be in sync with default SET_XY type */
    
    return p;
}


void set_data_free(set *p)
{
    if (p) {
        dataset_free(p->data);
        xfree(p->legstr);
    }
}

set *set_data_copy(set *p)
{
    set *p_new;
    
    if (!p) {
        return NULL;
    }
    
    p_new = xmalloc(sizeof(set));
    if (!p_new) {
        return NULL;
    }
    
    memcpy(p_new, p, sizeof(set));
    
    /* allocatables */
    p_new->data = dataset_copy(p->data);
    if (!p_new->data) {
        xfree(p_new);
        return NULL;
    }
    p->legstr  = copy_string(NULL, p->legstr);

    return p_new;
}

int copy_set_params(set *p1, set *p2)
{
    if (!p1 || !p2) {
        return RETURN_FAILURE;
    } else {
        Dataset *data;
        int type;
        char *legstr;
        
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

Dataset *dataset_get(Quark *pset)
{
    set *p = (set *) pset->data;
    if (p) {
        return p->data;
    } else {
        return NULL;
    }
}

/*
 * free set data, but preserve the parameter settings
 */
void killsetdata(Quark *pset)
{
    set *p;
    
    if (!pset) {
        return;
    }
    
    p = (set *) pset->data;
    dataset_empty(p->data);
}

/*
 * (re)allocate data arrays for a set of length len.
 */
int setlength(Quark *pset, int len)
{
    set *p;

    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = (set *) pset->data;

    set_dirtystate();
    
    return set_dataset_nrows(p->data, len);
}

/*
 * same as copyset(), but doesn't alter the to set appearance
 */
int copysetdata(Quark *psrc, Quark *pdest)
{
    set *p1, *p2;
    
    if (!psrc || !pdest || psrc == pdest) {
	return RETURN_FAILURE;
    }
    
    p1 = (set *) psrc;
    p2 = (set *) pdest;
    
    dataset_free(p2->data);
    p2->data = dataset_copy(p1->data);
    
    if (p2->data) {
        if (dataset_cols(pdest) != dataset_cols(psrc)) {
            p2->type = p1->type;
        }
        set_dirtystate();
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
    Quark *gr;
    graph *g;
    
    gr = pset->parent;
    g = (graph *) gr->data;
    
    storage_delete_by_data(g->sets, pset);
}

double *getcol(Quark *pset, int col)
{
    if (pset) {
        set *p = pset->data;
        return p->data->ex[col];
    } else {
        return NULL;
    }
}

void setcol(Quark *pset, int col, double *x, int len)
{
    if (pset) {
        set *p = pset->data;
        p->data->ex[col] = x;
        p->data->len = len;
        set_dirtystate();
    }
}

char **get_set_strings(Quark *pset)
{
    if (pset) {
        set *p = pset->data;
        return p->data->s;
    } else {
        return NULL;
    }
}

int set_set_strings(Quark *pset, int len, char **s)
{
    if (pset && len > 0 && s!= NULL) {
        set *p = pset->data;
        p->data->s = s;
        p->data->len = len;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int getsetlength(Quark *pset)
{
    if (pset) {
        set *p = pset->data;
        return p->data->len;
    } else {
        return -1;
    }
}

int setcomment(Quark *pset, char *s)
{ 
    Dataset *dsp;
    
    dsp = dataset_get(pset);
    if (dsp) {
        dsp->comment = copy_string(dsp->comment, s);
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *getcomment(Quark *pset)
{ 
    Dataset *dsp;
    
    dsp = dataset_get(pset);
    if (dsp) {
        return dsp->comment;
    } else {
        return NULL;
    }
}

int set_legend_string(Quark *pset, char *s)
{ 
    if (pset) {
        set *p = pset->data;
        p->legstr = copy_string(p->legstr, s);
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *get_legend_string(Quark *pset)
{ 
    if (pset) {
        set *p = pset->data;
        return p->legstr;
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
    
    p = pset->data;
    if (set_dataset_ncols(p->data, ncols_new) == RETURN_SUCCESS) {
        p->type = type;
        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int dataset_type(Quark *pset)
{ 
    if (pset) {
        set *p = pset->data;
        return p->type;
    } else {
        return -1;
    }
}


void set_hotlink(Quark *pset, int onoroff, char *fname, int src)
{
    Dataset *dsp;
    
    dsp = dataset_get(pset);
    if (dsp) {
        dsp->hotlink = onoroff;
        if (onoroff && fname != NULL) {
	    dsp->hotfile = copy_string(dsp->hotfile, fname);
	    dsp->hotsrc = src;
        }
        set_dirtystate();
    }
}

int is_hotlinked(Quark *pset)
{
    Dataset *dsp;
    
    dsp = dataset_get(pset);
    if (dsp && dsp->hotlink && dsp->hotfile) {
        return TRUE;
    } else { 
        return FALSE;
    }
}

char *get_hotlink_file(Quark *pset)
{
    Dataset *dsp;
    
    dsp = dataset_get(pset);
    if (dsp) {
        return dsp->hotfile;
    } else {
        return NULL;
    }
}

int get_hotlink_src(Quark *pset)
{
    Dataset *dsp;
    
    dsp = dataset_get(pset);
    if (dsp) {
        return dsp->hotsrc;
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
    set_dirtystate();
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
    set_dirtystate();
}

int is_set_active(Quark *pset)
{
    if (pset && getsetlength(pset) > 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * return number of active set(s) in gno
 */
int number_of_active_sets(Quark *gr)
{
    int i, nsets, na = 0;

    nsets = number_of_sets(gr);
    for (i = 0; i < nsets; i++) {
        Quark *pset = set_get(gr, i);
        if (is_set_active(pset) == TRUE) {
	    na++;
	}
    }
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
    
    ncols = dataset_cols(pset);
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

#if 0
/*
 * join several sets together; all but the first set in the list will be killed 
 */
int join_sets(int gno, int *sets, int nsets)
{
    int i, j, n, setno, setno_final, ncols, old_length, new_length;
    double *x1, *x2;
    char **s1, **s2;

    if (nsets < 2) {
        errmsg("nsets < 2");
        return RETURN_FAILURE;
    }
    
    setno_final = sets[0];
    ncols = dataset_cols(pset_final);
    for (i = 0; i < nsets; i++) {
        setno = sets[i];
        if (!pset) {
            errmsg("Invalid setno in the list");
            return RETURN_FAILURE;
        }
        if (dataset_cols(pset) != ncols) {
            errmsg("Can't join datasets with different number of cols");
            return RETURN_FAILURE;
        }
    }
    
    new_length = getsetlength(pset_final);
    for (i = 1; i < nsets; i++) {
        setno = sets[i];
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
#endif

void reverse_set(Quark *pset)
{
    int n, i, j, k, ncols;
    double *x;
    char **s;

    if (!pset) {
	return;
    }
    n = getsetlength(pset);
    ncols = dataset_cols(pset);
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
    set_dirtystate();
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
    
    nc = dataset_cols(pset);
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

    set_dirtystate();
}

/*
 * sort two arrays
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
        ncols = dataset_cols(pset);
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
        set_dirtystate();
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
        *ncols = dataset_cols(pset);
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
    int ncols = dataset_cols(pset);

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

#if 0
/*
 * split a set into lpart length sets
 */
void do_splitsets(Quark *pset, int lpart)
{
    int i, j, k, ncols, len, plen, tmpset, npsets;
    double *x;
    char s[256];
    set *p, *ptmp;
    Dataset *dsp;

    if ((len = getsetlength(pset)) < 2) {
	errmsg("Set length < 2");
	return;
    }
    if (lpart >= len) {
	errmsg("Split length >= set length");
	return;
    }
    if (lpart <= 0) {
	errmsg("Split length <= 0");
	return;
    }

    npsets = (len - 1)/lpart + 1;

    /* get number of columns in this set */
    ncols = dataset_cols(pset);

    p = set_get(pset);

    /* save the contents to a temporary buffer */
    dsp = p->data;

    /* zero data contents of the original set */
    p->data = dataset_new();

    /* now load each set */
    for (i = 0; i < npsets; i++) {
	plen = MIN2(lpart, len - i*lpart); 
        tmpset = nextset(gno);
        ptmp = set_get(gno, tmpset);
        if (!ptmp) {
            errmsg("Can't create new set");
            return;
        }
        
        /* set the plot parameters */
        copy_set_params(p, ptmp);

	if (setlength(gno, tmpset, plen) != RETURN_SUCCESS) {
            /* should not happen */
            return;
        }
        if (dsp->s) {
            ptmp->data->s = xmalloc(plen*sizeof(char *));
        }
        
        /* load the data into each column */
	for (k = 0; k < ncols; k++) {
	    x = getcol(gno, tmpset, k);
	    for (j = 0; j < plen; j++) {
		x[j] = dsp->ex[k][i*lpart + j];
	    }
	}
        if (dsp->s) {
	    for (j = 0; j < plen; j++) {
		ptmp->data->s[j] =
                    copy_string(NULL, dsp->s[i*lpart + j]);
	    }
        }
	
        sprintf(s, "partition %d of set G%d.S%d", i + 1, pset);
	setcomment(gno, tmpset, s);
    }
}
#endif

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
    if (!is_set_active(pset)) {
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
    
    gr = pset->parent;
    p = (set *) pset->data;
    get_graph_world(gr, &w);
    
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


int dataset_cols(Quark *pset)
{
    Dataset *dsp = dataset_get(pset);
    if (dsp) {
        return dsp->ncols;
    } else {
        return -1;
    }
}

int load_comments_to_legend(Quark *pset)
{
    return set_legend_string(pset, getcomment(pset));
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
    ncols = dataset_cols(pset);
    dsp = dataset_get(pset);
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

set *set_get_data(Quark *qset)
{
    if (qset) {
        return qset->data;
    } else {
        return NULL;
    }
}

Dataset *set_get_dataset(Quark *qset)
{
    if (qset) {
        set *p = (set *) qset->data;
        return p->data;
    } else {
        return NULL;
    }
}

int set_set_colors(Quark *pset, int color)
{
    set *p;
    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    if (color < number_of_colors(grace->rt->canvas) && color >= 0) {
        p->line.line.pen.color    = color;
        p->sym.line.pen.color = color;
        p->sym.fillpen.color  = color;
        p->errbar.pen.color  = color;

        set_dirtystate();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_set_symskip(Quark *pset, int symskip)
{
    set *p;
    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    p->symskip = symskip;
    
    return RETURN_SUCCESS;
}

int set_set_symbol(Quark *pset, const Symbol *sym)
{
    set *p;
    if (!pset || !sym) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    p->sym = *sym;
    
    return RETURN_SUCCESS;
}

int set_set_line(Quark *pset, const SetLine *sl)
{
    set *p;
    if (!pset || !sl) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    p->line = *sl;
    
    return RETURN_SUCCESS;
}

int set_set_avalue(Quark *pset, const AValue *av)
{
    set *p;
    if (!pset || !av) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    p->avalue = *av;
    
    return RETURN_SUCCESS;
}

int set_set_errbar(Quark *pset, const Errbar *ebar)
{
    set *p;
    if (!pset || !ebar) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    p->errbar = *ebar;
    
    return RETURN_SUCCESS;
}

int set_set_legstr(Quark *pset, const char *s)
{
    set *p;
    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = pset->data;
    
    p->legstr = copy_string(p->legstr, s);
    
    return RETURN_SUCCESS;
}
