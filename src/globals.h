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
 * Global variables of Grace
 *
 */

#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <stdlib.h>
#include <stdio.h>

#include "defines.h"
#include "graphs.h"
#include "draw.h"


#ifdef MAIN

char docname[GR_MAXPATHLEN] = NONAME;

int debuglevel = 0;

int ptofile = FALSE;            /* flag to indicate destination of hardcopy
                                 * output, ptofile = 0 means print to printer
                                 * non-zero print to file */

char sformat[128] = "%16.8g";   /* format for saving data sets */

int logwindow = FALSE;		/* TRUE if results are displayed in the log window */

/*
 * real-time input delay (prevents getting stuck reading)
 */
int timer_delay = 1000;         /* timer */

/*
 * scroll amount
 */
int scrolling_islinked = FALSE;	/* linked scroll */
double scrollper = 0.05;	/* scroll fraction */
double shexper = 0.05;		/* expand/shrink fraction */

int device;			/* graphics device */
int tdevice;                    /* default device */
int hdevice;                    /* hardcopy device */

int monomode = FALSE;		/* set mono mode */
int invert = FALSE;		/* use GXxor or GXinvert for xor'ing */
int autoscale_onread = AUTOSCALE_XY; /* autoscale after reading in data sets */
int auto_redraw = TRUE;		/* if true, redraw graph each time action is
				 * performed */
int allow_dc = TRUE;		/* allow double click ops */
int status_auto_redraw = TRUE;	/* if true, redraw graph each time action is
				 * performed in the status window */
int force_redraw = FALSE;	/* if no auto draw and re-draw pressed */
int noask = FALSE;              /* if TRUE, assume yes for everything (dangerous) */

int index_shift = 0; 		/* 0 for C, 1 for F77 index notation */

FILE *resfp;			/* file for results */

int inwin = FALSE;		/* true if running X */

defaults grdefaults;		/* default properties */


int curset;
int focus_policy = FOCUS_CLICK;

int draw_focus_flag = TRUE;

plotstr timestamp;       /* timestamp */

/*
 * used in the parser
 */
int cursource = SOURCE_DISK, curtype = SET_XY;

int format_types[] = {FORMAT_DECIMAL, FORMAT_EXPONENTIAL, FORMAT_GENERAL, FORMAT_POWER,
                      FORMAT_SCIENTIFIC, FORMAT_ENGINEERING,
		      FORMAT_DDMMYY, FORMAT_MMDDYY, FORMAT_YYMMDD, FORMAT_MMYY, FORMAT_MMDD,
        	      FORMAT_MONTHDAY, FORMAT_DAYMONTH, FORMAT_MONTHS, FORMAT_MONTHSY, FORMAT_MONTHL, FORMAT_DAYOFWEEKS,
        	      FORMAT_DAYOFWEEKL, FORMAT_DAYOFYEAR, FORMAT_HMS, FORMAT_MMDDHMS, FORMAT_MMDDYYHMS, FORMAT_YYMMDDHMS,
        	      FORMAT_DEGREESLON, FORMAT_DEGREESMMLON, FORMAT_DEGREESMMSSLON, FORMAT_MMSSLON,
        	      FORMAT_DEGREESLAT, FORMAT_DEGREESMMLAT, FORMAT_DEGREESMMSSLAT, FORMAT_MMSSLAT, FORMAT_INVALID};


/* block data globals */
double **blockdata;
int maxblock = MAXBLOCK;
int blocklen;
int blockncols;

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
int readcdf = FALSE;
char netcdf_name[512], xvar_name[128], yvar_name[128];
#endif

/* parameters for non-linear fit */
nonlparms nonl_parms[MAXPARM];
nonlopts nonl_opts;
nonlprefs nonl_prefs;

target target_set; /* target */


/* region definition */
region rg[MAXREGION];
int nr = 0;			/* the current region */


/* drawing objects */
int maxboxes = MAXBOXES;
int maxlines = MAXLINES;
int maxstr = MAXSTR;
int maxellipses = MAXELLIPSES;

plotstr *pstr;       /* strings */
boxtype *boxes;    /* boxes */
linetype *lines;   /* lines */
ellipsetype *ellip;   /* ellipses */

