/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "globals.h"
#include "utils.h"
#include "graphs.h"
#include "draw.h"
#include "t1fonts.h"
#include "device.h"
#include "plotone.h"
#include "protos.h"

#define is_set_drawable(gno, setno) (is_set_active(gno, setno) && !is_set_hidden(gno, setno))
FILE *prstream;

char print_cmd[GR_MAXPATHLEN]  = PRINT_CMD;
char print_file[GR_MAXPATHLEN] = "";

/*
 * draw all active graphs, when graphs are drawn, draw the focus markers
 */
void drawgraph(void)
{
    int i;
    VPoint vp1, vp2;
    Pen pen;
    int saveg;

    saveg = get_cg();
    
    if (initgraphics() == GRACE_EXIT_FAILURE) {
        errmsg("Device wasn't properly initialized");
        return;
    }
    
    update_t1();
        
    pen.color = getbgcolor();
    pen.pattern = 1;
    setpen(pen);
        
    vp1.x = 0.0;
    vp1.y = 0.0;
    get_page_viewport(&vp2.x, &vp2.y);
    
    setclipping(FALSE);
    FillRect(vp1, vp2);
    
    reset_bboxes();
    activate_bbox(BBOX_TYPE_GLOB, TRUE);
    activate_bbox(BBOX_TYPE_TEMP, FALSE);
    
    for (i = 0; i < number_of_graphs(); i++) {
        plotone(i);
    }
    
    /* draw objects NOT clipped to a particular graph */
    draw_objects(-1);

    draw_timestamp();
    
    select_graph(saveg);

    leavegraphics();
}

/*
 * If writing to a file, check to see if it exists
 */
void do_hardcopy(void)
{
    char tbuf[128];
    char fname[GR_MAXPATHLEN];
    view v;
    double vx, vy;
    int truncated_out;
    
    if (ptofile) {
        if (fexists(print_file)) {
            return;
        }
        strcpy(fname, print_file);
    } else {
        tmpnam(fname);
        /* VMS doesn't like extensionless files */
        strcat(fname, ".prn");
    }
    
    prstream = filter_write(fname);
    
    if (prstream == NULL) {
        sprintf(tbuf, "Can't open %s for write, hardcopy aborted", fname);
        errmsg(tbuf);
        return;
    }
    
    select_device(hdevice);
    
    drawgraph();
    
    filter_close(prstream);
    
    v = get_bbox(BBOX_TYPE_GLOB);
    get_page_viewport(&vx, &vy);
    if (v.xv1 < 0.0 || v.xv2 > vx || v.yv1 < 0.0 || v.yv2 > vy) {
        truncated_out = TRUE;
    } else {
        truncated_out = FALSE;
    }
    
    if (ptofile == FALSE) {
        sprintf(tbuf, "%s %s", print_cmd, fname);
        if (truncated_out == FALSE ||
            !yesno("Printout is truncated. Abort?", NULL, NULL, NULL)) {
            system(tbuf);
        }
#ifndef PRINT_CMD_UNLINKS
        unlink(fname);
#endif
    } else {
        if (truncated_out == TRUE) {
            errmsg("Output is truncated - tune device dimensions");
        }
    }
    
    select_device(tdevice);
}


void plotone(int gno)
{
    
    if (is_graph_active(gno) != TRUE || is_graph_hidden(gno) == TRUE) {
        return;
    }

    /* sanity checks */
    if (checkon_world(gno) == FALSE) {
        return;
    }
    
    if (checkon_viewport(gno) == FALSE) {
        return;
    }
   
    setclipping(TRUE);
    
    set_draw_mode(TRUE);
    
    select_graph(gno);
    
    /* fill frame */
    fillframe(gno);
    
    /* calculate tick mark positions for all axes */
    calculate_tickgrid(gno);
    
    /* draw grid lines */
    drawgrid(gno);
    
    /* plot type specific routines */
    if (get_graph_type(gno) == GRAPH_POLAR) {
        draw_polar_graph(gno);
    } else if (get_graph_type(gno) == GRAPH_SMITH) {
        draw_smith_chart(gno);
    } else {
        xyplot(gno);
    }

    /* plot axes and tickmarks */
    drawaxes(gno);
    
    /* plot frame */
    drawframe(gno);

    /* mark the reference point */
    draw_ref_point(gno);
    
    /* plot objects */
    draw_objects(gno);
    
    /* plot legends */
    dolegend(gno);
    
    /* draw title and subtitle */
    draw_titles(gno);
}

void draw_smith_chart(int gno)
{
}

void draw_polar_graph(int gno)
{
    int i;
    plotarr p;

    for (i = 0; i < number_of_sets(gno); i++) {
        if (is_set_drawable(gno, i)) {
            get_graph_plotarr(gno, i, &p);
            switch (dataset_type(gno, i)) {
            case SET_XY:
                drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                drawsetsyms(gno, i, &p, 0, NULL, NULL, 0.0);
                drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
                break;
            default:
                errmsg("Unsupported in polar graph set type");
                break;
            }
        }
    }

    /* draw any defined regions for this graph */
/*
 *     for (i = 0; i < MAXREGION; i++) {
 *         if (rg[i].active && rg[i].linkto[gno]) {
 *             setcolor(rg[i].color);
 *             setlinewidth(rg[i].linew);
 *             setlinestyle(rg[i].lines);
 *             draw_region(i);
 *         }
 *     }
 */
}

