/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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
 * Prototypes not involving X
 *
 */

#ifndef __NOXPROTOS_H_
#define __NOXPROTOS_H_

/* For FILE */
#include <stdio.h>

#include "defines.h"
#include "graphs.h"

double trapint(double *x, double *y, double *resx, double *resy, int n);
int apply_window(double *v, int ilen, int window, double beta);
int histogram(int ndata, double *data, int nbins, double *bins, int *hist);

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
int do_nonlfit(Quark *pset, NLFit *nlfit,
    double *warray, char *rarray, int nsteps);
int do_interp(Quark *psrc, Quark *pdest,
    double *mesh, int meshlen, int method, int strict);
int featext(Quark **sets, int nsets, Quark *pdest,
    char *formula);
int cumulative(Quark **sets, int nsets, Quark *pdest);


void set_region_defaults(region *r);
void set_default_framep(framep * f);
void set_default_world(world * w);
void set_default_view(view * v);
void set_default_legend(legend * l);

void set_default_string(plotstr *s);
void set_default_arrow(Arrow *arrowp);

void set_default_ticks(tickmarks *t);
void calculate_tickgrid(Quark *gr);
void drawgrid(Canvas *canvas, Quark *gr);
void drawaxes(Canvas *canvas, Quark *gr);

int csparse_proc(const Canvas *canvas, const char *s, CompositeString *cstring);
int fmap_proc(const Canvas *canvas, int font);
int init_font_db(Canvas *canvas);

void unregister_real_time_input(const char *name);
int register_real_time_input(int fd, const char *name, int reopen);
int real_time_under_monitoring(void);
int monitor_input(Input_buffer *tbl, int tblsize, int no_wait);

double comp_area(int n, double *x, double *y);
double comp_perimeter(int n, double *x, double *y);

void stasum(double *x, int n, double *xbar, double *sd);
void linearconv(double *x, int n, double *h, int m, double *y);

void spline(int n, double *x, double *y, double *b, double *c, double *d);
void aspline(int n, double *x, double *y, double *b, double *c, double *d);
int seval(double *u, double *v, int ulen,
    double *x, double *y, double *b, double *c, double *d, int n);

int fourier(double *jr, double *ji, int n, int iflag);

int find_item(Quark *gr, VPoint vp, view *bb, int *id);

int getsetminmax(Quark **sets, int nsets, double *x1, double *x2, double *y1, double *y2);
int getsetminmax_c(Quark **sets, int nsets, double *xmin, double *xmax, double *ymin, double *ymax, int ivec);
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax);
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax);
double vmin(double *x, int n);
double vmax(double *x, int n);
int set_point(Quark *pset, int seti, const WPoint *wp);
int get_point(Quark *pset, int seti, WPoint *wp);
int get_datapoint(Quark *pset, int ind, int *ncols, Datapoint *dpoint);
Datapoint *datapoint_new(void);
void datapoint_free(Datapoint *dpoint);
Dataset *dataset_new(void);
void dataset_free(Dataset *dsp);
Dataset *set_get_dataset(Quark *qset);
int set_dataset_nrows(Dataset *data, int nrows);
int set_dataset_ncols(Dataset *data, int ncols);
int dataset_set_datapoint(Dataset *dsp, const Datapoint *dpoint, int ind);
void setcol(Quark *pset, int col, double *x, int len);

void copycol2(Quark *psrc, Quark *pdest, int col);

set *set_data_new(void);
void set_data_free(set *p);
set *set_data_copy(set *p);
Quark *set_new(Quark *gr);

void killset(Quark *pset);
void killsetdata(Quark *pset);
int number_of_active_sets(Quark *gr);
int swapset(int gfrom, int j1, int gto, int j2);
int pushset(Quark *pset, int push_type);
void droppoints(Quark *pset, int startno, int endno);
int join_sets(int gno, int *sets, int nsets);
void sort_xy(double *tmp1, double *tmp2, int up, int sorton, int stype);
void reverse_set(Quark *pset);

void del_point(Quark *pset, int pt);
void add_point(Quark *pset, double px, double py);
void zero_datapoint(Datapoint *dpoint);
int add_point_at(Quark *pset, int ind, const Datapoint *dpoint);
void delete_byindex(Quark *pset, int *ind);

int do_copyset(int gfrom, int j1, int gto, int j2);
int do_moveset(int gfrom, int j1, int gto, int j2);
int do_swapset(int gno1, int setno1, int gno2, int setno2);
void do_splitsets(Quark *pset, int lpart);
void do_activate(int setno, int type, int len);
void do_hideset(Quark *pset);
void do_showset(Quark *pset);
void do_changetype(int setno, int type);
void do_copy(int j1, int gfrom, int j2, int gto);
void do_move(int j1, int gfrom, int j2, int gto);
void do_drop_points(Quark *pset, int startno, int endno);
void do_kill(Quark *pset, int soft);
void do_sort(Quark *pset, int sorton, int stype);
void do_cancel_pickop(void);


void set_hotlink(Quark *pset, int onoroff, char *fname, int src);
int is_hotlinked(Quark *pset);
void do_update_hotlink(Quark *pset);
char *get_hotlink_file(Quark *pset);
int get_hotlink_src(Quark *pset);

void sortset(Quark *pset, int sorton, int stype);
int get_restriction_array(Quark *pset,
    int rtype, int negate, char **rarray);

int monotonicity(double *array, int len, int strict);
int monospaced(double *array, int len, double *space);
int find_span_index(double *array, int len, int m, double x);

int inbounds(Quark *gr, double x, double y);
int intersect_to_left(double x, double y, double x1, double y1, double x2, double y2);
int inbound(double x, double y, double *xlist, double *ylist, int n);
int isleft(double x, double y, double x1, double y1, double x2, double y2);
int isright(double x, double y, double x1, double y1, double x2, double y2);
int isabove(double x, double y, double x1, double y1, double x2, double y2);
int isbelow(double x, double y, double x1, double y1, double x2, double y2);
void reporton_region(Quark *gr, int rno, int type);
int isactive_region(int regno);
char *region_types(int it, int which);
void kill_region(int r);
void kill_all_regions(void);
void activate_region(int r, int type, int gno);
void load_poly_region(int r, int gno, int n, WPoint *wps);
int inregion(Quark *gr, int regno, double x, double y);

void set_plotstr_string(plotstr * pstr, char *buf);

void cli_loop(void);

void reset_nonl(NLFit *nlfit);

int is_xaxis(int axis);
int is_yaxis(int axis);

void kill_blockdata(void);
void alloc_blockdata(int ncols);

void set_ref_date(double ref);
double get_ref_date(void);
void set_date_hint(Dates_format preferred);
Dates_format get_date_hint(void);
void allow_two_digits_years(int allowed);
int two_digits_years_allowed(void);
void set_wrap_year(int year);
int get_wrap_year(void);
long cal_to_jul(int y, int m, int d);
void jul_to_cal(long n, int *y, int *m, int *d);
double jul_and_time_to_jul(long jul, int hour, int min, double sec);
double cal_and_time_to_jul(int y, int m, int d,
                           int hour, int min, double sec);
void jul_to_cal_and_time(double jday, int rounding,
                         int *y, int *m, int *d,
                         int *hour, int *min, int *sec);
int parse_float(const char *s, double *value, const char **after);
int parse_date(const char* s, Dates_format preferred, int absolute,
               double *jul, Dates_format *recognized);
int parse_date_or_number(const char* s, int absolute, double *value);

#endif /* __NOXPROTOS_H_ */
