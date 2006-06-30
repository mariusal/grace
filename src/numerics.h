/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2005 Grace Development Team
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
 * Numeric stuff
 *
 */

#ifndef __NUMERICS_H_
#define __NUMERICS_H_

#include "graceapp.h"

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
DArray *featext(Quark **sets, int nsets, const char *formula);
int num_cumulative(DArray *src_arrays, unsigned int nsrc,
    DArray *dst_array, int type);

/* fourier.c */
int fourier(double *jr, double *ji, int n, int iflag);

/* nonlfit.c */
void reset_nonl(NLFit *nlfit);
int do_nonlfit(Quark *pset, NLFit *nlfit,
    double *warray, char *rarray, int nsteps);

#endif /* __NUMERICS_H_ */