plotstr defpstr;
linetype defline;
boxtype defbox;
ellipsetype defellip={TRUE,COORD_VIEW,0,0,0,0,0,1,1,1,1,0};

/* lines and ellipses and boxes flags */
int box_color = 1;
int box_lines = 1;
double box_linew = 1;
int box_fillpat = 0;
int box_fillcolor = 1;
int box_loctype = COORD_VIEW;

int ellipse_color = 1;
int ellipse_lines = 1;
double ellipse_linew = 1;
int ellipse_fillpat = 0;
int ellipse_fillcolor = 1;
int ellipse_loctype = COORD_VIEW;

int line_color = 1;
int line_arrow = 0;
int line_lines = 1;
double line_linew = 1;
int line_loctype = COORD_VIEW;
double line_asize = 1.0;
int line_atype = 0;

/* default string parameters */
int string_color = 1;
int string_font = 0;
int string_rot = 0;
int string_just = 0;
int string_loctype = COORD_VIEW;
double string_size = 1.0;

#endif

#ifndef MAIN

extern char docname[];

extern int debuglevel;

extern int inwin;		/* true if running X */
extern int ispipe;		/* true if reading from stdin */

extern int maxboxes;
extern int maxlines;
extern int maxstr;
extern int maxellipses;

extern int ptofile;		/* flag to indicate destination of hardcopy
                                 * output, ptofile = 0 means print to printer
                                 * non-zero print to file */

extern char sformat[];

extern int logwindow;		/* TRUE if results are displayed in the log window */

extern FILE *resfp;

extern int device, tdevice, hdevice;

extern int monomode;		/* set mono mode */
extern int invert;		/* use GXxor or GXinvert for xor'ing */
extern int autoscale_onread;	/* autoscale after reading data from fileswin.c */
extern int noask;		/* if TRUE, assume yes for everything (dangerous) */

extern int index_shift; 	/* 0 for C, 1 for F77 index notation */

extern int timer_delay;		/* timer to interrupt too long reads */

extern int scrolling_islinked;	/* linked scroll */
extern double scrollper;	/* scroll fraction */
extern double shexper;		/* expand/shrink fraction */

extern int allow_dc;		/* allow double click ops */

extern defaults grdefaults;	/* default properties */


extern int auto_redraw;
extern int status_auto_redraw;
extern int force_redraw;

extern double charsize, xlibcharsize;	/* declared in draw.c and xlib.c resp. */

extern int curset;
extern int focus_policy;
extern int draw_focus_flag;

extern plotstr timestamp;       /* timestamp */

extern int cursource, curtype;
extern int format_types[];

extern double **blockdata;
extern int maxblock;
extern int blocklen;
extern int blockncols;

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
extern int readcdf;
extern char netcdf_name[];
extern char xvar_name[];
extern char yvar_name[];
#endif

/* parameters for non-linear fit */
extern nonlparms nonl_parms[];
extern nonlopts nonl_opts;
extern nonlprefs nonl_prefs;

extern target target_set; /* target */


/* region definition */
extern region rg[];
extern int nr;


extern plotstr *pstr;		/* strings */
extern boxtype *boxes;		/* boxes */
extern linetype *lines;		/* lines */
extern ellipsetype *ellip;	/* ellipses */

extern plotstr defpstr;
extern linetype defline;
extern boxtype defbox;
extern ellipsetype defellip;

extern int box_color;
extern int box_lines;
extern double box_linew;
extern int box_fill;
extern int box_fillpat;
extern int box_fillcolor;
extern int box_loctype;

extern int ellipse_color;
extern int ellipse_lines;
extern double ellipse_linew;
extern int ellipse_fill;
extern int ellipse_fillpat;
extern int ellipse_fillcolor;
extern int ellipse_loctype;

extern int line_color;
extern int line_arrow;
extern int line_lines;
extern double line_linew;
extern int line_loctype;
extern double line_asize;
extern int line_atype;

extern int string_color;
extern int string_font;
extern int string_rot;
extern int string_just;
extern int string_loctype;
extern double string_size;

#endif

#endif /* __GLOBALS_H_ */
