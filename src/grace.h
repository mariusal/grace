/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2003 Grace Development Team
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

#ifndef __GRACE_H_
#define __GRACE_H_

#include <config.h>

/* For FILE */
#include <stdio.h>

#include "grace/baseP.h"
#include "grace/canvas.h"
#include "grace/core.h"
#include "defines.h"

typedef struct _Grace Grace;

typedef struct _X11Stuff X11Stuff;
typedef struct _ExplorerUI ExplorerUI;

typedef struct _GUI {
    /* Parent */
    Grace *P;

    /* true if running X */
    int inwin;

    /* true if running in the free page mode */
    int page_free;

    /* use GXxor or GXinvert for xor'ing */
    int invert;
    /* if true, redraw graph each time action is performed */
    int auto_redraw;
    /* allow double click ops */
    int allow_dc;
    int focus_policy;
    int draw_focus_flag;
    /* if TRUE, assume yes for everything */
    int noask;
    /* instant update enabled for controls on appearance dialogs */
    int instant_update;
    /* toolbars visible on/off */
    int toolbar;
    int statusbar;
    int locbar;

    /* colormap stuff */
    int install_cmap;
    int private_cmap;
    
    X11Stuff *xstuff;
    
    ExplorerUI *eui;
} GUI;

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

    /* print to file */
    int ptofile;
    
    /* target set */
    Quark *target_set;

    /* real-time input delay (prevents getting stuck reading) */
    int timer_delay;
    /* autoscale after reading in data sets */
    int autoscale_onread;
    /* used in the parser */
    int curtype;
    int cursource;
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
    
    /* print command */
    char *print_cmd;	

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
    
    /* Misc dictionaries */
    Dictionary *graph_type_dict;
    Dictionary *set_type_dict;
    Dictionary *inout_placement_dict;
    Dictionary *side_placement_dict;
    Dictionary *spec_ticks_dict;

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

Quark *project_new(QuarkFactory *qfactory);

GUI *gui_new(Grace *grace);
void gui_free(GUI *gui);

RunTime *runtime_new(Grace *grace);
void runtime_free(RunTime *rt);

Grace *grace_new(void);
void grace_free(Grace *grace);

Grace *grace_from_quark(const Quark *q);
RunTime *rt_from_quark(const Quark *q);
GUI *gui_from_quark(const Quark *q);

int grace_set_project(Grace *grace, Quark *project);

int set_page_dimensions(Grace *grace, int wpp, int hpp, int rescale);
int set_printer(Grace *grace, int device);
int set_printer_by_name(Grace *grace, const char *dname);
void set_ptofile(Grace *grace, int flag);
int get_ptofile(const Grace *grace);

void project_reset_version(Quark *q);

int project_get_graphs(Quark *q, Quark ***graphs);

void project_set_wrap_year(Quark *q, int wrap_year);

int get_font_by_name(const Quark *project, const char *name);

#endif /* __GRACE_H_ */
