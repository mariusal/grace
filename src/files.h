/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
 * File I/O 
 */

#ifndef __FILES_H_
#define __FILES_H_

#include <stdio.h>

#include "grace.h"

/* data load types */
#define LOAD_SINGLE 0
#define LOAD_NXY    1
#define LOAD_BLOCK  2

int add_io_filter( int type, int method, char *id, char *comm );
int add_input_filter( int method, char *id, char *comm );
int add_output_filter( int method, char *id, char *comm );
void clear_io_filters( int f );
FILE *filter_read(Grace *grace, char *fn);
FILE *filter_write(Grace *grace, char *fn);

char *grace_path(Grace *grace, char *fn);
char *grace_path2(Grace *grace, const char *prefix, char *fn);
char *grace_exe_path(Grace *grace, char *fn);

FILE *grace_openw(Grace *grace, char *fn);
FILE *grace_openr(Grace *grace, char *fn, int src);
void grace_close(FILE *fp);

int getparms(Grace *grace, char *plfile);
int getdata(Grace *grace, Quark *gr, char *fn, int src, int type);
int update_set_from_file(Quark *pset);

int readblockdata(Quark *gr, char *fn, FILE * fp);

int load_project_file(Grace *grace, char *fn, int as_template);

int new_project(Grace *grace, char *template);
int load_project(Grace *grace, char *fn);
int save_project(Quark *project, char *fn);

int write_set(Quark *pset, FILE *cp, char *format);
void outputset(Quark *pset, char *fname, char *dformat);

void unregister_real_time_input(const char *name);
int register_real_time_input(Grace *grace, int fd, const char *name, int reopen);
int real_time_under_monitoring(void);
int monitor_input(Grace *grace, Input_buffer *tbl, int tblsize, int no_wait);

int readnetcdf(Quark *pset,
	       char *netcdfname,
	       char *xvar,
	       char *yvar,
	       int nstart,
	       int nstop,
	       int nstride);
int write_netcdf(Quark *pr, char *fname);

#endif /* __FILES_H_ */
