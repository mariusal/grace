/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000-2003 Grace Development Team
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

#include <stdlib.h>

#include "grace/canvas.h"
#include "plotone.h"
#include "objutils.h"

static void draw_object(Canvas *canvas, Quark *q)
{
    VPoint anchor;
    DObject *o = object_get_data(q);

    if (o == NULL || o->active == FALSE) {
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
    
    activate_bbox(canvas, BBOX_TYPE_TEMP, TRUE);
    reset_bbox(canvas, BBOX_TYPE_TEMP);

    switch (o->type) {
    case DO_BOX:
        {
            DOBoxData *b = (DOBoxData *) o->odata;
            if (o->angle == 0.0) {
                VPoint vp1, vp2;

                vp1.x = anchor.x - b->width/2;
                vp2.x = anchor.x + b->width/2;
                vp1.y = anchor.y - b->height/2;
                vp2.y = anchor.y + b->height/2;

                setpen(canvas, &o->fillpen);
                FillRect(canvas, &vp1, &vp2);

                setline(canvas, &o->line);
                DrawRect(canvas, &vp1, &vp2);
            } else {
                VPoint vps[4];
                double x, y, co, si;

                x = b->width/2;
                y = b->height/2;

                co = cos(M_PI/180.0*o->angle);
                si = sin(M_PI/180.0*o->angle);
                
                vps[0].x = anchor.x + x*co - y*si;
                vps[0].y = anchor.y + x*si + y*co;
                vps[1].x = anchor.x - x*co - y*si;
                vps[1].y = anchor.y - x*si + y*co;
                vps[2].x = anchor.x - x*co + y*si;
                vps[2].y = anchor.y - x*si - y*co;
                vps[3].x = anchor.x + x*co + y*si;
                vps[3].y = anchor.y + x*si - y*co;

                setpen(canvas, &o->fillpen);
                DrawPolygon(canvas, vps, 4);

                setline(canvas, &o->line);
                DrawPolyline(canvas, vps, 4, POLYLINE_CLOSED);
            }
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

            setpen(canvas, &o->fillpen);
            /* FIXME: implement true ellipse rotation! */
            DrawFilledArc(canvas, &vp1, &vp2,
                e->angle1 + o->angle, e->angle2 + o->angle,
                e->fillmode);

            setline(canvas, &o->line);
            DrawArc(canvas, &vp1, &vp2,
                e->angle1 + o->angle, e->angle2 + o->angle);
        }
        break;
    case DO_STRING:
        {
            DOStringData *s = (DOStringData *) o->odata;
            
            /* FIXME AA background setpen(o->fillpen); */

            setpen(canvas, &o->line.pen);
            setcharsize(canvas, s->size);
            setfont(canvas, s->font);

            WriteString(canvas, &anchor, o->angle, s->just, s->s);
        }
        break;
    case DO_LINE:
        {
            VPoint vp;
            DOLineData *l = (DOLineData *) o->odata;
            
            vp.x = anchor.x + l->length*cos(M_PI/180.0*o->angle);
            vp.y = anchor.y + l->length*sin(M_PI/180.0*o->angle);

            setline(canvas, &o->line);
            DrawLine(canvas, &anchor, &vp);

            switch (l->arrow_end) {
            case 0:
                break;
            case 1:
                draw_arrowhead(canvas, &vp, &anchor, &l->arrow,
                    &o->line.pen, &o->fillpen);
                break;
            case 2:
                draw_arrowhead(canvas, &anchor, &vp, &l->arrow,
                    &o->line.pen, &o->fillpen);
                break;
            case 3:
                draw_arrowhead(canvas, &vp, &anchor, &l->arrow,
                    &o->line.pen, &o->fillpen);
                draw_arrowhead(canvas, &anchor, &vp, &l->arrow,
                    &o->line.pen, &o->fillpen);
                break;
            }
        }
        break;
    case DO_NONE:
        break;
    }

    get_bbox(canvas, BBOX_TYPE_TEMP, &o->bb);
}


static int object_draw_hook(unsigned int step, void *data, void *udata)
{
    Quark *q = (Quark *) data;
    Canvas *canvas = (Canvas *) udata;
    
    draw_object(canvas, q);
    
    return TRUE;
}

void draw_objects(Canvas *canvas, Quark *gr)
{
    if (gr) {
        graph *g = (graph *) gr->data;
        /* disable (?) clipping for object drawing */
        setclipping(canvas, FALSE);

        storage_traverse(g->dobjects, object_draw_hook, canvas);

        setclipping(canvas, TRUE);
    }
}
