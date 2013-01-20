/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2007 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
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
 * Draw axis bars, tick marks, and tick labels
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grace/plotP.h"

static void drawgrid(Quark *q, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr;
    tickmarks *t;
    gridprops gprops;
    int ttype;
    world w;
    view v;
    double wtpos;
    WPoint wp_grid_start, wp_grid_stop;
    VPoint vp_grid_start, vp_grid_stop;
    VPoint vpc, vp1, vp2;
    double phi_start, phi_stop, rho;
    double wc_start, wc_stop; /* world coordinates */
    int ittype_loop, itick;
        
    t = axisgrid_get_data(q);
    if (!t) {
        return;
    }
    
    gr = get_parent_graph(q);
    
    setclipping(canvas, TRUE);
    
    graph_get_viewport(gr, &v);
    graph_get_world(gr, &w);
    
    /* graph center; for polar plots */
    vpc.x = (v.xv1 + v.xv2)/2.0;
    vpc.y = (v.yv1 + v.yv2)/2.0;
    
    if (axisgrid_is_x(q)) { /* an X-axis */
        wc_start = w.xg1;
        wc_stop = w.xg2;
        wp_grid_start.y = w.yg1;
        wp_grid_stop.y = w.yg2;
    } else {              /* a Y-axis */
        wc_start = w.yg1;
        wc_stop = w.yg2;
        wp_grid_start.x = w.xg1;
        wp_grid_stop.x = w.xg2;
    }

    for (ittype_loop = 0; ittype_loop < 2; ittype_loop++) {
        if (ittype_loop == 0) { /* minor ticks */
            ttype = TICK_TYPE_MINOR;
            gprops = t->mgprops;
        } else {          /* major ticks */
            ttype = TICK_TYPE_MAJOR;
            gprops = t->gprops;
        }
        if (!gprops.onoff) {
            continue;
        }

        setline(canvas, &gprops.line);

        for (itick = 0; itick < t->nticks; itick++) {
            if (t->tloc[itick].type != ttype) {
                continue;
            }

            wtpos = t->tloc[itick].wtpos;

            if ((wtpos < wc_start) || (wtpos > wc_stop)) {
                continue;
            }

            if (axisgrid_is_x(q)) { /* an X-axis */
                wp_grid_start.x = wtpos;
                wp_grid_stop.x = wtpos;
            } else {              /* a Y-axis */
                wp_grid_start.y = wtpos;
                wp_grid_stop.y = wtpos;
            }

            Wpoint2Vpoint(gr, &wp_grid_start, &vp_grid_start);
            Wpoint2Vpoint(gr, &wp_grid_stop, &vp_grid_stop);


            if (!axisgrid_is_x(q) && graph_get_type(gr) == GRAPH_POLAR) {
                xy2polar(vp_grid_start.x - vpc.x, vp_grid_start.y - vpc.y,
                         &phi_start, &rho);
                xy2polar(vp_grid_stop.x - vpc.x, vp_grid_stop.y - vpc.y,
                         &phi_stop, &rho);
                vp1.x = vpc.x - rho;
                vp1.y = vpc.y + rho;
                vp2.x = vpc.x + rho;
                vp2.y = vpc.y - rho;
                if (graph_is_xinvert(gr) == TRUE) {
                    fswap(&phi_start, &phi_stop);
                }
                if (phi_stop < phi_start) {
                    phi_stop += 2*M_PI;
                } 
                DrawArc(canvas, &vp1, &vp2, 180.0/M_PI*phi_start,
                    180.0/M_PI*(phi_stop - phi_start), ARCCLOSURE_CHORD, FALSE);
            } else {
                DrawLine(canvas, &vp_grid_start, &vp_grid_stop);
            }
        }
    }
}

