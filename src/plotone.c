/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
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
 * plotone.c - entry for graphics
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "grace/canvas.h"

#include "utils.h"
#include "files.h"
#include "core_utils.h"
#include "plotone.h"
#include "protos.h"

static int plotone_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    plot_rt_t *plot_rt = (plot_rt_t *) udata;
    Canvas *canvas = plot_rt->canvas;
    
    switch (q->fid) {
    case QFlavorProject:
        if (!closure->post) {
            set_draw_mode(canvas, TRUE);   
            
            closure->post = TRUE;
        }
        break;
    case QFlavorFrame:
        if (!frame_is_active(q)) {
            closure->descend = FALSE;
            break;
        }
        if (!closure->post) {
            /* fill frame */
            fillframe(canvas, q);
    
            closure->post = TRUE;
        } else {
            draw_frame(q, plot_rt);
        }
        break;
    case QFlavorGraph:
        if (!closure->post) {
            /* FIXME: stacked graphs */
            plot_rt->last_pass  = TRUE;
            plot_rt->first_pass = TRUE;
            
            plot_rt->refn = 0;
            plot_rt->refx = NULL;
            plot_rt->refy = NULL;

            if (draw_graph(q, plot_rt) != RETURN_SUCCESS) {
                closure->descend = FALSE;
            }

            closure->post = TRUE;
        } else {
            XCFREE(plot_rt->refy);

            /* mark the reference points only if in interactive mode */
            if (terminal_device(plot_rt->canvas) == TRUE) {
                draw_ref_point(plot_rt->canvas, q);
            }
        }
        break;
    case QFlavorSet:
        draw_set(q, plot_rt);
        break;
    case QFlavorAxis:
        draw_axis(canvas, q);
        break;
    case QFlavorDObject:
        draw_object(canvas, q);
        break;
    case QFlavorAText:
        draw_atext(canvas, q);
        break;
    case QFlavorRegion:
        draw_region(canvas, q);
        break;
    }
    
    return TRUE;
}

static void dproc(Canvas *canvas, void *data)
{
    Quark *project = (Quark *) data;
    plot_rt_t plot_rt;
    
    plot_rt.canvas = canvas;

    quark_traverse(project, plotone_hook, &plot_rt);
}

/*
 * draw all active graphs
 */
void drawgraph(const Quark *project)
{
    Project *pr = project_get_data(project);
    RunTime *rt = rt_from_quark(project);

    if (pr && rt) {
        canvas_set_docname(rt->canvas, project_get_docname(project));
        canvas_set_pagefill(rt->canvas, pr->bgfill);
        setbgcolor(rt->canvas, pr->bgcolor);
        canvas_set_fontsize_scale(rt->canvas,  pr->fscale);
        canvas_set_linewidth_scale(rt->canvas, pr->lscale);

        canvas_draw(rt->canvas, dproc, (void *) project);
    }
}

/*
 * If writing to a file, check to see if it exists
 */
void do_hardcopy(const Quark *project)
{
    Grace *grace = grace_from_quark(project);
    RunTime *rt;
    Canvas *canvas;
    char tbuf[128], *s;
    char fname[GR_MAXPATHLEN];
    view v;
    double vx, vy;
    int truncated_out;
    FILE *prstream;
    
    if (!grace) {
        return;
    }
    
    rt = grace->rt;
    canvas = rt->canvas;
    
    if (get_ptofile(grace)) {
        if (is_empty_string(rt->print_file)) {
            Device_entry *dev = get_device_props(canvas, rt->hdevice);
            sprintf(rt->print_file, "%s.%s",
                get_docbname(project), dev->fext);
        }
        strcpy(fname, rt->print_file);
    } else {
        s = get_print_cmd(grace);
        if (is_empty_string(s)) {
            errmsg("No print command defined, output aborted");
            return;
        }
        tmpnam(fname);
        /* VMS doesn't like extensionless files */
        strcat(fname, ".prn");
    }
    
    prstream = grace_openw(grace, fname);
    if (prstream == NULL) {
        return;
    }
    
    canvas_set_prstream(canvas, prstream); 
    
    select_device(canvas, rt->hdevice);
    
    drawgraph(project);
    
    grace_close(prstream);
    
    get_bbox(canvas, BBOX_TYPE_GLOB, &v);
    project_get_viewport(project, &vx, &vy);
    if (v.xv1 < 0.0 || v.xv2 > vx || v.yv1 < 0.0 || v.yv2 > vy) {
        truncated_out = TRUE;
    } else {
        truncated_out = FALSE;
    }
    
    if (get_ptofile(grace) == FALSE) {
        sprintf(tbuf, "%s %s", get_print_cmd(grace), fname);
        if (truncated_out == FALSE ||
            yesno("Printout is truncated. Continue?", NULL, NULL, NULL)) {
            system_wrap(tbuf);
        }
#ifndef PRINT_CMD_UNLINKS
        unlink(fname);
#endif
    } else {
        if (truncated_out == TRUE) {
            errmsg("Output is truncated - tune device dimensions");
        }
    }
}


int draw_graph(Quark *gr, plot_rt_t *plot_rt)
{
    GraphType gtype;
    
    if (!graph_is_active(gr)) {
        return RETURN_FAILURE;
    }

    plot_rt->ndsets = 0;
    plot_rt->offset = 0.0;

    if (plot_rt->first_pass) {
        
        gtype = graph_get_type(gr);

        if (gtype == GRAPH_CHART) {
            int setno, nsets;
            Quark **psets;

            nsets = get_descendant_sets(gr, &psets);
            for (setno = 0; setno < nsets; setno++) {
                Quark *pset = psets[setno];
                if (is_set_drawable(pset)) {
                    set *p = set_get_data(pset);
                    if (set_get_length(pset) > plot_rt->refn) {
                        plot_rt->refn = set_get_length(pset);
                        plot_rt->refx = getx(pset);
                    }
                    if (graph_is_stacked(gr) != TRUE) {
                        plot_rt->offset -= 0.5*0.02*p->sym.size;
                    }
                }
            }
            plot_rt->offset -= 0.5*(number_of_active_sets(gr) - 1)*graph_get_bargap(gr);

            if (graph_is_stacked(gr) == TRUE) {
                plot_rt->refy = xcalloc(plot_rt->refn, SIZEOF_DOUBLE);
                if (plot_rt->refy == NULL) {
                    return FALSE;
                }
            }


            if (plot_rt->refx) {
                double xmin, xmax;
                int imin, imax;
                minmax(plot_rt->refx, plot_rt->refn, &xmin, &xmax, &imin, &imax);
                plot_rt->epsilon = 1.0e-3*(xmax - xmin)/plot_rt->refn;
            } else {
                plot_rt->epsilon = 0.0;
            }

        }
   
    } else
    if (plot_rt->refy) {
        int j;
        for (j = 0; j < plot_rt->refn; j++) {
            plot_rt->refy[j] = 0.0;
        }
    }
    
    return RETURN_SUCCESS;
}

