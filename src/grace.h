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
#include "defines.h"
#include "storage.h"
#include "dict3.h"
#include "grace/canvas.h"

#define QUARK_ETYPE_MODIFY  1
#define QUARK_ETYPE_DELETE  2

typedef struct _Grace Grace;

typedef struct _Quark Quark;

typedef void  (*Quark_data_free)(void *data); 
typedef void *(*Quark_data_new)(void); 
typedef void *(*Quark_data_copy)(void *data); 
typedef void  (*Quark_gui)(Quark *q, void *data); 

typedef int (*Quark_cb)(Quark *q, int etype, void *data); 

typedef struct _QuarkFlavor {
    Quark_data_new  data_new;
    Quark_data_free data_free;
    Quark_data_copy data_copy;
    Quark_gui gui;
} QuarkFlavor;

struct _Quark {
    Grace *grace;
    unsigned int fid;
    char *idstr;
    
    struct _Quark *parent;
    Storage *children;
    unsigned int dirtystate;
    unsigned int refcount;
    
    void *data;
    
    Quark_cb cb;
    void *cbdata;
};

typedef struct {
    unsigned int depth;
    unsigned int step;
    int pass2;
    int descend;
} QTraverseClosure;

typedef int (*Quark_traverse_hook)(Quark *q,
    void *udata, QTraverseClosure *closure); 

typedef struct _Project {
    /* Version ID */
    int version_id;
    
    /* textual description */
    char *description;
    
#if 0
    Storage *blockdata;
#endif
    /* (pointer to) current graph */
    Quark *cg;
#if 0
    Storage *regions;
#else
    /* region definition */
    region rg[MAXREGION];
    /* the current region */
    int nr;
#endif
    
    /* timestamp */
    char *timestamp;
    
    /* page size */
    int page_wpp, page_hpp;
    
    /* font map */
    unsigned int nfonts;
    Fontdef *fontmap;
    
    /* page fill */
    int bgcolor;
    int bgfill;
    
    /* format for saving data sets */
    char *sformat;

    /* project file name */
    char *docname;	
} Project;

typedef struct _GUI {
    /* Parent */
    Grace *P;

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
    /* Parent */
    Grace *P;
    
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

enum {
    QFlavorProject,
    QFlavorFrame,
    QFlavorGraph,
    QFlavorSet,
    QFlavorDObject,
    QFlavorContainer
};

QuarkFlavor *quark_flavor_get(Grace *grace, unsigned int fid);

Quark *quark_root(Grace *grace, unsigned int fid);
Quark *quark_new(Quark *parent, unsigned int fid);
void quark_free(Quark *q);
Quark *quark_copy(const Quark *q);
Quark *quark_copy2(Quark *newparent, const Quark *q);

void quark_dirtystate_set(Quark *q, int flag);
int quark_dirtystate_get(const Quark *q);

void quark_idstr_set(Quark *q, const char *s);
char *quark_idstr_get(const Quark *q);
Quark *quark_find_child_by_idstr(Quark *q, const char *s);
Quark *quark_find_descendant_by_idstr(Quark *q, const char *s);

int quark_cb_set(Quark *q, Quark_cb cb, void *cbdata);
void quark_traverse(Quark *q, Quark_traverse_hook hook, void *udata);

int quark_count_children(const Quark *q);
int quark_child_exist(const Quark *parent, const Quark *child);
int quark_reparent(Quark *q, Quark *newparent);
int quark_reparent_children(Quark *parent, Quark *newparent);

int quark_move(const Quark *q, int forward);
int quark_push(const Quark *q, int forward);

#define QIDSTR(q) (q->idstr ? q->idstr:"unnamed")

Project *project_data_new(void);
void project_data_free(Project *pr);
Quark *project_new(Grace *grace);
void project_free(Quark *q);

GUI *gui_new(Grace *grace);
void gui_free(GUI *gui);

RunTime *runtime_new(Grace *grace);
void runtime_free(RunTime *rt);

Grace *grace_new(void);
void grace_free(Grace *grace);

int grace_set_project(Grace *grace, Quark *project);

int set_page_dimensions(Grace *grace, int wpp, int hpp, int rescale);
int set_printer(Grace *grace, int device);
int set_printer_by_name(Grace *grace, const char *dname);
void set_ptofile(Grace *grace, int flag);
int get_ptofile(const Grace *grace);

int project_get_version_id(const Quark *q);
int project_set_version_id(Quark *q, int version_id);
void project_reset_version(Quark *q);

void project_set_description(Quark *q, char *descr);
char *project_get_description(const Quark *q);

void project_set_dirtystate(Quark *q);
void project_clear_dirtystate(Quark *q);

int project_get_graphs(Quark *q, Quark ***graphs);

char *project_get_sformat(const Quark *q);
void project_set_sformat(Quark *q, const char *s);

Project *project_get_data(const Quark *q);

int project_add_font(Quark *project, const Fontdef *f);
int get_font_by_name(const Quark *project, const char *name);

#endif /* __GRACE_H_ */
