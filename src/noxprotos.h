/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 *
 * Prototypes not involving X
 *
 */

#ifndef __NOXPROTOS_H_
#define __NOXPROTOS_H_

#include "graceapp.h"


/* set_utils.c */
int getsetminmax(Quark **sets, int nsets, 
    double *xmin, double *xmax, double *ymin, double *ymax);
int getsetminmax_c(Quark **sets, int nsets,
    double *xmin, double *xmax, double *ymin, double *ymax, int ivec);
int set_point(Quark *pset, int seti, const WPoint *wp);
int get_point(Quark *pset, int seti, WPoint *wp);
int get_datapoint(Quark *pset, int ind, int *ncols, Datapoint *dpoint);
int set_point_shift(Quark *pset, int seti, const VVector *vshift);

void killsetdata(Quark *pset);

void del_point(Quark *pset, int pt);

void set_date_hint(Dates_format preferred);
Dates_format get_date_hint(void);

#endif /* __NOXPROTOS_H_ */
