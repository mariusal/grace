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
 * operations on objects (strings, lines, and boxes)
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

#include "graphs.h"
#include "utils.h"
#include "objutils.h"

DObject *object_new(OType type)
{
    DObject *o;
    
    o = xmalloc(sizeof(DObject));
    if (o) {
        o->type = type;
        o->gno = get_cg();

        o->loctype = COORD_VIEW;
        o->ap.x = 0.0;
        o->ap.y = 0.0;
    
        o->offset.x = 0.0;
        o->offset.y = 0.0;
        o->angle = 0.0;
    
        o->pen.color = 1;
        o->pen.pattern = 1;
        o->lines = 1;
        o->linew = 1.0;
        o->fillpen.color = 1;
        o->fillpen.pattern = 1;

        switch (o->type) {
        case DO_LINE:
            {
                DOLineData *l;
                l = xmalloc(sizeof(DOLineData));
                l->length    = 0.0;
                l->arrow_end = 0; 
                set_default_arrow(&l->arrow);
                o->odata = (void *) l;
            }
            break;
        case DO_BOX:
            {
                DOBoxData *b;
                b = xmalloc(sizeof(DOBoxData));
                b->width  = 0.0;
                b->height = 0.0;
                o->odata = (void *) b;
            }
            break;
        case DO_ARC:
            {
                DOArcData *e;
                e = xmalloc(sizeof(DOArcData));
                e->width  = 0.0;
                e->height = 0.0;
                e->angle1 =   0.0;
                e->angle2 = 360.0;
                e->fillmode = ARCFILL_CHORD;
                o->odata = (void *) e;
            }
            break;
        case DO_STRING:
            {
                DOStringData *s;
                s = xmalloc(sizeof(DOStringData));
                s->s    = NULL;
                s->font = 0;
                s->just = 0;
                s->size = 1.0;
                o->odata = (void *) s;
            }
            break;
        }
    
        /* o->bb = ; */
    }
    
    return o;
}

void object_free(DObject *o)
{
    if (o) {
        xfree(o->odata);
        xfree(o);
    }
}

int next_object(OType type)
{
    int id;
    DObject *o;
    
    id = storage_get_unique_id(objects);
    if (id >= 0) {
        o = object_new(type);
        if (o) {
            if (storage_add(objects, id, (void *) o) != RETURN_SUCCESS) {
                object_free(o);
                id = -1;
            }
        } else {
            id = -1;
        }
    }
    
    return id;
}

DObject *object_get(int id)
{
    DObject *o;
    
    if (storage_get_data_by_id(objects, id, (void **) &o) != RETURN_SUCCESS) {
        o = NULL;
    }
    
    return o;
}

void do_clear_objects(void)
{
    storage_empty(objects);
}

void do_clear_lines(void){}
void do_clear_boxes(void){}
void do_clear_ellipses(void){}
void do_clear_text(void){}

void set_plotstr_string(plotstr *pstr, char *buf)
{
    pstr->s = copy_string(pstr->s, buf);
}

int get_object_bb(DObject *o, view *bb)
{
    if (o) {
        *bb = o->bb;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int kill_object(int id)
{
    return storage_delete_by_id(objects, id);
}

void copy_object(int type, int from, int to){}
int duplicate_object(int type, int id){return 0;}

void move_object(int type, int id, VVector shift){}

void init_string(int id, VPoint vp){}
void init_line(int id, VPoint vp1, VPoint vp2){}
void init_box(int id, VPoint vp1, VPoint vp2){}
void init_ellipse(int id, VPoint vp1, VPoint vp2){}

int isactive_object(DObject *o)
{
    return o->active;
}
