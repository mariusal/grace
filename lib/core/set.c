/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
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

Dataset *dataset_new(AMem *amem)
{
    Dataset *dsp;
    unsigned int k;
    
    dsp = amem_malloc(amem, sizeof(Dataset));
    if (!dsp) {
        return NULL;
    }

    for (k = 0; k < MAX_SET_COLS; k++) {
        dsp->cols[k] = COL_NONE;
    }
    dsp->scol = COL_NONE;
    
    dsp->comment = NULL;
    
    return dsp;
}


/*
 * free dataset columns
 */
int dataset_empty(Dataset *dsp)
{
    int k;
    
    if (dsp) {
        
        for (k = 0; k < MAX_SET_COLS; k++) {
            dsp->cols[k] = COL_NONE;
        }

        dsp->scol = COL_NONE;

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void dataset_free(AMem *amem, Dataset *dsp)
{
    if (dsp) {
        amem_free(amem, dsp->comment);
        amem_free(amem, dsp);
    }
}

set *set_data_new(AMem *amem)
{
    set *p;
    
    p = amem_malloc(amem, sizeof(set));
    if (!p) {
        return NULL;
    }
    memset(p, 0, sizeof(set));
    
    return p;
}


void set_data_free(AMem *amem, set *p)
{
    if (p) {
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
    p_new->ds.comment = amem_strdup(amem, p->ds.comment);

    p->legstr  = amem_strdup(amem, p->legstr);
    p->avalue.prestr = amem_strdup(amem, p->avalue.prestr);
    p->avalue.appstr = amem_strdup(amem, p->avalue.appstr);

    return p_new;
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

set *set_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorSet) {
        return q->data;
    } else {
        return NULL;
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

int set_set_dataset(Quark *q, const Dataset *dsp)
{
    set *pset = set_get_data(q);
    if (pset) {
        AMem *amem = quark_get_amem(q);
        memcpy(&pset->ds, dsp, sizeof(Dataset));
        pset->ds.comment = amem_strcpy(amem, pset->ds.comment, dsp->comment);

        quark_dirtystate_set(q, TRUE);
    
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

int set_get_ncols(Quark *pset)
{
    Dataset *dsp = set_get_dataset(pset);
    if (dsp) {
        unsigned int i, ncols = 0;
        for (i = 0; i < MAX_SET_COLS; i++) {
            if (dsp->cols[i] != COL_NONE) {
                ncols++;
            }
        }
        return ncols;
    } else {
        return -1;
    }
}

Dataset *set_get_dataset(Quark *qset)
{
    set *p = set_get_data(qset);
    if (p) {
        return &p->ds;
    } else {
        return NULL;
    }
}

int set_set_symskip(Quark *pset, int symskip)
{
    set *p = set_get_data(pset);
    if (!p || symskip < 0) {
        return RETURN_FAILURE;
    }
    
    p->symskip = symskip;
    quark_dirtystate_set(pset, TRUE);
    
    return RETURN_SUCCESS;
}

int set_set_symskipmindist(Quark *pset, double symskipmindist)
{
    set *p = set_get_data(pset);
    if (!p || symskipmindist < 0) {
        return RETURN_FAILURE;
    }
    
    p->symskipmindist = symskipmindist;
    quark_dirtystate_set(pset, TRUE);
    
    return RETURN_SUCCESS;
}

int set_set_symbol(Quark *pset, const Symbol *sym)
{
    set *p = set_get_data(pset);
    if (!p || !sym) {
        return RETURN_FAILURE;
    }
    
    p->sym = *sym;
    quark_dirtystate_set(pset, TRUE);
    
    return RETURN_SUCCESS;
}

int set_set_line(Quark *pset, const SetLine *sl)
{
    set *p = set_get_data(pset);
    if (!p || !sl) {
        return RETURN_FAILURE;
    }
    
    p->line = *sl;
    quark_dirtystate_set(pset, TRUE);
    
    return RETURN_SUCCESS;
}

int set_set_avalue(Quark *pset, const AValue *av)
{
    set *p = set_get_data(pset);
    if (!p || !av) {
        return RETURN_FAILURE;
    }
    
    AMEM_CFREE(pset->amem, p->avalue.prestr);
    AMEM_CFREE(pset->amem, p->avalue.appstr);
    p->avalue = *av;
    p->avalue.prestr = amem_strdup(pset->amem, av->prestr);
    p->avalue.appstr = amem_strdup(pset->amem, av->appstr);
    
    quark_dirtystate_set(pset, TRUE);
    return RETURN_SUCCESS;
}

int set_set_errbar(Quark *pset, const Errbar *ebar)
{
    set *p = set_get_data(pset);
    if (!p || !ebar) {
        return RETURN_FAILURE;
    }
    
    p->errbar = *ebar;
    
    quark_dirtystate_set(pset, TRUE);
    return RETURN_SUCCESS;
}

int set_set_legstr(Quark *pset, const char *s)
{
    set *p = set_get_data(pset);
    if (!p) {
        return RETURN_FAILURE;
    }
    
    p->legstr = amem_strcpy(pset->amem, p->legstr, s);
    
    quark_dirtystate_set(pset, TRUE);
    return RETURN_SUCCESS;
}

int set_get_length(Quark *pset)
{
    Quark *ss = get_parent_ssd(pset);
    if (ss) {
        return ssd_get_nrows(ss);
    } else {
        return -1;
    }
}

/*
 * (re)allocate data arrays for a set of length len.
 */
int set_set_length(Quark *pset, unsigned int len)
{
    Quark *ss = get_parent_ssd(pset);

    return ssd_set_nrows(ss, len);
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
    set *p = set_get_data(pset);
    if (!p) {
        return RETURN_FAILURE;
    }
    
    p->type = type;
    quark_dirtystate_set(pset, TRUE);
    return RETURN_SUCCESS;
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
    Quark *ss = get_parent_ssd(pset);
    set *p = set_get_data(pset);
    if (p && ss && col < MAX_SET_COLS) {
        ss_column *pcol = ssd_get_col(ss, p->ds.cols[col]);
        if (pcol && pcol->format != FFORMAT_STRING) {
            return (double *) pcol->data;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

char **set_get_strings(Quark *pset)
{
    Quark *ss = get_parent_ssd(pset);
    set *p = set_get_data(pset);
    if (p && ss) {
        ss_column *pcol = ssd_get_col(ss, p->ds.scol);
        if (pcol && pcol->format == FFORMAT_STRING) {
            return (char **) pcol->data;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}
