/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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
 * Object drawing
 */

#include <config.h>
#include <cmath.h>

#include <stdlib.h>

#include "draw.h"
#include "plotone.h"
#include "objutils.h"

static void draw_object(int gno, DObject *o)
{
    VPoint anchor;

    if (o == NULL || o->active == FALSE) {
        return;
    }
    
    if (o->loctype == COORD_VIEW && gno != -1) {
        return;
    }
    if (o->loctype == COORD_WORLD && o->gno != gno) {
        return;
    }
    
    if (o->loctype == COORD_WORLD) {
        WPoint wp;
        wp.x = o->ap.x;
        wp.y = o->ap.y;
        anchor = Wpoint2Vpoint(wp);
    } else {
        anchor.x = o->ap.x;
        anchor.y = o->ap.y;
    }
    anchor.x += o->offset.x;
    anchor.y += o->offset.y;
    
    activate_bbox(BBOX_TYPE_TEMP, TRUE);
    reset_bbox(BBOX_TYPE_TEMP);

    switch (o->type) {
    case DO_BOX:
        {
            VPoint vp1, vp2;
            DOBoxData *b = (DOBoxData *) o->odata;
            
            vp1.x = anchor.x - b->width/2;
            vp2.x = anchor.x + b->width/2;
            vp1.y = anchor.y - b->height/2;
            vp2.y = anchor.y + b->height/2;

            setpen(o->fillpen);
            FillRect(vp1, vp2);

            setpen(o->pen);
            setlinewidth(o->linew);
            setlinestyle(o->lines);
            DrawRect(vp1, vp2);
        }
        break;
    case DO_ARC:
        {
            VPoint vp1, vp2;
            DOArcData *e = (DOArcData *) o->odata;
            
            vp1.x = anchor.x - e->width/2;
            vp2.x = anchor.x + e->width/2;
            vp1.y = anchor.y - e->height/2;
            vp2.y = anchor.y + e->height/2;

            setpen(o->fillpen);
            DrawFilledArc(vp1, vp2, (int) e->angle1, (int) e->angle2, e->fillmode);

            setpen(o->pen);
            setlinewidth(o->linew);
            setlinestyle(o->lines);
            DrawArc(vp1, vp2, (int) e->angle1, (int) e->angle2);
        }
        break;
    case DO_STRING:
        {
            DOStringData *s = (DOStringData *) o->odata;
            
            /* FIXME AA background setpen(o->fillpen); */

            setpen(o->pen);
            setcharsize(s->size);
            setfont(s->font);

            WriteString(anchor, 180.0/M_PI*o->angle, s->just, s->s);
        }
        break;
    case DO_LINE:
        {
            VPoint vp;
            DOLineData *l = (DOLineData *) o->odata;
            
            vp.x = anchor.x + l->length*cos(o->angle);
            vp.y = anchor.y + l->length*sin(o->angle);

            setpen(o->pen);
            setlinewidth(o->linew);
            setlinestyle(o->lines);
            DrawLine(anchor, vp);

            switch (l->arrow_end) {
            case 0:
                break;
            case 1:
                draw_arrowhead(vp, anchor, &l->arrow);
                break;
            case 2:
                draw_arrowhead(anchor, vp, &l->arrow);
                break;
            case 3:
                draw_arrowhead(vp, anchor, &l->arrow);
                draw_arrowhead(anchor, vp, &l->arrow);
                break;
            }
        }
        break;
    }

    o->bb = get_bbox(BBOX_TYPE_TEMP);
}

void draw_objects(int gno)
{
    int i, n;
    int *ids;
                                
    /* disable (?) clipping for object drawing */
    setclipping(FALSE);
    
    n = get_object_ids(&ids);
    for (i = 0; i < n; i++) {
        DObject *o;
        
        o = object_get(ids[i]);
        draw_object(gno, o);
    }
    
    setclipping(TRUE);
}
