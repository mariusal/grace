/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
 * set defaults - changes to the types in defines.h
 * will require changes in here also
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "globals.h"
#include "graphs.h"
#include "objutils.h"
#include "draw.h"
#include "utils.h"
#include "protos.h"

#define grdefaults grace->rt->grdefaults

static world d_w =
{0.0, 1.0, 0.0, 1.0};

static view d_v =
{0.15, 0.85, 0.15, 0.85};

void set_region_defaults(region *r)
{
    if (!r) {
        return;
    }
    
    r->active = FALSE;
    r->type = 0;
    r->color = 1;
    r->lines = 1;
    r->linew = 1.0;
    r->n = 0;
    r->x = r->y = NULL;
    r->x1 = r->y1 = r->x2 = r->y2 = 0.0;

    r->linkto = 0;
}

void set_default_framep(framep * f)
{
    f->type = 0;                /* frame type */
    f->lines = grdefaults.lines;
    f->linew = grdefaults.linew;
    f->pen.color = grdefaults.color;
    f->pen.pattern = grdefaults.pattern;
    f->fillpen.color = grdefaults.bgcolor;      /* fill background */
    f->fillpen.pattern = 0;
}

void set_default_world(world * w)
{
    memcpy(w, &d_w, sizeof(world));
}

void set_default_view(view * v)
{
    memcpy(v, &d_v, sizeof(view));
}

void set_default_string(plotstr * s)
{
    s->active = FALSE;
    s->offset.x = s->offset.y = 0.0;
    s->color = 1;
    s->angle = 0.0;
    s->font = 0;
    s->just = JUST_LEFT|JUST_BLINE;
    s->charsize = 1.0;
    s->s = NULL;
}

void set_default_arrow(Arrow *arrowp)
{
    arrowp->type = 0;
    arrowp->length = 1.0;
    arrowp->dL_ff = 1.0;
    arrowp->lL_ff = 1.0;
}

void set_default_legend(legend *l)
{
    l->active = TRUE;
    l->loctype = COORD_VIEW;
    l->vgap = 1;
    l->hgap = 1;
    l->len = 4;
    l->invert = FALSE;
    l->legx = 0.5;
    l->legy = 0.8;
    l->font = grdefaults.font;
    l->charsize = grdefaults.charsize;
    l->color = grdefaults.color;
    l->boxpen.color = grdefaults.color;
    l->boxpen.pattern = grdefaults.pattern;
    l->boxfillpen.color = 0;
    l->boxfillpen.pattern = grdefaults.pattern;
    l->boxlinew = grdefaults.linew;     /* set plot sym line width */
    l->boxlines = grdefaults.lines;     /* set plot sym line style */
    
    l->bb.xv1 = l->bb.xv2 = l->bb.yv1 = l->bb.yv2 = 0.0;
}

void set_default_ticks(tickmarks *t)
{
    int i;

    if (t == NULL) {
        return;
    }
    
    t->active = TRUE;
    t->zero = FALSE;
    t->tl_flag = TRUE;
    t->t_flag = TRUE;
    
    set_default_string(&t->label);
    t->label.offset.x = 0.0;
    t->label.offset.y = 0.08;
    
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
    t->tl_angle = 0;
    t->tl_skip = 0;
    t->tl_staggered = 0;
    t->tl_starttype = TYPE_AUTO;
    t->tl_stoptype = TYPE_AUTO;
    t->tl_start = 0.0;
    t->tl_stop = 0.0;
    t->tl_op = PLACEMENT_NORMAL;
    t->tl_gaptype = TYPE_AUTO;
    t->tl_gap.x = 0.0;
    t->tl_gap.y = 0.01;
    t->tl_font = grdefaults.font;
    t->tl_charsize = 1.0;
    t->tl_color = grdefaults.color;
    t->tl_appstr[0] = 0;
    t->tl_prestr[0] = 0;
    t->t_spec = TICKS_SPEC_NONE;
    t->t_autonum = 6;
    t->t_inout = TICKS_IN;
    t->t_op = PLACEMENT_BOTH;
    t->props.size = grdefaults.charsize;
    t->mprops.size = grdefaults.charsize / 2;
    t->t_drawbar = TRUE;
    t->t_drawbarcolor = grdefaults.color;
    t->t_drawbarlines = grdefaults.lines;
    t->t_drawbarlinew = grdefaults.linew;
    t->props.gridflag = FALSE;
    t->mprops.gridflag = FALSE;
    t->props.color = grdefaults.color;
    t->props.lines = grdefaults.lines;
    t->props.linew = grdefaults.linew;
    t->mprops.color = grdefaults.color;
    t->mprops.lines = grdefaults.lines;
    t->mprops.linew = grdefaults.linew;
    t->nticks = 0;
    for (i = 0; i < MAX_TICKS; i++) {
        t->tloc[i].wtpos = 0.0;
        t->tloc[i].label = NULL;
    }
}
