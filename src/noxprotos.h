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
 * Prototypes not involving X
 *
 */

#ifndef __NOXPROTOS_H_
#define __NOXPROTOS_H_

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "graphs.h"

void set_lists_dirty(int dirtyflag);

void loadset(int gno, int selset, int toval, double startno, double stepno);
int formula(int gno, int selset, char *sscanstr);

void do_running_command(int type, int setno, int rlen);
void do_fourier_command(int ftype, int setno, int ltype);
int do_compute(int setno, int loadto, int graphto, char *fstr);
void do_load(int setno, int toval,  double start, double step );
void do_compute2(int gno, char *fstrx, char *fstry, char *startstr, char *stopstr, int npts, int toval);
double trapint(double *x, double *y, double *resx, double *resy, int n);
void do_digfilter(int set1, int set2);
void do_linearc(int set1, int set2);
void do_xcor(int set1, int set2, int lag);
void do_spline(int set, double start, double stop, int n, int type);
double do_int(int setno, int itype);
void do_differ(int setno, int itype);
void do_regress(int setno, int ideg, int iresid, int rno, int invr, int rset);
void do_runavg(int setno, int runlen, int runtype, int rno, int invr);
void do_fourier(int fftflag, int setno, int load, int loadx, int invflag, int type, int wind);
void do_window(int setno, int type, int wind);
void apply_window(double *xx, double *yy, int ilen, int type, int wind);
void do_histo(int fromgraph, int fromset, int tograph, int toset,
	      double binw, double xmin, double xmax, int hist_type);
int histogram(int ndata, double *data, int nbins, double *bins, int *hist);

void do_sample(int setno, int typeno, char *exprstr, int startno, int stepno);
void do_prune(int setno, int typeno, int deltatypeno, float deltax, float deltay, int dxtype, int dytype);

void set_program_defaults(void);
void set_region_defaults(int i);
void set_default_message(char *buf);
void set_default_framep(framep * f);
void set_default_world(world * w);
void set_default_view(view * v);
void set_default_string(plotstr * s);
plotstr copy_plotstr(plotstr p);
void set_default_line(linetype * l);
void set_default_box(boxtype * b);
void set_default_ellipse(ellipsetype * b);
void set_default_legend(int gno, legend * l);
void set_default_plotarr(plotarr * p);
void set_default_graph(int gno);
void realloc_lines(int n);
void realloc_boxes(int n);
void realloc_ellipses(int n);
void realloc_strings(int n);
void set_default_annotation(void);
void set_default_ticks(tickmarks * t, int a);

void calculate_tickgrid(int gno);
void drawgrid(int gno);
void drawaxes(int gno);


void gauss(int n, double *a, int adim, double *b, double *x);
void stasum(double *x, int n, double *xbar, double *sd);
double leasev(double *c, int degree, double x);
int fitcurve(double *x, double *y, int n, int ideg, double *fitted);
void runavg(double *x, double *y, double *ax, double *ay, int n, int ilen);
void runstddev(double *x, double *y, double *ax, double *ay, int n, int ilen);
void runmedian(double *x, double *y, double *ax, double *ay, int n, int ilen);
void runminmax(double *x, double *y, double *ax, double *ay, int n, int ilen, int type);
void filterser(int n, double *x, double *y, double *resx, double *resy, double *h, int len);
void linearconv(double *x, double *h, double *y, int n, int m);
int crosscorr(double *x, double *y, int n, int lag, double *xcov, double *xcor);
int transfit(int type, int n, double *x, double *y, double *fitted);
int linear_regression(int n, double *x, double *y, double *fitted);
void spline(int n, double *x, double *y, double *b, double *c, double *d);
void aspline(int n, double *x, double *y, double *b, double *c, double *d);
double seval(int n, double u, double *x, double *y, double *b, double *c, double *d);
double aseval(int n, double u, double *x, double *y, double *b, double *c, double *d);

void dft(double *jr, double *ji, int n, int iflag);
void fft(double *real_data, double *imag_data, int n_pts, int nu, int inv);

int getparms(char *plfile);
int read_param(char *pbuf);

void putparms(int gno, FILE * pp, int embed);
void put_fitparms(FILE * pp, int embed);

void get_graph_box(int i, boxtype * b);
void get_graph_line(int i, linetype * l);
void get_graph_ellipse(int i, ellipsetype * e);
void get_graph_string(int i, plotstr * s);
void set_graph_box(int i, boxtype *b);
void set_graph_line(int i, linetype *l);
void set_graph_string(int i, plotstr *s);
void set_graph_ellipse(int i, ellipsetype * e);

void pop_world(void);

void define_autos(int aon, int au, int ap);

int find_item(int gno, VPoint vp, view *bb, int *type, int *id);

int isactive_line(int lineno);
int isactive_box(int boxno);
int isactive_ellipse(int ellipno);
int isactive_string(int strno);

int next_line(void);
int next_box(void);
int next_ellipse(void);
int next_string(void);

