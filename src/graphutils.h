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
 * Graph utils
 *
 */

#ifndef __GRAPHUTILS_H_
#define __GRAPHUTILS_H_

#define GSCROLL_LEFT    0
#define GSCROLL_RIGHT   1
#define GSCROLL_DOWN    2
#define GSCROLL_UP      3

#define GZOOM_SHRINK    0
#define GZOOM_EXPAND    1

int get_format_index(int f);
char *get_format_types(int f);

int wipeout(void);

void scroll_proc(int value);
void scrollinout_proc(int value);
int graph_scroll(int type);
int graph_zoom(int type);

void make_format(int gno);

void arrange_graphs(int grows, int gcols);
int arrange_graphs2(int grows, int gcols, double vgap, double hgap,
		    double sx, double sy, double wx, double wy);
void define_arrange(int nrows, int ncols, int pack,
       double vgap, double hgap, double sx, double sy, double wx, double wy);

void autotick_axis(int gno, int axis);
void autoscale_byset(int gno, int setno, int autos_type);

void move_legend(int gno, VVector shift);
void move_timestamp(VVector shift);

#define autoscale_graph(gno, axis) autoscale_byset(gno, ALL_SETS, axis)

#endif /* __GRAPHUTILS_H_ */