void draw_set(Quark *pset, plot_rt_t *plot_rt)
{
    Quark *gr;
    int x_ok;
    double *x;
    int j;
    set *p;
    int gtype;


    if (!is_set_drawable(pset)) {
        return;
    }

    gr = get_parent_graph(pset);
    
    p = set_get_data(pset);

    /* draw sets */
    gtype = graph_get_type(gr);
    switch (gtype) {
    case GRAPH_XY:
        switch (set_get_type(pset)) {
        case SET_XY:
        case SET_XYSIZE:
        case SET_XYCOLOR:
        case SET_XYZ:
            drawsetline(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_BAR:
            drawsetline(pset, plot_rt);
            drawsetbars(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_XYDX:
        case SET_XYDY:
        case SET_XYDXDX:
        case SET_XYDYDY:
        case SET_XYDXDY:
        case SET_XYDXDXDYDY:
            drawsetline(pset, plot_rt);
            drawseterrbars(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_XYHILO:
            drawsethilo(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_XYVMAP:
            drawsetline(pset, plot_rt);
            drawsetvmap(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_BOXPLOT:
            drawsetline(pset, plot_rt);
            drawsetboxplot(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        default:
            errmsg("Unsupported in XY graph set type");
            break;
        }
        break;
    case GRAPH_FIXED:
        switch (set_get_type(pset)) {
        case SET_XY:
        case SET_XYSIZE:
        case SET_XYCOLOR:
        case SET_XYZ:
            drawsetline(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_XYDX:
        case SET_XYDY:
        case SET_XYDXDX:
        case SET_XYDYDY:
        case SET_XYDXDY:
        case SET_XYDXDXDYDY:
            drawsetline(pset, plot_rt);
            drawseterrbars(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_XYR:
            drawcirclexy(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        case SET_XYVMAP:
            drawsetline(pset, plot_rt);
            drawsetvmap(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        default:
            errmsg("Unsupported in XY graph set type");
            break;
        }
        break;
    case GRAPH_POLAR:
        switch (set_get_type(pset)) {
        case SET_XY:
        case SET_XYSIZE:
        case SET_XYCOLOR:
        case SET_XYZ:
            drawsetline(pset, plot_rt);
            drawsetsyms(pset, plot_rt);
            drawsetavalues(pset, plot_rt);
            break;
        default:
            errmsg("Unsupported in polar graph set type");
            break;
        }
        break;
    case GRAPH_PIE:
        switch (set_get_type(pset)) {
        case SET_XY:
        case SET_XYCOLOR:
        case SET_XYCOLPAT:
            draw_pie_chart_set(pset, plot_rt);
            break;
        default:
            errmsg("Unsupported in pie chart set type");
            break;
        }
        break;
    case GRAPH_CHART:
        /* check that abscissas are identical with refx */
        x = set_get_col(pset, DATA_X);
        x_ok = TRUE;
        for (j = 0; j < set_get_length(pset); j++) {
            if (fabs(x[j] - plot_rt->refx[j]) > plot_rt->epsilon) {
                x_ok = FALSE;
                break;
            }
        }
        if (x_ok != TRUE) {
            char buf[128];
            sprintf(buf, "Set %s has different abscissas, "
                         "skipped from the chart.",
                         quark_idstr_get(pset));
            errmsg(buf);
            return;
        }

        if (graph_is_stacked(gr) != TRUE) {
            plot_rt->offset += 0.5*0.02*p->sym.size;
        }

        switch (set_get_type(pset)) {
        case SET_XY:
        case SET_XYSIZE:
        case SET_XYCOLOR:
            if (plot_rt->first_pass) {
                drawsetline(pset, plot_rt);
            }
            if (plot_rt->last_pass) {
                drawsetsyms(pset, plot_rt);
                drawsetavalues(pset, plot_rt);
            }
            break;
        case SET_BAR:
            if (plot_rt->first_pass) {
                drawsetline(pset, plot_rt);
                drawsetbars(pset, plot_rt);
            }
            if (plot_rt->last_pass) {
                drawsetavalues(pset, plot_rt);
            }
            break;
        case SET_BARDY:
        case SET_BARDYDY:
            if (plot_rt->first_pass) {
                drawsetline(pset, plot_rt);
                drawsetbars(pset, plot_rt);
            }
            if (plot_rt->last_pass) {
                drawseterrbars(pset, plot_rt);
                drawsetavalues(pset, plot_rt);
            }
            break;
        case SET_XYDY:
        case SET_XYDYDY:
            if (plot_rt->first_pass) {
                drawsetline(pset, plot_rt);
            }
            if (plot_rt->last_pass) {
                drawseterrbars(pset, plot_rt);
                drawsetsyms(pset, plot_rt);
                drawsetavalues(pset, plot_rt);
            }
            break;
        default:
            if (plot_rt->first_pass) {
                errmsg("Unsupported in XY chart set type");
            }
            break;
        }

        if (graph_is_stacked(gr) != TRUE) {
            plot_rt->offset += 0.5*0.02*p->sym.size + graph_get_bargap(gr);
        } else {
            for (j = 0; j < set_get_length(pset); j++) {
                plot_rt->refy[j] += p->data->ex[1][j];
            }
        }
        
        break;
    }
}

void draw_ref_point(Canvas *canvas, Quark *gr)
{
    GLocator *locator;
    VPoint vp;
    
    if (is_refpoint_active(gr)) {      
        locator = graph_get_locator(gr);
        Wpoint2Vpoint(gr, &locator->origin, &vp);
        setcolor(canvas, 1);
        setpattern(canvas, 1);
        setlinewidth(canvas, 1.0);
        setlinestyle(canvas, 1);
        symplus(canvas, &vp, 0.01);
        DrawCircle(canvas, &vp, 0.01);
    }
}

/*
 * draw the graph frame
 */
void draw_frame(Quark *q, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    view v;
    frame *f;
    VPoint vps[4];

    f = frame_get_data(q);
    frame_get_view(q, &v);

    setclipping(canvas, TRUE);
    
    setline(canvas, &f->outline);

    switch (f->type) {
    case 0:
        vps[0].x = v.xv1;
        vps[0].y = v.yv1;
        vps[1].x = v.xv2;
        vps[1].y = v.yv2;
        DrawRect(canvas, &vps[0], &vps[1]);
        break;
    case 1:                     /* half open */
        vps[0].x = v.xv1;
        vps[0].y = v.yv2;
        vps[1].x = v.xv1;
        vps[1].y = v.yv1;
        vps[2].x = v.xv2;
        vps[2].y = v.yv1;
        DrawPolyline(canvas, vps, 3, POLYLINE_OPEN);
        break;
    case 2:                     /* break top */
        vps[0].x = v.xv1;
        vps[0].y = v.yv2;
        vps[1].x = v.xv1;
        vps[1].y = v.yv1;
        vps[2].x = v.xv2;
        vps[2].y = v.yv1;
        vps[3].x = v.xv2;
        vps[3].y = v.yv2;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    case 3:                     /* break bottom */
        vps[0].x = v.xv1;
        vps[0].y = v.yv1;
        vps[1].x = v.xv1;
        vps[1].y = v.yv2;
        vps[2].x = v.xv2;
        vps[2].y = v.yv2;
        vps[3].x = v.xv2;
        vps[3].y = v.yv1;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    case 4:                     /* break left */
        vps[0].x = v.xv1;
        vps[0].y = v.yv1;
        vps[1].x = v.xv2;
        vps[1].y = v.yv1;
        vps[2].x = v.xv2;
        vps[2].y = v.yv2;
        vps[3].x = v.xv1;
        vps[3].y = v.yv2;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    case 5:                     /* break right */
        vps[0].x = v.xv2;
        vps[0].y = v.yv1;
        vps[1].x = v.xv1;
        vps[1].y = v.yv1;
        vps[2].x = v.xv1;
        vps[2].y = v.yv2;
        vps[3].x = v.xv2;
        vps[3].y = v.yv2;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    }

    draw_legends(q, plot_rt);
}

void fillframe(Canvas *canvas, Quark *q)
{
    view v;
    frame *f;
    VPoint vp1, vp2;

    frame_get_view(q, &v);
    f = frame_get_data(q);

    canvas_set_clipview(canvas, &v);
    
    /* fill coordinate frame with background color */
    if (f->fillpen.pattern != 0) {
        setpen(canvas, &f->fillpen);
        vp1.x = v.xv1;
        vp1.y = v.yv1;
        vp2.x = v.xv2;
        vp2.y = v.yv2;
        FillRect(canvas, &vp1, &vp2);
    }
}    

/*
 * draw a set filling polygon
 */
void drawsetfill(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    set *p = set_get_data(pset);
    int i, len, setlen, polylen;
    int line_type = p->line.type;
    double *x, *y;
    double ybase;
    world w;
    WPoint wptmp;
    VPoint *vps;
    double xmin, xmax, ymin, ymax;
    int stacked_chart;
    
    if (p->line.filltype == SETFILL_NONE) {
        return;
    }
    
    if (graph_get_type(gr) == GRAPH_CHART) {
        x = plot_rt->refx;
        setlen = MIN2(set_get_length(pset), plot_rt->refn);
    } else {
        x = p->data->ex[0];
        setlen = set_get_length(pset);
    }
    y = p->data->ex[1];
    
    if (graph_get_type(gr) == GRAPH_CHART && graph_is_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    setclipping(canvas, TRUE);
    
    graph_get_world(gr, &w);

    switch (line_type) {
    case LINE_TYPE_STRAIGHT:
    case LINE_TYPE_SEGMENT2:
    case LINE_TYPE_SEGMENT3:
        if (stacked_chart == TRUE && p->line.filltype == SETFILL_BASELINE) {
            len = 2*setlen;
        } else {
            len = setlen;
        }
        vps = (VPoint *) xmalloc((len + 2) * sizeof(VPoint));
        if (vps == NULL) {
            errmsg("Can't xmalloc in drawsetfill");
            return;
        }
 
        for (i = 0; i < setlen; i++) {
            wptmp.x = x[i];
            wptmp.y = y[i];
            if (stacked_chart == TRUE) {
                wptmp.y += plot_rt->refy[i];
            }
            Wpoint2Vpoint(gr, &wptmp, &vps[i]);
    	    vps[i].x += plot_rt->offset;
        }
        if (stacked_chart == TRUE && p->line.filltype == SETFILL_BASELINE) {
            for (i = 0; i < setlen; i++) {
                wptmp.x = x[setlen - i - 1];
                wptmp.y = plot_rt->refy[setlen - i - 1];
                Wpoint2Vpoint(gr, &wptmp, &vps[setlen + i]);
    	        vps[setlen + i].x += plot_rt->offset;
            }
        }
        break;
    case LINE_TYPE_LEFTSTAIR:
    case LINE_TYPE_RIGHTSTAIR:
        len = 2*setlen - 1;
        vps = (VPoint *) xmalloc((len + 2) * sizeof(VPoint));
        if (vps == NULL) {
            errmsg("Can't xmalloc in drawsetfill");
            return;
        }
 
        for (i = 0; i < setlen; i++) {
            wptmp.x = x[i];
            wptmp.y = y[i];
            if (stacked_chart == TRUE) {
                wptmp.y += plot_rt->refy[i];
            }
            Wpoint2Vpoint(gr, &wptmp, &vps[2*i]);
    	    vps[2*i].x += plot_rt->offset;
        }
        for (i = 1; i < len; i += 2) {
            if (line_type == LINE_TYPE_LEFTSTAIR) {
                vps[i].x = vps[i - 1].x;
                vps[i].y = vps[i + 1].y;
            } else {
                vps[i].x = vps[i + 1].x;
                vps[i].y = vps[i - 1].y;
            }
        }
        break;
    default:
        return;
    }
    
    switch (p->line.filltype) {
    case SETFILL_POLYGON:
        polylen = len;
        break;
    case SETFILL_BASELINE:
        if (stacked_chart == TRUE) {
            polylen = len;
        } else {
            getsetminmax(&pset, 1, &xmin, &xmax, &ymin, &ymax);
            ybase = setybase(pset);
            polylen = len + 2;
            wptmp.x = MIN2(xmax, w.xg2);
            wptmp.y = ybase;
            Wpoint2Vpoint(gr, &wptmp, &vps[len]);
    	    vps[len].x += plot_rt->offset;
            wptmp.x = MAX2(xmin, w.xg1);
            wptmp.y = ybase;
            Wpoint2Vpoint(gr, &wptmp, &vps[len + 1]);
    	    vps[len + 1].x += plot_rt->offset;
        }
        break;
    default:
        xfree(vps);
        return;
    }
    
    setpen(canvas, &p->line.fillpen);
    setfillrule(canvas, p->line.fillrule);
    DrawPolygon(canvas, vps, polylen);
    
    xfree(vps);
}

/*
 * draw set's connecting line
 */
void drawsetline(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    set *p = set_get_data(pset);
    int setlen, len;
    int i;
    int line_type = p->line.type;
    VPoint vps[4], *vpstmp;
    WPoint wp;
    double *x, *y;
    double lw;
    double ybase;
    double xmin, xmax, ymin, ymax;
    int stacked_chart;
    
    if (graph_get_type(gr) == GRAPH_CHART) {
        x = plot_rt->refx;
        setlen = MIN2(set_get_length(pset), plot_rt->refn);
    } else {
        x = p->data->ex[0];
        setlen = set_get_length(pset);
    }
    y = p->data->ex[1];
    
    if (graph_get_type(gr) == GRAPH_CHART && graph_is_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    if (stacked_chart == TRUE) {
        ybase = 0.0;
    } else {
        ybase = setybase(pset);
    }
    
    setclipping(canvas, TRUE);

    drawsetfill(pset, plot_rt);

    setline(canvas, &p->line.line);

    if (stacked_chart == TRUE) {
        lw = getlinewidth(canvas);
    } else {
        lw = 0.0;
    }
    
/* draw the line */
    if (p->line.line.style != 0 && p->line.line.pen.pattern != 0) {
        
        switch (line_type) {
        case LINE_TYPE_NONE:
            break;
        case LINE_TYPE_STRAIGHT:
            vpstmp = (VPoint *) xmalloc(setlen*sizeof(VPoint));
            if (vpstmp == NULL) {
                errmsg("xmalloc failed in drawsetline()");
                break;
            }
            for (i = 0; i < setlen; i++) {
                wp.x = x[i];
                wp.y = y[i];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i];
                }
                Wpoint2Vpoint(gr, &wp, &vpstmp[i]);
    	        vpstmp[i].x += plot_rt->offset;
                
                vpstmp[i].y -= lw/2.0;
            }
            DrawPolyline(canvas, vpstmp, setlen, POLYLINE_OPEN);
            xfree(vpstmp);
            break;
        case LINE_TYPE_SEGMENT2:
            for (i = 0; i < setlen - 1; i += 2) {
                wp.x = x[i];
                wp.y = y[i];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i];
                }
                Wpoint2Vpoint(gr, &wp, &vps[0]);
    	        vps[0].x += plot_rt->offset;
                wp.x = x[i + 1];
                wp.y = y[i + 1];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i + 1];
                }
                Wpoint2Vpoint(gr, &wp, &vps[1]);
    	        vps[1].x += plot_rt->offset;
                
                vps[0].y -= lw/2.0;
                vps[1].y -= lw/2.0;
                
                DrawLine(canvas, &vps[0], &vps[1]);
            }
            break;
        case LINE_TYPE_SEGMENT3:
            for (i = 0; i < setlen - 2; i += 3) {
                wp.x = x[i];
                wp.y = y[i];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i];
                }
                Wpoint2Vpoint(gr, &wp, &vps[0]);
    	        vps[0].x += plot_rt->offset;
                wp.x = x[i + 1];
                wp.y = y[i + 1];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i + 1];
                }
                Wpoint2Vpoint(gr, &wp, &vps[1]);
    	        vps[1].x += plot_rt->offset;
                wp.x = x[i + 2];
                wp.y = y[i + 2];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i + 2];
                }
                Wpoint2Vpoint(gr, &wp, &vps[2]);
    	        vps[2].x += plot_rt->offset;
                DrawPolyline(canvas, vps, 3, POLYLINE_OPEN);
                
                vps[0].y -= lw/2.0;
                vps[1].y -= lw/2.0;
                vps[2].y -= lw/2.0;
            }
            if (i == setlen - 2) {
                wp.x = x[i];
                wp.y = y[i];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i];
                }
                Wpoint2Vpoint(gr, &wp, &vps[0]);
    	        vps[0].x += plot_rt->offset;
                wp.x = x[i + 1];
                wp.y = y[i + 1];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i + 1];
                }
                Wpoint2Vpoint(gr, &wp, &vps[1]);
    	        vps[1].x += plot_rt->offset;
                
                vps[0].y -= lw/2.0;
                vps[1].y -= lw/2.0;
                
                DrawLine(canvas, &vps[0], &vps[1]);
            }
            break;
        case LINE_TYPE_LEFTSTAIR:
        case LINE_TYPE_RIGHTSTAIR:
            len = 2*setlen - 1;
            vpstmp = (VPoint *) xmalloc(len*sizeof(VPoint));
            if (vpstmp == NULL) {
                errmsg("xmalloc failed in drawsetline()");
                break;
            }
            for (i = 0; i < setlen; i++) {
                wp.x = x[i];
                wp.y = y[i];
                if (stacked_chart == TRUE) {
                    wp.y += plot_rt->refy[i];
                }
                Wpoint2Vpoint(gr, &wp, &vpstmp[2*i]);
    	        vpstmp[2*i].x += plot_rt->offset;
            }
            for (i = 1; i < len; i += 2) {
                if (line_type == LINE_TYPE_LEFTSTAIR) {
                    vpstmp[i].x = vpstmp[i - 1].x;
                    vpstmp[i].y = vpstmp[i + 1].y;
                } else {
                    vpstmp[i].x = vpstmp[i + 1].x;
                    vpstmp[i].y = vpstmp[i - 1].y;
                }
            }
            DrawPolyline(canvas, vpstmp, len, POLYLINE_OPEN);
            xfree(vpstmp);
            break;
        default:
            errmsg("Invalid line type");
            break;
        }
    }

    if (p->line.droplines == TRUE) {
        for (i = 0; i < setlen; i ++) {
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y = plot_rt->refy[i];
            } else {
                wp.y = ybase;
            }
            Wpoint2Vpoint(gr, &wp, &vps[0]);
    	    vps[0].x += plot_rt->offset;
            wp.x = x[i];
            wp.y = y[i];
            if (stacked_chart == TRUE) {
                wp.y += plot_rt->refy[i];
            }
            Wpoint2Vpoint(gr, &wp, &vps[1]);
    	    vps[1].x += plot_rt->offset;
            
            vps[1].y -= lw/2.0;
 
            DrawLine(canvas, &vps[0], &vps[1]);
        }
    }
    
    getsetminmax(&pset, 1, &xmin, &xmax, &ymin, &ymax);
       
    if (p->line.baseline == TRUE && stacked_chart != TRUE) {
        wp.x = xmin;
        wp.y = ybase;
        Wpoint2Vpoint(gr, &wp, &vps[0]);
    	vps[0].x += plot_rt->offset;
        wp.x = xmax;
        Wpoint2Vpoint(gr, &wp, &vps[1]);
    	vps[1].x += plot_rt->offset;
 
        DrawLine(canvas, &vps[0], &vps[1]);
    }
}    

