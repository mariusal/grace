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
 * Axis stuff
 *
 */

#include <string.h>

#include "grace/coreP.h"

static void set_default_ticks(Quark *q)
{
    int i;

    tickmarks *t = axisgrid_get_data(q);
    defaults grdefaults;
    Project *pr = project_get_data(get_parent_project(q));
    
    if (!t || !pr) {
        return;
    }
    
    grdefaults = pr->grdefaults;
    
    t->type = AXIS_TYPE_X;
    
    t->tmajor = 0.5;
    t->nminor = 1;
    t->t_round = TRUE;
    t->tl_format = FORMAT_GENERAL;
    t->tl_prec = 5;
    t->tl_formula = NULL;

    set_default_textprops(&t->tl_tprops, &grdefaults);
    t->tl_gap.x = 0.0;
    t->tl_gap.y = 0.01;

    t->tl_skip = 0;
    t->tl_staggered = 0;
    t->tl_starttype = TYPE_AUTO;
    t->tl_stoptype = TYPE_AUTO;
    t->tl_start = 0.0;
    t->tl_stop = 0.0;
    t->tl_appstr = NULL;
    t->tl_prestr = NULL;

    t->t_spec = TICKS_SPEC_NONE;
    t->t_autonum = 6;
    t->props.inout = TICKS_IN;
    t->mprops.inout = TICKS_IN;
    t->props.size = grdefaults.charsize;
    t->mprops.size = grdefaults.charsize / 2;
    t->bar = grdefaults.line;
    t->gprops.onoff = FALSE;
    t->gprops.line = grdefaults.line;
    t->mgprops.onoff = FALSE;
    t->mgprops.line = grdefaults.line;
    t->props.line = grdefaults.line;
    t->mprops.line = grdefaults.line;
    t->nticks = 0;
    for (i = 0; i < MAX_TICKS; i++) {
        t->tloc[i].wtpos = 0.0;
        t->tloc[i].label = NULL;
    }
}

Quark *axisgrid_new(Quark *q)
{
    Quark *a; 
    a = quark_new(q, QFlavorAGrid);
    set_default_ticks(a);
    axisgrid_autotick(a);
    return a;
}

tickmarks *axisgrid_data_new(void)
{
    tickmarks *retval;
    
    retval = xmalloc(sizeof(tickmarks));
    if (retval != NULL) {
        memset(retval, 0, sizeof(tickmarks));
    }
    return retval;
}

tickmarks *axisgrid_data_copy(tickmarks *t)
{
    tickmarks *retval;
    int i;
    
    if (t == NULL) {
        return NULL;
    } else {
        retval = axisgrid_data_new();
        if (retval != NULL) {
            memcpy(retval, t, sizeof(tickmarks));
	    retval->tl_formula = copy_string(NULL, t->tl_formula);
	    retval->tl_prestr = copy_string(NULL, t->tl_prestr);
	    retval->tl_appstr = copy_string(NULL, t->tl_appstr);
            for (i = 0; i < MAX_TICKS; i++) {
                retval->tloc[i].label = copy_string(NULL, t->tloc[i].label);
            }
        }
        return retval;
    }
}

void axisgrid_data_free(tickmarks *t)
{
    if (t) {
        int i;

        xfree(t->tl_formula);
        xfree(t->tl_prestr);
        xfree(t->tl_appstr);
        
        for (i = 0; i < MAX_TICKS; i++) {
            xfree(t->tloc[i].label);
        }
        
        xfree(t);
    }
}

tickmarks *axisgrid_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorAGrid) {
        return (tickmarks *) q->data;
    } else {
        return NULL;
    }
}

int axisgrid_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorAGrid,
        (Quark_data_new) axisgrid_data_new,
        (Quark_data_free) axisgrid_data_free,
        (Quark_data_copy) axisgrid_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
}