void kill_box(int boxno);
void kill_ellipse(int ellipseno);
void kill_line(int lineno);
void kill_string(int stringno);

void copy_object(int type, int from, int to);
int kill_object(int type, int id);
int next_object(int type);
int duplicate_object(int type, int id);

int get_object_bb(int type, int id, view *bb);
void move_object(int type, int id, VVector shift);

int number_of_boxes(void);
int number_of_ellipses(void);
int number_of_lines(void);
int number_of_strings(void);

void init_string(int id, VPoint vp);
void init_line(int id, VPoint vp1, VPoint vp2);
void init_box(int id, VPoint vp1, VPoint vp2);
void init_ellipse(int id, VPoint vp1, VPoint vp2);

void do_clear_lines(void);
void do_clear_boxes(void);
void do_clear_text(void);
void do_clear_ellipses(void);

void scanner(char *s, double *x, double *y, int len, double *a, double *b, double *c, double *d, int lenscr, int i, int setno, int *errpos);
double rnorm(double mean, double sdev);
double fx(double x);
void init_symtab(void);
int addto_symtab(symtab_entry newkey);


int init_array(double **a, int n);
int init_scratch_arrays(int n);
int getsetminmax(int gno, int setno, double *x1, double *x2, double *y1, double *y2);
int getsetminmax_c(int gno, int setno, double *xmin, double *xmax, double *ymin, double *ymax, int ivec);
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax);
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax);
int set_point(int gno, int setn, int seti, WPoint wp);
int get_point(int gno, int setn, int seti, WPoint *wp);
void setcol(int gno, double *x, int setno, int len, int col);
int getncols(int gno, int setno);
void setxy(int gno, double **ex, int setno, int len, int ncols);

void copycol2(int gfrom, int setfrom, int gto, int setto, int col);
#define copyx(gno, setfrom, setto)      copycol2(gno, setfrom, gno, setto, 0)
#define copyy(gno, setfrom, setto)      copycol2(gno, setfrom, gno, setto, 1)

void packsets(int gno);
int nextset(int gno);
void killset(int gno, int setno);
void killsetdata(int gno, int setno);
int activeset(int gno);
int swapset(int gfrom, int j1, int gto, int j2);
void droppoints(int gno, int setno, int startno, int endno, int dist);
void joinsets(int g1, int j1, int g2, int j2);
void sort_xy(double *tmp1, double *tmp2, int up, int sorton, int stype);
void reverse_set(int gno, int setno);

void del_point(int gno, int setno, int pt);
void add_point(int gno, int setno, double px, double py);
void zero_datapoint(Datapoint *dpoint);
int add_point_at(int gno, int setno, int ind, const Datapoint *dpoint);
void delete_byindex(int gno, int setno, int *ind);

double *getvptr(int gno, int setno, int v);

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
void do_drop_points(int setno, int startno, int endno);
void do_join_sets(int gfrom, int j1, int gto, int j2);
void do_kill(int gno, int setno, int soft);
void do_sort(int setno, int sorton, int stype);
void do_cancel_pickop(void);


void set_hotlink(int gno, int setno, int onoroff, char *fname, int src);
int is_hotlinked(int gno, int setno);
void do_update_hotlink(int gno, int setno);
char *get_hotlink_file(int gno, int setno);
int get_hotlink_src(int gno, int setno);

void do_breakset(int gno, int setno, int ind);
void sortset(int gno, int setno, int sorton, int stype);
void do_seasonal_diff(int setno, int period);
int do_nonlfit(int gno, int setno, int nsteps);
void do_interp( int yset, int xset, int method );

int inbounds(int gno, double x, double y);
int intersect_to_left(double x, double y, double x1, double y1, double x2, double y2);
int inbound(double x, double y, double *xlist, double *ylist, int n);
int isleft(double x, double y, double x1, double y1, double x2, double y2);
int isright(double x, double y, double x1, double y1, double x2, double y2);
int isabove(double x, double y, double x1, double y1, double x2, double y2);
int isbelow(double x, double y, double x1, double y1, double x2, double y2);
void extractsets_region(int gfrom, int gto, int rno);
void deletesets_region(int gno, int rno);
void reporton_region(int gno, int rno, int type, int strict);
int isactive_region(int regno);
char *region_types(int it, int which);
void kill_region(int r);
void activate_region(int r, int type);
void extract_region(int gno, int fromset, int toset, int regno);
void delete_region(int gno, int setno, int regno);
void evaluate_region(int regno, int gno, int setno, char *buf);
void load_poly_region(int r, int n, WPoint *wps);
int inregion(int regno, double x, double y);

void set_plotstr_string(plotstr * pstr, char *buf);
void my_ellipse(double xc, double yc, double w, double h, int fill);

int lists_dirty(void);
int work_pending(void);

void do_main_loop(void);

void initialize_nonl(void);
void reset_nonl(void);

int is_xaxis(int axis);
int is_yaxis(int axis);

void kill_blockdata(void);
void alloc_blockdata(int ncols);

#endif /* __NOXPROTOS_H_ */
