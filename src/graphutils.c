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
 * utilities for graphs
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>

#include "globals.h"
#include "utils.h"
#include "draw.h"
#include "device.h"
#include "t1fonts.h"
#include "graphs.h"
#include "graphutils.h"
#include "protos.h"

extern char print_file[];

static void auto_ticks(int gno, int axis);

int get_format_index(int f)
{
    int i = 0;

    while (f != format_types[i] && format_types[i] != FORMAT_INVALID) {
	i++;
    }
    return i;
}

char *get_format_types(int f)
{
    static char s[128];

    strcpy(s, "decimal");
    switch (f) {
    case FORMAT_DECIMAL:
	strcpy(s, "decimal");
	break;
    case FORMAT_EXPONENTIAL:
	strcpy(s, "exponential");
	break;
    case FORMAT_GENERAL:
	strcpy(s, "general");
	break;
    case FORMAT_POWER:
	strcpy(s, "power");
	break;
    case FORMAT_SCIENTIFIC:
	strcpy(s, "scientific");
	break;
    case FORMAT_ENGINEERING:
	strcpy(s, "engineering");
	break;
    case FORMAT_DDMMYY:
	strcpy(s, "ddmmyy");
	break;
    case FORMAT_MMDDYY:
	strcpy(s, "mmddyy");
	break;
    case FORMAT_MMYY:
	strcpy(s, "mmyy");
	break;
    case FORMAT_MMDD:
	strcpy(s, "mmdd");
	break;
    case FORMAT_MONTHDAY:
	strcpy(s, "monthday");
	break;
    case FORMAT_DAYMONTH:
	strcpy(s, "daymonth");
	break;
    case FORMAT_MONTHS:
	strcpy(s, "months");
	break;
    case FORMAT_MONTHSY:
	strcpy(s, "monthsy");
	break;
    case FORMAT_MONTHL:
	strcpy(s, "monthl");
	break;
    case FORMAT_DAYOFWEEKS:
	strcpy(s, "dayofweeks");
	break;
    case FORMAT_DAYOFWEEKL:
	strcpy(s, "dayofweekl");
	break;
    case FORMAT_DAYOFYEAR:
	strcpy(s, "dayofyear");
	break;
    case FORMAT_HMS:
	strcpy(s, "hms");
	break;
    case FORMAT_MMDDHMS:
	strcpy(s, "mmddhms");
	break;
    case FORMAT_MMDDYYHMS:
	strcpy(s, "mmddyyhms");
	break;
    case FORMAT_DEGREESLON:
	strcpy(s, "degreeslon");
	break;
    case FORMAT_DEGREESMMLON:
	strcpy(s, "degreesmmlon");
	break;
    case FORMAT_DEGREESMMSSLON:
	strcpy(s, "degreesmmsslon");
	break;
    case FORMAT_MMSSLON:
	strcpy(s, "mmsslon");
	break;
    case FORMAT_DEGREESLAT:
	strcpy(s, "degreeslat");
	break;
    case FORMAT_DEGREESMMLAT:
	strcpy(s, "degreesmmlat");
	break;
    case FORMAT_DEGREESMMSSLAT:
	strcpy(s, "degreesmmsslat");
	break;
    case FORMAT_MMSSLAT:
	strcpy(s, "mmsslat");
	break;
    }
    return s;
}


int wipeout(void)
{
    if (!noask && is_dirtystate()) {
        if (!yesno("Abandon unsaved project?", NULL, NULL, NULL)) {
            return 1;
        }
    }
    kill_all_graphs();
    do_clear_lines();
    do_clear_boxes();
    do_clear_ellipses();
    do_clear_text();
    reset_project_version();
    map_fonts(FONT_MAP_DEFAULT);
    select_graph(0);
    strcpy(docname, NONAME);
    description[0] = '\0';
    print_file[0] = '\0';
    clear_dirtystate();
    return 0;
}


/* The following routines determine default axis range and tickmarks */

static void autorange_byset(int gno, int setno, int autos_type);
static double nicenum(double x, int round);