int axisgrid_set_type(Quark *q, int type)
{
    tickmarks *t = axisgrid_get_data(q);
    if (t) {
        if (t->type != type) {
            t->type = type;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axisgrid_is_x(const Quark *q)
{
    tickmarks *t = axisgrid_get_data(q);
    return (t && t->type == AXIS_TYPE_X);
}

int axisgrid_is_y(const Quark *q)
{
    tickmarks *t = axisgrid_get_data(q);
    return (t && t->type == AXIS_TYPE_Y);
}

void axisgrid_autotick(Quark *q)
{
    Quark *gr;
    tickmarks *t;
    world w;
    double range, d, tmpmax, tmpmin;
    int axis_scale;

    t = axisgrid_get_data(q);
    if (t == NULL) {
        return;
    }
    gr = get_parent_graph(q);
    graph_get_world(gr, &w);

    if (axisgrid_is_x(q)) {
        tmpmin = w.xg1;
        tmpmax = w.xg2;
        axis_scale = graph_get_xscale(gr);
    } else {
        tmpmin = w.yg1;
        tmpmax = w.yg2;
        axis_scale = graph_get_yscale(gr);
    }

    if (axis_scale == SCALE_LOG) {
	if (t->tmajor <= 1.0) {
            t->tmajor = 10.0;
        }
        tmpmax = log10(tmpmax)/log10(t->tmajor);
	tmpmin = log10(tmpmin)/log10(t->tmajor);
    } else if (axis_scale == SCALE_LOGIT) {
    	if (t->tmajor >= 0.5) {
            t->tmajor = 0.4;
        }
        tmpmax = log(tmpmax/(1-tmpmax))/log(t->tmajor/(1-t->tmajor));
	tmpmin = log(tmpmin/(1-tmpmin))/log(t->tmajor/(1-t->tmajor)); 
    } else if (t->tmajor <= 0.0) {
        t->tmajor = 1.0;
    }
    
    range = tmpmax - tmpmin;
    if (axis_scale == SCALE_LOG) {
	d = ceil(range/(t->t_autonum - 1));
	t->tmajor = pow(t->tmajor, d);
    } 
    else if (axis_scale == SCALE_LOGIT ){
        d = ceil(range/(t->t_autonum - 1));
	t->tmajor = exp(d)/(1.0 + exp(d));
    } 
    else {
	d = nicenum(range/(t->t_autonum - 1), 0, NICE_ROUND);
	t->tmajor = d;
    }

    /* alter # of minor ticks only if the current value is anomalous */
    if (t->nminor < 0 || t->nminor > 10) {
        if (axis_scale != SCALE_LOG) {
	    t->nminor = 1;
        } else {
            t->nminor = 8;
        }
    }
    
    quark_dirtystate_set(q, TRUE);
}

Quark *get_parent_axisgrid(const Quark *child)
{
    Quark *p = (Quark *) child;
    
    while (p) {
        p = quark_parent_get(p);
        if (p && p->fid == QFlavorAGrid) {
            return p;
        }
    }
    
    return NULL;
}


/* Axis instances */
Quark *axis_new(Quark *q)
{
    Quark *a; 
    a = quark_new(q, QFlavorAxis);
    axis_enable_bar(a, TRUE);
    axis_enable_ticks(a, TRUE);
    axis_enable_labels(a, TRUE);
    return a;
}

Axis *axis_data_new(void)
{
    Axis *retval;
    
    retval = xmalloc(sizeof(Axis));
    if (retval != NULL) {
        memset(retval, 0, sizeof(Axis));
    }
    return retval;
}

Axis *axis_data_copy(Axis *a)
{
    Axis *retval;
    
    if (a == NULL) {
        return NULL;
    } else {
        retval = axis_data_new();
        if (retval != NULL) {
            memcpy(retval, a, sizeof(Axis));
        }
        return retval;
    }
}

void axis_data_free(Axis *a)
{
    if (a) {
        xfree(a);
    }
}

Axis *axis_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorAxis) {
        return (Axis *) q->data;
    } else {
        return NULL;
    }
}

int axis_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorAxis,
        (Quark_data_new) axis_data_new,
        (Quark_data_free) axis_data_free,
        (Quark_data_copy) axis_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
}


int axis_set_offset(Quark *q, double offset)
{
    Axis *a = axis_get_data(q);
    if (a) {
        if (a->offset != offset) {
            a->offset = offset;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_enable_bar(Quark *q, int onoff)
{
    Axis *a = axis_get_data(q);
    if (a) {
        if (a->draw_bar != onoff) {
            a->draw_bar = onoff;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_enable_ticks(Quark *q, int onoff)
{
    Axis *a = axis_get_data(q);
    if (a) {
        if (a->draw_ticks != onoff) {
            a->draw_ticks = onoff;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_enable_labels(Quark *q, int onoff)
{
    Axis *a = axis_get_data(q);
    if (a) {
        if (a->draw_labels != onoff) {
            a->draw_labels = onoff;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_set_position(Quark *q, int pos)
{
    Axis *a = axis_get_data(q);
    if (a) {
        if (a->position != pos) {
            a->position = pos;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

double axis_get_offset(const Quark *q)
{
    Axis *a = axis_get_data(q);
    if (a) {
        return a->offset;
    } else {
        return 0.0;
    }
}

int axis_get_position(const Quark *q)
{
    Axis *a = axis_get_data(q);
    if (a) {
        return a->position;
    } else {
        return AXIS_POS_NORMAL;
    }
}

int axis_bar_enabled(const Quark *q)
{
    Axis *a = axis_get_data(q);
    if (a) {
        return a->draw_bar;
    } else {
        return FALSE;
    }
}

int axis_ticks_enabled(const Quark *q)
{
    Axis *a = axis_get_data(q);
    if (a) {
        return a->draw_ticks;
    } else {
        return FALSE;
    }
}

int axis_labels_enabled(const Quark *q)
{
    Axis *a = axis_get_data(q);
    if (a) {
        return a->draw_labels;
    } else {
        return FALSE;
    }
}

int axis_get_bb(const Quark *q, view *bbox)
{
    Axis *a = axis_get_data(q);
    if (a) {
        *bbox = a->bb;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_set_bb(const Quark *q, const view *bbox)
{
    Axis *a = axis_get_data(q);
    if (a) {
        a->bb = *bbox;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_shift(Quark *q, const VVector *vshift)
{
    Quark *ag = get_parent_axisgrid(q);
    Axis *a = axis_get_data(q);
    if (ag && a) {
        int sign = (a->position == AXIS_POS_OPPOSITE) ? +1:-1;
        
        if (axisgrid_is_x(ag)) {
            a->offset += sign*vshift->y;
        } else {
            a->offset += sign*vshift->x;
        }
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
