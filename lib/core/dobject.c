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
 * DObject stuff
 *
 */

#include <string.h>

#include "grace/core.h"

static int object_odata_size(OType type)
{
    int size;

    switch (type) {
    case DO_LINE:
        size = sizeof(DOLineData);
        break;
    case DO_BOX:
        size = sizeof(DOBoxData);
        break;
    case DO_ARC:
        size = sizeof(DOArcData);
        break;
    default:
        size = 0;
    }
    
    return size;
}

DObject *object_data_new(void)
{
    DObject *o;

    o = xmalloc(sizeof(DObject));
    memset(o, 0, sizeof(DObject));
    if (o) {
        o->active = FALSE;
        
        o->type = DO_NONE;

        o->ap.x = 0.0;
        o->ap.y = 0.0;
    
        o->offset.x = 0.0;
        o->offset.y = 0.0;
        o->angle = 0.0;
    
        o->line.pen.color = 1;
        o->line.pen.pattern = 1;
        o->line.style = 1;
        o->line.width = 1.0;
        o->fillpen.color = 1;
        o->fillpen.pattern = 0;
        
        o->odata = NULL;
        
        /* o->bb = ; */
    }
    
    return o;
}

static int object_set_data(DObject *o, void *odata)
{
    int size = object_odata_size(o->type);
    
    if (!size) {
        return RETURN_FAILURE;
    }

    memcpy(o->odata, (void *) odata, size);
    
    return RETURN_SUCCESS;
}

DObject *object_data_copy(DObject *osrc)
{
    DObject *odest;
    void *odata;
    
    if (!osrc) {
        return NULL;
    }
    
    odest = object_data_new_complete(osrc->type);
    if (!odest) {
        return NULL;
    }
    
    /* Save odata pointer before memcpy overwrites it */
    odata = odest->odata;
    
    memcpy(odest, osrc, sizeof(DObject));
    
    /* Restore odata */
    odest->odata = odata;
    
    if (object_set_data(odest, osrc->odata) != RETURN_SUCCESS) {
        object_data_free(odest);
        return NULL;
    }
    
    return odest;
}

void object_data_free(DObject *o)
{
    if (o) {
        xfree(o->odata);
        xfree(o);
    }
}

static void set_default_arrow(Arrow *arrowp)
{
    arrowp->type = 0;
    arrowp->length = 1.0;
    arrowp->dL_ff = 1.0;
    arrowp->lL_ff = 1.0;
}

void *object_odata_new(OType type)
{
    void *odata;
    odata = xmalloc(object_odata_size(type));
    if (odata == NULL) {
        return NULL;
    }
    memset(odata, 0, object_odata_size(type));
    switch (type) {
    case DO_LINE:
        {
            DOLineData *l = (DOLineData *) odata;
            l->width  = 0.0;
            l->height = 0.0;
            l->arrow_end = 0; 
            set_default_arrow(&l->arrow);
        }
        break;
    case DO_BOX:
        {
            DOBoxData *b = (DOBoxData *) odata;
            b->width  = 0.0;
            b->height = 0.0;
        }
        break;
    case DO_ARC:
        {
            DOArcData *e = (DOArcData *) odata;
            e->width  = 0.0;
            e->height = 0.0;
            e->angle1 =   0.0;
            e->angle2 = 360.0;
            e->fillmode = ARCFILL_CHORD;
        }
        break;
    case DO_NONE:
        break;
    }
    
    return odata;
}

Quark *object_new(Quark *gr)
{
    Quark *o; 
    o = quark_new(gr, QFlavorDObject);
    return o;
}

DObject *object_data_new_complete(OType type)
{
    DObject *o;
    
    o = object_data_new();
    if (o) {
        o->active = TRUE;
        o->type   = type;
        o->odata  = object_odata_new(type);
        if (o->odata == NULL) {
            xfree(o);
            return NULL;
        }
    }
    
    return o;
}

Quark *object_new_complete(Quark *gr, OType type)
{
    Quark *q;
    DObject *o;
    
    q = object_new(gr);
    o = object_get_data(q);
    if (o) {
        o->active = TRUE;
        o->type   = type;
        o->odata  = object_odata_new(type);
        if (o->odata == NULL) {
            xfree(o);
            return NULL;
        }
    }
    
    return q;
}

DObject *object_get_data(const Quark *q)
{
    DObject *o;
    
    if (q && q->fid == QFlavorDObject) {
        o = (DObject *) q->data;
    } else {
        o = NULL;
    }
    
    return o;
}

int object_get_bb(DObject *o, view *bb)
{
    if (o) {
        *bb = o->bb;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


int object_set_active(Quark *q, int flag)
{
    if (q && q->fid == QFlavorDObject) {
        DObject *o = object_get_data(q);
        
        o->active = flag;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int object_set_angle(Quark *q, double angle)
{
    if (q && q->fid == QFlavorDObject) {
        DObject *o = object_get_data(q);
        
        o->angle = angle;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int object_set_offset(Quark *q, const VPoint *offset)
{
    if (q && q->fid == QFlavorDObject) {
        DObject *o = object_get_data(q);
        
        o->offset = *offset;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int object_set_line(Quark *q, const Line *line)
{
    if (q && q->fid == QFlavorDObject) {
        DObject *o = object_get_data(q);
        
        o->line = *line;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int object_set_fillpen(Quark *q, const Pen *pen)
{
    if (q && q->fid == QFlavorDObject) {
        DObject *o = object_get_data(q);
        
        o->fillpen = *pen;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int object_set_location(Quark *q, const APoint *ap)
{
    if (q && q->fid == QFlavorDObject) {
        DObject *o = object_get_data(q);
        
        o->ap      = *ap;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
