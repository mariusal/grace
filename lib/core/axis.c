/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003 Grace Development Team
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

static void set_default_ticks(Quark *q)
{
    int i;

    tickmarks *t = axis_get_data(q);
    defaults grdefaults;
    Project *pr = project_get_data(get_parent_project(q));
    
    if (!t || !pr) {
        return;
    }
    
    grdefaults = pr->grdefaults;
    
    t->active = TRUE;
    t->type = AXIS_TYPE_X;
    t->zero = FALSE;
    t->tl_flag = TRUE;
    t->t_flag = TRUE;
    
    set_default_textprops(&t->label_tprops, &grdefaults);
    t->label_active = TRUE;
    t->label = NULL;
    t->label_offset.x = 0.0;
    t->label_offset.y = 0.08;
    
    t->tmajor = 0.5;
    t->nminor = 1;
    t->t_round = TRUE;
    t->offsx = 0.0;
    t->offsy = 0.0;
    t->label_layout = LAYOUT_PARALLEL;
    t->label_place = TYPE_AUTO;
    t->label_op = PLACEMENT_NORMAL;
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
    t->tl_op = PLACEMENT_NORMAL;
    t->tl_gaptype = TYPE_AUTO;
    t->tl_appstr[0] = 0;
    t->tl_prestr[0] = 0;
    t->t_spec = TICKS_SPEC_NONE;
    t->t_autonum = 6;
    t->t_op = PLACEMENT_BOTH;
    t->props.inout = TICKS_IN;
    t->mprops.inout = TICKS_IN;
    t->props.size = grdefaults.charsize;
    t->mprops.size = grdefaults.charsize / 2;
    t->t_drawbar = TRUE;
    t->t_drawbarcolor = grdefaults.line.pen.color;
    t->t_drawbarlines = grdefaults.line.style;
    t->t_drawbarlinew = grdefaults.line.width;
    t->props.gridflag = FALSE;
    t->mprops.gridflag = FALSE;
    t->props.color = grdefaults.line.pen.color;
    t->props.lines = grdefaults.line.style;
    t->props.linew = grdefaults.line.width;
    t->mprops.color = grdefaults.line.pen.color;
    t->mprops.lines = grdefaults.line.style;
    t->mprops.linew = grdefaults.line.width;
    t->nticks = 0;
    for (i = 0; i < MAX_TICKS; i++) {
        t->tloc[i].wtpos = 0.0;
        t->tloc[i].label = NULL;
    }
}

Quark *axis_new(Quark *q)
{
    Quark *a; 
    a = quark_new(q, QFlavorAxis);
    set_default_ticks(a);
    axis_autotick(a);
    return a;
}

tickmarks *axis_data_new(void)
{
    tickmarks *retval;
    
    retval = xmalloc(sizeof(tickmarks));
    if (retval != NULL) {
        memset(retval, 0, sizeof(tickmarks));
    }
    return retval;
}

tickmarks *axis_data_copy(tickmarks *t)
{
    tickmarks *retval;
    int i;
    
    if (t == NULL) {
        return NULL;
    } else {
        retval = axis_data_new();
        if (retval != NULL) {
            memcpy(retval, t, sizeof(tickmarks));
	    retval->label = copy_string(NULL, t->label);
	    retval->tl_formula = copy_string(NULL, t->tl_formula);
            for (i = 0; i < MAX_TICKS; i++) {
                retval->tloc[i].label = copy_string(NULL, t->tloc[i].label);
            }
        }
        return retval;
    }
}

void axis_data_free(tickmarks *t)
{
    if (t) {
        int i;

        xfree(t->label);
        xfree(t->tl_formula);
        
        for (i = 0; i < MAX_TICKS; i++) {
            xfree(t->tloc[i].label);
        }
        
        xfree(t);
    }
}

tickmarks *axis_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorAxis) {
        return (tickmarks *) q->data;
    } else {
        return NULL;
    }
}

int axis_set_type(Quark *q, int type)
{
    tickmarks *t = axis_get_data(q);
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

int axis_set_active(Quark *q, int flag)
{
    tickmarks *t = axis_get_data(q);
    if (t) {
        if (t->active != flag) {
            t->active = flag;
            quark_dirtystate_set(q, TRUE);
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int axis_is_x(const Quark *q)
{
    tickmarks *t = axis_get_data(q);
    return (t && t->type == AXIS_TYPE_X);
}

int axis_is_y(const Quark *q)
{
    tickmarks *t = axis_get_data(q);
    return (t && t->type == AXIS_TYPE_Y);
}

void axis_autotick(Quark *q)
{
    Quark *gr;
    tickmarks *t;
    world w;
    double range, d, tmpmax, tmpmin;
    int axis_scale;

    t = axis_get_data(q);
    if (t == NULL) {
        return;
    }
    gr = get_parent_graph(q);
    graph_get_world(gr, &w);

    if (axis_is_x(q)) {
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
