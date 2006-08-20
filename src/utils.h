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

#ifndef __UTILS_H_
#define __UTILS_H_

#include "graceapp.h"

int argmatch(char *s1, char *s2, int atleast);
void lowtoupper(char *s);

char *escapequotes(char *s);

void bailout(GraceApp *gapp);

void installSignal(void);
void emergency_exit(GraceApp *gapp, int is_my_bug, char *msg);

char *get_gapp_home(const GraceApp *gapp);

char *get_help_viewer(const GraceApp *gapp);
void set_help_viewer(GraceApp *gapp,const char *dir);

int get_print_dest(const GraceApp *gapp);
void set_print_dest(GraceApp *gapp, int dest);
char *get_print_cmd(const GraceApp *gapp);
void set_print_cmd(GraceApp *gapp, const char *cmd);

char *get_editor(const GraceApp *gapp);
void set_editor(GraceApp *gapp, const char *cmd);

int set_workingdir(GraceApp *gapp, const char *wd);
char *get_workingdir(const GraceApp *gapp);

void set_date_hint(GraceApp *gapp, Dates_format preferred);
Dates_format get_date_hint(const GraceApp *gapp);

void errmsg(const char *msg);
void echomsg(char *msg);
void stufftext(char *msg);

int yesnoterm(char *msg);
int yesno(char *msg, char *s1, char *s2, char *help_anchor);

char *mybasename(const char *s);

int system_wrap(const char *string);
void msleep_wrap(unsigned int msec);

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

char *q_labeling(Quark *q);

#endif /* __UTILS_H_*/