/* draw the symbols */
void drawsetsyms(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    set *p = set_get_data(pset);
    int setlen;
    int i;
    VPoint vp;
    WPoint wp;
    double *x, *y, *z, *c;
    int skip = p->symskip + 1;
    int stacked_chart;
    double znorm = graph_get_znorm(gr);
    
    if (graph_get_type(gr) == GRAPH_CHART) {
        x = plot_rt->refx;
        setlen = MIN2(set_get_length(pset), plot_rt->refn);
    } else {
        x = p->data->ex[0];
        setlen = set_get_length(pset);
    }
    y = p->data->ex[1];
    
    if (graph_get_type(gr) == GRAPH_CHART && graph_is_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    if (p->type == SET_XYSIZE) {
        if (znorm == 0.0) {
            return;
        }
        z = p->data->ex[2];
    } else {
        z = NULL;
    }
    
    if (p->type == SET_XYCOLOR) {
        c = p->data->ex[2];
    } else {
        c = NULL;
    }
    
    setclipping(canvas, FALSE);
    
    if ((p->sym.line.pen.pattern != 0 && p->sym.line.style != 0) ||
                        (p->sym.fillpen.pattern != 0)) {
              
        Symbol sym = p->sym;
        
        setline(canvas, &sym.line);
        setfont(canvas, sym.charfont);
        for (i = 0; i < setlen; i += skip) {
            wp.x = x[i];
            wp.y = y[i];
            if (stacked_chart == TRUE) {
                wp.y += plot_rt->refy[i];
            }
            
            if (!is_validWPoint(gr, &wp)){
                continue;
            }
        
            Wpoint2Vpoint(gr, &wp, &vp);
    	    vp.x += plot_rt->offset;
            
            if (z) {
                sym.size = z[i]/znorm;
            }
            if (c) {
                int color = (int) rint(c[i]);
                if (get_colortype(canvas, color) != COLOR_MAIN) {
                    color = 1;
                }
                sym.fillpen.color = color;
            }
            if (drawxysym(canvas, &vp, &sym) != RETURN_SUCCESS) {
                return;
            }
        } 
    }
}


void drawtext(Canvas *canvas,
    const VPoint *vp, const TextProps *tprops, const char *s)
{
    setcolor(canvas, tprops->color);
    setfont(canvas, tprops->font);
    setcharsize(canvas, tprops->charsize);

    WriteString(canvas, vp, tprops->angle, tprops->just, s);
}

/* draw the annotative values */
void drawsetavalues(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    Quark *pr = get_parent_project(gr);
    set *p = set_get_data(pset);
    int i;
    int setlen;
    double *x, *y, *z;
    WPoint wp;
    VPoint vp;
    int skip = p->symskip + 1;
    AValue avalue;
    char str[MAX_STRING_LENGTH];
    int stacked_chart;

    avalue = p->avalue;
    if (avalue.active != TRUE) {
        return;
    }

    if (graph_get_type(gr) == GRAPH_CHART) {
        x = plot_rt->refx;
        setlen = MIN2(set_get_length(pset), plot_rt->refn);
    } else {
        x = p->data->ex[0];
        setlen = set_get_length(pset);
    }
    y = p->data->ex[1];
    
    if (set_get_ncols(pset) > 2) {
        z = p->data->ex[2];
    } else {
        z = NULL;
    }
    
    if (graph_get_type(gr) == GRAPH_CHART && graph_is_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    for (i = 0; i < setlen; i += skip) {
        wp.x = x[i];
        wp.y = y[i];
        if (stacked_chart == TRUE) {
            wp.y += plot_rt->refy[i];
        }
        
        if (!is_validWPoint(gr, &wp)){
            continue;
        }
        
        Wpoint2Vpoint(gr, &wp, &vp);
        
        vp.x += avalue.offset.x;
        vp.y += avalue.offset.y;
    	vp.x += plot_rt->offset;
        
        strcpy(str, avalue.prestr);
        
        switch(avalue.type) {
        case AVALUE_TYPE_NONE:
            break;
        case AVALUE_TYPE_X:
            strcat(str, create_fstring(pr, avalue.format, avalue.prec, wp.x, 
                                                 LFORMAT_TYPE_EXTENDED));
            break;
        case AVALUE_TYPE_Y:
            strcat(str, create_fstring(pr, avalue.format, avalue.prec, wp.y,
                                                 LFORMAT_TYPE_EXTENDED));
            break;
        case AVALUE_TYPE_XY:
            strcat(str, create_fstring(pr, avalue.format, avalue.prec, wp.x,
                                                 LFORMAT_TYPE_EXTENDED));
            strcat(str, ", ");
            strcat(str, create_fstring(pr, avalue.format, avalue.prec, wp.y,
                                                 LFORMAT_TYPE_EXTENDED));
            break;
        case AVALUE_TYPE_STRING:
            if (p->data->s != NULL && p->data->s[i] != NULL) {
                strcat(str, p->data->s[i]);
            }
            break;
        case AVALUE_TYPE_Z:
            if (z != NULL) {
                strcat(str, create_fstring(pr, avalue.format, avalue.prec, z[i], 
                                                 LFORMAT_TYPE_EXTENDED));
            }
            break;
        default:
            errmsg("Invalid type of ann. value");
            return;
        }
        
        strcat(str, avalue.appstr);
        
        drawtext(canvas, &vp, &avalue.tprops, str); 
    }
}

void drawseterrbars(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    set *p = set_get_data(pset);
    int i, n;
    double *x, *y;
    double *dx_plus, *dx_minus, *dy_plus, *dy_minus, *dtmp;
    PlacementType ptype = p->errbar.ptype;
    WPoint wp1, wp2;
    VPoint vp1, vp2;
    int stacked_chart;
    int skip = p->symskip + 1;
    
    if (p->errbar.active != TRUE) {
        return;
    }
    
    if (graph_get_type(gr) == GRAPH_CHART) {
        x = plot_rt->refx;
        n = MIN2(set_get_length(pset), plot_rt->refn);
    } else {
        x = p->data->ex[0];
        n = set_get_length(pset);
    }
    y = p->data->ex[1];
    
    if (graph_get_type(gr) == GRAPH_CHART && graph_is_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    dx_plus  = NULL;
    dx_minus = NULL;
    dy_plus  = NULL;
    dy_minus = NULL;
    switch (p->type) {
    case SET_XYDX:
        dx_plus = p->data->ex[2];
        break;
    case SET_XYDY:
    case SET_BARDY:
        dy_plus = p->data->ex[2];
        break;
    case SET_XYDXDX:
        dx_plus  = p->data->ex[2];
        dx_minus = p->data->ex[3];
        break;
    case SET_XYDYDY:
    case SET_BARDYDY:
        dy_plus  = p->data->ex[2];
        dy_minus = p->data->ex[3];
        break;
    case SET_XYDXDY:
        dx_plus = p->data->ex[2];
        dy_plus = p->data->ex[3];
        break;
    case SET_XYDXDXDYDY:
        dx_plus  = p->data->ex[2];
        dx_minus = p->data->ex[3];
        dy_plus  = p->data->ex[4];
        dy_minus = p->data->ex[5];
        break;
    default:
        return;
    }
    
    switch (ptype) {
    case PLACEMENT_OPPOSITE:
        dtmp     = dx_minus;
        dx_minus = dx_plus;
        dx_plus  = dtmp;
        dtmp     = dy_minus;
        dy_minus = dy_plus;
        dy_plus  = dtmp;
        break;
    case PLACEMENT_BOTH:
        if (dx_minus == NULL && dy_minus == NULL) {
            dx_minus = dx_plus;
            dy_minus = dy_plus;
        }
        break;
    default:
        break;
    }
    
    setclipping(canvas, TRUE);
    
    for (i = 0; i < n; i += skip) {
        wp1.x = x[i];
        wp1.y = y[i];
        if (stacked_chart == TRUE) {
            wp1.y += plot_rt->refy[i];
        }
        if (is_validWPoint(gr, &wp1) == FALSE) {
            continue;
        }

        Wpoint2Vpoint(gr, &wp1, &vp1);
        vp1.x += plot_rt->offset;

        if (dx_plus != NULL) {
            wp2 = wp1;
            wp2.x += fabs(dx_plus[i]);
            Wpoint2Vpoint(gr, &wp2, &vp2);
            vp2.x += plot_rt->offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
        if (dx_minus != NULL) {
            wp2 = wp1;
            wp2.x -= fabs(dx_minus[i]);
            Wpoint2Vpoint(gr, &wp2, &vp2);
            vp2.x += plot_rt->offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
        if (dy_plus != NULL) {
            wp2 = wp1;
            wp2.y += fabs(dy_plus[i]);
            Wpoint2Vpoint(gr, &wp2, &vp2);
            vp2.x += plot_rt->offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
        if (dy_minus != NULL) {
            wp2 = wp1;
            wp2.y -= fabs(dy_minus[i]);
            Wpoint2Vpoint(gr, &wp2, &vp2);
            vp2.x += plot_rt->offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
    }
}

/*
 * draw hi/lo-open/close
 */
void drawsethilo(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    set *p = set_get_data(pset);
    int i;
    double *x = p->data->ex[0], *y1 = p->data->ex[1];
    double *y2 = p->data->ex[2], *y3 = p->data->ex[3], *y4 = p->data->ex[4];
    double ilen = 0.02*p->sym.size;
    int skip = p->symskip + 1;
    WPoint wp;
    VPoint vp1, vp2;

    if (p->sym.line.style != 0) {
        setline(canvas, &p->sym.line);
        for (i = 0; i < set_get_length(pset); i += skip) {
            wp.x = x[i];
            wp.y = y1[i];
            Wpoint2Vpoint(pset, &wp, &vp1);
            wp.y = y2[i];
            Wpoint2Vpoint(pset, &wp, &vp2);
            DrawLine(canvas, &vp1, &vp2);
            wp.y = y3[i];
            Wpoint2Vpoint(pset, &wp, &vp1);
            vp2 = vp1;
            vp2.x -= ilen;
            DrawLine(canvas, &vp1, &vp2);
            wp.y = y4[i];
            Wpoint2Vpoint(pset, &wp, &vp1);
            vp2 = vp1;
            vp2.x += ilen;
            DrawLine(canvas, &vp1, &vp2);
        }
    }
}

/*
 * draw 2D bars
 */
void drawsetbars(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    set *p = set_get_data(pset);
    int i, n;
    double *x, *y;
    double lw, bw = 0.01*p->sym.size;
    int skip = p->symskip + 1;
    double ybase;
    WPoint wp;
    VPoint vp1, vp2;
    int stacked_chart;
    
    if (graph_get_type(gr) == GRAPH_CHART) {
        x = plot_rt->refx;
        n = MIN2(set_get_length(pset), plot_rt->refn);
    } else {
        x = p->data->ex[0];
        n = set_get_length(pset);
    }
    y = p->data->ex[1];
    
    if (graph_get_type(gr) == GRAPH_CHART && graph_is_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }


    
    if (stacked_chart == TRUE) {
        ybase = 0.0;
    } else {
        ybase = setybase(pset);
    }

    setline(canvas, &p->sym.line);
    if (graph_get_type(gr) == GRAPH_CHART &&
        p->sym.line.style != 0 && p->sym.line.pen.pattern != 0) {
        lw = getlinewidth(canvas);
    } else {
        lw = 0.0;
    }

    if (p->sym.fillpen.pattern != 0) {
	setpen(canvas, &p->sym.fillpen);
        for (i = 0; i < n; i += skip) {
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y = plot_rt->refy[i];
            } else {
                wp.y = ybase;
            }
    	    Wpoint2Vpoint(gr, &wp, &vp1);
            vp1.x -= bw;
    	    vp1.x += plot_rt->offset;
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y += y[i];
            } else {
                wp.y = y[i];
            }
    	    Wpoint2Vpoint(gr, &wp, &vp2);
            vp2.x += bw;
    	    vp2.x += plot_rt->offset;
            
            vp1.x += lw/2.0;
            vp2.x -= lw/2.0;
            vp1.y += lw/2.0;
            
            FillRect(canvas, &vp1, &vp2);
        }
    }
    if (p->sym.line.style != 0 && p->sym.line.pen.pattern != 0) {
        setpen(canvas, &p->sym.line.pen);
        for (i = 0; i < n; i += skip) {
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y = plot_rt->refy[i];
            } else {
                wp.y = ybase;
            }
    	    Wpoint2Vpoint(gr, &wp, &vp1);
            vp1.x -= bw;
    	    vp1.x += plot_rt->offset;
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y += y[i];
            } else {
                wp.y = y[i];
            }
    	    Wpoint2Vpoint(gr, &wp, &vp2);
            vp2.x += bw;
    	    vp2.x += plot_rt->offset;

            vp1.x += lw/2.0;
            vp2.x -= lw/2.0;
            vp1.y += lw/2.0;

    	    DrawRect(canvas, &vp1, &vp2);
        }
    }
}

void drawcirclexy(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    set *p = set_get_data(pset);
    int i, setlen;
    double *x, *y, *r;
    int skip = p->symskip + 1;
    WPoint wp;
    VPoint vp1, vp2;

    setclipping(canvas, TRUE);
    
    setlen = set_get_length(pset);
    x = p->data->ex[0];
    y = p->data->ex[1];
    r = p->data->ex[2];

    setfillrule(canvas, p->line.fillrule);
    setline(canvas, &p->line.line);

    for (i = 0; i < setlen; i += skip) {
        wp.x = x[i];
        wp.y = y[i];
        /* TODO: remove once ellipse clipping works */
        if (!is_validWPoint(pset, &wp)){
            continue;
        }
        wp.x = x[i] - r[i];
        wp.y = y[i] - r[i];
        Wpoint2Vpoint(pset, &wp, &vp1);
        wp.x = x[i] + r[i];
        wp.y = y[i] + r[i];
        Wpoint2Vpoint(pset, &wp, &vp2);
        if (p->line.filltype != SETFILL_NONE) {
            setpen(canvas, &p->line.fillpen);
            DrawFilledEllipse(canvas, &vp1, &vp2);
        }
        setpen(canvas, &p->line.line.pen);
        DrawEllipse(canvas, &vp1, &vp2);
    }
}

/* Arrows for vector map plots */
void drawsetvmap(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    Quark *gr = get_parent_graph(pset);
    set *p = set_get_data(pset);
    int i, setlen;
    double znorm = graph_get_znorm(gr);
    int skip = p->symskip + 1;
    double *x, *y, *vx, *vy;
    WPoint wp;
    VPoint vp1, vp2;
    Arrow arrow = {0, 1.0, 1.0, 0.0};
    
    Errbar eb = p->errbar;
    
    setclipping(canvas, TRUE);
    
    if (znorm == 0.0) {
        return;
    }
    
    setlen = set_get_length(pset);
    x = p->data->ex[DATA_X];
    y = p->data->ex[DATA_Y];
    vx = p->data->ex[DATA_Y1];
    vy = p->data->ex[DATA_Y2];

    arrow.length = 2*eb.barsize;

    setpen(canvas, &p->errbar.pen);

    for (i = 0; i < setlen; i += skip) {
        wp.x = x[i];
        wp.y = y[i];
        if (!is_validWPoint(gr, &wp)){
            continue;
        }
        Wpoint2Vpoint(gr, &wp, &vp1);
        vp2.x = vp1.x + vx[i]/znorm;
        vp2.y = vp1.y + vy[i]/znorm;

        setlinewidth(canvas, eb.riser_linew);
        setlinestyle(canvas, eb.riser_lines);
        DrawLine(canvas, &vp1, &vp2);

        setlinewidth(canvas, eb.linew);
        setlinestyle(canvas, eb.lines);
        draw_arrowhead(canvas, &vp1, &vp2, &arrow, &p->errbar.pen, &p->errbar.pen);
    }
}

void drawsetboxplot(Quark *pset, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    set *p = set_get_data(pset);
    int i;
    double *x, *md, *lb, *ub, *lw, *uw;
    double size = 0.01*p->sym.size;
    int skip = p->symskip + 1;
    WPoint wp;
    VPoint vp1, vp2;

    x  = p->data->ex[0];
    md = p->data->ex[1];
    lb = p->data->ex[2];
    ub = p->data->ex[3];
    lw = p->data->ex[4];
    uw = p->data->ex[5];

    setclipping(canvas, TRUE);

    for (i = 0; i < set_get_length(pset); i += skip) {
        wp.x =  x[i];

        wp.y = lb[i];
        Wpoint2Vpoint(pset, &wp, &vp1);
        wp.y = ub[i];
        Wpoint2Vpoint(pset, &wp, &vp2);
        
        /* whiskers */
        if (p->errbar.active == TRUE) {
            VPoint vp3;
            wp.y = lw[i];
            Wpoint2Vpoint(pset, &wp, &vp3);
            drawerrorbar(canvas, &vp1, &vp3, &p->errbar);
            wp.y = uw[i];
            Wpoint2Vpoint(pset, &wp, &vp3);
            drawerrorbar(canvas, &vp2, &vp3, &p->errbar);
        }

        /* box */
        vp1.x -= size;
        vp2.x += size;
        setpen(canvas, &p->sym.fillpen);
        FillRect(canvas, &vp1, &vp2);

        setline(canvas, &p->sym.line);
        DrawRect(canvas, &vp1, &vp2);

        /* median line */
        wp.y = md[i];
        Wpoint2Vpoint(pset, &wp, &vp1);
        vp2 = vp1;
        vp1.x -= size;
        vp2.x += size;
        DrawLine(canvas, &vp1, &vp2);
    }
}

void draw_pie_chart_set(Quark *pset, plot_rt_t *plot_rt)
{
    int i;
    view v;
    world w;
    int sgn;
    VPoint vpc, vp1, vp2, vps[3], vpa;
    VVector offset;
    double r, start_angle, stop_angle;
    double e_max, norm;
    double *x, *c, *e, *pt;
    AValue avalue;
    char str[MAX_STRING_LENGTH];
    set *p;
    Quark *gr, *pr;
    Canvas *canvas = plot_rt->canvas;

    gr = get_parent_graph(pset);
    pr = get_parent_project(gr);

    graph_get_viewport(gr, &v);
    vpc.x = (v.xv1 + v.xv2)/2;
    vpc.y = (v.yv1 + v.yv2)/2;

    graph_get_world(gr, &w);
    sgn = graph_is_xinvert(gr) ? -1:1;
    
    p = set_get_data(pset);
    
    plot_rt->ndsets++;
    if (plot_rt->ndsets > 1) {
        errmsg("Only one set per pie chart can be drawn");
        return;
    }

    /* data */
    x = set_get_col(pset, DATA_X);
    /* explode factor */
    e = set_get_col(pset, DATA_Y);
    /* colors */
    c = set_get_col(pset, DATA_Y1);
    /* patterns */
    pt = set_get_col(pset, DATA_Y2);

    /* get max explode factor */
    e_max = 0.0;
    for (i = 0; i < set_get_length(pset); i++) {
        e_max = MAX2(e_max, e[i]);
    }

    r = 0.8/(1.0 + e_max)*MIN2(v.xv2 - v.xv1, v.yv2 - v.yv1)/2;

    norm = 0.0;
    for (i = 0; i < set_get_length(pset); i++) {
        if (x[i] < 0.0) {
            errmsg("No negative values in pie charts allowed");
            return;
        }
        if (e[i] < 0.0) {
            errmsg("No negative offsets in pie charts allowed");
            return;
        }
        norm += x[i];
    }

    stop_angle = w.xg1;
    for (i = 0; i < set_get_length(pset); i++) {
        Pen pen;

        start_angle = stop_angle;
        stop_angle = start_angle + sgn*2*M_PI*x[i]/norm;
        offset.x = e[i]*r*cos((start_angle + stop_angle)/2.0);
        offset.y = e[i]*r*sin((start_angle + stop_angle)/2.0);
        vps[0].x = vpc.x + r*cos(start_angle) + offset.x;
        vps[0].y = vpc.y + r*sin(start_angle) + offset.y;
        vps[1].x = vpc.x + offset.x;
        vps[1].y = vpc.y + offset.y;
        vps[2].x = vpc.x + r*cos(stop_angle) + offset.x;
        vps[2].y = vpc.y + r*sin(stop_angle) + offset.y;
        vp1.x = vpc.x - r + offset.x;
        vp1.y = vpc.y - r + offset.y;
        vp2.x = vpc.x + r + offset.x;
        vp2.y = vpc.y + r + offset.y;

        if (c != NULL) {
            pen.color   = (int) rint(c[i]);
        } else {
            pen.color = p->sym.fillpen.color;
        }
        if (pt != NULL) {
            pen.pattern   = (int) rint(pt[i]);
        } else {
            pen.pattern = p->sym.fillpen.pattern;
        }
        setpen(canvas, &pen);
        DrawFilledArc(canvas, &vp1, &vp2,
            180.0/M_PI*start_angle,
            180.0/M_PI*(stop_angle - start_angle),
            ARCFILL_PIESLICE);

        setline(canvas, &p->sym.line);
        DrawPolyline(canvas, vps, 3, POLYLINE_OPEN);
        DrawArc(canvas, &vp1, &vp2,
            180.0/M_PI*start_angle,
            180.0/M_PI*(stop_angle - start_angle));

        avalue = p->avalue;

        if (avalue.active == TRUE) {
            TextProps tprops = avalue.tprops;

            vpa.x = vpc.x + ((1 + e[i])*r + avalue.offset.y)*
                cos((start_angle + stop_angle)/2.0);
            vpa.y = vpc.y + ((1 + e[i])*r + avalue.offset.y)*
                sin((start_angle + stop_angle)/2.0);

            strcpy(str, avalue.prestr);

            switch (avalue.type) {
            case AVALUE_TYPE_X:
                strcat(str, create_fstring(pr, avalue.format, avalue.prec, x[i], 
                                                     LFORMAT_TYPE_EXTENDED));
                break;
            case AVALUE_TYPE_STRING:
                if (p->data->s != NULL && p->data->s[i] != NULL) {
                    strcat(str, p->data->s[i]);
                }
                break;
            default:
                continue;
            }

            strcat(str, avalue.appstr);

            drawtext(canvas, &vpa, &tprops, str); 
        }
    }
}


void symplus(Canvas *canvas, const VPoint *vp, double s)
{
    VPoint vp1, vp2;
    vp1.x = vp->x - s;
    vp1.y = vp->y;
    vp2.x = vp->x + s;
    vp2.y = vp->y;
    
    DrawLine(canvas, &vp1, &vp2);
    vp1.x = vp->x;
    vp1.y = vp->y - s;
    vp2.x = vp->x;
    vp2.y = vp->y + s;
    DrawLine(canvas, &vp1, &vp2);
}

void symx(Canvas *canvas, const VPoint *vp, double s)
{
    VPoint vp1, vp2;
    double side = M_SQRT1_2*s;
    
    vp1.x = vp->x - side;
    vp1.y = vp->y - side;
    vp2.x = vp->x + side;
    vp2.y = vp->y + side;
    DrawLine(canvas, &vp1, &vp2);
    
    vp1.x = vp->x - side;
    vp1.y = vp->y + side;
    vp2.x = vp->x + side;
    vp2.y = vp->y - side;
    DrawLine(canvas, &vp1, &vp2);
}

void symsplat(Canvas *canvas, const VPoint *vp, double s)
{
    symplus(canvas, vp, s);
    symx(canvas, vp, s);
}

int drawxysym(Canvas *canvas, const VPoint *vp, const Symbol *sym)
{
    double symsize;
    VPoint vps[4];
    char buf[2];
    
    symsize = sym->size*0.01;
    
    switch (sym->type) {
    case SYM_NONE:
        break;
    case SYM_CIRCLE:
        setpen(canvas, &sym->fillpen);
        DrawFilledCircle(canvas, vp, symsize);
        setpen(canvas, &sym->line.pen);
        DrawCircle(canvas, vp, symsize);
        break;
    case SYM_SQUARE:
        symsize *= 0.85;
        vps[0].x = vp->x - symsize;
        vps[0].y = vp->y - symsize;
        vps[1].x = vps[0].x;
        vps[1].y = vp->y + symsize;
        vps[2].x = vp->x + symsize;
        vps[2].y = vps[1].y;
        vps[3].x = vps[2].x;
        vps[3].y = vps[0].y;
        
        setpen(canvas, &sym->fillpen);
        DrawPolygon(canvas, vps, 4);
        setline(canvas, &sym->line);
        DrawPolyline(canvas, vps, 4, POLYLINE_CLOSED);
        break;
    case SYM_DIAMOND:
        vps[0].x = vp->x;
        vps[0].y = vp->y + symsize;
        vps[1].x = vp->x - symsize;
        vps[1].y = vp->y;
        vps[2].x = vps[0].x;
        vps[2].y = vp->y - symsize;
        vps[3].x = vp->x + symsize;
        vps[3].y = vps[1].y;
        
        setpen(canvas, &sym->fillpen);
        DrawPolygon(canvas, vps, 4);
        setline(canvas, &sym->line);
        DrawPolyline(canvas, vps, 4, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG1:
        vps[0].x = vp->x;
        vps[0].y = vp->y + 2*M_SQRT1_3*symsize;
        vps[1].x = vp->x - symsize;
        vps[1].y = vp->y - M_SQRT1_3*symsize;
        vps[2].x = vp->x + symsize;
        vps[2].y = vps[1].y;
        
        setpen(canvas, &sym->fillpen);
        DrawPolygon(canvas, vps, 3);
        setline(canvas, &sym->line);
        DrawPolyline(canvas, vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG2:
        vps[0].x = vp->x - 2*M_SQRT1_3*symsize;
        vps[0].y = vp->y;
        vps[1].x = vp->x + M_SQRT1_3*symsize;
        vps[1].y = vp->y - symsize;
        vps[2].x = vps[1].x;
        vps[2].y = vp->y + symsize;
        
        setpen(canvas, &sym->fillpen);
        DrawPolygon(canvas, vps, 3);
        setline(canvas, &sym->line);
        DrawPolyline(canvas, vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG3:
        vps[0].x = vp->x - symsize;
        vps[0].y = vp->y + M_SQRT1_3*symsize;
        vps[1].x = vp->x;
        vps[1].y = vp->y - 2*M_SQRT1_3*symsize;
        vps[2].x = vp->x + symsize;
        vps[2].y = vps[0].y;
        
        setpen(canvas, &sym->fillpen);
        DrawPolygon(canvas, vps, 3);
        setline(canvas, &sym->line);
        DrawPolyline(canvas, vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG4:
        vps[0].x = vp->x - M_SQRT1_3*symsize;
        vps[0].y = vp->y + symsize;
        vps[1].x = vps[0].x;
        vps[1].y = vp->y - symsize;
        vps[2].x = vp->x + 2*M_SQRT1_3*symsize;
        vps[2].y = vp->y;
        
        setpen(canvas, &sym->fillpen);
        DrawPolygon(canvas, vps, 3);
        setline(canvas, &sym->line);
        DrawPolyline(canvas, vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_PLUS:
        setline(canvas, &sym->line);
        symplus(canvas, vp, symsize);
        break;
    case SYM_X:
        setline(canvas, &sym->line);
        symx(canvas, vp, symsize);
        break;
    case SYM_SPLAT:
        setline(canvas, &sym->line);
        symsplat(canvas, vp, symsize);
        break;
    case SYM_CHAR:
        setline(canvas, &sym->line);
        buf[0] = sym->symchar;
        buf[1] = '\0';
        setcharsize(canvas, sym->size);
        WriteString(canvas, vp, 0.0, JUST_CENTER|JUST_MIDDLE, buf);
        break;
    default:
        errmsg("Invalid symbol type");
        return RETURN_FAILURE;
    }
    return RETURN_SUCCESS;
}

static void drawlegbarsym(Canvas *canvas,
    const VPoint *vp, double size, const Pen *sympen, const Pen *symfillpen)
{
    double width, height;
    VPoint vps[4];

    width  = 0.02*size;
    height = 0.02*getcharsize(canvas);
    
    vps[0].x = vps[1].x = vp->x - width/2;
    vps[2].x = vps[3].x = vp->x + width/2;
    vps[0].y = vps[3].y = vp->y - height/2;
    vps[1].y = vps[2].y = vp->y + height/2;
    
    setpen(canvas, symfillpen);
    DrawPolygon(canvas, vps, 4);
    setpen(canvas, sympen);
    DrawPolyline(canvas, vps, 4, POLYLINE_CLOSED);
}

void drawerrorbar(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, Errbar *eb)
{
    double ilen;
    VPoint vp_plus, vp_minus;
    VVector lvv;
    double vlength;
    static Arrow arrow = {0, 1.0, 1.0, 0.0};

    lvv.x = vp2->x - vp1->x;
    lvv.y = vp2->y - vp1->y;

    vlength = hypot(lvv.x, lvv.y);
    if (vlength == 0.0) {
        return;
    }
    
    lvv.x /= vlength;
    lvv.y /= vlength;
    
    setpen(canvas, &eb->pen);
    
    if (eb->arrow_clip && is_validVPoint(canvas, vp2) == FALSE) {
        vp_plus.x = vp1->x + eb->cliplen*lvv.x;
        vp_plus.y = vp1->y + eb->cliplen*lvv.y;
        setlinewidth(canvas, eb->riser_linew);
        setlinestyle(canvas, eb->riser_lines);
        DrawLine(canvas, vp1, &vp_plus);
        arrow.length = 2*eb->barsize;
        setlinewidth(canvas, eb->linew);
        setlinestyle(canvas, eb->lines);
        draw_arrowhead(canvas, vp1, &vp_plus, &arrow, &eb->pen, &eb->pen);
    } else {
        setlinewidth(canvas, eb->riser_linew);
        setlinestyle(canvas, eb->riser_lines);
        DrawLine(canvas, vp1, vp2);
        setlinewidth(canvas, eb->linew);
        setlinestyle(canvas, eb->lines);
        ilen = 0.01*eb->barsize;
        vp_minus.x = vp2->x - ilen*lvv.y;
        vp_minus.y = vp2->y + ilen*lvv.x;
        vp_plus.x  = vp2->x + ilen*lvv.y;
        vp_plus.y  = vp2->y - ilen*lvv.x;
        DrawLine(canvas, &vp_minus, &vp_plus);
    }
}

/*
 * draw arrow head
 */
void draw_arrowhead(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, const Arrow *arrowp,
    const Pen *pen, const Pen *fill)
{
    double L, l, d, vlength;
    VVector vnorm;
    VPoint vpc, vpl, vpr, vps[4];

    vlength = hypot((vp2->x - vp1->x), (vp2->y - vp1->y));
    if (vlength == 0.0) {
        return;
    }

    vnorm.x = (vp2->x - vp1->x)/vlength;
    vnorm.y = (vp2->y - vp1->y)/vlength;
    
    L = 0.01*arrowp->length;
    d = L*arrowp->dL_ff;
    l = L*arrowp->lL_ff;

    vpc.x = vp2->x - L*vnorm.x;
    vpc.y = vp2->y - L*vnorm.y;
    vpl.x = vpc.x + 0.5*d*vnorm.y;
    vpl.y = vpc.y - 0.5*d*vnorm.x;
    vpr.x = vpc.x - 0.5*d*vnorm.y;
    vpr.y = vpc.y + 0.5*d*vnorm.x;
    vpc.x += l*vnorm.x;
    vpc.y += l*vnorm.y;
    
    vps[0] = vpl;
    vps[1] = *vp2;
    vps[2] = vpr;
    vps[3] = vpc;
    
    setlinestyle(canvas, 1);
    
    switch (arrowp->type) {
    case ARROW_TYPE_LINE:
        setpen(canvas, pen);
        DrawPolyline(canvas, vps, 3, POLYLINE_OPEN);
        break;
    case ARROW_TYPE_FILLED:
        setpen(canvas, fill);
        DrawPolygon(canvas, vps, 4);
        setpen(canvas, pen);
        DrawPolyline(canvas, vps, 4, POLYLINE_CLOSED);
        break;
    case ARROW_TYPE_CIRCLE:
        setpen(canvas, fill);
        DrawFilledCircle(canvas, vp2, d/2);
        setpen(canvas, pen);
        DrawCircle(canvas, vp2, d/2);
        break;
    default:
        errmsg("Internal error in draw_arrowhead()");
        break;
    }

    return;
}

void draw_region(Canvas *canvas, Quark *q)
{
    Quark *f = get_parent_frame(q);
    view v;
    region *r = region_get_data(q);
    VPoint vp;
    WPoint wp;
    double dv = 0.003;

    frame_get_view(f, &v);
    if (terminal_device(canvas) != TRUE || !r || !r->active) {
        return;
    }
    
    setcolor(canvas, r->color);
    setpattern(canvas, 1);
    
    vp.x = v.xv1;
    while (vp.x <= v.xv2) {
        vp.y = v.yv1;
        while (vp.y <= v.yv2) {
            if (Vpoint2Wpoint(q, &vp, &wp) == RETURN_SUCCESS &&
                inregion(q, &wp)) {
                DrawPixel(canvas, &vp);
            }
            vp.y += dv;
        }
        vp.x += dv;
    }
}


/* ---------------------- legends ---------------------- */

void draw_legend_syms(Quark *pset,
    plot_rt_t *plot_rt, const VPoint *vp, const legend *l)
{
    set *p = set_get_data(pset);
    Quark *gr = get_parent_graph(pset);

    if (is_set_drawable(pset) &&
        !is_empty_string(p->legstr) &&
        graph_get_type(gr) != GRAPH_PIE) {
        
        Canvas *canvas = plot_rt->canvas;
        double symvshift;
        int draw_line, singlesym;
        VPoint vp1, vp2;

        setfont(canvas, p->sym.charfont);

        if (l->len <= 0.0 || p->line.type == 0 ||
            ((p->line.line.style == 0 ||
              p->line.line.pen.pattern == 0) &&
             (p->line.filltype == SETFILL_NONE ||
              p->line.fillpen.pattern == 0))) {
            draw_line = FALSE;
        } else {
            draw_line = TRUE;
        }

        symvshift = 0.0;
        if (draw_line) {
            vp1.x = vp->x - l->len/2;
            vp2.x = vp->x + l->len/2;
            vp2.y = vp1.y = vp->y;

            setline(canvas, &p->line.line);
            if (p->line.filltype != SETFILL_NONE &&
                p->line.fillpen.pattern != 0) {
                VPoint vpf1, vpf2;
                
                vpf1.x = vp1.x;
                vpf1.y = vp1.y - 0.01*l->charsize;
                vpf2.x = vp2.x;
                vpf2.y = vp2.y + 0.01*l->charsize;

                /* TODO: settable option for setfill presentation */
                setpen(canvas, &p->line.fillpen);
                if (1) {
                    FillRect(canvas, &vpf1, &vpf2);
                } else {
                    DrawFilledEllipse(canvas, &vpf1, &vpf2);
                }
                setpen(canvas, &p->line.line.pen);
                if (1) {
                    DrawRect(canvas, &vpf1, &vpf2);
                } else {
                    DrawEllipse(canvas, &vpf1, &vpf2);
                }

                symvshift = 0.01*l->charsize;
            } else {
                DrawLine(canvas, &vp1, &vp2);
            }
        }

        if (draw_line) {
            singlesym = l->singlesym;
        } else {
            singlesym = TRUE;
        }

        setline(canvas, &p->sym.line);
        if (!singlesym) {
            if (p->type == SET_BAR   || p->type == SET_BOXPLOT ||
                p->type == SET_BARDY || p->type == SET_BARDYDY) {
                drawlegbarsym(canvas,
                    &vp1, p->sym.size, &p->sym.line.pen, &p->sym.fillpen);
                drawlegbarsym(canvas,
                    &vp2, p->sym.size, &p->sym.line.pen, &p->sym.fillpen);
            } else {
                drawxysym(canvas, &vp1, &p->sym);
                drawxysym(canvas, &vp2, &p->sym);
            }
        } else {
            VPoint vptmp;
            vptmp.x = vp->x;
            vptmp.y = vp->y + symvshift;

            if (p->type == SET_BAR   || p->type == SET_BOXPLOT ||
                p->type == SET_BARDY || p->type == SET_BARDYDY) {
                drawlegbarsym(canvas,
                    &vptmp, p->sym.size, &p->sym.line.pen, &p->sym.fillpen);
            } else {
                drawxysym(canvas, &vptmp, &p->sym);
            }
        }
    }
}

static int is_legend_drawable(Quark *pset)
{
    set *p = set_get_data(pset);
    Quark *gr = get_parent_graph(pset);
    
    if (is_set_drawable(pset)       &&
        !is_empty_string(p->legstr) &&
        graph_is_active(gr)         &&
        graph_get_type(gr) != GRAPH_PIE) {
        
        return TRUE;
    } else {
        return FALSE;
    }
}

typedef struct {
    view sym_bbox;
    view str_bbox;
    Quark *pset;
} leg_rt_t;

/*
 * draw the legends
 */
void draw_legends(Quark *q, plot_rt_t *plot_rt)
{
    Canvas *canvas = plot_rt->canvas;
    VPoint vp, vp2;
    double bb_width, bb_height;
    int i, nsets, setno, nleg_entries;
    double max_sym_width, max_str_width;
    
    view sym_bbox, str_bbox, v;
    legend *l;

    Quark **psets;
    
    leg_rt_t *leg_entries;

    l = frame_get_legend(q);
    if (l->active == FALSE) {
        return;
    }

    setclipping(canvas, FALSE);
    
    vp.x = vp.y = 0.0;
    bb_height = l->vgap;
    max_sym_width = 0.0;
    max_str_width = 0.0;

    set_draw_mode(canvas, FALSE);
    
    nsets = get_descendant_sets(q, &psets);
    leg_entries = xmalloc(nsets*sizeof(leg_rt_t));
    nleg_entries = 0;
    for (setno = 0; setno < nsets; setno++) {
        Quark *pset;
        double sym_width, str_width, y_middle, ascent, descent, height;

        pset = psets[setno];
        
        if (is_legend_drawable(pset)) {
            set *p = set_get_data(pset);

            activate_bbox(canvas, BBOX_TYPE_TEMP, TRUE);
            reset_bbox(canvas, BBOX_TYPE_TEMP);
            update_bbox(canvas, BBOX_TYPE_TEMP, &vp);
            draw_legend_syms(pset, plot_rt, &vp, l);
            get_bbox(canvas, BBOX_TYPE_TEMP, &sym_bbox);
            sym_width = fabs(sym_bbox.xv2 - sym_bbox.xv1);
            if (sym_width > max_sym_width) {
                max_sym_width = sym_width;
            }
            
            setcharsize(canvas, l->charsize);
            setfont(canvas, l->font);

            get_string_bbox(canvas,
                &vp, 0.0, JUST_LEFT|JUST_BLINE, p->legstr, &str_bbox);
                
            y_middle = str_bbox.yv2/2;
            str_width = fabs(str_bbox.xv2 - str_bbox.xv1);
            if (str_width > max_str_width) {
                max_str_width = str_width;
            }
            
            ascent  = MAX2(sym_bbox.yv2 + y_middle, str_bbox.yv2);
            descent = MIN2(sym_bbox.yv1 + y_middle, str_bbox.yv1);
            
            height  = ascent - descent;
            bb_height += height + l->vgap;
            
            leg_entries[nleg_entries].pset     = pset;
            leg_entries[nleg_entries].sym_bbox = sym_bbox;
            leg_entries[nleg_entries].str_bbox = str_bbox;
            nleg_entries++;
        }
    }
    
    xfree(psets);

    set_draw_mode(canvas, TRUE);
    
    bb_width = max_sym_width + max_str_width + 3*l->hgap;
    
    frame_get_view(q, &v);
    
    switch (l->acorner) {
    case CORNER_LL:
        vp.x = v.xv1 + l->offset.x;
        vp.y = v.yv1 + l->offset.y + bb_height;
        break;
    case CORNER_UL:
        vp.x = v.xv1 + l->offset.x;
        vp.y = v.yv2 - l->offset.y;
        break;
    case CORNER_UR:
    default:
        vp.x = v.xv2 - l->offset.x - bb_width;
        vp.y = v.yv2 - l->offset.y;
        break;
    case CORNER_LR:
        vp.x = v.xv2 - l->offset.x - bb_width;
        vp.y = v.yv1 + l->offset.y + bb_height;
        break;
    }
    
    vp2.x = vp.x + bb_width;
    vp2.y = vp.y - bb_height;

    l->bb.xv1 = vp.x;
    l->bb.yv1 = vp2.y;
    l->bb.xv2 = vp2.x;
    l->bb.yv2 = vp.y;
    
    if (nleg_entries) {
        setpen(canvas, &l->boxfillpen);
        FillRect(canvas, &vp, &vp2);

        setline(canvas, &l->boxline);
        DrawRect(canvas, &vp, &vp2);

        /* correction */
        vp.x += l->hgap + max_sym_width/2;
        vp.y -= l->vgap;

    }
    
    for (i = 0; i < nleg_entries; i++) {
        leg_rt_t *leg_entry;
        Quark *pset;
        set *p;
        double y_middle, ascent, descent;
        VPoint vpsym, vpstr;

        if (l->invert == FALSE) {
            setno = i;
        } else {
            setno = nleg_entries - i - 1;
        }
        
        leg_entry = &leg_entries[setno];
        pset = leg_entry->pset;
        p = set_get_data(pset);
        
        y_middle = leg_entry->str_bbox.yv2/2;
        ascent  = MAX2(leg_entry->sym_bbox.yv2 + y_middle, leg_entry->str_bbox.yv2);
        descent = MIN2(leg_entry->sym_bbox.yv1 + y_middle, leg_entry->str_bbox.yv1);

        vp.y -= ascent;
        
        vpsym.x = vp.x;
        vpsym.y = vp.y + y_middle;
        draw_legend_syms(pset, plot_rt, &vpsym, l);

        vpstr.x = vp.x + max_sym_width/2 + l->hgap;
        vpstr.y = vp.y;
        setcharsize(canvas, l->charsize);
        setfont(canvas, l->font);
        setcolor(canvas, l->color);
        WriteString(canvas, &vpstr, 0.0, JUST_LEFT|JUST_BLINE, p->legstr);

        vp.y -= (l->vgap - descent);
    }
    
    xfree(leg_entries);
}