#define NICE_FLOOR   0
#define NICE_CEIL    1
#define NICE_ROUND   2
#define NICE_DOWN    NICE_FLOOR
#define NICE_UP      NICE_CEIL
#define WITH_ZERO    0
#define WITHOUT_ZERO 1

void autotick_axis(int gno, int axis)
{
    switch (axis) {
    case ALL_AXES:
        auto_ticks(gno, X_AXIS);
        auto_ticks(gno, ZX_AXIS);
        auto_ticks(gno, Y_AXIS);
        auto_ticks(gno, ZY_AXIS);
        break;
    case ALL_X_AXES:
        auto_ticks(gno, X_AXIS);
        auto_ticks(gno, ZX_AXIS);
        break;
    case ALL_Y_AXES:
        auto_ticks(gno, Y_AXIS);
        auto_ticks(gno, ZY_AXIS);
        break;
    default:
        auto_ticks(gno, axis);
        break;
    }
}

void autoscale_byset(int gno, int setno, int autos_type)
{
    if (setno == ALL_SETS || is_set_active(gno, setno)) {
	autorange_byset(gno, setno, autos_type);
	switch (autos_type) {
        case AUTOSCALE_X:
            autotick_axis(gno, ALL_X_AXES);
            break;
        case AUTOSCALE_Y:
            autotick_axis(gno, ALL_Y_AXES);
            break;
        case AUTOSCALE_XY:
            autotick_axis(gno, ALL_AXES);
            break;
        }
#ifndef NONE_GUI
        update_ticks(gno);
#endif
    }
}

static void round_axis_limits(double *amin, double *amax, int scale)
{
/*
 *     double extra_range;
 */
    
    if (*amin == *amax) {
        switch (sign(*amin)) {
        case 0:
            *amin = -1.0;
            *amax = +1.0;
            break;
        case 1:
            *amin /= 2.0;
            *amax *= 2.0;
            break;
        case -1:
            *amin *= 2.0;
            *amax /= 2.0;
            break;
        }
    } 

    *amin = nicenum(*amin, NICE_FLOOR);
    *amax = nicenum(*amax, NICE_CEIL);
    
    if (scale == SCALE_NORMAL && sign(*amin) == sign(*amax)) {
        if ((*amax)/(*amin) > 5.0) {
            *amin = 0.0;
        } else if ((*amin)/(*amax) > 5.0) {
            *amax = 0.0;
        }
    }
}

static void autorange_byset(int gno, int setno, int autos_type)
{
    world w;
    double xmax, xmin, ymax, ymin;
    int scale;

    if (autos_type == AUTOSCALE_NONE) {
        return;
    }
    
    get_graph_world(gno, &w);
    
    if (get_graph_type(gno) == GRAPH_SMITH) {
        if (autos_type == AUTOSCALE_X || autos_type == AUTOSCALE_XY) {
            w.xg1 = -1.0;
            w.yg1 = -1.0;
        }
        if (autos_type == AUTOSCALE_Y || autos_type == AUTOSCALE_XY) {
            w.xg2 = 1.0;
            w.yg2 = 1.0;
	}
        set_graph_world(gno, w);
        return;
    }

    xmin=w.xg1;
    xmax=w.xg2;
    ymin=w.yg1;
    ymax=w.yg2;
    if (autos_type == AUTOSCALE_XY) {
        getsetminmax(gno, setno, &xmin, &xmax, &ymin, &ymax);
    } else if (autos_type == AUTOSCALE_X) {
        getsetminmax_c(gno, setno, &xmin, &xmax, &ymin, &ymax, 2);
    } else if (autos_type == AUTOSCALE_Y) {
        getsetminmax_c(gno, setno, &xmin, &xmax, &ymin, &ymax, 1);
    }

/*  Ensure we have finite endpoints for axis */
    if ((finite(xmin) == 0) && (xmin < 0.)) {
      xmin = -MAXNUM;
    }
    if ((finite(xmax) == 0) && (xmax > 0.)) {
      xmax = +MAXNUM;
    }
    if ((finite(ymin) == 0) && (ymin < 0.)) {
      ymin = -MAXNUM;
    }
    if ((finite(ymax) == 0) && (ymax > 0.)) {
      ymax = +MAXNUM;
    }

    if (autos_type == AUTOSCALE_X || autos_type == AUTOSCALE_XY) {
        scale = get_graph_xscale(gno);
        round_axis_limits(&xmin, &xmax, scale);
        w.xg1 = xmin;
        w.xg2 = xmax;
    }

    if (autos_type == AUTOSCALE_Y || autos_type == AUTOSCALE_XY) {
        scale = get_graph_yscale(gno);
        round_axis_limits(&ymin, &ymax, scale);
        w.yg1 = ymin;
        w.yg2 = ymax;
    }

    set_graph_world(gno, w);
}

