/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2006 Grace Development Team
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

#ifndef __GRACEAPP_H_
#define __GRACEAPP_H_

#include <config.h>

/* For FILE */
#include <stdio.h>

#include "grace/baseP.h"
#include "grace/canvas.h"
#include "grace/core.h"
#include "grace/plot.h"
#include "grace/grace.h"
#include "defines.h"

typedef struct _GraceApp GraceApp;

typedef struct _X11Stuff X11Stuff;
typedef struct _ExplorerUI ExplorerUI;
typedef struct _MainWinUI MainWinUI;

typedef struct _GUI {
    /* Parent */
    GraceApp *P;

    /* true if running X */
    int inwin;

    /* true if running in the free page mode */
    int page_free;

    /* Zoom factor */
    double zoom;

    /* use GXxor or GXinvert for xor'ing */
    int invert;
    int focus_policy;
    int draw_focus_flag;
    
    int crosshair_cursor;

    int force_external_viewer;
    
    /* if TRUE, assume yes for everything */
    int noask;
    /* instant update enabled for controls on appearance dialogs */
    int instant_update;
    /* toolbars visible on/off */
    int toolbar;
    int statusbar;
    int locbar;
    
    /* if TRUE, color selectors need update */
    int need_colorsel_update;

    /* if TRUE, font selectors need update */
    int need_fontsel_update;

    /* colormap stuff */
    int install_cmap;
    int private_cmap;
    
    X11Stuff *xstuff;
    
    MainWinUI *mwui;
    ExplorerUI *eui;
} GUI;

typedef struct {
    char *name;
    char *text;
    Dictionary *choices;
    int selected;
} PrintOption;

typedef struct {
    char *name;
    char *text;
    int nopts;
    PrintOption *opts;
} PrintOptGroup;

typedef struct {
    char *name;
    char *inst;
    char *printer;
    int nogroups;
    PrintOptGroup *ogroups;
    int nopts;
} PrintDest;


typedef struct _RunTime {
    /* Parent */
    GraceApp *P;
    
    /* terminal device */
    int tdevice;
    /* hardcopy device */
    int hdevice;
    
    /* real-time input delay (prevents getting stuck reading) */
    int timer_delay;
    /* autoscale after reading in data sets */
    int autoscale_onread;

    /* file for results */
    FILE *resfp;
    
    /* scroll fraction */
    double scrollper;
    /* expand/shrink fraction */
    double shexper;

    /* whether to use CUPS printing system */
    int use_cups;
    /* print command */
    char *print_cmd;
    /* print destinations */
    int num_print_dests;
    PrintDest *print_dests;
    int print_dest;
    /* print to file */
    int ptofile;
    
    /* Hint for parsing dates */
    Dates_format date_hint;
    
    /* editor */
    char *gapp_editor;

    /* html viewer */
    char *help_viewer;

    /* working directory */
    char *workingdir;

    /* safe mode flag */
    int safe_mode;

    /* printout */
    char print_file[GR_MAXPATHLEN];
    
    /* flag raised on emergency save */
    int emergency_save;
    /* number of interrupts received during the emergency save */
    int interrupts;

    /* color index for autocolorization of new sets */
    unsigned int setcolor;
} RunTime;

struct _GraceApp {
    Grace *grace;
    RunTime *rt;
    GUI *gui;
    GProject *gp;
    Quark *pc;

    unsigned int gpcount;
    GProject **gplist;
};

GUI *gui_new(GraceApp *gapp);
void gui_free(GUI *gui);

RunTime *runtime_new(GraceApp *gapp);
void runtime_free(RunTime *rt);

GraceApp *gapp_new(void);
void gapp_free(GraceApp *gapp);

GraceApp *gapp_from_quark(const Quark *q);
RunTime *rt_from_quark(const Quark *q);
GUI *gui_from_quark(const Quark *q);
GProject *gproject_from_quark(const Quark *q);

int gui_is_page_free(const GUI *gui);
void gui_set_page_free(GUI *gui, int onoff);
void gui_set_barebones(GUI *gui);

int gapp_add_project(GraceApp *gapp, GProject *gp);
int gapp_delete_project(GraceApp *gapp, GProject *gp);
int gapp_set_active_project(GraceApp *gapp, GProject *gp);

int set_page_dimensions(GraceApp *gapp, int wpp, int hpp, int rescale);
int set_printer(GraceApp *gapp, int device);
int set_printer_by_name(GraceApp *gapp, const char *dname);
void set_ptofile(GraceApp *gapp, int flag);
int get_ptofile(const GraceApp *gapp);

int project_get_graphs(Quark *q, Quark ***graphs);

char *gapp_exe_path(GraceApp *gapp, const char *fn);

FILE *gapp_openw(GraceApp *gapp, const char *fn);
FILE *gapp_openr(GraceApp *gapp, const char *fn, int src);
void gapp_close(FILE *fp);
FILE *gapp_tmpfile(char *templateval);

int gapp_init_print(RunTime *rt);

int gapp_print(const GraceApp *gapp, const char *fname);

void do_hardcopy(const GProject *gp);

#endif /* __GRACEAPP_H_ */
