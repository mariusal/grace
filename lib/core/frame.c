/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2003,2004 Grace Development Team
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
 * Frame stuff
 *
 */

#include <string.h>

#include "grace/coreP.h"

static void set_default_legend(legend *l, const defaults *grdefaults)
{
    l->active = TRUE;
    
    l->acorner = CORNER_UR;
    l->offset.x = 0.05;
    l->offset.y = 0.05;
    
    l->boxline    = grdefaults->line;
    l->boxfillpen = grdefaults->fillpen;
    
    l->singlesym = FALSE;
    l->vgap = 0.01;
    l->hgap = 0.01;
    l->len = 0.04;
    l->invert = FALSE;
    
    l->font = grdefaults->font;
    l->charsize = grdefaults->charsize;
    l->color = grdefaults->line.pen.color;
    
    l->bb.xv1 = l->bb.xv2 = l->bb.yv1 = l->bb.yv2 = 0.0;
}

static void set_default_frame(Quark *q)
{
    frame *f = frame_get_data(q);
    defaults grdefaults;
    static const view d_v = {0.15, 0.85, 0.15, 0.85};
    Project *pr = project_get_data(get_parent_project(q));
    
    if (!f || !pr) {
        return;
    }
    
    pr = project_get_data(get_parent_project(q));
    grdefaults = pr->grdefaults;
    
    f->active = TRUE;
    f->type = 0;                /* frame type */
    f->outline = grdefaults.line;
    f->fillpen = grdefaults.fillpen;

    memcpy(&f->v, &d_v, sizeof(view));
    set_default_legend(&f->l, &grdefaults);
}

Quark *frame_new(Quark *project)
{
    Quark *f; 
    f = quark_new(project, QFlavorFrame);
    set_default_frame(f);
    return f;
}

frame *frame_data_new(void)
{
    frame *f;
    
    f = xmalloc(sizeof(frame));
    if (f) {
        memset(f, 0, sizeof(frame));
    }
    return f;
}

void frame_data_free(frame *f)
{
    xfree(f);
}

frame *frame_data_copy(frame *f)
{
    frame *f_new;
    
    if (!f) {
        return NULL;
    }
    
    f_new = xmalloc(sizeof(frame));
    if (!f_new) {
        return NULL;
    }
    
    memcpy(f_new, f, sizeof(frame));

    return f_new;
}

frame *frame_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorFrame) {
        return (frame *) q->data;
    } else {
        return NULL;
    }
}

int frame_get_view(const Quark *q, view *v)
{
    frame *f = frame_get_data(q);
    if (f) {
        *v = f->v;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

legend *frame_get_legend(const Quark *q)
{
    frame *f = frame_get_data(q);
    
    if (f) {
        return &f->l;
    } else {
        return NULL;
    }
}

int frame_is_active(const Quark *q)
{
    frame *f = frame_get_data(q);
    if (f) {
        return f->active;
    } else {
        return FALSE;
    }
}

int frame_set_active(Quark *q, int flag)
{
    frame *f = frame_get_data(q);
    if (f) {
        f->active = flag;
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_type(Quark *q, int type)
{
    frame *f = frame_get_data(q);

    if (f) {
        f->type = type;
        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_outline(Quark *q, const Line *line)
{
    frame *f = frame_get_data(q);

    if (f) {
        f->outline = *line;
        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_fillpen(Quark *q, const Pen *pen)
{
    frame *f = frame_get_data(q);

    if (f) {
        f->fillpen = *pen;
        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int update_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (q->fid == QFlavorGraph) {
        closure->descend = FALSE;
        update_graph_ccache(q);
    }

    return TRUE;
}

int frame_set_view(Quark *q, const view *v)
{
    frame *f = frame_get_data(q);

    if (f) {
        /* Safety checks */
        if (!isvalid_viewport(v)) {
            errmsg("Invalid viewport");
            return RETURN_FAILURE;
        } else {
            f->v = *v;
            quark_traverse(q, update_hook, NULL);
            quark_dirtystate_set(q, TRUE);

            return RETURN_SUCCESS;
        }
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_legend(Quark *q, const legend *leg)
{
    frame *f = frame_get_data(q);

    if (f) {
        if (&f->l != leg) {
            memcpy(&f->l, leg, sizeof(legend));
        }

        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

Quark *get_parent_frame(const Quark *q)
{
    Quark *p = (Quark *) q;
    
    while (p) {
        p = quark_parent_get(p);
        if (p->fid == QFlavorFrame) {
            return p;
        }
    }
    
    return NULL;
}

int frame_shift(Quark *q, const VVector *vshift)
{
    view v;

    if (frame_get_view(q, &v) == RETURN_SUCCESS) {
        v.xv1 += vshift->x;
        v.xv2 += vshift->x;
        v.yv1 += vshift->y;
        v.yv2 += vshift->y;

        return frame_set_view(q, &v);
    } else {
        return RETURN_FAILURE;
    }
}

int frame_legend_shift(Quark *q, const VVector *vshift)
{
    legend *l = frame_get_legend(q);

    if (l) {
        switch (l->acorner) {
        case CORNER_LL:
            l->offset.x += vshift->x;
            l->offset.y += vshift->y;
            break;
        case CORNER_UL:
            l->offset.x += vshift->x;
            l->offset.y -= vshift->y;
            break;
        case CORNER_UR:
            l->offset.x -= vshift->x;
            l->offset.y -= vshift->y;
            break;
        case CORNER_LR:
            l->offset.x -= vshift->x;
            l->offset.y += vshift->y;
            break;
        }

        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