static void auto_ticks(int gno, int axis)
{
    tickmarks t;
    world w;
    double range, d, tmpmax, tmpmin;
    int axis_scale;

    get_graph_tickmarks(gno, &t, axis);
    get_graph_world(gno, &w);

    if (is_xaxis(axis)) {
        tmpmin = w.xg1;
        tmpmax = w.xg2;
        axis_scale = get_graph_xscale(gno);
    } else {
        tmpmin = w.yg1;
        tmpmax = w.yg2;
        axis_scale = get_graph_yscale(gno);
    }

    if (axis_scale == SCALE_LOG) {
	if (t.tmajor <= 1.0) {
            t.tmajor = 10.0;
        }
        tmpmax = log10(tmpmax)/log10(t.tmajor);
	tmpmin = log10(tmpmin)/log10(t.tmajor);
    } else if (t.tmajor <= 0.0) {
        t.tmajor = 1.0;
    }
    if (t.nminor < 0) {
	t.nminor = 1;
    }
    
    range = tmpmax - tmpmin;
    if (axis_scale != SCALE_LOG) {
        d = nicenum(range/(t.t_autonum - 1), NICE_ROUND);
	t.tmajor = d;
    } else {
        d = ceil(range/(t.t_autonum - 1));
	t.tmajor = pow(t.tmajor, d);
    }
    
    set_graph_tickmarks(gno, &t, axis);
    free_ticklabels(&t);
}

/*
 * nicenum: find a "nice" number approximately equal to x
 */

static double nicenum(double x, int round)
{
    int xsign;
    double f, y, exp, nnres, smallx;
    double maxf, maxexp;

    if (x == 0.0) {
        return(0.0);
    }

    maxf  = MAXNUM/pow(10.0,floor(log10(MAXNUM)));
    maxexp = floor(log10(MAXNUM));

    xsign = sign(x);
    x = fabs(x);
    exp = floor(log10(x));
    smallx = pow(10.0, exp);
    f = x/smallx;	/* fraction between 1 and 10 */
    if ((round == NICE_FLOOR && xsign == +1) ||
        (round == NICE_CEIL  && xsign == -1)) {
	if (f < 2.0)
	    y = 1.;
	else if (f < 5.0)
	    y = 2.;
	else if (f < 10.0)
	    y = 5.;
	else
	    y = 10.;
    } else if ((round == NICE_FLOOR && xsign == -1) ||
               (round == NICE_CEIL  && xsign == +1)) {
        if (f <= 1.)
            y = 1.;
        else if (f <= 2.)
            y = 2.;
        else if (f <= 5.)
            y = 5.;
        else
            y = 10.;
    } else {
	if (f < 1.5)
	    y = 1.;
	else if (f < 3.)
	    y = 2.;
	else if (f < 7.)
	    y = 5.;
	else
	    y = 10.;
    }
    if (exp==maxexp)
       nnres = (y > maxf) ? xsign*floor(maxf)*smallx : xsign*y*smallx;
    else
       nnres = xsign*y*smallx;
    return (nnres);
}

/*
 * set scroll amount
 */
void scroll_proc(int value)
{
    scrollper = value / 100.0;
}

void scrollinout_proc(int value)
{
    shexper = value / 100.0;
}

/*
 * pan through world coordinates
 */
