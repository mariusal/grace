/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001 Grace Development Team
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

#include "defines.h"
#include "storage.h"
#include "dict3.h"

typedef struct _Project {
    /* Version ID */
    int version_id;
    
    /* textual description */
    char *description;
    
#if 0
    Storage *blockdata;
#endif
    Storage *graphs;
#if 0
    Storage *regions;
#else
    /* region definition */
    region rg[MAXREGION];
    /* the current region */
    int nr;
#endif
    
    /* timestamp */
    plotstr timestamp;
    
    /* page size */
    int page_wpp, page_hpp;
    
    /* format for saving data sets */
    char *sformat;

    /* project file name */
    char *docname;	

    /* dirtystate stuff */
    int dirtystate;
    int dirtystate_lock;
} Project;

typedef struct _GUI {
    /* true if running X */
    int inwin;
    /* use GXxor or GXinvert for xor'ing */
    int invert;
    /* if true, redraw graph each time action is performed */
    int auto_redraw;
    /* allow double click ops */
    int allow_dc;
    int focus_policy;
    int draw_focus_flag;
    /* set mono mode */
    int monomode;
    /* if TRUE, assume yes for everything */
    int noask;
    /* instant update enabled for controls on appearance dialogs */
    int instant_update;
    /* toolbars visible on/off */
    int toolbar;
    int statusbar;
    int locbar;
} GUI;

typedef struct _RunTime {
    /* terminal device */
    int tdevice;
    /* hardcopy device */
    int hdevice;

    /* print to file */
    int ptofile;
    
    /* target set */
    target target_set;
    /* parameters for non-linear fit */
    NLFit *nlfit;
    /* real-time input delay (prevents getting stuck reading) */
    int timer_delay;
    /* autoscale after reading in data sets */
    int autoscale_onread;
    /* used in the parser */
    int curtype;
    int cursource;
    /* file for results */
    FILE *resfp;
    
    /* default graphics properties */
    defaults grdefaults;
    
    /* linked scroll */
    int scrolling_islinked;
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

typedef struct _Grace {
#if 0
    Preferences *prefs;
#endif
    RunTime *rt;
    GUI *gui;
    Project *project;
} Grace;

Project *project_new(void);
void project_free(Project *pr);

GUI *gui_new(void);
void gui_free(GUI *gui);

RunTime *runtime_new(void);
void runtime_free(RunTime *rt);

Grace *grace_new(void);
void grace_free(Grace *grace);

int project_get_version_id(Project *pr);
int project_set_version_id(Project *pr, int version_id);
void project_reset_version(Project *pr);

void project_set_description(Project *pr, char *descr);
char *project_get_description(Project *pr);

void project_set_dirtystate(Project *pr);
void project_clear_dirtystate(Project *pr);
void project_lock_dirtystate(Project *pr, int flag);
int project_is_dirtystate(Project *pr);

Storage *project_get_graphs(Project *pr);

#endif /* __GRACE_H_ */
