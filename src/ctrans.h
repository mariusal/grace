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
 * ------------- coordinate transformation routines ------------
 */

#ifndef __CTRANS_H_
#define __CTRANS_H_

#include "core_utils.h"

/*
 * types of coordinate frames
 */
#define COORDINATES_XY      0       /* Cartesian coordinates */
#define COORDINATES_POLAR   1       /* Polar coordinates */
                                
char *scale_types(ScaleType it);
ScaleType get_scale_type_by_name(const char *name);

int is_wpoint_inside(WPoint *wp, world *w);
int is_validWPoint(WPoint wp);

double fscale(double wc, int scale);
double ifscale(double vc, int scale);

int polar2xy(double phi, double rho, double *x, double *y);
void xy2polar(double x, double y, double *phi, double *rho);

double xy_xconv(double wx);
double xy_yconv(double wy);
VPoint Wpoint2Vpoint(WPoint wp);
int world2view(double x, double y, double *xv, double *yv);
void view2world(double xv, double yv, double *xw, double *yw);

int Fpoint2Vpoint(Quark *f, const FPoint *fp, VPoint *vp);

int definewindow(const world *w, const view *v,
    int ctrans_type, int xscale, int yscale, int xyfixed, int invx, int invy);

#endif /* __CTRANS_H_ */