int graph_scroll(int type)
{
    world w;
    double dwc = 0.0;
    int gstart, gstop, i;

    if (scrolling_islinked) {
        gstart = 0;
        gstop = number_of_graphs() - 1;
    } else {
        gstart = get_cg();
        gstop = gstart;
    }
    
    for (i = gstart; i <= gstop; i++) {
        if (get_graph_world(i, &w) == GRACE_EXIT_SUCCESS) {
            switch (type) {
            case GSCROLL_LEFT:    
            case GSCROLL_RIGHT:    
                if (islogx(i) == TRUE) {
                    errmsg("Scrolling of LOG axes is not implemented");
                    return GRACE_EXIT_FAILURE;
                }
                dwc = scrollper * (w.xg2 - w.xg1);
                break;
            case GSCROLL_DOWN:    
            case GSCROLL_UP:    
                if (islogy(i) == TRUE) {
                    errmsg("Scrolling of LOG axes is not implemented");
                    return GRACE_EXIT_FAILURE;
                }
                dwc = scrollper * (w.yg2 - w.yg1);
                break;
            }
            
            switch (type) {
            case GSCROLL_LEFT:    
                w.xg1 -= dwc;
                w.xg2 -= dwc;
                break;
            case GSCROLL_RIGHT:    
                w.xg1 += dwc;
                w.xg2 += dwc;
                break;
            case GSCROLL_DOWN:    
                w.yg1 -= dwc;
                w.yg2 -= dwc;
                break;
            case GSCROLL_UP:    
                w.yg1 += dwc;
                w.yg2 += dwc;
                break;
            }
            set_graph_world(i, w);
        }
    }
    
    return GRACE_EXIT_SUCCESS;
}

int graph_zoom(int type)
{
    double dx, dy;
    world w;
    int cg = get_cg();
    
    if (!islogx(cg) && !islogy(cg)) {
        if (get_graph_world(cg, &w) == GRACE_EXIT_SUCCESS) {
            dx = shexper * (w.xg2 - w.xg1);
            dy = shexper * (w.yg2 - w.yg1);
            if (type == GZOOM_SHRINK) {
                dx *= -1;
                dy *= -1;
            }
 
	    w.xg1 -= dx;
	    w.xg2 += dx;
	    w.yg1 -= dy;
	    w.yg2 += dy;
 
            set_graph_world(cg, w);
            return GRACE_EXIT_SUCCESS;
        } else {
            return GRACE_EXIT_FAILURE;
        }
    } else {
	errmsg("Zooming is not implemented for LOG plots");
        return GRACE_EXIT_FAILURE;
    }
}

/*
 * set format string for locator
 */
static char *fchar[3] = {"lf", "le", "g"};
static char *typestr[6] = {"X, Y",
                           "DX, DY",
			   "DIST",
			   "Phi, Rho",
			   "VX, VY",
                           "SX, SY"};
char locator_format[128] = {"G%1d: X, Y = [%.6g, %.6g]"};

void make_format(int gno)
{
    int type, locpx, locfx, locpy, locfy;
    GLocator locator;

    get_graph_locator(gno, &locator);
    
    type = locator.pt_type;
    locfx = get_format_index(locator.fx);
    locfy = get_format_index(locator.fy);
    locpx = locator.px;
    locpy = locator.py;
    switch (type) {
    case 0:
	if (locfx < 3 && locfy < 3) {
	    sprintf(locator_format, "G%%1d: %s = [%%.%d%s, %%.%d%s]",
	            typestr[type], locpx, fchar[locfx], locpy, fchar[locfy]);
	} else {
	    locator_format[0] = 0;
	}
	break;
    case 2:
	locfx = locfx == FORMAT_DECIMAL ? 0 :
		locfx == FORMAT_EXPONENTIAL ? 1 :
		locfx == FORMAT_GENERAL ? 2 : 0;
	sprintf(locator_format, "G%%1d: %s = [%%.%d%s]", typestr[type], locpx,
	        fchar[locfx]);
	break;
    case 1:
    case 3:
    case 4:
	locfx = locfx == FORMAT_DECIMAL ? 0 :
		locfx == FORMAT_EXPONENTIAL ? 1 :
		locfx == FORMAT_GENERAL ? 2 : 0;
	locfy = locfy == FORMAT_DECIMAL ? 0 :
		locfy == FORMAT_EXPONENTIAL ? 1 :
		locfy == FORMAT_GENERAL ? 2 : 0;
	sprintf(locator_format, "G%%1d: %s = [%%.%d%s, %%.%d%s]", typestr[type],
	        locpx, fchar[locfx], locpy, fchar[locfy]);
	break;
    case 5:
	sprintf(locator_format, "G%%1d: %s = [%%d, %%d]", typestr[type]);
	break;
    }
}

