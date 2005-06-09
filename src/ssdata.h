/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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
 * SS data
 *
 */
 
#ifndef __SSDATA_H_
#define __SSDATA_H_

#include "grace.h"

double *copy_data_column_simple(double *src, int nrows);
double *allocate_index_data(int nrows);
double *allocate_mesh(double start, double stop, int len);

char *cols_to_field_string(int nc, unsigned int *cols, int scol);
int field_string_to_cols(const char *fs, int *nc, int **cols, int *scol);

int parse_ss_row(Quark *pr, const char *s, int *nncols, int *nscols, int **formats);
int insert_data_row(Quark *q, unsigned int row, char *s);
int store_data(Quark *q, int load_type);

#endif /* __SSDATA_H_ */
