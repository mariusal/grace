/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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
 * plotone.c - entry for graphics
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "graphs.h"
#include "graphutils.h"
#include "grace/canvas.h"
#include "objutils.h"
#include "plotone.h"
#include "protos.h"

char print_file[GR_MAXPATHLEN] = "";

static int plotone_hook(Quark *q,
    void *udata, QTraverseClosure *closure)
{
    Canvas *canvas = (Canvas *) udata;
    
    switch (q->fid) {
    case QFlavorFrame:
        if (!closure->pass2) {
            /* fill frame */
            fillframe(canvas, q);
    
            closure->pass2 = TRUE;
        } else {
            drawframe(canvas, q);
        }
        break;
    case QFlavorGraph:
        if (draw_graph(canvas, q) != RETURN_SUCCESS) {
            closure->descend = FALSE;
        }
        break;
    case QFlavorAxis:
        draw_axis(canvas, q);
        break;
    case QFlavorDObject:
        draw_object(canvas, q);
        break;
    }
    
    return TRUE;
}

static void dproc(Canvas *canvas, void *data)
{
    Quark *project = (Quark *) data;

    set_draw_mode(canvas, TRUE);   
    
    quark_traverse(project, plotone_hook, canvas);
}

/*
 * draw all active graphs
 */
void drawgraph(Grace *grace)
{
    Canvas *canvas = grace->rt->canvas;
    Project *pr = project_get_data(grace->project);
    Quark *saveg;

    saveg = graph_get_current(grace->project);
    
    canvas_set_docname(canvas, get_docname(grace->project));
    canvas_set_username(canvas, get_username(grace));
    canvas_set_pagefill(canvas, pr->bgfill);
    setbgcolor(canvas, pr->bgcolor);
    
    canvas_draw(canvas, dproc, grace->project);
    
    if (graph_get_current(grace->project) != saveg) {
        select_graph(saveg);
    }
}

/*
 * If writing to a file, check to see if it exists
 */
