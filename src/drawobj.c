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

void draw_object(Canvas *canvas, Quark *q)
{
    VPoint anchor;
    DObject *o = object_get_data(q);
    int loctype;

    if (o == NULL || o->active == FALSE) {
        return;
    }
    
    loctype = object_get_loctype(q);
    if (loctype == COORD_WORLD) {
        WPoint wp;
        wp.x = o->ap.x;
        wp.y = o->ap.y;
        anchor = Wpoint2Vpoint(wp);
    } else
    if (loctype == COORD_FRAME) {
        FPoint fp;
        Quark *f = get_parent_frame(q);
        fp.x = o->ap.x;
        fp.y = o->ap.y;
        Fpoint2Vpoint(f, &fp, &anchor);
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
            int savebg;
            
            setcharsize(canvas, s->size);
            setfont(canvas, s->font);

            if (s->line.pen.pattern || s->fillpen.pattern) {
                view bbox;
                VPoint vp1, vp2;
                
                get_string_bbox(canvas, &anchor, o->angle, s->just, s->s, &bbox);
                view_extend(&bbox, 0.01);
                vp1.x = bbox.xv1;
                vp1.y = bbox.yv1;
                vp2.x = bbox.xv2;
                vp2.y = bbox.yv2;
                setpen(canvas, &s->fillpen);
                FillRect(canvas, &vp1, &vp2);
                setline(canvas, &s->line);
                DrawRect(canvas, &vp1, &vp2);
            }

            setpen(canvas, &o->line.pen);
            savebg = getbgcolor(canvas);
            /* If frame is filled with a solid color, alter bgcolor to
               make AA look good */
            if (s->fillpen.pattern == 1) {
                setbgcolor(canvas, s->fillpen.color);
            }
            WriteString(canvas, &anchor, o->angle, s->just, s->s);
            setbgcolor(canvas, savebg);
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