void draw_axis(Canvas *canvas, Quark *qa)
{
    Quark *ag, *gr;
    tickmarks *t;
    tickprops tprops;
    world w;
    view v, bb;
    double vbase1, vbase1_start, vbase1_stop;
    double vbase_tlabel, vbase_tlabel1;
    double tsize, tlsize, wtpos, vtpos;
    double tl_offset, tl_trans;
    WPoint wp1_start, wp1_stop;
    VPoint vp1_start, vp1_stop;
    VPoint vp_tick1_start, vp_tick1_stop;
    VPoint vp_tlabel;
    VPoint vpc, vp1, vp2;
    double phi_start, phi_stop, rho;
    VVector ort_para, ort_perp;
    double wc_start, wc_stop, wc_start_labels, wc_stop_labels; /* world
                                                                coordinates */
    int ittype_loop, itick, itcur;
    int ttype;
    char *tlabel = NULL;
    int tlabel1_just;
    
    int tick_dir_sign;
    
    double (*coord_conv) (const Quark *gr, double wx);

    ag = get_parent_axisgrid(qa);
    
    t = axisgrid_get_data(ag);
    if (!t) {
        return;
    }
    
    setclipping(canvas, FALSE);

    gr = get_parent_graph(qa);
    
    graph_get_viewport(gr, &v);
    graph_get_world(gr, &w);

    /* graph center; for polar plots */
    vpc.x = (v.xv1 + v.xv2)/2.0;
    vpc.y = (v.yv1 + v.yv2)/2.0;
    
       
    if (axis_get_position(qa) == AXIS_POS_OPPOSITE) {
        tick_dir_sign = -1;
    } else {
        tick_dir_sign = +1;
    }

    if (axisgrid_is_x(ag)) { /* an X-axis */
        ort_para.x = 1.0;
        ort_para.y = 0.0;
        ort_perp.x = 0.0;
        ort_perp.y = 1.0;

        coord_conv = xy_xconv;

        wc_start = w.xg1;
        wc_stop  = w.xg2;

        wp1_start.x = w.xg1;
        wp1_stop.x  = w.xg2;

        switch (axis_get_position(qa)) {
        case AXIS_POS_NORMAL:
            if (!graph_is_yinvert(gr)) {
                wp1_start.y = w.yg1;
                wp1_stop.y  = w.yg1;
            } else {
                wp1_start.y = w.yg2;
                wp1_stop.y  = w.yg2;
            }
            break;
        case AXIS_POS_OPPOSITE:
            if (!graph_is_yinvert(gr)) {
                wp1_start.y = w.yg2;
                wp1_stop.y  = w.yg2;
            } else {
                wp1_start.y = w.yg1;
                wp1_stop.y  = w.yg1;
            }
            break;
        case AXIS_POS_ZERO:
            if (w.yg1 <= 0.0 && w.yg2 >= 0.0) {
                wp1_start.y = 0.0;
                wp1_stop.y  = 0.0;
            } else {
                return;
            }
            break;
        }

        Wpoint2Vpoint(gr, &wp1_start, &vp1_start);
        Wpoint2Vpoint(gr, &wp1_stop,  &vp1_stop);

        /* TODO axis offset for polar plots */
        if (graph_get_type(gr) != GRAPH_POLAR) {
             vp1_start.y -= tick_dir_sign*axis_get_offset(qa);
             vp1_stop.y  -= tick_dir_sign*axis_get_offset(qa);
        }

        vbase1 = vp1_start.y;

        if (axis_get_position(qa) == AXIS_POS_OPPOSITE) {
            tlabel1_just = JUST_CENTER|JUST_BOTTOM;
        } else {
            tlabel1_just = JUST_CENTER|JUST_TOP;
        }

    } else {              /* a Y-axis */
        ort_para.x = 0.0;
        ort_para.y = 1.0;
        ort_perp.x = 1.0;
        ort_perp.y = 0.0;

        coord_conv = xy_yconv;

        wc_start = w.yg1;
        wc_stop  = w.yg2;

        wp1_start.y = w.yg1;
        wp1_stop.y  = w.yg2;

        switch (axis_get_position(qa)) {
        case AXIS_POS_NORMAL:
            if (!graph_is_xinvert(gr)) {
                wp1_start.x = w.xg1;
                wp1_stop.x  = w.xg1;
            } else {
                wp1_start.x = w.xg2;
                wp1_stop.x  = w.xg2;
            }
            break;
        case AXIS_POS_OPPOSITE:
            if (!graph_is_xinvert(gr)) {
                wp1_start.x = w.xg2;
                wp1_stop.x  = w.xg2;
            } else {
                wp1_start.x = w.xg1;
                wp1_stop.x  = w.xg1;
            }
            break;
        case AXIS_POS_ZERO:
            if (w.xg1 <= 0.0 && w.xg2 >= 0.0) {
                wp1_start.x = 0.0;
                wp1_stop.x  = 0.0;
            } else {
                return;
            }
            break;
        }

        Wpoint2Vpoint(gr, &wp1_start, &vp1_start);
        Wpoint2Vpoint(gr, &wp1_stop,  &vp1_stop);

        if (graph_get_type(gr) != GRAPH_POLAR) {
            vp1_start.x -= tick_dir_sign*axis_get_offset(qa);
            vp1_stop.x  -= tick_dir_sign*axis_get_offset(qa);
        }

        vbase1 = vp1_start.x;

        if (axis_get_position(qa) == AXIS_POS_OPPOSITE) {
            tlabel1_just = JUST_LEFT|JUST_MIDDLE;
        } else {
            tlabel1_just = JUST_RIGHT|JUST_MIDDLE;
        }
    }

    tl_trans  = t->tl_gap.x;
    tl_offset = t->tl_gap.y;

    activate_bbox(canvas, BBOX_TYPE_TEMP, TRUE);
    reset_bbox(canvas, BBOX_TYPE_TEMP);

    /* Make sure we don't end up with an empty BBox if nothing is drawn */
    update_bbox(canvas, BBOX_TYPE_TEMP, &vp1_start);
    update_bbox(canvas, BBOX_TYPE_TEMP, &vp1_stop);

    /* Begin axis bar stuff */
    if (axis_bar_enabled(qa)) {
        setline(canvas, &t->bar);

        if (axisgrid_is_x(ag) && graph_get_type(gr) == GRAPH_POLAR) {
            xy2polar(vp1_start.x - vpc.x, vp1_start.y - vpc.y,
                     &phi_start, &rho);
            xy2polar(vp1_stop.x - vpc.x, vp1_stop.y - vpc.y,
                     &phi_stop, &rho);
            vp1.x = vpc.x - rho;
            vp1.y = vpc.y + rho;
            vp2.x = vpc.x + rho;
            vp2.y = vpc.y - rho;
            if (graph_is_xinvert(gr) == TRUE) {
                fswap(&phi_start, &phi_stop);
            }
            if (phi_stop < phi_start) {
                phi_stop += 2*M_PI;
            } 
            DrawArc(canvas, &vp1, &vp2, 180.0/M_PI*phi_start,
                180.0/M_PI*(phi_stop - phi_start), ARCCLOSURE_CHORD, FALSE);
        } else {
            DrawLine(canvas, &vp1_start, &vp1_stop);
        }
    }
    /* End axis bar stuff*/


    /* TODO ticks, labels and axis labels for polar plots */
    if (graph_get_type(gr) == GRAPH_POLAR) {
        return;
    }
    
    vbase_tlabel1 = vbase1 - tick_dir_sign*tl_offset;

    /* Begin axis tick stuff */
    if (axis_ticks_enabled(qa)) {
        for (ittype_loop = 0; ittype_loop < 2; ittype_loop++) {

            if (ittype_loop == 0) { /* minor ticks */
                ttype = TICK_TYPE_MINOR;
                tprops = t->mprops;
            } else {      /* major ticks */
                ttype = TICK_TYPE_MAJOR;
                tprops = t->props;
            }
            tsize = 0.02 * tprops.size;

            switch (tprops.inout) {
            case TICKS_IN:
                vbase1_start = vbase1;
                vbase1_stop  = vbase1 + tick_dir_sign*tsize;
                vbase_tlabel1 = vbase1 - tick_dir_sign*tl_offset;
                break;
            case TICKS_OUT:
                vbase1_start = vbase1;
                vbase1_stop  = vbase1 - tick_dir_sign*tsize;
                vbase_tlabel1 = vbase1 - tick_dir_sign*(tsize + tl_offset);
                break;
            case TICKS_BOTH:
                vbase1_start = vbase1 - tsize;
                vbase1_stop  = vbase1 + tsize;
                vbase_tlabel1 = vbase1 - tick_dir_sign*(tsize + tl_offset);
                break;
            default:
                errmsg("Internal error in drawaxis()");
                return;
            }

            setline(canvas, &tprops.line);

            itcur = 0;
            for (itick = 0; itick < t->nticks; itick++) {
                if (t->tloc[itick].type != ttype) {
                    continue;
                }

                wtpos = t->tloc[itick].wtpos;

                if ((wtpos < wc_start) || (wtpos > wc_stop)) {
                    continue;
                }

                vtpos = coord_conv(gr, wtpos);
                vp_tick1_start.x = vtpos*ort_para.x + vbase1_start*ort_perp.x;
                vp_tick1_start.y = vtpos*ort_para.y + vbase1_start*ort_perp.y;
                vp_tick1_stop.x  = vtpos*ort_para.x + vbase1_stop*ort_perp.x;
                vp_tick1_stop.y  = vtpos*ort_para.y + vbase1_stop*ort_perp.y;
                DrawLine(canvas, &vp_tick1_start, &vp_tick1_stop);
                itcur++;
            }
        }
    }
    /* End axis ticks stuff */


    /* Begin tick label stuff */

    if (axis_labels_enabled(qa)) {
        if (t->tl_starttype == TYPE_SPEC) {
            wc_start_labels = t->tl_start;
        } else {
            wc_start_labels = wc_start;
        }

        if (t->tl_stoptype == TYPE_SPEC) {
            wc_stop_labels = t->tl_stop;
        } else {
            wc_stop_labels = wc_stop;
        }

        tlsize = 0.02*t->tl_tprops.charsize;

        itcur = 0;
        for (itick = 0; itick < t->nticks; itick++) {
            if (t->tloc[itick].type != TICK_TYPE_MAJOR) {
                continue;
            }

            wtpos = t->tloc[itick].wtpos;

            if ((wtpos < wc_start_labels) || (wtpos > wc_stop_labels)) {
                continue;
            }

            tlabel = copy_string(tlabel, t->tl_prestr);
            tlabel = concat_strings(tlabel, t->tloc[itick].label);
            tlabel = concat_strings(tlabel, t->tl_appstr);

            vtpos = coord_conv(gr, wtpos);

            if (itcur % (t->tl_skip + 1) == 0) {
                TextProps tprops = t->tl_tprops;

                vbase_tlabel = vbase_tlabel1 - (tl_offset + tlsize)*
                                        (itcur % (t->tl_staggered + 1));
                vp_tlabel.x = (vtpos + tl_trans)*ort_para.x +
                                               vbase_tlabel*ort_perp.x;
                vp_tlabel.y = (vtpos + tl_trans)*ort_para.y +
                                               vbase_tlabel*ort_perp.y;
                tprops.just = tlabel1_just;
                drawtext(canvas, &vp_tlabel, &tprops, NULL, tlabel, NULL); 
            }
            itcur++;
        }
    }
    
    xfree(tlabel);

    /* End tick label stuff */

    get_bbox(canvas, BBOX_TYPE_TEMP, &bb);
    axis_set_bb(qa, &bb);
}

