/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Set stuff
 *
 */

#include <string.h>

#define ADVANCED_MEMORY_HANDLERS
#include "grace/coreP.h"

double *copy_data_column(AMem *amem, double *src, int nrows)
{
    double *dest;
    
    if (!src) {
        return NULL;
    }
    
    dest = amem_malloc(amem, nrows*SIZEOF_DOUBLE);
    if (dest != NULL) {
        memcpy(dest, src, nrows*SIZEOF_DOUBLE);
    }
    return dest;
}

char **copy_string_column(AMem *amem, char **src, int nrows)
{
    char **dest;
    int i;

    if (!src) {
        return NULL;
    }
    
    dest = amem_malloc(amem, nrows*sizeof(char *));
    if (dest != NULL) {
        for (i = 0; i < nrows; i++)
            dest[i] = amem_strdup(amem, src[i]);
    }
    return dest;
}


Dataset *dataset_new(AMem *amem)
{
    Dataset *dsp;
    int k;
    
    dsp = amem_malloc(amem, sizeof(Dataset));
    if (!dsp) {
        return NULL;
    }
    
    dsp->amem  = amem;
    
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
        AMem *amem = dsp->amem;
        
        if (dsp->len) {
            for (k = 0; k < dsp->ncols; k++) {
	        AMEM_CFREE(amem, dsp->ex[k]);
            }
            if (dsp->s) {
	        for (k = 0; k < dsp->len; k++) {
		    AMEM_CFREE(amem, dsp->s[k]);
	        }
                AMEM_CFREE(amem, dsp->s);
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
        AMem *amem = dsp->amem;

        dataset_empty(dsp);
        amem_free(amem, dsp->hotfile);
        amem_free(amem, dsp->comment);
        amem_free(amem, dsp);
    }
}


int dataset_set_nrows(Dataset *dsp, int len)
{
    int i, j, oldlen;
    
    if (!dsp || len < 0) {
	return RETURN_FAILURE;
    }
    
    oldlen = dsp->len;
    if (len == oldlen) {
	return RETURN_SUCCESS;
    }
    
    for (i = 0; i < dsp->ncols; i++) {
	if ((dsp->ex[i] = amem_realloc(dsp->amem,
            dsp->ex[i], len*SIZEOF_DOUBLE)) == NULL
            && len != 0) {
	    return RETURN_FAILURE;
	}
        for (j = oldlen; j < len; j++) {
            dsp->ex[i][j] = 0.0;
        }
    }
    
    if (dsp->s != NULL) {
        for (i = len; i < oldlen; i++) {
            amem_free(dsp->amem, dsp->s[i]);
        }
        dsp->s = amem_realloc(dsp->amem, dsp->s, len*sizeof(char *));
        for (j = oldlen; j < len; j++) {
            dsp->s[j] = amem_strdup(dsp->amem, "");
        }
    }
    
    dsp->len = len;

    return RETURN_SUCCESS;
}

int dataset_set_ncols(Dataset *dsp, int ncols)
{
    if (ncols < 0 || ncols > MAX_SET_COLS) {
        return RETURN_FAILURE;
    }
    
    if (dsp->ncols == ncols) {
        /* nothing changed */
        return RETURN_SUCCESS;
    } else {
        int i, ncols_old = dsp->ncols;
        
        for (i = ncols_old; i < ncols; i++) {
            dsp->ex[i] = amem_calloc(dsp->amem, dsp->len, SIZEOF_DOUBLE);
        }
        for (i = ncols; i < ncols_old; i++) {
            AMEM_CFREE(dsp->amem, dsp->ex[i]);
        }

        dsp->ncols = ncols;
        
        return RETURN_SUCCESS;
    }
}

int dataset_enable_scol(Dataset *dsp, int yesno)
{
    if (yesno) {
        if (dsp->s) {
            return RETURN_SUCCESS;
        } else {
            dsp->s = amem_calloc(dsp->amem, dsp->len, sizeof(char *));
            if (dsp->len && !dsp->s) {
                return RETURN_FAILURE;
            } else {
                return RETURN_SUCCESS;
            }
        }
    } else {
        if (dsp->s) {
            int i;
            for (i = 0; i < dsp->len; i++) {
                amem_free(dsp->amem, dsp->s[i]);
            }
            amem_free(dsp->amem, dsp->s);
        }
        return RETURN_SUCCESS;
    }
}



Dataset *dataset_copy(AMem *amem, Dataset *data)
{
    Dataset *data_new;
    int k;

    if (!data) {
        return NULL;
    }
    
    data_new = dataset_new(amem);
    if (!data_new) {
        return NULL;
    }
    
    data_new->len   = data->len;
    data_new->ncols = data->ncols;
    
    for (k = 0; k < data->ncols; k++) {
        data_new->ex[k] = copy_data_column(amem, data->ex[k], data->len);
        if (!data_new->ex[k]) {
            dataset_free(data_new);
            return NULL;
        }
    }
    
    if (data->s != NULL) {
        data_new->s = copy_string_column(amem, data->s, data->len);
        if (!data_new->s) {
            dataset_free(data_new);
            return NULL;
        }
    }
    
    data_new->hotlink = data->hotlink;
    data_new->hotsrc  = data->hotsrc;
    data_new->comment = amem_strdup(amem, data->comment);
    data_new->hotfile = amem_strdup(amem, data->hotfile);
    
    return data_new;
}


static void set_default_set(Quark *pset)
{
    set *p = set_get_data(pset);
    Quark *gr = get_parent_graph(pset);
    Project *pr = project_get_data(get_parent_project(pset));
    defaults grdefs;
    
    if (!p || !gr || !pr) {
        return;
    }
    
    grdefs = pr->grdefaults;
    
    p->type = SET_XY;                            /* dataset type */

    p->symskip = 0;                              /* How many symbols to skip */
    p->symskipmindist = 0;                  /* Min. distance between symbols */
    p->sym.type = 0;                             /* set plot symbol */
    p->sym.size = grdefs.charsize;           /* size of symbols */
    p->sym.line = grdefs.line;
    p->sym.fillpen = grdefs.fillpen;
    p->sym.symchar = 'A';
    p->sym.charfont = grdefs.font;

    p->avalue.active = FALSE;                    /* active or not */
    p->avalue.type = AVALUE_TYPE_Y;              /* type */

    set_default_textprops(&p->avalue.tprops, &grdefs);
    
    if (graph_get_type(gr) == GRAPH_PIE) {
        p->avalue.tprops.just = JUST_CENTER|JUST_MIDDLE;
    } else {
        p->avalue.tprops.just = JUST_CENTER|JUST_BOTTOM;
    }
    p->avalue.format = FORMAT_GENERAL;            /* format */
    p->avalue.prec = 3;                           /* precision */
    p->avalue.prestr = NULL;
    p->avalue.appstr = NULL;

    p->line.type = LINE_TYPE_STRAIGHT;
    p->line.line = grdefs.line;
    
    p->line.baseline_type = BASELINE_TYPE_0;
    p->line.baseline = FALSE;
    p->line.droplines = FALSE;

    p->line.filltype = SETFILL_NONE;              /* fill type */
    p->line.fillrule = FILLRULE_WINDING;          /* fill rule */
    p->line.fillpen = grdefs.fillpen;

    p->errbar.active = TRUE;                      /* on by default */
    p->errbar.ptype = PLACEMENT_BOTH;             /* error bar placement */
    p->errbar.pen = grdefs.line.pen;
    p->errbar.lines = grdefs.line.style;          /* error bar line width */
    p->errbar.linew = grdefs.line.width;          /* error bar line style */
    p->errbar.riser_linew = grdefs.line.width;    /* riser line width */
    p->errbar.riser_lines = grdefs.line.style;    /* riser line style */
    p->errbar.barsize = grdefs.charsize;          /* size of error bar */
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

set *set_data_new(AMem *amem)
{
    set *p;
    
    p = amem_malloc(amem, sizeof(set));
    if (!p) {
        return NULL;
    }
    memset(p, 0, sizeof(set));
    p->data = dataset_new(amem);
    if (!p->data) {
        amem_free(amem, p);
        return NULL;
    }
    p->data->ncols = 2; /* To be in sync with default SET_XY type */
    
    return p;
}


void set_data_free(AMem *amem, set *p)
{
    if (p) {
        dataset_free(p->data);
        amem_free(amem, p->legstr);
        amem_free(amem, p->avalue.prestr);
        amem_free(amem, p->avalue.appstr);
        amem_free(amem, p);
    }
}

set *set_data_copy(AMem *amem, set *p)
{
    set *p_new;
    
    if (!p) {
        return NULL;
    }
    
    p_new = amem_malloc(amem, sizeof(set));
    if (!p_new) {
        return NULL;
    }
    
    memcpy(p_new, p, sizeof(set));
    
    /* allocatables */
    p_new->data = dataset_copy(amem, p->data);
    if (!p_new->data) {
        amem_free(amem, p_new);
        return NULL;
    }
    p->legstr  = amem_strdup(amem, p->legstr);
    p->avalue.prestr = amem_strdup(amem, p->avalue.prestr);
    p->avalue.appstr = amem_strdup(amem, p->avalue.appstr);

    return p_new;
}

int set_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorSet,
        (Quark_data_new) set_data_new,
        (Quark_data_free) set_data_free,
        (Quark_data_copy) set_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
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
	dsp->hotfile = amem_strcpy(pset->amem, dsp->hotfile, fname);
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

int set_get_ncols(Quark *pset)
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

int set_set_symskipmindist(Quark *pset, double symskipmindist)
{
    set *p;
    if (!pset || symskipmindist < 0) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    p->symskipmindist = symskipmindist;
    
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
    
    AMEM_CFREE(pset->amem, p->avalue.prestr);
    AMEM_CFREE(pset->amem, p->avalue.appstr);
    p->avalue = *av;
    p->avalue.prestr = amem_strdup(pset->amem, av->prestr);
    p->avalue.appstr = amem_strdup(pset->amem, av->appstr);
    
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
    
    p->legstr = amem_strcpy(pset->amem, p->legstr, s);
    
    return RETURN_SUCCESS;
}

char **set_get_strings(Quark *pset)
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

int set_get_length(Quark *pset)
{
    set *p = set_get_data(pset);
    if (p) {
        return p->data->len;
    } else {
        return -1;
    }
}

/*
 * (re)allocate data arrays for a set of length len.
 */
int set_set_length(Quark *pset, unsigned int len)
{
    set *p = set_get_data(pset);

    if (!p) {
        return RETURN_FAILURE;
    }
    
    quark_dirtystate_set(pset, TRUE);
    
    return dataset_set_nrows(p->data, len);
}

int set_set_comment(Quark *pset, char *s)
{ 
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        dsp->comment = amem_strcpy(pset->amem, dsp->comment, s);
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *set_get_comment(Quark *pset)
{ 
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    if (dsp) {
        return dsp->comment;
    } else {
        return NULL;
    }
}

int set_set_type(Quark *pset, int type)
{ 
    int ncols_new = settype_cols(type);
    set *p = set_get_data(pset);
    if (!p) {
        return RETURN_FAILURE;
    }
    
    if (dataset_set_ncols(p->data, ncols_new) == RETURN_SUCCESS) {
        p->type = type;
        quark_dirtystate_set(pset, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int set_get_type(Quark *pset)
{ 
    set *p = set_get_data(pset);
    if (p) {
        return p->type;
    } else {
        return -1;
    }
}

double *set_get_col(Quark *pset, unsigned int col)
{
    if (pset) {
        set *p = set_get_data(pset);
        return p->data->ex[col];
    } else {
        return NULL;
    }
}

void set_set_col(Quark *pset, unsigned int col, double *x, unsigned int len)
{
    if (pset) {
        set *p = set_get_data(pset);
        p->data->ex[col] = x;
        p->data->len = len;
        quark_dirtystate_set(pset, TRUE);
    }
}
