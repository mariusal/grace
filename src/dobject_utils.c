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
 * operations on objects (strings, lines, boxes, and arcs)
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "grace/core.h"
#include "core_utils.h"

char *object_types(OType type)
{
    char *s = "";
    switch (type) {
    case DO_LINE:
        s = "line";
        break;
    case DO_BOX:
        s = "box";
        break;
    case DO_ARC:
        s = "arc";
        break;
    case DO_NONE:
        s = "none";
        break;
    }
    
    return s;
}


void move_object(Quark *q, VVector shift)
{
#if 0
    DObject *o = object_get_data(q);
    
    if (!o) {
        return;
    }

    if (object_get_loctype(q) == COORD_VIEW) {
        o->ap.x += shift.x;
        o->ap.y += shift.y;
    } else {
        WPoint wp;
        VPoint vp;
        Quark *gr = get_parent_graph(q);
        
        wp.x = o->ap.x;
        wp.y = o->ap.y;
        
        Wpoint2Vpoint(gr, &wp, &vp);
        vp.x += shift.x;
        vp.y += shift.y;
        
        Vpoint2Wpoint(gr, &vp, &wp);
        o->ap.x = wp.x;
        o->ap.y = wp.y;
    }
    
    quark_dirtystate_set(q, TRUE);
#endif
}

int object_place_at_vp(Quark *q, VPoint vp)
{
#if 0
    DObject *o = object_get_data(q);

    if (!o) {
        return RETURN_FAILURE;
    }
    
    if (object_get_loctype(q) == COORD_VIEW) {
        o->ap.x = vp.x;
        o->ap.y = vp.y;
    } else {
        Vpoint2Wpoint(vp.x, vp.y, &o->ap.x, &o->ap.y);
    }
    
    quark_dirtystate_set(q, TRUE);
#endif
    return RETURN_SUCCESS;
}


int object_get_loctype(const Quark *q)
{
    Quark *p = (Quark *) q;
    
    while (p) {
        p = quark_parent_get(p);
        if (p->fid == QFlavorGraph) {
            return COORD_WORLD;
        } else
        if (p->fid == QFlavorFrame) {
            return COORD_FRAME;
        } else
        if (p->fid == QFlavorProject) {
            return COORD_VIEW;
        }
    }
    
    /* default */
    return COORD_VIEW;
}

int Apoint2Vpoint(const Quark *q, const APoint *ap, VPoint *vp)
{
    int loctype = object_get_loctype(q);
    if (loctype == COORD_WORLD) {
        WPoint wp;
        Quark *gr = get_parent_graph(q);
        wp.x = ap->x;
        wp.y = ap->y;
        
        if (!is_validWPoint(gr, &wp)) {
            return RETURN_FAILURE;
        }
        
        Wpoint2Vpoint(gr, &wp, vp);
    } else
    if (loctype == COORD_FRAME) {
        FPoint fp;
        Quark *f = get_parent_frame(q);
        fp.x = ap->x;
        fp.y = ap->y;
        Fpoint2Vpoint(f, &fp, vp);
    } else {
        vp->x = ap->x;
        vp->y = ap->y;
    }
    
    return RETURN_SUCCESS;
}
