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

#include "grace.h"

/* computils.c */
double trapint(double *x, double *y, double *resx, double *resy, int n);
int apply_window(double *v, int ilen, int window, double beta);
int histogram(int ndata, double *data, int nbins, double *bins, int *hist);
double comp_area(int n, double *x, double *y);
double comp_perimeter(int n, double *x, double *y);
void stasum(double *x, int n, double *xbar, double *sd);
void linearconv(double *x, int n, double *h, int m, double *y);
void spline(int n, double *x, double *y, double *b, double *c, double *d);
void aspline(int n, double *x, double *y, double *b, double *c, double *d);
int seval(double *u, double *v, int ulen,
    double *x, double *y, double *b, double *c, double *d, int n);
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax);
double vmin(double *x, int n);
double vmax(double *x, int n);
int monotonicity(double *array, int len, int strict);
int monospaced(double *array, int len, double *space);
int find_span_index(double *array, int len, int m, double x);

int get_restriction_array(Quark *pset, Quark *r, int negate, char **rarray);
int filter_set(Quark *pset, char *rarray);

int do_compute(Quark *psrc, Quark *pdest,
    char *rarray, char *fstr);
int do_linearc(Quark *psrc, Quark *pdest,
    Quark *pconv);
int do_xcor(Quark *psrc, Quark *pdest,
    Quark *pcor, int maxlag, int covar);
int do_int(Quark *psrc, Quark *pdest,
    int disponly, double *sum);
int do_differ(Quark *psrc, Quark *pdest,
    int type, int xplace, int period);
int do_runavg(Quark *psrc, Quark *pdest,
    int runlen, char *formula, int xplace);
int do_fourier(Quark *psrc, Quark *pdest,
    int invflag, int xscale, int norm, int complexin, int dcdump,
    double oversampling, int round2n, int window, double beta, int halflen,
    int output);
int do_histo(Quark *psrc, Quark *pdest,
    double *bins, int nbins, int cumulative, int normalize);
int do_sample(Quark *psrc, Quark *pdest,
    char *formula);
int do_prune(Quark *psrc, Quark *pdest, 
    int interp, int elliptic, double dx, int reldx, double dy, int reldy);
int do_interp(Quark *psrc, Quark *pdest,
    double *mesh, int meshlen, int method, int strict);
int featext(Quark **sets, int nsets, Quark *pdest,
    char *formula);
int cumulative(Quark **sets, int nsets, Quark *pdest);

/* fourier.c */
int fourier(double *jr, double *ji, int n, int iflag);

/* nonlfit.c */
void reset_nonl(NLFit *nlfit);
int do_nonlfit(Quark *pset, NLFit *nlfit,
    double *warray, char *rarray, int nsteps);


/* typeset.c */
int csparse_proc(const Canvas *canvas, const char *s, CompositeString *cstring);
int fmap_proc(const Canvas *canvas, int font);
int init_font_db(Grace *grace, Canvas *canvas);


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
void sortset(Quark *pset, int sorton, int stype);
void droppoints(Quark *pset, int startno, int endno);
int join_sets(Quark **sets, int nsets);
void reverse_set(Quark *pset);

void del_point(Quark *pset, int pt);

int do_splitsets(Quark *pset, int lpart);
void do_drop_points(Quark *pset, int startno, int endno);
void do_sort(Quark *pset, int sorton, int stype);


/* dates.c */
void set_date_hint(Dates_format preferred);
Dates_format get_date_hint(void);
int parse_float(const char* s, double *value, const char **after);
int parse_date(const Quark *q, const char* s, Dates_format preferred, int absolute,
               double *jul, Dates_format *recognized);
int parse_date_or_number(const Quark *q, const char* s, int absolute, double *value);

#endif /* __NOXPROTOS_H_ */
