/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2004 Grace Development Team
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

#ifndef __GRACE_H_
#define __GRACE_H_

#include <config.h>

/* For FILE */
#include <stdio.h>

#include "grace/baseP.h"
#include "grace/canvas.h"
#include "grace/core.h"
#include "grace/plot.h"
#include "defines.h"

typedef struct _Grace Grace;

typedef struct _X11Stuff X11Stuff;
typedef struct _ExplorerUI ExplorerUI;
typedef struct _MainWinUI MainWinUI;

typedef struct _GUI {
    /* Parent */
    Grace *P;

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
    Grace *P;
    
    QuarkFactory *qfactory;
    
    /* safe mode flag */
    int safe_mode;
    
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

    /* flag raised on emergency save */
    int emergency_save;
    /* number of interrupts received during the emergency save */
    int interrupts;
    
    /* location of the Grace home directory */
    char *grace_home;
    
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
    
    /* editor */
    char *grace_editor;	

    /* html viewer */
    char *help_viewer;	

    /* working directory */
    char *workingdir;	

    /* username */
    char *username;

    /* $HOME */
    char *userhome;
    
    /* printout */
    char print_file[GR_MAXPATHLEN];

    Canvas *canvas;

    Graal  *graal;
    
    /* Misc dictionaries */
    Dictionary *graph_type_dict;
    Dictionary *set_type_dict;
    Dictionary *object_type_dict;
    Dictionary *inout_placement_dict;
    Dictionary *spec_ticks_dict;
    Dictionary *region_type_dict;
    Dictionary *axis_position_dict;
    Dictionary *arrow_type_dict;
    Dictionary *glocator_type_dict;
    Dictionary *sym_type_dict;
    Dictionary *line_type_dict;
    Dictionary *setfill_type_dict;
    Dictionary *baseline_type_dict;
    Dictionary *framedecor_type_dict;
    Dictionary *scale_type_dict;
    Dictionary *arrow_placement_dict;
    Dictionary *arcclosure_type_dict;
    Dictionary *format_type_dict;
    Dictionary *frame_type_dict;
    
    /* color index for autocolorization of new sets */
    unsigned int setcolor;

    /* debug level */
#ifdef DEBUG
    int debuglevel;
#endif    
} RunTime;

struct _Grace {
#if 0
    Quark *prefs;
#endif
    RunTime *rt;
    GUI *gui;
    Quark *project;
};

Quark *grace_project_new(Grace *grace, int mmodel);

GUI *gui_new(Grace *grace);
void gui_free(GUI *gui);

RunTime *runtime_new(Grace *grace);
void runtime_free(RunTime *rt);

Grace *grace_new(void);
void grace_free(Grace *grace);

Grace *grace_from_quark(const Quark *q);
RunTime *rt_from_quark(const Quark *q);
GUI *gui_from_quark(const Quark *q);

int gui_is_page_free(const GUI *gui);
void gui_set_page_free(GUI *gui, int onoff);
void gui_set_barebones(GUI *gui);

int grace_set_project(Grace *grace, Quark *project);

int set_page_dimensions(Grace *grace, int wpp, int hpp, int rescale);
int set_printer(Grace *grace, int device);
int set_printer_by_name(Grace *grace, const char *dname);
void set_ptofile(Grace *grace, int flag);
int get_ptofile(const Grace *grace);

void project_reset_version(Quark *q);

int project_get_graphs(Quark *q, Quark ***graphs);

char *grace_path(Grace *grace, char *fn);
char *grace_path2(Grace *grace, const char *prefix, char *fn);
char *grace_exe_path(Grace *grace, char *fn);

FILE *grace_openw(Grace *grace, char *fn);
FILE *grace_openr(Grace *grace, char *fn, int src);
void grace_close(FILE *fp);

int grace_init_print(RunTime *rt);

int grace_print(const Grace *grace, const char *fname);

void do_hardcopy(const Quark *project);

#endif /* __GRACE_H_ */