void do_hardcopy(Grace *grace)
{
    Canvas *canvas = grace->rt->canvas;
    char tbuf[128], *s;
    char fname[GR_MAXPATHLEN];
    view v;
    double vx, vy;
    int truncated_out;
    FILE *prstream;
    
    if (get_ptofile(grace)) {
        if (is_empty_string(print_file)) {
            Device_entry *dev = get_device_props(canvas, grace->rt->hdevice);
            sprintf(print_file, "%s.%s",
                get_docbname(grace->project), dev->fext);
        }
        strcpy(fname, print_file);
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
    
    prstream = grace_openw(fname);
    if (prstream == NULL) {
        return;
    }
    
    canvas_set_prstream(canvas, prstream); 
    
    select_device(canvas, grace->rt->hdevice);
    
    drawgraph(grace);
    
    grace_close(prstream);
    
    get_bbox(canvas, BBOX_TYPE_GLOB, &v);
    get_page_viewport(canvas, &vx, &vy);
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
    
    select_device(canvas, grace->rt->tdevice);
}


int draw_graph(Canvas *canvas, Quark *gr)
{
    GraphType gtype;
    graph *g = graph_get_data(gr);
    
    if (!g || !g->active) {
        return RETURN_FAILURE;
    }

    if (select_graph(gr) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    gtype = get_graph_type(gr);
    
    /* plot type specific routines */
    switch (gtype) {
    case GRAPH_POLAR:
        draw_polar_graph(canvas, gr);
        break;
    case GRAPH_SMITH:
        draw_smith_chart(canvas, gr);
        break;
    case GRAPH_PIE:
        draw_pie_chart(canvas, gr);
        break;
    default:
        xyplot(canvas, gr);
        break;
    }

    /* draw regions and mark the reference points only if in interactive mode */
    if (terminal_device(canvas) == TRUE) {
        draw_regions(canvas, gr);
        draw_ref_point(canvas, gr);
    }
    
    return RETURN_SUCCESS;
}

void draw_smith_chart(Canvas *canvas, Quark *gr)
{
}

void draw_pie_chart(Canvas *canvas, Quark *gr)
{
    int i, setno, nsets, ndsets = 0;
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
    Quark **psets;

    get_graph_viewport(gr, &v);
    vpc.x = (v.xv1 + v.xv2)/2;
    vpc.y = (v.yv1 + v.yv2)/2;

    get_graph_world(gr, &w);
    sgn = is_graph_xinvert(gr) ? -1:1;
    
    nsets = graph_get_sets(gr, &psets);
    
    for (setno = 0; setno < nsets; setno++) {
        Quark *pset = psets[setno];
        if (is_set_drawable(pset)) {
            set *p = (set *) pset->data;
            ndsets++;
            if (ndsets > 1) {
                errmsg("Only one set per pie chart can be drawn");
                return;
            }
            
            switch (dataset_type(pset)) {
            case SET_XY:
            case SET_XYCOLOR:
            case SET_XYCOLPAT:
                /* data */
                x = getcol(pset, DATA_X);
                /* explode factor */
                e = getcol(pset, DATA_Y);
                /* colors */
                c = getcol(pset, DATA_Y1);
                /* patterns */
                pt = getcol(pset, DATA_Y2);
                
                /* get max explode factor */
                e_max = 0.0;
                for (i = 0; i < getsetlength(pset); i++) {
                    e_max = MAX2(e_max, e[i]);
                }
                
                r = 0.8/(1.0 + e_max)*MIN2(v.xv2 - v.xv1, v.yv2 - v.yv1)/2;

                norm = 0.0;
                for (i = 0; i < getsetlength(pset); i++) {
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
                for (i = 0; i < getsetlength(pset); i++) {
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

                        vpa.x = vpc.x + ((1 + e[i])*r + avalue.offset.y)*
                            cos((start_angle + stop_angle)/2.0);
                        vpa.y = vpc.y + ((1 + e[i])*r + avalue.offset.y)*
                            sin((start_angle + stop_angle)/2.0);

                        strcpy(str, avalue.prestr);

                        switch (avalue.type) {
                        case AVALUE_TYPE_X:
                            strcat(str, create_fstring(avalue.format, avalue.prec, x[i], 
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

                        setcharsize(canvas, avalue.size);
                        setfont(canvas, avalue.font);
                        setcolor(canvas, avalue.color);

                        WriteString(canvas, &vpa,
                            (double) avalue.angle, JUST_CENTER|JUST_MIDDLE, str);
                    }
                }
                break;
            default:
                errmsg("Unsupported in pie chart set type");
                break;
            }
        }
    }
    xfree(psets);
}

void draw_polar_graph(Canvas *canvas, Quark *gr)
{
    int setno, nsets;
    Quark **psets;

    nsets = graph_get_sets(gr, &psets);
    
    for (setno = 0; setno < nsets; setno++) {
        Quark *pset = psets[setno];
        if (is_set_drawable(pset)) {
            switch (dataset_type(pset)) {
            case SET_XY:
            case SET_XYSIZE:
            case SET_XYCOLOR:
            case SET_XYZ:
                drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                break;
            default:
                errmsg("Unsupported in polar graph set type");
                break;
            }
        }
    }
    xfree(psets);
}

void xyplot(Canvas *canvas, Quark *gr)
{
    int j, setno, nsets;
    int refn;
    double *refx, *refy;
    double offset, epsilon;
    Quark **psets;

    refn = 0;
    offset = 0.0;
    refx = NULL;
    refy = NULL;

    nsets = graph_get_sets(gr, &psets);

    /* draw sets */
    switch (get_graph_type(gr)) {
    case GRAPH_XY:
        for (setno = 0; setno < nsets; setno++) {
            Quark *pset = psets[setno];
            if (is_set_drawable(pset)) {
                switch (dataset_type(pset)) {
                case SET_XY:
                case SET_XYSIZE:
                case SET_XYCOLOR:
                case SET_XYZ:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_BAR:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetbars(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYDX:
                case SET_XYDY:
                case SET_XYDXDX:
                case SET_XYDYDY:
                case SET_XYDXDY:
                case SET_XYDXDXDYDY:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawseterrbars(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYHILO:
                    drawsethilo(canvas, pset);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYVMAP:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetvmap(canvas, pset);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_BOXPLOT:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetboxplot(canvas, pset);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                default:
                    errmsg("Unsupported in XY graph set type");
                    break;
                }
            }
        }
        break;
    case GRAPH_CHART:
        for (setno = 0; setno < nsets; setno++) {
            Quark *pset = psets[setno];
            if (is_set_drawable(pset)) {
                set *p = (set *) pset->data;
                if (getsetlength(pset) > refn) {
                    refn = getsetlength(pset);
                    refx = getx(pset);
                }
                if (is_graph_stacked(gr) != TRUE) {
                    offset -= 0.5*0.02*p->sym.size;
                }
            }
        }
        offset -= 0.5*(number_of_active_sets(gr) - 1)*get_graph_bargap(gr);
        
        if (is_graph_stacked(gr) == TRUE) {
            refy = xcalloc(refn, SIZEOF_DOUBLE);
            if (refy == NULL) {
                return;
            }
        }

        
        if (refx) {
            double xmin, xmax;
            int imin, imax;
            minmax(refx, refn, &xmin, &xmax, &imin, &imax);
            epsilon = 1.0e-3*(xmax - xmin)/refn;
        } else {
            epsilon = 0.0;
        }

        for (setno = 0; setno < nsets; setno++) {
            Quark *pset = psets[setno];
            int x_ok;
            double *x;
            
            if (is_set_drawable(pset)) {
                set *p = set_get_data(pset);

                /* check that abscissas are identical with refx */
                x = getcol(pset, DATA_X);
                x_ok = TRUE;
                for (j = 0; j < getsetlength(pset); j++) {
                    if (fabs(x[j] - refx[j]) > epsilon) {
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
                    continue;
                }


                if (is_graph_stacked(gr) != TRUE) {
                    offset += 0.5*0.02*p->sym.size;
                }
                switch (dataset_type(pset)) {
                case SET_XY:
                case SET_XYSIZE:
                case SET_XYCOLOR:
                    drawsetline(canvas, pset, refn, refx, refy, offset);
                    if (is_graph_stacked(gr) != TRUE) {
                        drawsetsyms(canvas, pset, refn, refx, refy, offset);
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                    }
                    break;
                case SET_BAR:
                    drawsetline(canvas, pset, refn, refx, refy, offset);
                    drawsetbars(canvas, pset, refn, refx, refy, offset);
                    if (is_graph_stacked(gr) != TRUE) {
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                    }
                    break;
                case SET_BARDY:
                case SET_BARDYDY:
                    drawsetline(canvas, pset, refn, refx, refy, offset);
                    drawsetbars(canvas, pset, refn, refx, refy, offset);
                    if (is_graph_stacked(gr) != TRUE) {
                        drawseterrbars(canvas, pset, refn, refx, refy, offset);
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                    }
                    break;
                case SET_XYDY:
                case SET_XYDYDY:
                    drawsetline(canvas, pset, refn, refx, refy, offset);
                    if (is_graph_stacked(gr) != TRUE) {
                        drawseterrbars(canvas, pset, refn, refx, refy, offset);
                        drawsetsyms(canvas, pset, refn, refx, refy, offset);
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                    }
                    break;
                default:
                    errmsg("Unsupported in XY chart set type");
                    break;
                }
                if (is_graph_stacked(gr) != TRUE) {
                    offset += 0.5*0.02*p->sym.size + get_graph_bargap(gr);
                } else {
                    for (j = 0; j < getsetlength(pset); j++) {
                        refy[j] += p->data->ex[1][j];
                    }
                }
            }
        }
        
        if (is_graph_stacked(gr) == TRUE) {
            /* Second pass for stacked charts: symbols and avalues */
            offset = 0.0;
            for (j = 0; j < refn; j++) {
                refy[j] = 0.0;
            }
            
            for (setno = 0; setno < nsets; setno++) {
                Quark *pset = psets[setno];
                if (is_set_drawable(pset)) {
                    switch (dataset_type(pset)) {
                    case SET_XY:
                    case SET_XYSIZE:
                    case SET_XYCOLOR:
                        drawsetsyms(canvas, pset, refn, refx, refy, offset);
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                        break;
                    case SET_BAR:
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                        break;
                    case SET_BARDY:
                    case SET_BARDYDY:
                        drawseterrbars(canvas, pset, refn, refx, refy, offset);
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                        break;
                    case SET_XYDY:
                    case SET_XYDYDY:
                        drawseterrbars(canvas, pset, refn, refx, refy, offset);
                        drawsetsyms(canvas, pset, refn, refx, refy, offset);
                        drawsetavalues(canvas, pset, refn, refx, refy, offset);
                        break;
                    }
                    
                    for (j = 0; j < getsetlength(pset); j++) {
                        refy[j] += gety(pset)[j];
                    }
                }
            }
        }

        if (refy != NULL) {
            xfree(refy);
        }
        break;
    case GRAPH_FIXED:
        for (setno = 0; setno < nsets; setno++) {
            Quark *pset = psets[setno];
            if (is_set_drawable(pset)) {
                switch (dataset_type(pset)) {
                case SET_XY:
                case SET_XYSIZE:
                case SET_XYCOLOR:
                case SET_XYZ:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYDX:
                case SET_XYDY:
                case SET_XYDXDX:
                case SET_XYDYDY:
                case SET_XYDXDY:
                case SET_XYDXDXDYDY:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawseterrbars(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYR:
                    drawcirclexy(canvas, pset);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYVMAP:
                    drawsetline(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetvmap(canvas, pset);
                    drawsetsyms(canvas, pset, 0, NULL, NULL, 0.0);
                    drawsetavalues(canvas, pset, 0, NULL, NULL, 0.0);
                    break;
                default:
                    errmsg("Unsupported in XY graph set type");
                    break;
                }
            }
        }
        break;
    } /* end g.type */

    xfree(psets);
}

void draw_regions(Canvas *canvas, Quark *gr)
{
#if 0
    int i;
    Project *pr = (Project *) grace->project->data;

    setclipping(canvas, TRUE);
    
    /* draw any defined regions for this graph */
    for (i = 0; i < MAXREGION; i++) {
        region r = pr->rg[i];
        if (r.active && r.linkto == gr) {
            setcolor(canvas, r.color);
            setpattern(canvas, 1);
            setlinewidth(canvas, r.linew);
            setlinestyle(canvas, r.lines);
            draw_region(canvas, &r);
        }
    }
#endif
}

void draw_ref_point(Canvas *canvas, Quark *gr)
{
    GLocator *locator;
    WPoint wp;
    VPoint vp;
    
    if (is_refpoint_active(gr)) {      
        locator = get_graph_locator(gr);
        wp.x = locator->dsx;
        wp.y = locator->dsy;
        vp = Wpoint2Vpoint(wp);
        setcolor(canvas, 1);
        setpattern(canvas, 1);
        setlinewidth(canvas, 1.0);
        setlinestyle(canvas, 1);
        symplus(canvas, &vp, 0.01);
        DrawCircle(canvas, &vp, 0.01);
    }
}


/* draw title and subtitle */
void draw_titles(Canvas *canvas, Quark *q)
{
    view *v;
    labels *lab;
    VPoint vp1, vp2;
    
    v = frame_get_view(q);
    lab = frame_get_labels(q);

    vp1.x = (v->xv2 + v->xv1) / 2;
    vp1.y = (v->yv2 < v->yv1)? v->yv1 : v->yv2;
    vp2 = vp1;
    if (lab->title.s && lab->title.s[0]) {
        setcolor(canvas, lab->title.color);
        setcharsize(canvas, lab->title.charsize);
        setfont(canvas, lab->title.font);
        vp1.y += 0.06;
        WriteString(canvas, &vp1, 0.0, JUST_CENTER|JUST_BOTTOM, lab->title.s);
    }
    if (lab->stitle.s && lab->stitle.s[0]) {
        setcolor(canvas, lab->stitle.color);
        setcharsize(canvas, lab->stitle.charsize);
        setfont(canvas, lab->stitle.font);
        vp2.y += 0.02;
        WriteString(canvas, &vp2, 0.0, JUST_CENTER|JUST_BOTTOM, lab->stitle.s);
    }
}

/*
 * draw the graph frame
 */
void drawframe(Canvas *canvas, Quark *q)
{
    view *v;
    frame *f;
    VPoint vps[4];

    f = frame_get_data(q);
    v = frame_get_view(q);

    setclipping(canvas, TRUE);
    
    setline(canvas, &f->outline);

    switch (f->type) {
    case 0:
        vps[0].x = v->xv1;
        vps[0].y = v->yv1;
        vps[1].x = v->xv2;
        vps[1].y = v->yv2;
        DrawRect(canvas, &vps[0], &vps[1]);
        break;
    case 1:                     /* half open */
        vps[0].x = v->xv1;
        vps[0].y = v->yv2;
        vps[1].x = v->xv1;
        vps[1].y = v->yv1;
        vps[2].x = v->xv2;
        vps[2].y = v->yv1;
        DrawPolyline(canvas, vps, 3, POLYLINE_OPEN);
        break;
    case 2:                     /* break top */
        vps[0].x = v->xv1;
        vps[0].y = v->yv2;
        vps[1].x = v->xv1;
        vps[1].y = v->yv1;
        vps[2].x = v->xv2;
        vps[2].y = v->yv1;
        vps[3].x = v->xv2;
        vps[3].y = v->yv2;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    case 3:                     /* break bottom */
        vps[0].x = v->xv1;
        vps[0].y = v->yv1;
        vps[1].x = v->xv1;
        vps[1].y = v->yv2;
        vps[2].x = v->xv2;
        vps[2].y = v->yv2;
        vps[3].x = v->xv2;
        vps[3].y = v->yv1;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    case 4:                     /* break left */
        vps[0].x = v->xv1;
        vps[0].y = v->yv1;
        vps[1].x = v->xv2;
        vps[1].y = v->yv1;
        vps[2].x = v->xv2;
        vps[2].y = v->yv2;
        vps[3].x = v->xv1;
        vps[3].y = v->yv2;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    case 5:                     /* break right */
        vps[0].x = v->xv2;
        vps[0].y = v->yv1;
        vps[1].x = v->xv1;
        vps[1].y = v->yv1;
        vps[2].x = v->xv1;
        vps[2].y = v->yv2;
        vps[3].x = v->xv2;
        vps[3].y = v->yv2;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        break;
    }

#if 0
    if (gtype != GRAPH_PIE) {
        /* plot legends */
        dolegend(canvas, q);
    }
#endif
    
    /* draw title and subtitle */
    draw_titles(canvas, q);
}

void fillframe(Canvas *canvas, Quark *q)
{
    view *v;
    frame *f;
    VPoint vp1, vp2;

    v = frame_get_view(q);
    f = frame_get_data(q);

    canvas_set_clipview(canvas, v);
    
    /* fill coordinate frame with background color */
    if (f->fillpen.pattern != 0) {
        setpen(canvas, &f->fillpen);
        vp1.x = v->xv1;
        vp1.y = v->yv1;
        vp2.x = v->xv2;
        vp2.y = v->yv2;
        FillRect(canvas, &vp1, &vp2);
    }
}    

/*
 * draw a set filling polygon
 */
void drawsetfill(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
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
    
    if (get_graph_type(gr) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(getsetlength(pset), refn);
    } else {
        x = p->data->ex[0];
        setlen = getsetlength(pset);
    }
    y = p->data->ex[1];
    
    if (get_graph_type(gr) == GRAPH_CHART && is_graph_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    setclipping(canvas, TRUE);
    
    get_graph_world(gr, &w);

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
                wptmp.y += refy[i];
            }
            vps[i] = Wpoint2Vpoint(wptmp);
    	    vps[i].x += offset;
        }
        if (stacked_chart == TRUE && p->line.filltype == SETFILL_BASELINE) {
            for (i = 0; i < setlen; i++) {
                wptmp.x = x[setlen - i - 1];
                wptmp.y = refy[setlen - i - 1];
                vps[setlen + i] = Wpoint2Vpoint(wptmp);
    	        vps[setlen + i].x += offset;
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
                wptmp.y += refy[i];
            }
            vps[2*i] = Wpoint2Vpoint(wptmp);
    	    vps[2*i].x += offset;
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
            vps[len] = Wpoint2Vpoint(wptmp);
    	    vps[len].x += offset;
            wptmp.x = MAX2(xmin, w.xg1);
            wptmp.y = ybase;
            vps[len + 1] = Wpoint2Vpoint(wptmp);
    	    vps[len + 1].x += offset;
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
void drawsetline(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
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
    
    if (get_graph_type(gr) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(getsetlength(pset), refn);
    } else {
        x = p->data->ex[0];
        setlen = getsetlength(pset);
    }
    y = p->data->ex[1];
    
    if (get_graph_type(gr) == GRAPH_CHART && is_graph_stacked(gr) == TRUE) {
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

    drawsetfill(canvas, pset, refn, refx, refy, offset);

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
                    wp.y += refy[i];
                }
                vpstmp[i] = Wpoint2Vpoint(wp);
    	        vpstmp[i].x += offset;
                
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
                    wp.y += refy[i];
                }
                vps[0] = Wpoint2Vpoint(wp);
    	        vps[0].x += offset;
                wp.x = x[i + 1];
                wp.y = y[i + 1];
                if (stacked_chart == TRUE) {
                    wp.y += refy[i + 1];
                }
                vps[1] = Wpoint2Vpoint(wp);
    	        vps[1].x += offset;
                
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
                    wp.y += refy[i];
                }
                vps[0] = Wpoint2Vpoint(wp);
    	        vps[0].x += offset;
                wp.x = x[i + 1];
                wp.y = y[i + 1];
                if (stacked_chart == TRUE) {
                    wp.y += refy[i + 1];
                }
                vps[1] = Wpoint2Vpoint(wp);
    	        vps[1].x += offset;
                wp.x = x[i + 2];
                wp.y = y[i + 2];
                if (stacked_chart == TRUE) {
                    wp.y += refy[i + 2];
                }
                vps[2] = Wpoint2Vpoint(wp);
    	        vps[2].x += offset;
                DrawPolyline(canvas, vps, 3, POLYLINE_OPEN);
                
                vps[0].y -= lw/2.0;
                vps[1].y -= lw/2.0;
                vps[2].y -= lw/2.0;
            }
            if (i == setlen - 2) {
                wp.x = x[i];
                wp.y = y[i];
                if (stacked_chart == TRUE) {
                    wp.y += refy[i];
                }
                vps[0] = Wpoint2Vpoint(wp);
    	        vps[0].x += offset;
                wp.x = x[i + 1];
                wp.y = y[i + 1];
                if (stacked_chart == TRUE) {
                    wp.y += refy[i + 1];
                }
                vps[1] = Wpoint2Vpoint(wp);
    	        vps[1].x += offset;
                
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
                    wp.y += refy[i];
                }
                vpstmp[2*i] = Wpoint2Vpoint(wp);
    	        vpstmp[2*i].x += offset;
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
                wp.y = refy[i];
            } else {
                wp.y = ybase;
            }
            vps[0] = Wpoint2Vpoint(wp);
    	    vps[0].x += offset;
            wp.x = x[i];
            wp.y = y[i];
            if (stacked_chart == TRUE) {
                wp.y += refy[i];
            }
            vps[1] = Wpoint2Vpoint(wp);
    	    vps[1].x += offset;
            
            vps[1].y -= lw/2.0;
 
            DrawLine(canvas, &vps[0], &vps[1]);
        }
    }
    
    getsetminmax(&pset, 1, &xmin, &xmax, &ymin, &ymax);
       
    if (p->line.baseline == TRUE && stacked_chart != TRUE) {
        wp.x = xmin;
        wp.y = ybase;
        vps[0] = Wpoint2Vpoint(wp);
    	vps[0].x += offset;
        wp.x = xmax;
        vps[1] = Wpoint2Vpoint(wp);
    	vps[1].x += offset;
 
        DrawLine(canvas, &vps[0], &vps[1]);
    }
}    

/* draw the symbols */
void drawsetsyms(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
    int setlen;
    int i;
    VPoint vp;
    WPoint wp;
    double *x, *y, *z, *c;
    int skip = p->symskip + 1;
    int stacked_chart;
    double znorm = get_graph_znorm(gr);
    
    if (get_graph_type(gr) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(getsetlength(pset), refn);
    } else {
        x = p->data->ex[0];
        setlen = getsetlength(pset);
    }
    y = p->data->ex[1];
    
    if (get_graph_type(gr) == GRAPH_CHART && is_graph_stacked(gr) == TRUE) {
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
                wp.y += refy[i];
            }
            
            if (!is_validWPoint(wp)){
                continue;
            }
        
            vp = Wpoint2Vpoint(wp);
    	    vp.x += offset;
            
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


/* draw the annotative values */
void drawsetavalues(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
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

    if (get_graph_type(gr) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(getsetlength(pset), refn);
    } else {
        x = p->data->ex[0];
        setlen = getsetlength(pset);
    }
    y = p->data->ex[1];
    
    if (dataset_cols(pset) > 2) {
        z = p->data->ex[2];
    } else {
        z = NULL;
    }
    
    if (get_graph_type(gr) == GRAPH_CHART && is_graph_stacked(gr) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    setcharsize(canvas, avalue.size);
    setfont(canvas, avalue.font);

    for (i = 0; i < setlen; i += skip) {
        wp.x = x[i];
        wp.y = y[i];
        if (stacked_chart == TRUE) {
            wp.y += refy[i];
        }
        
        if (!is_validWPoint(wp)){
            continue;
        }
        
        vp = Wpoint2Vpoint(wp);
        
        vp.x += avalue.offset.x;
        vp.y += avalue.offset.y;
    	vp.x += offset;
        
        strcpy(str, avalue.prestr);
        
        switch(avalue.type) {
        case AVALUE_TYPE_NONE:
            break;
        case AVALUE_TYPE_X:
            strcat(str, create_fstring(avalue.format, avalue.prec, wp.x, 
                                                 LFORMAT_TYPE_EXTENDED));
            break;
        case AVALUE_TYPE_Y:
            strcat(str, create_fstring(avalue.format, avalue.prec, wp.y,
                                                 LFORMAT_TYPE_EXTENDED));
            break;
        case AVALUE_TYPE_XY:
            strcat(str, create_fstring(avalue.format, avalue.prec, wp.x,
                                                 LFORMAT_TYPE_EXTENDED));
            strcat(str, ", ");
            strcat(str, create_fstring(avalue.format, avalue.prec, wp.y,
                                                 LFORMAT_TYPE_EXTENDED));
            break;
        case AVALUE_TYPE_STRING:
            if (p->data->s != NULL && p->data->s[i] != NULL) {
                strcat(str, p->data->s[i]);
            }
            break;
        case AVALUE_TYPE_Z:
            if (z != NULL) {
                strcat(str, create_fstring(avalue.format, avalue.prec, z[i], 
                                                 LFORMAT_TYPE_EXTENDED));
            }
            break;
        default:
            errmsg("Invalid type of ann. value");
            return;
        }
        
        strcat(str, avalue.appstr);
        
        setcolor(canvas, avalue.color);
        WriteString(canvas,
            &vp, (double) avalue.angle, JUST_CENTER|JUST_BOTTOM, str);
    } 
}

void drawseterrbars(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
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
    
    if (get_graph_type(gr) == GRAPH_CHART) {
        x = refx;
        n = MIN2(getsetlength(pset), refn);
    } else {
        x = p->data->ex[0];
        n = getsetlength(pset);
    }
    y = p->data->ex[1];
    
    if (get_graph_type(gr) == GRAPH_CHART && is_graph_stacked(gr) == TRUE) {
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
            wp1.y += refy[i];
        }
        if (is_validWPoint(wp1) == FALSE) {
            continue;
        }

        vp1 = Wpoint2Vpoint(wp1);
        vp1.x += offset;

        if (dx_plus != NULL) {
            wp2 = wp1;
            wp2.x += fabs(dx_plus[i]);
            vp2 = Wpoint2Vpoint(wp2);
            vp2.x += offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
        if (dx_minus != NULL) {
            wp2 = wp1;
            wp2.x -= fabs(dx_minus[i]);
            vp2 = Wpoint2Vpoint(wp2);
            vp2.x += offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
        if (dy_plus != NULL) {
            wp2 = wp1;
            wp2.y += fabs(dy_plus[i]);
            vp2 = Wpoint2Vpoint(wp2);
            vp2.x += offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
        if (dy_minus != NULL) {
            wp2 = wp1;
            wp2.y -= fabs(dy_minus[i]);
            vp2 = Wpoint2Vpoint(wp2);
            vp2.x += offset;
            drawerrorbar(canvas, &vp1, &vp2, &p->errbar);
        }
    }
}

/*
 * draw hi/lo-open/close
 */
void drawsethilo(Canvas *canvas, Quark *pset)
{
    set *p = (set *) pset->data;
    int i;
    double *x = p->data->ex[0], *y1 = p->data->ex[1];
    double *y2 = p->data->ex[2], *y3 = p->data->ex[3], *y4 = p->data->ex[4];
    double ilen = 0.02*p->sym.size;
    int skip = p->symskip + 1;
    WPoint wp;
    VPoint vp1, vp2;

    if (p->sym.line.style != 0) {
        setline(canvas, &p->sym.line);
        for (i = 0; i < getsetlength(pset); i += skip) {
            wp.x = x[i];
            wp.y = y1[i];
            vp1 = Wpoint2Vpoint(wp);
            wp.y = y2[i];
            vp2 = Wpoint2Vpoint(wp);
            DrawLine(canvas, &vp1, &vp2);
            wp.y = y3[i];
            vp1 = Wpoint2Vpoint(wp);
            vp2 = vp1;
            vp2.x -= ilen;
            DrawLine(canvas, &vp1, &vp2);
            wp.y = y4[i];
            vp1 = Wpoint2Vpoint(wp);
            vp2 = vp1;
            vp2.x += ilen;
            DrawLine(canvas, &vp1, &vp2);
        }
    }
}

/*
 * draw 2D bars
 */
void drawsetbars(Canvas *canvas, Quark *pset,
                 int refn, double *refx, double *refy, double offset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
    int i, n;
    double *x, *y;
    double lw, bw = 0.01*p->sym.size;
    int skip = p->symskip + 1;
    double ybase;
    WPoint wp;
    VPoint vp1, vp2;
    int stacked_chart;
    
    if (get_graph_type(gr) == GRAPH_CHART) {
        x = refx;
        n = MIN2(getsetlength(pset), refn);
    } else {
        x = p->data->ex[0];
        n = getsetlength(pset);
    }
    y = p->data->ex[1];
    
    if (get_graph_type(gr) == GRAPH_CHART && is_graph_stacked(gr) == TRUE) {
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
    if (get_graph_type(gr) == GRAPH_CHART &&
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
                wp.y = refy[i];
            } else {
                wp.y = ybase;
            }
    	    vp1 = Wpoint2Vpoint(wp);
            vp1.x -= bw;
    	    vp1.x += offset;
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y += y[i];
            } else {
                wp.y = y[i];
            }
    	    vp2 = Wpoint2Vpoint(wp);
            vp2.x += bw;
    	    vp2.x += offset;
            
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
                wp.y = refy[i];
            } else {
                wp.y = ybase;
            }
    	    vp1 = Wpoint2Vpoint(wp);
            vp1.x -= bw;
    	    vp1.x += offset;
            wp.x = x[i];
            if (stacked_chart == TRUE) {
                wp.y += y[i];
            } else {
                wp.y = y[i];
            }
    	    vp2 = Wpoint2Vpoint(wp);
            vp2.x += bw;
    	    vp2.x += offset;

            vp1.x += lw/2.0;
            vp2.x -= lw/2.0;
            vp1.y += lw/2.0;

    	    DrawRect(canvas, &vp1, &vp2);
        }
    }
}

void drawcirclexy(Canvas *canvas, Quark *pset)
{
    set *p = (set *) pset->data;
    int i, setlen;
    double *x, *y, *r;
    int skip = p->symskip + 1;
    WPoint wp;
    VPoint vp1, vp2;

    setclipping(canvas, TRUE);
    
    setlen = getsetlength(pset);
    x = p->data->ex[0];
    y = p->data->ex[1];
    r = p->data->ex[2];

    setfillrule(canvas, p->line.fillrule);
    setline(canvas, &p->line.line);

    for (i = 0; i < setlen; i += skip) {
        wp.x = x[i];
        wp.y = y[i];
        /* TODO: remove once ellipse clipping works */
        if (!is_validWPoint(wp)){
            continue;
        }
        wp.x = x[i] - r[i];
        wp.y = y[i] - r[i];
        vp1 = Wpoint2Vpoint(wp);
        wp.x = x[i] + r[i];
        wp.y = y[i] + r[i];
        vp2 = Wpoint2Vpoint(wp);
        if (p->line.filltype != SETFILL_NONE) {
            setpen(canvas, &p->line.fillpen);
            DrawFilledEllipse(canvas, &vp1, &vp2);
        }
        setpen(canvas, &p->line.line.pen);
        DrawEllipse(canvas, &vp1, &vp2);
    }
}

/* Arrows for vector map plots */
void drawsetvmap(Canvas *canvas, Quark *pset)
{
    Quark *gr = pset->parent;
    set *p = (set *) pset->data;
    int i, setlen;
    double znorm = get_graph_znorm(gr);
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
    
    setlen = getsetlength(pset);
    x = p->data->ex[DATA_X];
    y = p->data->ex[DATA_Y];
    vx = p->data->ex[DATA_Y1];
    vy = p->data->ex[DATA_Y2];

    arrow.length = 2*eb.barsize;

    setpen(canvas, &p->errbar.pen);

    for (i = 0; i < setlen; i += skip) {
        wp.x = x[i];
        wp.y = y[i];
        if (!is_validWPoint(wp)){
            continue;
        }
        vp1 = Wpoint2Vpoint(wp);
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

void drawsetboxplot(Canvas *canvas, Quark *pset)
{
    set *p = (set *) pset->data;
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

    for (i = 0; i < getsetlength(pset); i += skip) {
        wp.x =  x[i];

        wp.y = lb[i];
        vp1 = Wpoint2Vpoint(wp);
        wp.y = ub[i];
        vp2 = Wpoint2Vpoint(wp);
        
        /* whiskers */
        if (p->errbar.active == TRUE) {
            VPoint vp3;
            wp.y = lw[i];
            vp3 = Wpoint2Vpoint(wp);
            drawerrorbar(canvas, &vp1, &vp3, &p->errbar);
            wp.y = uw[i];
            vp3 = Wpoint2Vpoint(wp);
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
        vp2 = vp1 = Wpoint2Vpoint(wp);
        vp1.x -= size;
        vp2.x += size;
        DrawLine(canvas, &vp1, &vp2);
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
    default:
        errmsg("Internal error in draw_arrowhead()");
        break;
    }

    return;
}

void draw_region(Canvas *canvas, region *this)
{
    int i;
    double vshift = 0.05;
    double xshift = 0.0, yshift = 0.0;
    
    int rgndouble=0;
    Arrow arrow;
    
    WPoint wptmp, wp1, wp2, wp3, wp4;
    VPoint vps[4], *vpstmp;
    Pen pen;
    getpen(canvas, &pen);

    set_default_arrow(&arrow);
    
    switch (this->type) {
    case REGION_POLYI:
    case REGION_POLYO:
        if (this->x != NULL && this->y != NULL && this->n > 2) {
            vpstmp = xmalloc (this->n*sizeof(VPoint));
            if (vpstmp == NULL) {
                errmsg("xmalloc error in draw_region()");
                return;
            } else {
                for (i = 0; i < this->n; i++) {
                    wptmp.x = this->x[i];
                    wptmp.y = this->y[i];
                    vpstmp[i] = Wpoint2Vpoint(wptmp);
                }
                DrawPolyline(canvas, vpstmp, this->n, POLYLINE_CLOSED);
		xfree(vpstmp);
            }
        }
        return;
    case REGION_ABOVE:
        xshift = 0.0;
        yshift = vshift;
        break;
    case REGION_BELOW:
        xshift = 0.0;
        yshift = -vshift;
        break;
    case REGION_TOLEFT:
        xshift = -vshift;
        yshift = 0.0;
        break;
    case REGION_TORIGHT:
        xshift = vshift;
        yshift = 0.0;
        break;
    case REGION_HORIZI:
    case REGION_HORIZO:
        wp1.x=this->x1;
	wp1.y=this->y1;
	wp2.x=this->x1;
	wp2.y=this->y2;
        wp3.x=this->x2;
	wp3.y=this->y1;
	wp4.x=this->x2;
	wp4.y=this->y2;
	rgndouble=1;
	break;
    case REGION_VERTI:
    case REGION_VERTO:
        wp1.x=this->x1;
	wp1.y=this->y1;
	wp2.x=this->x2;
	wp2.y=this->y1;
        wp3.x=this->x1;
	wp3.y=this->y2;
	wp4.x=this->x2;
	wp4.y=this->y2;
	rgndouble=1;
	break;
    default:
        errmsg("Internal error in draw_region");
        return;
    }
    
    if(!rgndouble) {
        wptmp.x = this->x1;
        wptmp.y = this->y1;
        vps[1] = Wpoint2Vpoint(wptmp);
        wptmp.x = this->x2;
        wptmp.y = this->y2;
        vps[2] = Wpoint2Vpoint(wptmp);
        vps[0].x = vps[1].x + xshift;
        vps[0].y = vps[1].y + yshift;
        vps[3].x = vps[2].x + xshift;
        vps[3].y = vps[2].y + yshift;
        DrawPolyline(canvas, vps, 4, POLYLINE_OPEN);
        draw_arrowhead(canvas, &vps[1], &vps[0], &arrow, &pen, &pen);
        draw_arrowhead(canvas, &vps[2], &vps[3], &arrow, &pen, &pen);
    } else {
        vps[0] = Wpoint2Vpoint(wp1);
        vps[1] = Wpoint2Vpoint(wp2);
        DrawLine(canvas, &vps[0], &vps[1]);
        vps[0] = Wpoint2Vpoint(wp3);
        vps[1] = Wpoint2Vpoint(wp4);
        DrawLine(canvas, &vps[0], &vps[1]);
        wp1.x=(wp1.x+wp2.x)/2;
        wp1.y=(wp1.y+wp2.y)/2;
        wp3.x=(wp3.x+wp4.x)/2;
        wp3.y=(wp3.y+wp4.y)/2;
        vps[0] = Wpoint2Vpoint(wp1);
        vps[1] = Wpoint2Vpoint(wp3);
        DrawLine(canvas, &vps[0], &vps[1]);
        draw_arrowhead(canvas, &vps[0], &vps[1], &arrow, &pen, &pen);
    }
}

/* ---------------------- legends ---------------------- */


/*
 * draw the legend
 */
void dolegend(Canvas *canvas, Quark *q)
{
#if 0
    int setno, nsets;
    int draw_flag;
    double maxsymsize;
    
    VPoint vp, vp2;
    double bb_width, bb_height;
    
    view v;
    legend *l;
    set *p;

    Quark **psets;

    l = frame_get_legend(gr);
    if (l->active == FALSE) {
        return;
    }
    
    maxsymsize = 0.0;
    draw_flag = FALSE;
    nsets = graph_get_sets(gr, &psets);
    for (setno = 0; setno < nsets; setno++) {
        Quark *pset = psets[setno];
        if (is_set_drawable(pset)) {
            p = (set *) pset->data;
            if (!is_empty_string(p->legstr)) {
                draw_flag = TRUE;
            }
            if (p->sym.size > maxsymsize) {
                maxsymsize = p->sym.size;
            }
        }  
    }
    xfree(psets);
    
    if (draw_flag == FALSE) {
        l->bb.xv1 = l->bb.xv2 = l->bb.yv1 = l->bb.yv2 = 0.0;
        return;
    }
        
    setclipping(canvas, FALSE);
    
    vp.x = vp.y = 0.0;
    activate_bbox(canvas, BBOX_TYPE_TEMP, TRUE);
    reset_bbox(canvas, BBOX_TYPE_TEMP);
    update_bbox(canvas, BBOX_TYPE_TEMP, &vp);
    
    set_draw_mode(canvas, FALSE);
    putlegends(canvas, gr, &vp, maxsymsize);
    get_bbox(canvas, BBOX_TYPE_TEMP, &v);
    
    bb_width  = fabs(v.xv2 - v.xv1) + 2*l->hgap;
    bb_height = fabs(v.yv2 - v.yv1) + 2*l->vgap;
    
    get_graph_viewport(gr, &v);
    
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
    
    set_draw_mode(canvas, TRUE);
    
    setpen(canvas, &l->boxfillpen);
    FillRect(canvas, &vp, &vp2);

    setline(canvas, &l->boxline);
    DrawRect(canvas, &vp, &vp2);
    
    /* correction */
    vp.x += l->hgap;
    vp.y -= l->vgap;
   
    reset_bbox(canvas, BBOX_TYPE_TEMP);
    update_bbox(canvas, BBOX_TYPE_TEMP, &vp);

    putlegends(canvas, gr, &vp, maxsymsize);
#endif
}

void putlegends(Canvas *canvas, Quark *gr, const VPoint *vp, double maxsymsize)
{
    int i, setno, nsets;
    VPoint vp1, vp2, vpstr;
    set *p;
    legend *l;
    int draw_line, singlesym;
    Quark **psets;
    
    l = frame_get_legend(gr);

    vp1.x = vp->x + 0.01*maxsymsize;
    vp2.y = vp->y;
    vp2.x = vp1.x + l->len;
    vpstr.y = vp->y;
    vpstr.x = vp2.x + l->hgap + 0.01*maxsymsize;
    
    nsets = graph_get_sets(gr, &psets);
    for (i = 0; i < nsets; i++) {
        Quark *pset;
        
        if (l->invert == FALSE) {
            setno = i;
        } else {
            setno = nsets - i - 1;
        }
        
        pset = psets[setno];
        
        if (is_set_drawable(pset)) {
            view vtmp;
            double symvshift;
            
            p = (set *) pset->data;
            
            if (is_empty_string(p->legstr)) {
                continue;
            }
            
            setcharsize(canvas, l->charsize);
            setfont(canvas, l->font);
            setcolor(canvas, l->color);
            WriteString(canvas, &vpstr, 0.0, JUST_LEFT|JUST_TOP, p->legstr);
            get_bbox(canvas, BBOX_TYPE_TEMP, &vtmp);
            vp1.y = (vpstr.y + vtmp.yv1)/2;
            vp2.y = vp1.y;
            vpstr.y = vtmp.yv1 - l->vgap;
            
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
                    
                    /* FIXME: BBox calculations */
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
                vptmp.x = (vp1.x + vp2.x)/2;
                vptmp.y = vp1.y + symvshift;
                
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
    xfree(psets);
}
