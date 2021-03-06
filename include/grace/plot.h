/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
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

#ifndef __PLOT_H_
#define __PLOT_H_

#include "grace/core.h"
#include "grace/graal.h"

#define LFORMAT_TYPE_PLAIN      0
#define LFORMAT_TYPE_EXTENDED   1

int drawgraph(Canvas *canvas, Graal *g, const Quark *project);

char *create_fstring(const Quark *q, const Format *form, double loc, int type);

void jdate_to_datetime(const Quark *q, double jday, int rounding,
                         int *y, int *m, int *d,
                         int *hour, int *min, int *sec);

#endif /* __PLOT_H_ */
