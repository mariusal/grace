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
 * Frame stuff
 *
 */

#include <config.h>

#include <string.h>

#include "grace.h"
#include "graphs.h"
#include "protos.h"

Quark *frame_new(Quark *project)
{
    Quark *f; 
    f = quark_new(project, QFlavorFrame);
    return f;
}

frame *frame_data_new(void)
{
    frame *f;
    
    f = xmalloc(sizeof(frame));
    if (f) {
        memset(f, 0, sizeof(frame));
        set_default_frame(f);
    }
    return f;
}

void frame_data_free(frame *f)
{
    if (f) {
        xfree(f->labs.title.s);
        xfree(f->labs.stitle.s);
        xfree(f);
    }
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

    /* duplicate allocatable storage */
    f_new->labs.title.s = copy_string(NULL, f->labs.title.s);
    f_new->labs.stitle.s = copy_string(NULL, f->labs.stitle.s);
    
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

view *frame_get_view(Quark *q)
{
    frame *f = frame_get_data(q);
    if (f) {
        return &f->v;
    } else {
        return NULL;
    }
}

labels *frame_get_labels(Quark *q)
{
    frame *f = frame_get_data(q);
    if (f) {
        return &f->labs;
    } else {
        return NULL;
    }
}

legend *frame_get_legend(Quark *q)
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

int frame_set_view(Quark *q, const view *v)
{
    frame *f = frame_get_data(q);

    if (f) {
        f->v = *v;
        quark_dirtystate_set(q, TRUE);
    
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_title(Quark *q, const plotstr *s)
{
    frame *f = frame_get_data(q);

    if (f) {
        xfree(f->labs.title.s);
        memcpy(&f->labs.title, s, sizeof(plotstr));
        f->labs.title.s = copy_string(NULL, s->s);

        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_stitle(Quark *q, const plotstr *s)
{
    frame *f = frame_get_data(q);

    if (f) {
        xfree(f->labs.stitle.s);
        memcpy(&f->labs.stitle, s, sizeof(plotstr));
        f->labs.stitle.s = copy_string(NULL, s->s);

        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int frame_set_labels(Quark *q, const labels *labs)
{
    frame *f = frame_get_data(q);

    if (f) {
        xfree(f->labs.title.s);
        xfree(f->labs.stitle.s);
        memcpy(&f->labs, labs, sizeof(labels));
        f->labs.title.s = copy_string(NULL, labs->title.s);
        f->labs.stitle.s = copy_string(NULL, labs->stitle.s);

        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
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