/*
 *  Arrange procedures
 */
void arrange_graphs(int grows, int gcols)
{
    double hgap, vgap; /* inter-graph gaps*/
    double sx, sy; /* offsets */
    double wx, wy;
    double vx, vy;

    if (gcols < 1 ||  grows < 1) {
        return;
    }
    
    get_page_viewport(&vx, &vy);
    sx = 0.1;
    sy = 0.1;
    hgap = 0.07;
    vgap = 0.07;
    wx = ((vx - 2*sx) - (gcols - 1)*hgap)/gcols;
    wy = ((vy - 2*sy) - (grows - 1)*vgap)/grows;
    
    arrange_graphs2(grows, gcols, hgap, vgap, sx, sy, wx, wy);
}

int arrange_graphs2(int grows, int gcols, double vgap, double hgap,
		    double sx, double sy, double wx, double wy)
{
    int i, j;
    int gtmp = 0;
    view v;

    if (gcols < 1 || grows < 1) {
        return GRACE_EXIT_FAILURE;
    }
    
    for (i = 0; i < gcols; i++) {
        for (j = 0; j < grows; j++) {
            if (!is_graph_active(gtmp)) {
                set_graph_active(gtmp, TRUE);
            }
            v.xv1 = sx + i*(hgap + wx);
            v.xv2 = v.xv1 + wx;
            v.yv1 = sy + j*(vgap + wy);
            v.yv2 = v.yv1 + wy;
            set_graph_viewport(gtmp, v);
            gtmp++;
        }
    }
    return GRACE_EXIT_SUCCESS;
}

void define_arrange(int nrows, int ncols, int pack,
       double vgap, double hgap, double sx, double sy, double wx, double wy)
{
    int i, j, k, gno;

    if (arrange_graphs2(nrows, ncols, vgap, hgap, sx, sy, wx, wy) != GRACE_EXIT_SUCCESS) {
	return;
    }
    
    switch (pack) {
    case 0:
	for (j = 0; j < ncols; j++) {
	    for (i = 0; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < MAXAXES; k++) {
		    activate_tick_labels(gno, k, TRUE);
		}
	    }
	}
	break;
    case 1:
	hgap = 0.0;
	for (j = 1; j < ncols; j++) {
	    for (i = 0; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < MAXAXES; k++) {
		    if (is_yaxis(k) == TRUE) {
                        activate_tick_labels(gno, k, FALSE);
                    }
		}
	    }
	}
	break;
    case 2:
	vgap = 0.0;
	for (j = 0; j < ncols; j++) {
	    for (i = 1; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < MAXAXES; k++) {
		    if (is_xaxis(k) == TRUE) {
		        activate_tick_labels(gno, k, FALSE);
                    }
		}
	    }
	}
	break;
    case 3:
	hgap = 0.0;
	vgap = 0.0;
	for (j = 1; j < ncols; j++) {
	    for (i = 0; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < MAXAXES; k++) {
		    if (is_yaxis(k) == TRUE) {
		        activate_tick_labels(gno, k, FALSE);
                    }
		}
	    }
	}
	for (j = 0; j < ncols; j++) {
	    for (i = 1; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < MAXAXES; k++) {
		    if (is_xaxis(k) == TRUE) {
		        activate_tick_labels(gno, k, FALSE);
                    }
		}
	    }
	}
	break;
    }
    set_dirtystate();
}