void xyplot(int gno)
{
    int i, j;
    plotarr p;
    int refn;
    double *refx, *refy;
    double offset;

    refn = 0;
    offset = 0.0;
    refx = NULL;
    refy = NULL;

    /* draw sets */
    switch (get_graph_type(gno)) {
    case GRAPH_XY:
        for (i = 0; i < number_of_sets(gno); i++) {
            if (is_set_drawable(gno, i)) {
                get_graph_plotarr(gno, i, &p);
                switch (dataset_type(gno, i)) {
                case SET_XY:
                    drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetsyms(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
                    break;
                case SET_BAR:
                    drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetbars(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYDX:
                case SET_XYDY:
                case SET_XYDXDX:
                case SET_XYDYDY:
                case SET_XYDXDY:
                    drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetsyms(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawseterrbars(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYHILO:
                    drawsethilo(&p);
                    break;
                case SET_XYZ:
                    drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetsyms(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
                    break;
                case SET_XYR:
                    drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetsyms(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
/*
 *                  drawcirclexy(&g[gno].p[i]);
 */
                    break;
                case SET_XYSTRING:
                    drawsetline(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetsyms(gno, i, &p, 0, NULL, NULL, 0.0);
                    drawsetavalues(gno, i, &p, 0, NULL, NULL, 0.0);
                    break;
                default:
                    errmsg("Unsupported in XY graph set type");
                    break;
                }
            }
        }
        break;
    case GRAPH_CHART:
        for (i = 0; i < number_of_sets(gno); i++) {
            get_graph_plotarr(gno, i, &p);
            if (is_set_drawable(gno, i)) {
                if (p.len > refn) {
                    refn = p.len;
                    refx = p.ex[0];
                }
                if (is_graph_stacked(gno) != TRUE) {
                    offset -= 0.5*0.02*p.symsize;
                }
            }
        }
        offset -= 0.5*(nactive(gno) - 1)*get_graph_bargap(gno);
        
        if (is_graph_stacked(gno) == TRUE) {
            refy = calloc(refn, SIZEOF_DOUBLE);
            if (refy == NULL) {
                errmsg("Memory allocation failed in plotone()");
                return;
            }
        }

        for (i = 0; i < number_of_sets(gno); i++) {
            get_graph_plotarr(gno, i, &p);
            if (is_set_drawable(gno, i)) {
                if (is_graph_stacked(gno) != TRUE) {
                    offset += 0.5*0.02*p.symsize;
                }
                switch (dataset_type(gno, i)) {
                case SET_XY:
                    drawsetline(gno, i, &p, refn, refx, refy, offset);
                    if (is_graph_stacked(gno) != TRUE) {
                        drawsetsyms(gno, i, &p, refn, refx, refy, offset);
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                    }
                    break;
                case SET_BAR:
                    drawsetline(gno, i, &p, refn, refx, refy, offset);
                    drawsetbars(gno, i, &p, refn, refx, refy, offset);
                    if (is_graph_stacked(gno) != TRUE) {
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                    }
                    break;
                case SET_BARDY:
                case SET_BARDYDY:
                    drawsetline(gno, i, &p, refn, refx, refy, offset);
                    drawsetbars(gno, i, &p, refn, refx, refy, offset);
                    if (is_graph_stacked(gno) != TRUE) {
                        drawseterrbars(gno, i, &p, refn, refx, refy, offset);
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                    }
                    break;
                case SET_XYDY:
                case SET_XYDYDY:
                    drawsetline(gno, i, &p, refn, refx, refy, offset);
                    if (is_graph_stacked(gno) != TRUE) {
                        drawsetsyms(gno, i, &p, refn, refx, refy, offset);
                        drawseterrbars(gno, i, &p, refn, refx, refy, offset);
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                    }
                    break;
                default:
                    errmsg("Unsupported in XY chart set type");
                    break;
                }
                if (is_graph_stacked(gno) != TRUE) {
                    offset += 0.5*0.02*p.symsize + get_graph_bargap(gno);
                } else {
                    for (j = 0; j < p.len; j++) {
                        refy[j] += p.ex[1][j];
                    }
                }
            }
        }
        
        if (is_graph_stacked(gno) == TRUE) {
            /* Second pass for stacked charts: symbols and avalues */
            offset = 0.0;
            for (j = 0; j < refn; j++) {
                refy[j] = 0.0;
            }
            
            for (i = 0; i < number_of_sets(gno); i++) {
                get_graph_plotarr(gno, i, &p);
                if (is_set_drawable(gno, i)) {
                    switch (dataset_type(gno, i)) {
                    case SET_XY:
                        drawsetsyms(gno, i, &p, refn, refx, refy, offset);
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                        break;
                    case SET_BAR:
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                        break;
                    case SET_BARDY:
                    case SET_BARDYDY:
                        drawseterrbars(gno, i, &p, refn, refx, refy, offset);
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                        break;
                    case SET_XYDY:
                    case SET_XYDYDY:
                        drawsetsyms(gno, i, &p, refn, refx, refy, offset);
                        drawseterrbars(gno, i, &p, refn, refx, refy, offset);
                        drawsetavalues(gno, i, &p, refn, refx, refy, offset);
                        break;
                    }
                    
                    for (j = 0; j < p.len; j++) {
                        refy[j] += p.ex[1][j];
                    }
                }
            }
        }

        if (refy != NULL) {
            free(refy);
        }
        break;
    } /* end g.type */

    /* draw any defined regions for this graph */
    for (i = 0; i < MAXREGION; i++) {
        if (rg[i].active && rg[i].linkto[gno]) {
            setcolor(rg[i].color);
            setpattern(1);
            setlinewidth(rg[i].linew);
            setlinestyle(rg[i].lines);
            draw_region(i);
        }
    }
}

void draw_ref_point(int gno)
{
    GLocator locator;
    WPoint wp;
    VPoint vp;
    
    if (is_refpoint_active(gno)) {      
        get_graph_locator(gno, &locator);
        wp.x = locator.dsx;
        wp.y = locator.dsy;
        vp = Wpoint2Vpoint(wp);
        setcolor(1);
        setpattern(1);
        symplus(vp, 0.01);
        DrawCircle (vp, 0.01);
    }
}


/* draw title and subtitle */
void draw_titles(int gno)
{
    view v;
    labels lab;
    VPoint vp1, vp2;
    
    get_graph_viewport(gno, &v);
    get_graph_labels(gno, &lab);

    vp1.x = (v.xv2 + v.xv1) / 2;
    vp1.y = (v.yv2 < v.yv1)? v.yv1 : v.yv2;
    vp2 = vp1;
    if (lab.title.s[0]) {
        setcolor(lab.title.color);
        setcharsize(lab.title.charsize);
        setfont(lab.title.font);
        vp1.y += 0.06;
        WriteString(vp1, 0, JUST_CENTER|JUST_BOTTOM|JUST_BBOX, lab.title.s);
    }
    if (lab.stitle.s[0]) {
        setcolor(lab.stitle.color);
        setcharsize(lab.stitle.charsize);
        setfont(lab.stitle.font);
        vp2.y += 0.02;
        WriteString(vp2, 0, JUST_CENTER|JUST_BOTTOM|JUST_BBOX, lab.stitle.s);
    }
}

/*
 * draw the graph frame
 */
void drawframe(int gno)
{
    view v;
    framep f;
    VPoint vps[4];

    get_graph_viewport(gno, &v);
    get_graph_framep(gno, &f);

    setpen(f.pen);
    setlinewidth(f.linew);
    setlinestyle(f.lines);

    switch (f.type) {
    case 0:
        vps[0].x = v.xv1;
        vps[0].y = v.yv1;
        vps[1].x = v.xv2;
        vps[1].y = v.yv2;
        DrawRect(vps[0], vps[1]);
        break;
    case 1:                     /* half open */
        vps[0].x = v.xv1;
        vps[0].y = v.yv2;
        vps[1].x = v.xv1;
        vps[1].y = v.yv1;
        vps[2].x = v.xv2;
        vps[2].y = v.yv1;
        DrawPolyline(vps, 3, POLYLINE_OPEN);
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
        DrawPolyline(vps, 4, POLYLINE_OPEN);
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
        DrawPolyline(vps, 4, POLYLINE_OPEN);
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
        DrawPolyline(vps, 4, POLYLINE_OPEN);
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
        DrawPolyline(vps, 4, POLYLINE_OPEN);
        break;
    }
}

void fillframe(int gno)
{
    view v;
    framep f;
    VPoint vp1, vp2;

    get_graph_viewport(gno, &v);
    get_graph_framep(gno, &f);
    
    /* fill coordinate frame with background color */
    if (f.fillpen.pattern != 0) {
        setpen(f.fillpen);
        vp1.x = v.xv1;
        vp1.y = v.yv1;
        vp2.x = v.xv2;
        vp2.y = v.yv2;
        FillRect(vp1, vp2);
    }
}    

/*
 * draw a set filling polygon
 */
void drawsetfill(int gno, int setno, plotarr *p,
                 int refn, double *refx, double *refy, double offset)
{
    int i, len, setlen, polylen;
    int line_type = p->linet;
    double *x, *y;
    double ybase;
    world w;
    WPoint wptmp;
    VPoint *vps;
    double xmin, xmax, ymin, ymax;
    int stacked_chart;
    
    if (p->filltype == SETFILL_NONE) {
        return;
    }
    
    if (get_graph_type(gno) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(p->len, refn);
    } else {
        x = p->ex[0];
        setlen = p->len;
    }
    y = p->ex[1];
    
    if (get_graph_type(gno) == GRAPH_CHART && is_graph_stacked(gno) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    setclipping(TRUE);
    
    get_graph_world(gno, &w);

    switch (line_type) {
    case LINE_TYPE_STRAIGHT:
    case LINE_TYPE_SEGMENT2:
    case LINE_TYPE_SEGMENT3:
        if (stacked_chart == TRUE && p->filltype == SETFILL_BASELINE) {
            len = 2*setlen;
        } else {
            len = setlen;
        }
        vps = (VPoint *) malloc((len + 2) * sizeof(VPoint));
        if (vps == NULL) {
            errmsg("Can't malloc in drawsetfill");
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
        if (stacked_chart == TRUE && p->filltype == SETFILL_BASELINE) {
            for (i = 0; i < setlen; i++) {
                wptmp.x = x[setlen - i - 1];
                wptmp.y = refy[setlen - i - 1];
                vps[setlen + i] = Wpoint2Vpoint(wptmp);
    	        vps[setlen + i].x += offset;
            }
        }
        break;
    case LINE_TYPE_LEFTSTAIR:
        len = 2*setlen - 1;
        vps = (VPoint *) malloc((len + 2) * sizeof(VPoint));
        if (vps == NULL) {
            errmsg("Can't malloc in drawsetfill");
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
            vps[i].x = vps[i - 1].x;
            vps[i].y = vps[i + 1].y;
        }
        break;
    case LINE_TYPE_RIGHTSTAIR:
        len = 2*setlen - 1;
        vps = (VPoint *) malloc((len + 2) * sizeof(VPoint));
        if (vps == NULL) {
            errmsg("Can't malloc in drawsetfill");
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
            vps[i].x = vps[i + 1].x;
            vps[i].y = vps[i - 1].y;
        }
        break;
    default:
        return;
    }
    
    
    switch (p->filltype) {
    case SETFILL_POLYGON:
        polylen = len;
        break;
    case SETFILL_BASELINE:
        if (stacked_chart == TRUE) {
            polylen = len;
        } else {
            getsetminmax(gno, setno, &xmin, &xmax, &ymin, &ymax);
            ybase = setybase(gno, setno);
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
        free(vps);
        return;
    }
    
    setpen(p->setfillpen);
    setfillrule(p->fillrule);
    DrawPolygon(vps, polylen);
    
    free(vps);
}

/*
 * draw set's connecting line
 */
void drawsetline(int gno, int setno, plotarr *p,
                 int refn, double *refx, double *refy, double offset)
{
    int setlen;
    int i, ly = p->lines, wy = p->linew;
    int line_type = p->linet;
    VPoint vps[4], *vpstmp;
    WPoint wp;
    double *x, *y;
    double ybase;
    double xmin, xmax, ymin, ymax;
    int stacked_chart;
    
    if (get_graph_type(gno) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(p->len, refn);
    } else {
        x = p->ex[0];
        setlen = p->len;
    }
    y = p->ex[1];
    
    if (get_graph_type(gno) == GRAPH_CHART && is_graph_stacked(gno) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    if (stacked_chart == TRUE) {
        ybase = 0.0;
    } else {
        ybase = setybase(gno, setno);
    }
    
    setclipping(TRUE);

    drawsetfill(gno, setno, p, refn, refx, refy, offset);

/* draw the line */
    if (ly != 0) {
        setpen(p->linepen);
        setlinewidth(wy);
        setlinestyle(ly);
        
        switch (line_type) {
        case LINE_TYPE_NONE:
            break;
        case LINE_TYPE_STRAIGHT:
            vpstmp = (VPoint *) malloc(setlen*sizeof(VPoint));
            if (vpstmp == NULL) {
                errmsg("malloc failed in drawsetline()");
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
                if (stacked_chart == TRUE) {
                    vpstmp[i].y -= getlinewidth()/2.0;
                }
            }
            DrawPolyline(vpstmp, setlen, POLYLINE_OPEN);
            free(vpstmp);
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
                if (stacked_chart == TRUE) {
                    vps[0].y -= getlinewidth()/2.0;
                    vps[1].y -= getlinewidth()/2.0;
                }
                DrawLine(vps[0], vps[1]);
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
                DrawPolyline(vps, 3, POLYLINE_OPEN);
                if (stacked_chart == TRUE) {
                    vps[0].y -= getlinewidth()/2.0;
                    vps[1].y -= getlinewidth()/2.0;
                    vps[2].y -= getlinewidth()/2.0;
                }
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
                if (stacked_chart == TRUE) {
                    vps[0].y -= getlinewidth()/2.0;
                    vps[1].y -= getlinewidth()/2.0;
                }
                DrawLine(vps[0], vps[1]);
            }
            break;
        case LINE_TYPE_LEFTSTAIR:
            for (i = 0; i < setlen - 1; i ++) {
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
                vps[2] = Wpoint2Vpoint(wp);
    	        vps[2].x += offset;
                vps[1].x = vps[0].x;
                vps[1].y = vps[2].y;
                if (stacked_chart == TRUE) {
                    vps[0].y -= getlinewidth()/2.0;
                    vps[1].y -= getlinewidth()/2.0;
                    vps[2].y -= getlinewidth()/2.0;
                }
               
                DrawPolyline(vps, 3, POLYLINE_OPEN);
            }
            break;
        case LINE_TYPE_RIGHTSTAIR:
            for (i = 0; i < setlen - 1; i ++) {
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
                vps[2] = Wpoint2Vpoint(wp);
    	        vps[2].x += offset;
                vps[1].x = vps[2].x;
                vps[1].y = vps[0].y;
                if (stacked_chart == TRUE) {
                    vps[0].y -= getlinewidth()/2.0;
                    vps[1].y -= getlinewidth()/2.0;
                    vps[2].y -= getlinewidth()/2.0;
                }
               
                DrawPolyline(vps, 3, POLYLINE_OPEN);
            }
            break;
        default:
            errmsg("Invalid line type");
            break;
        }
    }

    if (p->dropline == TRUE) {
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
            if (stacked_chart == TRUE) {
                vps[1].y -= getlinewidth()/2.0;
            }
 
            DrawLine(vps[0], vps[1]);
        }
    }
    
    getsetminmax(gno, setno, &xmin, &xmax, &ymin, &ymax);
       
    if (p->baseline == TRUE && stacked_chart != TRUE) {
        wp.x = xmin;
        wp.y = ybase;
        vps[0] = Wpoint2Vpoint(wp);
    	vps[0].x += offset;
        wp.x = xmax;
        vps[1] = Wpoint2Vpoint(wp);
    	vps[1].x += offset;
 
        DrawLine(vps[0], vps[1]);
    }
}    

/* draw the symbols */
void drawsetsyms(int gno, int setno, plotarr *p,
                 int refn, double *refx, double *refy, double offset)
{
    int setlen;
    int i, sy = p->sym;
    double symsize = p->symsize;
    VPoint vp;
    WPoint wp;
    double *x, *y;
    int skip = p->symskip;
    int stacked_chart;
    
    if (get_graph_type(gno) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(p->len, refn);
    } else {
        x = p->ex[0];
        setlen = p->len;
    }
    y = p->ex[1];
    
    if (get_graph_type(gno) == GRAPH_CHART && is_graph_stacked(gno) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    skip++;
    
    setclipping(FALSE);
    
    if ((p->sympen.pattern != 0 && p->symlines != 0) ||
                        (p->symfillpen.pattern != 0)) {
              
        setlinewidth(p->symlinew);
        setlinestyle(p->symlines);
        setfont(p->charfont);
        setcharsize(symsize);
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
            
            if (drawxysym(vp, sy, p->sympen, p->symfillpen, p->symchar) != GRACE_EXIT_SUCCESS) {
                return;
            }
        } 
    }
}


/* draw the annotative values */
void drawsetavalues(int gno, int setno, plotarr *p,
                 int refn, double *refx, double *refy, double offset)
{
    int i;
    int setlen;
    double *x, *y, *z;
    WPoint wp;
    VPoint vp;
    int skip = p->symskip;
    AValue avalue;
    char str[MAX_STRING_LENGTH];
    int stacked_chart;

    avalue = p->avalue;
    if (avalue.active != TRUE) {
        return;
    }

    skip++;
    
    if (get_graph_type(gno) == GRAPH_CHART) {
        x = refx;
        setlen = MIN2(p->len, refn);
    } else {
        x = p->ex[0];
        setlen = p->len;
    }
    y = p->ex[1];
    
    if (dataset_cols(gno, setno) > 2) {
        z = p->ex[2];
    } else {
        z = NULL;
    }
    
    if (get_graph_type(gno) == GRAPH_CHART && is_graph_stacked(gno) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    setcharsize(avalue.size);
    setfont(avalue.font);
    setcolor(avalue.color);
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
            if (p->s != NULL && p->s[i] != NULL) {
                strcat(str, p->s[i]);
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
        
        WriteString(vp, avalue.angle,
                        JUST_CENTER|JUST_BOTTOM|JUST_BBOX, str);
    } 
}

void drawseterrbars(int gno, int setno, plotarr *p,
                 int refn, double *refx, double *refy, double offset)
{
    int i, n;
    double *x, *y;
    double *dx, *dy;
    int etype = p->errbar.type;
    double ebarlen = p->errbar.length;
    WPoint wp1, wp2, wp3, wp4;
    VPoint vp1, vp2;
    int stacked_chart;

    if (get_graph_type(gno) == GRAPH_CHART) {
        x = refx;
        n = MIN2(p->len, refn);
    } else {
        x = p->ex[0];
        n = p->len;
    }
    y = p->ex[1];
    
    if (get_graph_type(gno) == GRAPH_CHART && is_graph_stacked(gno) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    switch (p->type) {
    case SET_XYDX:
    case SET_XYDY:
    case SET_BARDY:
        dx = p->ex[2];
        dy = p->ex[2];
        break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_BARDYDY:
        dx = p->ex[2];
        dy = p->ex[3];
        break;
    default:
        return;
    }

    setclipping(FALSE);
    
    setpen(p->sympen);
    
/*
 * draw the riser
 */
    if (p->errbar.riser_lines != 0) {
        setlinewidth(p->errbar.riser_linew);
        setlinestyle(p->errbar.riser_lines);
        for (i = 0; i < n; i++) {
            wp1.x = x[i];
            wp1.y = y[i];
            if (stacked_chart == TRUE) {
                wp1.y += refy[i];
            }
            if (is_validWPoint(wp1) == FALSE) {
                continue;
            }
            wp4 = wp3 = wp2 = wp1;
            switch (p->type) {
            case SET_XYDY:
            case SET_XYDYDY:
            case SET_BARDY:
            case SET_BARDYDY:
                switch (etype) {
                case PLACE_BOTH:
                    wp1.y -= dy[i];
                    wp2.y += dx[i];
                    break;
                case PLACE_TOP:
                    wp2.y += dx[i];
                    break;
                case PLACE_BOTTOM:
                    wp2.y -= dy[i];
                    break;
                }
                break;
            case SET_XYDX:
            case SET_XYDXDX:
                switch (etype) {
                case PLACE_BOTH:
                    wp1.x -= dy[i];
                    wp2.x += dx[i];
                    break;
                case PLACE_LEFT:
                    wp2.x -= dy[i];
                    break;
                case PLACE_RIGHT:
                    wp2.x += dx[i];
                    break;
                }
                break;
            case SET_XYDXDY:
                switch (etype) {
                case PLACE_BOTH:
                    wp1.x -= dx[i];
                    wp2.x += dx[i];
                    wp3.y -= dy[i];
                    wp4.y += dy[i];
                    break;
                case PLACE_LEFT:
                    wp2.x -= dx[i];
                    wp4.y -= dy[i];
                    break;
                case PLACE_RIGHT:
                    wp2.x += dx[i];
                    wp4.y += dy[i];
                    break;
                }
                vp1 = Wpoint2Vpoint(wp3);
                vp2 = Wpoint2Vpoint(wp4);
    	        vp1.x += offset;
    	        vp2.x += offset;
                DrawLine(vp1, vp2);
                break;
            }
            vp1 = Wpoint2Vpoint(wp1);
            vp2 = Wpoint2Vpoint(wp2);
    	    vp1.x += offset;
    	    vp2.x += offset;
            DrawLine(vp1, vp2);
        }
    }
/*
 * draw the bar
 */
    if (p->errbar.lines != 0) {
        setlinewidth(p->errbar.linew);
        setlinestyle(p->errbar.lines);
        for (i = 0; i < n; i++) {
            wp1.x = x[i];
            wp1.y = y[i];
            if (stacked_chart == TRUE) {
                wp1.y += refy[i];
            }
            if (is_validWPoint(wp1) == FALSE) {
                continue;
            }
            wp4 = wp3 = wp2 = wp1;
            switch (p->type) {
            case SET_XYDY:
            case SET_XYDYDY:
            case SET_BARDY:
            case SET_BARDYDY:
                switch (etype) {
                case PLACE_BOTH:
                    wp1.y -= dy[i];
                    wp2.y += dx[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_HORIZONTAL);
                    drawerrorbar(wp2, offset, ebarlen, BAR_HORIZONTAL);
                    break;
                case PLACE_TOP:
                    wp1.y += dx[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_HORIZONTAL);
                    break;
                case PLACE_BOTTOM:
                    wp1.y -= dy[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_HORIZONTAL);
                    break;
                }
                break;
            case SET_XYDX:
            case SET_XYDXDX:
                switch (etype) {
                case PLACE_BOTH:
                    wp1.x -= dy[i];
                    wp2.x += dx[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_VERTICAL);
                    drawerrorbar(wp2, offset, ebarlen, BAR_VERTICAL);
                    break;
                case PLACE_LEFT:
                    wp1.x -= dy[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_VERTICAL);
                    break;
                case PLACE_RIGHT:
                    wp1.x += dx[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_VERTICAL);
                    break;
                }
                break;
            case SET_XYDXDY:
                switch (etype) {
                case PLACE_BOTH:
                    wp1.x -= dx[i];
                    wp2.x += dx[i];
                    wp3.y -= dy[i];
                    wp4.y += dy[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_VERTICAL);
                    drawerrorbar(wp2, offset, ebarlen, BAR_VERTICAL);
                    drawerrorbar(wp3, offset, ebarlen, BAR_HORIZONTAL);
                    drawerrorbar(wp4, offset, ebarlen, BAR_HORIZONTAL);
                    break;
                case PLACE_LEFT:
                    wp1.x -= dx[i];
                    wp3.y -= dy[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_VERTICAL);
                    drawerrorbar(wp3, offset, ebarlen, BAR_HORIZONTAL);
                    break;
                case PLACE_RIGHT:
                    wp1.x += dx[i];
                    wp3.y += dy[i];
                    drawerrorbar(wp1, offset, ebarlen, BAR_VERTICAL);
                    drawerrorbar(wp3, offset, ebarlen, BAR_HORIZONTAL);
                    break;
                }
                break;
            }
        }
    }
}

/*
 * draw hi/lo-open/close
 */
void drawsethilo(plotarr *p)
{
    int i;
    double *x = p->ex[0], *y1 = p->ex[1];
    double *y2 = p->ex[2], *y3 = p->ex[3], *y4 = p->ex[4];
    double ilen = 0.02*p->symsize;
    WPoint wp;
    VPoint vp1, vp2;

    if (p->symlines != 0) {
        setpen(p->sympen);
        setlinewidth(p->symlinew);
        setlinestyle(p->symlines);
        for (i = 0; i < p->len; i++) {
            wp.x = x[i];
            wp.y = y1[i];
            vp1 = Wpoint2Vpoint(wp);
            wp.y = y2[i];
            vp2 = Wpoint2Vpoint(wp);
            DrawLine(vp1, vp2);
            wp.y = y3[i];
            vp1 = Wpoint2Vpoint(wp);
            vp2 = vp1;
            vp2.x -= ilen;
            DrawLine(vp1, vp2);
            wp.y = y4[i];
            vp1 = Wpoint2Vpoint(wp);
            vp2 = vp1;
            vp2.x += ilen;
            DrawLine(vp1, vp2);
        }
    }
}

/*
 * draw 2D bars
 */
void drawsetbars(int gno, int setno, plotarr *p,
                 int refn, double *refx, double *refy, double offset)
{
    int i, n;
    double *x, *y;
    double bw = 0.01*p->symsize;
    double ybase;
    WPoint wp;
    VPoint vp1, vp2;
    int stacked_chart;
    
    if (get_graph_type(gno) == GRAPH_CHART) {
        x = refx;
        n = MIN2(p->len, refn);
    } else {
        x = p->ex[0];
        n = p->len;
    }
    y = p->ex[1];
    
    if (get_graph_type(gno) == GRAPH_CHART && is_graph_stacked(gno) == TRUE) {
        stacked_chart = TRUE;
    } else {
        stacked_chart = FALSE;
    }
    
    if (stacked_chart == TRUE) {
        ybase = 0.0;
    } else {
        ybase = setybase(gno, setno);
    }

    if (p->symfillpen.pattern != 0) {
	setpen(p->symfillpen);
        for (i = 0; i < n; i++) {
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
            FillRect(vp1, vp2);
        }
    }
    if (p->symlines != 0 && p->sympen.pattern != 0) {
        setpen(p->sympen);
        setlinewidth(p->symlinew);
        setlinestyle(p->symlines);
        for (i = 0; i < n; i++) {
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
            if (get_graph_type(gno) == GRAPH_CHART) {
                vp1.x += getlinewidth()/2.0;
                vp2.x -= getlinewidth()/2.0;
            }
            vp1.y += getlinewidth()/2.0;
    	    DrawRect(vp1, vp2);
        }
    }
}


int drawxysym(VPoint vp, int symtype, Pen sympen, Pen symfillpen, char s)
{   
    VPoint vps[4];
    char buf[2];
    double symsize = 0.01 * getcharsize();
    
    switch (symtype) {
    case SYM_NONE:
        break;
    case SYM_CIRCLE:
        setpen(symfillpen);
        DrawFilledCircle (vp, symsize);
        setpen(sympen);
        DrawCircle (vp, symsize);
        break;
    case SYM_SQUARE:
        symsize *= 0.85;
        vps[0].x = vp.x - symsize;
        vps[0].y = vp.y - symsize;
        vps[1].x = vps[0].x;
        vps[1].y = vp.y + symsize;
        vps[2].x = vp.x + symsize;
        vps[2].y = vps[1].y;
        vps[3].x = vps[2].x;
        vps[3].y = vps[0].y;
        
        setpen(symfillpen);
        DrawPolygon (vps, 4);
        setpen(sympen);
        DrawPolyline (vps, 4, POLYLINE_CLOSED);
        break;
    case SYM_DIAMOND:
        vps[0].x = vp.x;
        vps[0].y = vp.y + symsize;
        vps[1].x = vp.x - symsize;
        vps[1].y = vp.y;
        vps[2].x = vps[0].x;
        vps[2].y = vp.y - symsize;
        vps[3].x = vp.x + symsize;
        vps[3].y = vps[1].y;
        
        setpen(symfillpen);
        DrawPolygon (vps, 4);
        setpen(sympen);
        DrawPolyline (vps, 4, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG1:
        vps[0].x = vp.x;
        vps[0].y = vp.y + symsize;
        vps[1].x = vp.x - symsize;
        vps[1].y = vp.y - symsize;
        vps[2].x = vp.x + symsize;
        vps[2].y = vps[1].y;
        
        setpen(symfillpen);
        DrawPolygon (vps, 3);
        setpen(sympen);
        DrawPolyline (vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG2:
        vps[0].x = vp.x - symsize;
        vps[0].y = vp.y;
        vps[1].x = vp.x + symsize;
        vps[1].y = vp.y - symsize;
        vps[2].x = vps[1].x;
        vps[2].y = vp.y + symsize;
        
        setpen(symfillpen);
        DrawPolygon (vps, 3);
        setpen(sympen);
        DrawPolyline (vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG3:
        vps[0].x = vp.x - symsize;
        vps[0].y = vp.y + symsize;
        vps[1].x = vp.x;
        vps[1].y = vp.y - symsize;
        vps[2].x = vp.x + symsize;
        vps[2].y = vps[0].y;
        
        setpen(symfillpen);
        DrawPolygon (vps, 3);
        setpen(sympen);
        DrawPolyline (vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_TRIANG4:
        vps[0].x = vp.x - symsize;
        vps[0].y = vp.y + symsize;
        vps[1].x = vps[0].x;
        vps[1].y = vp.y - symsize;
        vps[2].x = vp.x + symsize;
        vps[2].y = vp.y;
        
        setpen(symfillpen);
        DrawPolygon (vps, 3);
        setpen(sympen);
        DrawPolyline (vps, 3, POLYLINE_CLOSED);
        break;
    case SYM_PLUS:
        setpen(sympen);
        symplus(vp, symsize);
        break;
    case SYM_X:
        setpen(sympen);
        symx(vp, symsize);
        break;
    case SYM_SPLAT:
        setpen(sympen);
        symsplat(vp, symsize);
        break;
    case SYM_CHAR:
        setcolor(sympen.color);
        buf[0] = s;
        buf[1] = '\0';
        WriteString(vp, 0, JUST_CENTER|JUST_MIDDLE|JUST_BBOX, buf);
        break;
    default:
        errmsg("Invalid symbol type");
        return GRACE_EXIT_FAILURE;
    }
    return GRACE_EXIT_SUCCESS;
}

void drawerrorbar(WPoint wp, double offset, double ebarlen, int orient)
{
    double ilen;
    VPoint vp, vp1, vp2;
    
    ilen = 0.01*ebarlen;
    vp = Wpoint2Vpoint(wp);
    vp.x += offset;
    vp2 = vp1 = vp;
    if (orient == BAR_HORIZONTAL) {
        vp1.x -= ilen;
        vp2.x += ilen;
    } else {
        vp1.y -= ilen;
        vp2.y += ilen;
    }
    DrawLine(vp1, vp2);
}

/* --------------------------------------------------------------- */
/* Objects ... TODO: move to draw.c or separate file */

void draw_objects(int gno)
{
    int i;

    setclipping(FALSE);         /* shut down clipping for strings, boxes,
                                 * lines, and legends */
    
    /* Temporarily; pattern property should be part of object props */
    setpattern(1);
    
    if (debuglevel == 5) {
        printf("Boxes\n");
    }
    for (i = 0; i < maxboxes; i++) {
        if (isactive_box(i)) {
            draw_box(gno, i);
        }
    }
    if (debuglevel == 5) {
        printf("Ellipses\n");
    }
    for (i = 0; i < maxellipses; i++) {
        if (isactive_ellipse(i)) {
            draw_ellipse(gno, i);
        }
    }
    if (debuglevel == 5) {
        printf("Lines\n");
    }
    for (i = 0; i < maxlines; i++) {
        if (isactive_line(i)) {
            draw_line(gno, i);
        }
    }
    if (debuglevel == 5) {
        printf("Strings\n");
    }
    for (i = 0; i < maxstr; i++) {
        if (isactive_string(i)) {
            if (debuglevel == 5) {
                printf("String %d\n", i);
            }
            draw_string(gno, i);
        }
    }
    setclipping(TRUE);
}


/*
 * draw annotative text
 */
void draw_string(int gno, int i)
{
    plotstr pstr;
    VPoint vp;
    WPoint wptmp;

    get_graph_string(i, &pstr);

    if (gno != -2) {
        if (pstr.loctype == COORD_WORLD && pstr.gno != gno) {
            return;
        }
        if (pstr.loctype == COORD_VIEW && gno != -1) {
            return;
        }
    }

    if (strlen(pstr.s) && (pstr.charsize > 0.0) && pstr.active) {
        if (pstr.loctype == COORD_WORLD) {
            wptmp.x = pstr.x;
            wptmp.y = pstr.y;
            vp = Wpoint2Vpoint(wptmp);
        } else {
            vp.x = pstr.x;
            vp.y = pstr.y;
        }

        setcolor(pstr.color);
        setpattern(1);
        setcharsize(pstr.charsize);
        setfont(pstr.font);
        WriteString(vp, pstr.rot, pstr.just|JUST_MIDDLE|JUST_BBOX, pstr.s);
    }
}

/*
 * draw annotative boxes
 */
void draw_box(int gno, int i)
{
    boxtype b;
    WPoint wptmp;
    VPoint vp1, vp2;

    get_graph_box(i, &b);
    if (gno != -2) {
        if (b.loctype == COORD_WORLD && b.gno != gno) {
            return;
        }
        if (b.loctype == COORD_VIEW && gno != -1) {
            return;
        }
    }
    if (b.active) {
        setclipping(FALSE);

        if (b.loctype == COORD_WORLD) {
            wptmp.x = b.x1;
            wptmp.y = b.y1;
            vp1 = Wpoint2Vpoint(wptmp);
            wptmp.x = b.x2;
            wptmp.y = b.y2;
            vp2 = Wpoint2Vpoint(wptmp);
        } else {
            vp1.x = b.x1;
            vp1.y = b.y1;
            vp2.x = b.x2;
            vp2.y = b.y2;
        }
        
        setcolor(b.fillcolor);
        setpattern(b.fillpattern);
        FillRect(vp1, vp2);
        
        setcolor(b.color);
        setlinewidth(b.linew);
        setlinestyle(b.lines);
        setpattern(1);
        DrawRect(vp1, vp2);
        
        setclipping(TRUE);
    }
}

/* draw annotative ellipses */
void draw_ellipse(int gno, int i)
{
    WPoint wptmp;
    VPoint vp1, vp2;
    ellipsetype b;

    b = ellip[i];
        
    if (gno != -2) {
        if (b.loctype == COORD_WORLD && b.gno != gno) {
            return;
        }
        if (b.loctype == COORD_VIEW && gno != -1) {
            return;
        }
    }
    if (b.active) {
        setclipping(FALSE);

        if (b.loctype == COORD_WORLD) {
            wptmp.x = b.x1;
            wptmp.y = b.y1;
            vp1 = Wpoint2Vpoint(wptmp);
            wptmp.x = b.x2;
            wptmp.y = b.y2;
            vp2 = Wpoint2Vpoint(wptmp);
        } else {
            vp1.x = b.x1;
            vp1.y = b.y1;
            vp2.x = b.x2;
            vp2.y = b.y2;
        }
        
        setcolor(b.fillcolor);
        setpattern(b.fillpattern);
        DrawFilledEllipse(vp1, vp2);
        
        setcolor(b.color);
        setlinewidth(b.linew);
        setlinestyle(b.lines);
        setpattern(1);
        DrawEllipse(vp1, vp2);
        
        setclipping(TRUE);
    }
}

/*
 * draw annotative lines
 */
void draw_line(int gno, int i)
{
    linetype l;
    WPoint wptmp;
    VPoint vp1, vp2;

    get_graph_line(i, &l);
    if (gno != -2) {
        if (l.loctype == COORD_WORLD && l.gno != gno) {
            return;
        }
        if (l.loctype == COORD_VIEW && gno != -1) {
            return;
        }
    }
    if (l.active) {
        setclipping(FALSE);
        
        if (l.loctype == COORD_WORLD) {
            wptmp.x = l.x1;
            wptmp.y = l.y1;
            vp1 = Wpoint2Vpoint(wptmp);
            wptmp.x = l.x2;
            wptmp.y = l.y2;
            vp2 = Wpoint2Vpoint(wptmp);
        } else {
            vp1.x = l.x1;
            vp1.y = l.y1;
            vp2.x = l.x2;
            vp2.y = l.y2;
        }
        
        setcolor(l.color);
        setlinewidth(l.linew);
        setlinestyle(l.lines);
        DrawLine(vp1, vp2);

        switch (l.arrow) {
        case 0:
            break;
        case 1:
            draw_arrowhead(vp2, vp1, l.asize, l.atype);
            break;
        case 2:
            draw_arrowhead(vp1, vp2, l.asize, l.atype);
            break;
        case 3:
            draw_arrowhead(vp2, vp1, l.asize, l.atype);
            draw_arrowhead(vp1, vp2, l.asize, l.atype);
            break;
        }
        setclipping(TRUE);
    }
}

/*
 * draw arrow head
 */
void draw_arrowhead(VPoint vp1, VPoint vp2, double size, int type)
{
    double asize, length;
    VPoint vpc, vpl, vpr, vps[3];
    int polylinetype;
    
    length = hypot((vp2.x - vp1.x), (vp2.y - vp1.y));
    if (length == 0.0) {
        errmsg("Can't draw arrow, length = 0.0");
        return;
    }
    
    asize = 0.02*size;
    vpc.x = vp2.x - asize * (vp2.x - vp1.x)/length;
    vpc.y = vp2.y - asize * (vp2.y - vp1.y)/length;
    vpl.x = vpc.x + 0.5*asize * (vp2.y - vp1.y)/length;
    vpl.y = vpc.y - 0.5*asize * (vp2.x - vp1.x)/length;
    vpr.x = vpc.x - 0.5*asize * (vp2.y - vp1.y)/length;
    vpr.y = vpc.y + 0.5*asize * (vp2.x - vp1.x)/length;
    
    vps[0] = vpl;
    vps[1] = vp2;
    vps[2] = vpr;
    
    switch (type) {
    case 0:
        polylinetype = POLYLINE_OPEN;
        break;
    case 1:
        setpattern(1);
        DrawPolygon(vps, 3);
        polylinetype = POLYLINE_CLOSED;
        break;
    case 2:
        polylinetype = POLYLINE_CLOSED;
        break;
    default:
        errmsg("Internal error in draw_arrowhead()");
        return;
    }
    DrawPolyline(vps, 3, polylinetype);
}

void draw_region(int r)
{
    int i;
    double vshift = 0.05;
    double xshift = 0.0, yshift = 0.0;
    
    region *this;

    int atype = 0;
    double asize = 1.0;
    
    int rgndouble=0;
    WPoint wptmp, wp1, wp2, wp3, wp4;
    VPoint vps[4], *vpstmp;

    this=&rg[r];
    
    switch (this->type) {
    case REGION_POLYI:
    case REGION_POLYO:
        if (this->x != NULL && this->y != NULL && this->n > 2) {
            vpstmp = (VPoint *) malloc (this->n*sizeof(VPoint));
            if (vpstmp == NULL) {
                errmsg("malloc error in draw_region()");
                return;
            } else {
                for (i = 0; i < this->n; i++) {
                    wptmp.x = this->x[i];
                    wptmp.y = this->y[i];
                    vpstmp[i] = Wpoint2Vpoint(wptmp);
                }
                DrawPolyline(vpstmp, this->n, POLYLINE_CLOSED);
		free(vpstmp);
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
        DrawPolyline(vps, 4, POLYLINE_OPEN);
        draw_arrowhead(vps[1], vps[0], asize, atype);
        draw_arrowhead(vps[2], vps[3], asize, atype);
    } else {
        vps[0] = Wpoint2Vpoint(wp1);
        vps[1] = Wpoint2Vpoint(wp2);
        DrawLine(vps[0], vps[1]);
        vps[0] = Wpoint2Vpoint(wp3);
        vps[1] = Wpoint2Vpoint(wp4);
        DrawLine(vps[0], vps[1]);
        wp1.x=(wp1.x+wp2.x)/2;
        wp1.y=(wp1.y+wp2.y)/2;
        wp3.x=(wp3.x+wp4.x)/2;
        wp3.y=(wp3.y+wp4.y)/2;
        vps[0] = Wpoint2Vpoint(wp1);
        vps[1] = Wpoint2Vpoint(wp3);
        DrawLine(vps[0], vps[1]);
        draw_arrowhead(vps[0], vps[1], asize, atype);
    }
}

/* ---------------------- legends ---------------------- */


/*
 * draw the legend
 */
void dolegend(int gno)
{
    int i;
    int maxsymsize;
    double ldist, sdist, yskip;
    
    WPoint wptmp;
    VPoint vp, vp2;
    
    view v;
    legend l;
    plotarr p;

    get_graph_legend(gno, &l);
    if (l.active == FALSE) {
        return;
    }
    
    setclipping(FALSE);
    
    if (l.loctype == COORD_WORLD) {
        wptmp.x = l.legx;
        wptmp.y = l.legy;
        vp = Wpoint2Vpoint(wptmp);
    } else {
        vp.x = l.legx;
        vp.y = l.legy;
    }
    
    maxsymsize = 0;
    for (i = 0; i < number_of_sets(gno); i++) {
        if (is_set_drawable(gno, i)) {
            get_graph_plotarr(gno, i, &p);
            if (p.symsize > maxsymsize) {
                maxsymsize = p.symsize;
            }
        }  
    }
        
    ldist = 0.01*l.len;
    sdist = 0.01*(l.hgap + maxsymsize);
    
    yskip = 0.01*l.vgap;
    
    activate_bbox(BBOX_TYPE_TEMP, TRUE);
    reset_bbox(BBOX_TYPE_TEMP);
    update_bbox(BBOX_TYPE_TEMP, vp);
    
    set_draw_mode(FALSE);
    putlegends(gno, vp, ldist, sdist, yskip);
    v = get_bbox(BBOX_TYPE_TEMP);
    
    vp2.x = vp.x + (v.xv2 - v.xv1) + 2*0.01*l.hgap;
    vp2.y = vp.y - (v.yv2 - v.yv1) - 2*0.01*l.vgap;
    
    set_draw_mode(TRUE);
    
    setpen(l.boxfillpen);
    FillRect(vp, vp2);

    if (l.boxlines != 0 && l.boxpen.pattern != 0) {
        setpen(l.boxpen);
        setlinewidth(l.boxlinew);
        setlinestyle(l.boxlines);
        DrawRect(vp, vp2);
    }
    
    /* correction */
    vp.x += (vp.x - v.xv1) + 0.01*l.hgap;
    vp.y += (vp.y - v.yv2) - 0.01*l.vgap;
   
    
    
    reset_bbox(BBOX_TYPE_TEMP);
    update_bbox(BBOX_TYPE_TEMP, vp);
    
    putlegends(gno, vp, ldist, sdist, yskip);
}

void putlegends(int gno, VPoint vp, double ldist, double sdist, double yskip)
{
    int i, setno;
    VPoint vp2, vpstr;
    plotarr p;
    legend l;
    
    vp2.y = vp.y;
    vp2.x = vp.x + ldist;
    vpstr.y = vp.y;
    vpstr.x = vp2.x + sdist;
    
    get_graph_legend(gno, &l);
    
    for (i = 0; i < number_of_sets(gno); i++) {
        if (l.invert == FALSE) {
            setno = i;
        } else {
            setno = number_of_sets(gno) - i - 1;
        }
        if (is_set_drawable(gno, setno)) {
            get_graph_plotarr(gno, setno, &p);
            
            if (p.lstr == NULL || p.lstr[0] == '\0') {
                continue;
            }
            
            setcharsize(l.charsize);
            setfont(l.font);
            setcolor(l.color);
            WriteString(vpstr, 0, JUST_LEFT|JUST_TOP|JUST_BBOX, p.lstr);
            vp.y = (vpstr.y + get_bbox(BBOX_TYPE_TEMP).yv1)/2;
            vp2.y = vp.y;
            vpstr.y = get_bbox(BBOX_TYPE_TEMP).yv1 - yskip;
            
            setfont(p.charfont);
            setcharsize(p.symsize);
            if (p.type == SET_BAR) {
                p.sym = SYM_SQUARE;
            }
            
            if (l.len != 0 && p.lines != 0 && p.linet != 0) { 
                setpen(p.linepen);
                setlinewidth(p.linew);
                setlinestyle(p.lines);
                DrawLine(vp, vp2);
        
                setlinewidth(p.symlinew);
                setlinestyle(p.symlines);
                drawxysym(vp, p.sym, p.sympen, p.symfillpen, p.symchar);
                drawxysym(vp2, p.sym, p.sympen, p.symfillpen, p.symchar);
            } else {
                VPoint vptmp;
                vptmp.x = (vp.x + vp2.x)/2;
                vptmp.y = vp.y;
                
                setlinewidth(p.symlinew);
                setlinestyle(p.symlines);
                drawxysym(vptmp, p.sym, p.sympen, p.symfillpen, p.symchar);
            }
        }
    }
}

/* plot time stamp */
void draw_timestamp(void)
{
    if (timestamp.active) {
        VPoint vp;
        setfont(timestamp.font);
        setcharsize(timestamp.charsize);
        setcolor(timestamp.color);
        vp.x = timestamp.x;
        vp.y = timestamp.y;
        WriteString(vp, timestamp.rot, timestamp.just|JUST_BOTTOM|JUST_OBJECT, timestamp.s);
    }
}
