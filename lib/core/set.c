/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Axis stuff
 *
 */

#include <string.h>

#include "grace/core.h"

double *copy_data_column(double *src, int nrows)
{
    double *dest;
    
    if (!src) {
        return NULL;
    }
    
    dest = xmalloc(nrows*SIZEOF_DOUBLE);
    if (dest != NULL) {
        memcpy(dest, src, nrows*SIZEOF_DOUBLE);
    }
    return dest;
}

char **copy_string_column(char **src, int nrows)
{
    char **dest;
    int i;

    if (!src) {
        return NULL;
    }
    
    dest = xmalloc(nrows*sizeof(char *));
    if (dest != NULL) {
        for (i = 0; i < nrows; i++)
            dest[i] = copy_string(NULL, src[i]);
    }
    return dest;
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


static void set_default_set(Quark *pset)
{
    set *p = set_get_data(pset);
    Project *pr = project_get_data(get_parent_project(pset));
    defaults grdefaults;
    
    if (!p || !pr) {
        return;
    }
    
    grdefaults = pr->grdefaults;
    
    p->active = TRUE;
    p->type = SET_XY;                            /* dataset type */

    p->symskip = 0;                              /* How many symbols to skip */

    p->sym.type = 0;                             /* set plot symbol */
    p->sym.size = grdefaults.charsize;           /* size of symbols */
    p->sym.line = grdefaults.line;
    p->sym.fillpen = grdefaults.fillpen;
    p->sym.symchar = 'A';
    p->sym.charfont = grdefaults.font;

    p->avalue.active = FALSE;                    /* active or not */
    p->avalue.type = AVALUE_TYPE_Y;              /* type */
    p->avalue.size = 1.0;                        /* char size */
    p->avalue.font = grdefaults.font;            /* font */
    p->avalue.color = grdefaults.line.pen.color; /* color */
    p->avalue.angle = 0;                         /* rotation angle */
    p->avalue.format = FORMAT_GENERAL;           /* format */
    p->avalue.prec = 3;                          /* precision */
    p->avalue.prestr[0] = '\0';
    p->avalue.appstr[0] = '\0';
    p->avalue.offset.x = 0.0;
    p->avalue.offset.y = 0.0;

    p->line.type = LINE_TYPE_STRAIGHT;
    p->line.line = grdefaults.line;
    
    p->line.baseline_type = BASELINE_TYPE_0;
    p->line.baseline = FALSE;
    p->line.droplines = FALSE;

    p->line.filltype = SETFILL_NONE;              /* fill type */
    p->line.fillrule = FILLRULE_WINDING;          /* fill rule */
    p->line.fillpen = grdefaults.fillpen;

    p->errbar.active = TRUE;                      /* on by default */
    p->errbar.ptype = PLACEMENT_BOTH;             /* error bar placement */
    p->errbar.pen = grdefaults.line.pen;
    p->errbar.lines = grdefaults.line.style;      /* error bar line width */
    p->errbar.linew = grdefaults.line.width;      /* error bar line style */
    p->errbar.riser_linew = grdefaults.line.width;/* riser line width */
    p->errbar.riser_lines = grdefaults.line.style;/* riser line style */
    p->errbar.barsize = grdefaults.charsize;      /* size of error bar */
    p->errbar.arrow_clip = FALSE;                 /* draw arrows if clipped */
    p->errbar.cliplen = 0.1;                      /* max v.p. riser length */

    p->legstr = NULL;                             /* legend string */
}

Quark *set_new(Quark *gr)
{
    Quark *pset; 
    pset = quark_new(gr, QFlavorSet);
    set_default_set(pset);
    return pset;
}

set *set_data_new(void)
{
    set *p;
    
    p = xmalloc(sizeof(set));
    if (!p) {
        return NULL;
    }
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


int set_is_active(Quark *pset)
{
    if (pset) {
        set *p = set_get_data(pset);
        return p->active;
    } else {
        return FALSE;
    }
}

int set_set_active(Quark *pset, int flag)
{
    if (pset) {
        set *p = set_get_data(pset);
        p->active = flag;
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

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

int set_set_dataset(Quark *q, Dataset *dsp)
{
    set *pset = set_get_data(q);
    if (pset) {
        dataset_free(pset->data);
        pset->data = dsp;
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *set_get_legstr(Quark *pset)
{ 
    if (pset) {
        set *p = set_get_data(pset);
        return p->legstr;
    } else {
        return NULL;
    }
}

void set_set_hotlink(Quark *pset, int onoroff, char *fname, int src)
{
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        dsp->hotlink = onoroff;
	dsp->hotfile = copy_string(dsp->hotfile, fname);
	dsp->hotsrc = src;
        quark_dirtystate_set(pset, TRUE);
    }
}

int set_is_hotlinked(Quark *pset)
{
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp && dsp->hotlink && dsp->hotfile) {
        return TRUE;
    } else { 
        return FALSE;
    }
}

char *set_get_hotlink_file(Quark *pset)
{
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        return dsp->hotfile;
    } else {
        return NULL;
    }
}

int set_get_hotlink_src(Quark *pset)
{
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        return dsp->hotsrc;
    } else {
        return -1;
    }
}

int set_get_dataset_ncols(Quark *pset)
{
    Dataset *dsp = set_get_dataset(pset);
    if (dsp) {
        return dsp->ncols;
    } else {
        return -1;
    }
}

set *set_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorSet) {
        return q->data;
    } else {
        return NULL;
    }
}

Dataset *set_get_dataset(Quark *qset)
{
    if (qset) {
        set *p = set_get_data(qset);
        return p->data;
    } else {
        return NULL;
    }
}

int set_set_symskip(Quark *pset, int symskip)
{
    set *p;
    if (!pset || symskip < 0) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->symskip = symskip;
    
    return RETURN_SUCCESS;
}

int set_set_symbol(Quark *pset, const Symbol *sym)
{
    set *p;
    if (!pset || !sym) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->sym = *sym;
    
    return RETURN_SUCCESS;
}

int set_set_line(Quark *pset, const SetLine *sl)
{
    set *p;
    if (!pset || !sl) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->line = *sl;
    
    return RETURN_SUCCESS;
}

int set_set_avalue(Quark *pset, const AValue *av)
{
    set *p;
    if (!pset || !av) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->avalue = *av;
    
    return RETURN_SUCCESS;
}

int set_set_errbar(Quark *pset, const Errbar *ebar)
{
    set *p;
    if (!pset || !ebar) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->errbar = *ebar;
    
    return RETURN_SUCCESS;
}

int set_set_legstr(Quark *pset, const char *s)
{
    set *p;
    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->legstr = copy_string(p->legstr, s);
    
    return RETURN_SUCCESS;
}
