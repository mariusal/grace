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

#ifndef __UTILS_H_
#define __UTILS_H_

/* for time_t */
#ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
#  ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#include "grace.h"

#define on_or_off(x) ((x)?"on":"off")
#define true_or_false(x) ((x)?"true":"false")

#define LFORMAT_TYPE_PLAIN      0
#define LFORMAT_TYPE_EXTENDED   1

void fswap(double *x, double *y);
void iswap(int *x, int *y);
int isoneof(int c, char *s);
int argmatch(char *s1, char *s2, int atleast);
void lowtoupper(char *s);
void convertchar(char *s);

char *create_fstring(const Quark *q, int form, int prec, double loc, int type);
char *escapequotes(char *s);

double mytrunc(double a);

void bailout(Grace *grace);

void installSignal(void);
void emergency_exit(Grace *grace, int is_my_bug, char *msg);

char *get_grace_home(const Grace *grace);

char *get_help_viewer(const Grace *grace);
void set_help_viewer(Grace *grace,const char *dir);

char *get_print_cmd(const Grace *grace);
void set_print_cmd(Grace *grace, const char *cmd);

char *get_editor(const Grace *grace);
void set_editor(Grace *grace, const char *cmd);

int set_workingdir(Grace *grace, const char *wd);
char *get_workingdir(const Grace *grace);

char *get_username(const Grace *grace);
char *get_userhome(const Grace *grace);

char *get_docbname(const Quark *q);

void errmsg(const char *msg);
void echomsg(char *msg);
void stufftext(char *msg);

int yesnoterm(char *msg);
int yesno(char *msg, char *s1, char *s2, char *help_anchor);

char *mybasename(const char *s);

void expand_tilde(const Grace *grace, char *buf);

void update_app_title(Quark *q);

int system_wrap(const char *string);
void msleep_wrap(unsigned int msec);

int init_locale(void);
void set_locale_num(int flag);

long bi_version_id(void);
char *bi_version_string(void);
char *bi_system(void);
char *bi_date(void);
char *bi_gui(void);
#ifdef MOTIF_GUI
char *bi_gui_xbae(void);
#endif
char *bi_ccompiler(void);
char *bi_t1lib(void);

char *bi_home(void);
char *bi_print_cmd(void);
char *bi_editor(void);
char *bi_helpviewer(void);

#ifdef DEBUG
void set_debuglevel(Grace *grace, int level);
int get_debuglevel(Grace *grace);
#endif

#endif /* __UTILS_H_*/
