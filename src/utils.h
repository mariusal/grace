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

#ifndef __UTILS_H_
#define __UTILS_H_

/* for size_t */
#include <sys/types.h>

#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))

#define MIN2(a, b) (((a) < (b)) ? a : b)
#define MAX2(a, b) (((a) > (b)) ? a : b)
#define MIN3(a, b, c) (((a) < (b)) ? MIN2(a, c) : MIN2(b, c))
#define MAX3(a, b, c) (((a) > (b)) ? MAX2(a, c) : MAX2(b, c))

#define yes_or_no(x) ((x)?"yes":"no")
#define on_or_off(x) ((x)?"on":"off")
#define true_or_false(x) ((x)?"true":"false")
#define w_or_v(x) ((x == COORD_WORLD)?"world":"view")

#define LFORMAT_TYPE_PLAIN      0
#define LFORMAT_TYPE_EXTENDED   1

#define cxfree(ptr) xfree(ptr); ptr = NULL

void xfree(void *ptr);
void *xrealloc(void *ptr, size_t size);
void fswap(double *x, double *y);
void iswap(int *x, int *y);
int isoneof(int c, char *s);
int argmatch(char *s1, char *s2, int atleast);
void lowtoupper(char *s);
void convertchar(char *s);
int ilog2(int n);
double comp_area(int n, double *x, double *y);
double comp_perimeter(int n, double *x, double *y);

double julday(int mon, int day, int year, int h, int mi, double se);
void calcdate(double jd, int *m, int *d, int *y, int *h, int *mi, double *sec);
int dayofweek(double j);
int leapyear(int year);
void getmoday(int days, int yr, int *mo, int *da);
int getndays(double j);
char *create_fstring(int form, int prec, double loc, int type);
char *escapequotes (char *s);

int sign(double a);
double mytrunc(double a);

void bailout(void);

void installSignal(void);

int bin_dump(char *value, int i, int pad);
unsigned char reversebits(unsigned char inword);

char *copy_string(char *dest, const char *src);
void reverse_string(char *s);

char *get_grace_home(void);
void set_grace_home(char *dir);

char *get_help_viewer(void);
void set_help_viewer(char *dir);

char *get_print_cmd(void);
void set_print_cmd(char *cmd);

void errmsg(char *buf);
void echomsg(char *msg);
void stufftext(char *s, int sp);
void log_results(char *buf);

int yesnoterm(char *msg);
int yesno(char *msg, char *s1, char *s2, char *help_anchor);

int fexists(char *to);

char *mybasename(char *s);

void expand_tilde(char *buf);
int set_workingdir(char *wd);
char *get_workingdir(void);

void update_app_title(void);

void set_dirtystate(void);
void clear_dirtystate(void);
void lock_dirtystate(int flag);
int is_dirtystate(void);

int system_wrap(const char *string);
void sleep_wrap(unsigned int nsec);

char *set_locale_num(int flag);

#endif /* __UTILS_H_*/
