/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2003 Grace Development Team
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

#ifndef __CANVASP_H_
#define __CANVASP_H_

#include <stdio.h>

#include <t1lib.h>

#include <grace/canvas.h>

/* Drawing properties */
typedef struct {
    Pen pen;
    int bgcolor;
    int lines;
    double linew;
    int linecap;
    int linejoin;
    double charsize;
    int font;
    int fillrule;
} DrawProps;

typedef struct {
    char *alias;
    char used;
    char chars_used[256];
} FontDB;

typedef struct {
    int active;
    view v;
    view fv;
} BBox_type;

/* Canvas */
struct _Canvas {
    /* drawing properties */
    DrawProps draw_props;
    
    /* page background fill */
    int pagefill;
    
    /* colors */
    unsigned int ncolors;
    CMap_entry *cmap;

    /* patterns */
    unsigned int npatterns;
    PMap_entry *pmap;
    
    /* linestyles */
    unsigned int nlinestyles;
    LMap_entry *lmap;
    
    /* fonts */
    unsigned int nfonts;
    FontDB *FontDBtable;
    char **DefEncoding;

    /* clipping */
    int clipflag;
    view clipview;
    
    int draw_mode;
    int drypass;

    BBox_type bboxes[2];

    int max_path_length;

    /* device array */
    unsigned int ndevices;
    Device_entry **device_table;
    
    /* current device */
    Device_entry *curdevice;
    
    /* "device ready" flag */
    int device_ready;
    
    /* output stream */
    FILE *prstream;
    
    /* info */
    char *username;
    char *docname;
    char *description;
    
    /* user-supplied procedure for parsing composite strings */
    CanvasCSParseProc csparse_proc;
    /* user-supplied procedure for mapping font ids */
    CanvasFMapProc fmap_proc;
    
    /* user data */
    void *udata;

    /* cached values of the grayscale AA levels and validity flags */
    int aacolors_low_ok;
    unsigned long aacolors_low[T1_AALEVELS_LOW];
    int aacolors_high_ok;
    unsigned long aacolors_high[T1_AALEVELS_HIGH];
};

#endif /* __CANVASP_H_ */