static void calculate_tickgrid(Quark *q, plot_rt_t *plot_rt)
{
    Graal *g = plot_rt->graal;
    Quark *gr;
    int itick, imtick, itmaj;
    int nmajor;
    double swc_start, swc_stop, stmajor;
    int scale;
    double wtmaj;
    world w;
    tickmarks *t;
    int res;
    AMem *amem;
    
    t = axisgrid_get_data(q);

    if (!t) {
        return;
    }
    
    amem = quark_get_amem(q);

    gr = get_parent_graph(q);

    graph_get_world(gr, &w);
    
reenter:
    if (t->t_spec == TICKS_SPEC_NONE) {
        if (axisgrid_is_x(q)) {
            scale = graph_get_xscale(gr);
            if (scale == SCALE_LOG) {
                swc_start = fscale(w.xg1, scale);
                swc_stop  = fscale(w.xg2, scale);
            } else {
                swc_start = w.xg1;
                swc_stop  = w.xg2;
            }
        } else {
            scale = graph_get_yscale(gr);
            if (scale == SCALE_LOG) {
                swc_start = fscale(w.yg1, scale);
                swc_stop  = fscale(w.yg2, scale);
            } else {
                swc_start = w.yg1;
                swc_stop  = w.yg2;
            }
        }
        if (scale == SCALE_LOG) {
            stmajor = fscale(t->tmajor, scale);
        } else {
            stmajor = t->tmajor;
        }

        if (stmajor <= 0.0) {
            errmsg("Invalid major tick spacing, autoticking");
            axisgrid_autotick(q);
            goto reenter;
        }

        if (t->t_round == TRUE) {
            swc_start = floor(swc_start/stmajor)*stmajor;
        }

        nmajor = (int) ceil((swc_stop - swc_start) / stmajor + 1);
        t->nticks = (nmajor - 1)*(t->nminor + 1) + 1;

        if (t->nticks > MAX_TICKS) {
            errmsg("Too many ticks ( > MAX_TICKS ), autoticking");
            axisgrid_autotick(q);
            goto reenter;
        }

        itick = 0;
        itmaj = 0;
        while (itick < t->nticks) {
            if (scale == SCALE_LOG) {
                wtmaj = ifscale(swc_start + itmaj*stmajor, scale);
            } else {
                wtmaj = swc_start + itmaj*stmajor;
                if (t->t_round == TRUE && fabs(wtmaj) < 1.0e-6*stmajor) {
                    wtmaj = 0.0;
                }
            }
            t->tloc[itick].wtpos = wtmaj;
            t->tloc[itick].type = TICK_TYPE_MAJOR;

            itick++;
            for (imtick = 0; imtick < t->nminor && itick < t->nticks; imtick++) {
                if (scale == SCALE_LOG) {
                    t->tloc[itick].wtpos = wtmaj * (imtick + 2);
                } else {
                    t->tloc[itick].wtpos = wtmaj + (imtick + 1)*stmajor/(t->nminor + 1);
                }
                t->tloc[itick].type = TICK_TYPE_MINOR;
                AMEM_CFREE(amem, t->tloc[itick].label);
                itick++;
            }
            itmaj++;
        }
    }

    if (t->t_spec != TICKS_SPEC_BOTH) {
        nmajor = 0;
        for (itick = 0; itick < t->nticks; itick++) {
            if (t->tloc[itick].type == TICK_TYPE_MAJOR) {
                nmajor++;
            }
        }
        if (!string_is_empty(t->tl_formula)) {
            DArray *da = darray_new(nmajor);

            for (itick = 0, itmaj = 0; itick < t->nticks; itick++) {
                if (t->tloc[itick].type == TICK_TYPE_MAJOR) {
                    darray_set_val(da, itmaj, t->tloc[itick].wtpos);
                    itmaj++;
                }
            }

            res = graal_transform_arr(g, t->tl_formula, "$t", da, q);
            if (res != RETURN_SUCCESS) {
                errmsg("Error in tick transformation formula");
                darray_free(da);
                return;
            }

            for (itick = 0, itmaj = 0; itick < t->nticks; itick++) {
                if (t->tloc[itick].type == TICK_TYPE_MAJOR) {
                    darray_get_val(da, itmaj, &wtmaj);
                    t->tloc[itick].label = amem_strcpy(amem,
                        t->tloc[itick].label, 
                        create_fstring(get_parent_project(q),
                            &t->tl_format,
                            wtmaj, LFORMAT_TYPE_EXTENDED));
                    itmaj++;
                }
            }
            
            darray_free(da);
        } else {
            for (itick = 0; itick < t->nticks; itick++) {
                if (t->tloc[itick].type == TICK_TYPE_MAJOR) {
                    t->tloc[itick].label = amem_strcpy(amem,
                        t->tloc[itick].label, 
                        create_fstring(get_parent_project(q),
                            &t->tl_format,
                            t->tloc[itick].wtpos, LFORMAT_TYPE_EXTENDED));
                }
            }
        }
    }
}

void draw_axisgrid(Quark *q, plot_rt_t *plot_rt)
{
    tickmarks *t = axisgrid_get_data(q);
    if (t && graph_get_type(get_parent_graph(q)) != GRAPH_PIE) {
        /* calculate tick mark positions */
        calculate_tickgrid(q, plot_rt);
        
        /* draw grid lines */
        drawgrid(q, plot_rt);
    }
}
