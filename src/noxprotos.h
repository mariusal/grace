/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2001 Grace Development Team
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

int filter_set(int gno, int setno, char *rarray);

int do_compute(int gno, int setno, int graphto, int loadto, char *rarray, char *fstr);
double trapint(double *x, double *y, double *resx, double *resy, int n);
int do_linearc(int gsrc, int setfrom, int gdest, int setto,
    int gconv, int setconv);
int do_xcor(int gsrc, int setfrom, int gdest, int setto,
    int gcor, int setcor, int maxlag, int covar);
int do_int(int gsrc, int setfrom, int gdest, int setto,
    int disponly, double *sum);
int do_differ(int gsrc, int setfrom, int gdest, int setto,
    int type, int xplace, int period);
int do_runavg(int gsrc, int setfrom, int gdest, int setto,
    int runlen, char *formula, int xplace);
int do_fourier(int gsrc, int setfrom, int gdest, int setto,
    int invflag, int xscale, int norm, int complexin, int dcdump,
    double oversampling, int round2n, int window, double beta, int halflen,
    int output);
int apply_window(double *v, int ilen, int window, double beta);
int do_histo(int fromgraph, int fromset, int tograph, int toset,
	      double *bins, int nbins, int cumulative, int normalize);
int histogram(int ndata, double *data, int nbins, double *bins, int *hist);

int do_sample(int gsrc, int setfrom, int gdest, int setto, char *formula);
int do_prune(int gsrc, int setfrom, int gdest, int setto, 
    int interp, int elliptic, double dx, int reldx, double dy, int reldy);

void set_region_defaults(region *r);
void set_default_framep(framep * f);
void set_default_world(world * w);
void set_default_view(view * v);
void set_default_legend(legend * l);

void set_default_string(plotstr *s);
void set_default_arrow(Arrow *arrowp);

void set_default_ticks(tickmarks *t);
void calculate_tickgrid(int gno);
void drawgrid(Canvas *canvas, int gno);
void drawaxes(Canvas *canvas, int gno);

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

void pop_world(void);

int find_item(graph *g, VPoint vp, view *bb, int *id);

int getsetminmax(int gno, int *sets, int nsets, double *x1, double *x2, double *y1, double *y2);
int getsetminmax_c(int gno, int *sets, int nsets, double *xmin, double *xmax, double *ymin, double *ymax, int ivec);
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax);
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax);
double vmin(double *x, int n);
double vmax(double *x, int n);
int set_point(int gno, int setn, int seti, WPoint wp);
int get_point(int gno, int setn, int seti, WPoint *wp);
int get_datapoint(int gno, int setn, int seti, int *ncols, Datapoint *wp);
Datapoint *datapoint_new(void);
void datapoint_free(Datapoint *dpoint);
Dataset *dataset_new(void);
void dataset_free(Dataset *dsp);
int set_dataset_nrows(Dataset *data, int nrows);
int set_dataset_ncols(Dataset *data, int ncols);
int dataset_set_datapoint(Dataset *dsp, const Datapoint *dpoint, int ind);
void setcol(int gno, int setno, int col, double *x, int len);

void copycol2(int gfrom, int setfrom, int gto, int setto, int col);

set *set_new(void);

int nextset(int gno);
void killset(int gno, int setno);
void killsetdata(int gno, int setno);
int number_of_active_sets(int gno);
int swapset(int gfrom, int j1, int gto, int j2);
int pushset(int gno, int setno, int push_type);
void droppoints(int gno, int setno, int startno, int endno);
int join_sets(int gno, int *sets, int nsets);
void sort_xy(double *tmp1, double *tmp2, int up, int sorton, int stype);
void reverse_set(int gno, int setno);

void del_point(int gno, int setno, int pt);
void add_point(int gno, int setno, double px, double py);
void zero_datapoint(Datapoint *dpoint);
int add_point_at(int gno, int setno, int ind, const Datapoint *dpoint);
void delete_byindex(int gno, int setno, int *ind);

int do_copyset(int gfrom, int j1, int gto, int j2);
int do_moveset(int gfrom, int j1, int gto, int j2);
int do_swapset(int gno1, int setno1, int gno2, int setno2);
void do_splitsets(int gno, int setno, int lpart);
void do_activate(int setno, int type, int len);
void do_hideset(int gno, int setno);
void do_showset(int gno, int setno);
void do_changetype(int setno, int type);
void do_copy(int j1, int gfrom, int j2, int gto);
void do_move(int j1, int gfrom, int j2, int gto);
void do_drop_points(int gno, int setno, int startno, int endno);
void do_kill(int gno, int setno, int soft);
void do_sort(int setno, int sorton, int stype);
void do_cancel_pickop(void);


void set_hotlink(int gno, int setno, int onoroff, char *fname, int src);
int is_hotlinked(int gno, int setno);
void do_update_hotlink(int gno, int setno);
char *get_hotlink_file(int gno, int setno);
int get_hotlink_src(int gno, int setno);

void sortset(int gno, int setno, int sorton, int stype);
int do_nonlfit(int gno, int setno, double *warray, char *rarray, int nsteps);
int do_interp(int gno_src, int setno_src, int gno_dest, int setno_dest,
    double *mesh, int meshlen, int method, int strict);
int get_restriction_array(int gno, int setno,
    int rtype, int negate, char **rarray);

int monotonicity(double *array, int len, int strict);
int monospaced(double *array, int len, double *space);
int find_span_index(double *array, int len, int m, double x);

int featext(int gfrom, int *sfrom, int nsets, int gto, int setto, char *formula);
int cumulative(int gsrc, int *ssids, int nsrc, int gdest, int sdest);

int inbounds(int gno, double x, double y);
int intersect_to_left(double x, double y, double x1, double y1, double x2, double y2);
int inbound(double x, double y, double *xlist, double *ylist, int n);
int isleft(double x, double y, double x1, double y1, double x2, double y2);
int isright(double x, double y, double x1, double y1, double x2, double y2);
int isabove(double x, double y, double x1, double y1, double x2, double y2);
int isbelow(double x, double y, double x1, double y1, double x2, double y2);
void reporton_region(int gno, int rno, int type);
int isactive_region(int regno);
char *region_types(int it, int which);
void kill_region(int r);
void kill_all_regions(void);
void activate_region(int r, int type, int gno);
void load_poly_region(int r, int gno, int n, WPoint *wps);
int inregion(int regno, double x, double y);

void set_plotstr_string(plotstr * pstr, char *buf);

void cli_loop(void);

void reset_nonl(NLFit *nlfit);

int is_xaxis(int axis);
int is_yaxis(int axis);
int is_log_axis(int gno, int axis);
int is_logit_axis(int gno, int axis);

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
