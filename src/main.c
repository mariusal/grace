/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

/* for globals.h */
#define MAIN

#include "globals.h"

#include "utils.h"
#include "files.h"

#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"

#include "device.h"
#include "dummydrv.h"
#include "psdrv.h"
#include "mfdrv.h"
#ifndef NONE_GUI
#  include "x11drv.h"
#endif
#ifdef HAVE_LIBPDF         
#  include "pdfdrv.h"
#endif
#ifdef HAVE_LIBGD         
#  include "rstdrv.h"
#endif
#include "t1fonts.h"
#include "protos.h"

#include "buildinfo.h"


extern char batchfile[];
extern char print_file[];
extern int install_cmap;

extern Input_buffer *ib_tbl;
extern int ib_tblsize;

static void usage(FILE *stream, char *progname);
static void VersionInfo(void);

int inpipe = FALSE;		/* if xmgrace is to participate in a pipe */

Device_entry dev_dummy = {DEVICE_TERM,
          "Dummy",
          dummyinitgraphics,
          NULL,
          NULL,
          "",
          TRUE,
          FALSE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0, 72.0}
         };

#ifndef NONE_GUI
Device_entry dev_x11 = {DEVICE_TERM,
          "X11",
          xlibinitgraphics,
          NULL,
          NULL,
          "",
          FALSE,
          TRUE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0, 72.0}
         };
#endif

Device_entry dev_ps = {DEVICE_PRINT,
          "PostScript",
          psprintinitgraphics,
          ps_op_parser,
          ps_gui_setup,
          "ps",
          TRUE,
          FALSE,
          {3300, 2550, 300.0, 300.0}
         };

Device_entry dev_eps = {DEVICE_FILE,
          "EPS",
          epsinitgraphics,
          eps_op_parser,
          eps_gui_setup,
          "eps",
          TRUE,
          FALSE,
          {2500, 2500, 300.0, 300.0}
         };

Device_entry dev_mf = {DEVICE_FILE,
          "Metafile",
          mfinitgraphics,
          NULL,
          NULL,
          "gmf",
          FALSE,
          TRUE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0, 72.0}
         };

#ifdef HAVE_LIBPDF         
Device_entry dev_pdf = {DEVICE_FILE,
          "PDF",
          pdfinitgraphics,
          pdf_op_parser,
          pdf_gui_setup,
          "pdf",
          TRUE,
          FALSE,
          {612, 792, 72.0, 72.0}
         };
#endif

#ifdef HAVE_LIBGD         
Device_entry dev_gd = {DEVICE_FILE,
          "GD",
          gdinitgraphics,
          NULL,
          NULL,
          "gd",
          FALSE,
          TRUE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0, 72.0}
         };
Device_entry dev_gif = {DEVICE_FILE,
          "GIF",
          gifinitgraphics,
          gif_op_parser,
          gif_gui_setup,
          "gif",
          FALSE,
          TRUE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0, 72.0}
         };
Device_entry dev_pnm = {DEVICE_FILE,
          "PNM",
          pnminitgraphics,
          pnm_op_parser,
          pnm_gui_setup,
          "pnm",
          FALSE,
          TRUE,
          {DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT, 72.0, 72.0}
         };
#endif

#if defined(DEBUG)    
    extern int yydebug;
#endif

int main(int argc, char *argv[])
{
    char *s;
    int i, j;
    int gno;
    int fd;
    world w;
    view v;
    int cur_graph = 0;	        /* default graph is graph 0 */
    int loadlegend = 0;		/* legend on and load file names */
    int gcols = 0, grows = 0;
    int gracebat;		/* if executed as 'gracebat' then TRUE */
    int cli = FALSE;            /* command line interface only */
    int remove_flag = FALSE;	/* remove file after read */
    int noprint = FALSE;	/* if gracebat, then don't print if true */
    int sigcatch = TRUE;	/* we handle signals ourselves */

    char fd_name[GR_MAXPATHLEN];

    Page_geometry pg;
    
    /*
     * set version
     */
    reset_project_version();
    /*
     * grace home directory
     */
    if ((s = getenv("GRACE_HOME")) != NULL) {
	set_grace_home(s);
    }
    
    /* define the user's name */
    init_username();
        
    /* set the starting directory */
    set_workingdir(NULL);

    /*
     * print command
     */
    if ((s = getenv("GRACE_PRINT_CMD")) != NULL) {
	set_print_cmd(s);
    }

    /* if no print command defined, print to file by default */
    s = get_print_cmd();
    if (s == NULL || s[0] == '\0') {
        set_ptofile(TRUE);
    } else {
        set_ptofile(FALSE);
    }
    
    /*
     * editor
     */
    if ((s = getenv("GRACE_EDITOR")) != NULL) {
	set_editor(s);
    }
    
    /*
     * check for changed help file viewer command
     */
    if ((s = getenv("GRACE_HELPVIEWER")) != NULL) {
	set_help_viewer(s);
    }

    /* initialize plots, strings, graphs */
    set_program_defaults();

    select_graph(cur_graph);
    
    /* initialize the nonl-fit parameters */
    initialize_nonl();

    /* initialize the parser symbol table */
    init_symtab();
    
    /* initialize the rng */
    srand48(100L);
    
    set_graph_active(cur_graph, TRUE);

    /* initialize T1lib */
    if (init_t1() != GRACE_EXIT_SUCCESS) {
        errmsg("Initialization of t1-library failed!");
	exit (1);
    }

    /* initialize colormap data */
    initialize_cmap();

    /*
     * if program name is gracebat* then don't initialize the X toolkit
     */
    s = mybasename(argv[0]);

    if (strstr(s, "gracebat") == s) {
	gracebat = TRUE;
    } else {
	gracebat = FALSE;
#ifndef NONE_GUI    
        if (initialize_gui(&argc, argv) == GRACE_EXIT_SUCCESS) {
            cli = FALSE;
        } else {
	    errmsg("Switching to CLI");
	    cli = TRUE;
        }
#endif
    }

    /* initialize devices */
#ifndef NONE_GUI
    if (cli == TRUE) {
        tdevice = register_device(dev_dummy);
    } else {
        tdevice = register_device(dev_x11);
    }
#else
    tdevice = register_device(dev_dummy);
#endif
    select_device(tdevice);

    hdevice = register_device(dev_ps);
    register_device(dev_eps);

    register_device(dev_mf);

#ifdef HAVE_LIBPDF
    register_device(dev_pdf);
#endif

#ifdef HAVE_LIBGD
    register_device(dev_gd);
    register_device(dev_gif);
    register_device(dev_pnm);
#endif

    /* check whether locale is correctly set */
    if (set_locale_num(TRUE) == NULL) {
        errmsg("Invalid or unsupported locale");
    }
    /* default is POSIX */
    set_locale_num(FALSE);
    
    /* load startup file */
    getparms("gracerc");

    /* load default template */
    new_project(NULL);

    if (argc >= 2) {
	for (i = 1; i < argc; i++) {
	    if (argv[i][0] == '-' && argv[i][1] != '\0') {
		if (argmatch(argv[i], "-version", 2)) {
                    VersionInfo();
		    exit(0);
		}
#if defined(DEBUG)
		if (argmatch(argv[i], "-debug", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for debug flag\n");
			usage(stderr, argv[0]);
		    } else {
		    	debuglevel = atoi(argv[i]);
		    	if (debuglevel == 4) { 
			    /* turn on debugging in pars.y */
			    yydebug = TRUE;
		    	}
		    }
		} else
#endif
		       if (argmatch(argv[i], "-nosigcatch", 6)) {
		    sigcatch = FALSE;
		} else if (argmatch(argv[i], "-autoscale", 2)) {
		    i++;
		    if (i == argc) {
			errmsg("Missing argument for autoscale flag");
			usage(stderr, argv[0]);
		    } else {
			if (!strcmp("x", argv[i])) {
			    autoscale_onread = AUTOSCALE_X;
			} else if (!strcmp("y", argv[i])) {
			    autoscale_onread = AUTOSCALE_Y;
			} else if (!strcmp("xy", argv[i])) {
			    autoscale_onread = AUTOSCALE_XY;
			} else if (!strcmp("none", argv[i])) {
			    autoscale_onread = AUTOSCALE_NONE;
			} else {
			    errmsg("Improper argument for autoscale flag");
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-batch", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for batch file\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(batchfile, argv[i]);
		    }
		} else if (argmatch(argv[i], "-pipe", 5)) {
		    inpipe = TRUE;
		} else if (argmatch(argv[i], "-noprint", 8)) {
		    noprint = TRUE;
		} else if (argmatch(argv[i], "-logwindow", 5)) {
		    logwindow = TRUE;
		} else if (argmatch(argv[i], "-nologwindow", 7)) {
		    logwindow = FALSE;
		} else if (argmatch(argv[i], "-dpipe", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for descriptor pipe\n");
			usage(stderr, argv[0]);
		    } else {
                        fd = atoi(argv[i]);
                        sprintf(fd_name, "pipe<%d>", fd);
                        if (register_real_time_input(fd, fd_name, FALSE) !=
                            GRACE_EXIT_SUCCESS) {
                            exit(1);
                        }
		    }
		} else if (argmatch(argv[i], "-npipe", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for named pipe\n");
			usage(stderr, argv[0]);
		    } else {
                        fd = open(argv[i], O_RDONLY | O_NONBLOCK);
                        if (fd < 0) {
                            fprintf(stderr, "Can't open fifo\n");
                        } else {
                            register_real_time_input(fd, argv[i], TRUE);
                        }
		    }
#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
		} else if (argmatch(argv[i], "-netcdf", 7) || argmatch(argv[i], "-hdf", 4)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for netcdf file\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(netcdf_name, argv[i]);
			readcdf = 1;
		    }
		} else if (argmatch(argv[i], "-netcdfxy", 9) || argmatch(argv[i], "-hdfxy", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for netcdf X variable name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(xvar_name, argv[i]);
		    }
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for netcdf Y variable name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(yvar_name, argv[i]);
		    }
		    if (strcmp(xvar_name, "null")) {
			readnetcdf(get_cg(), -1, netcdf_name, xvar_name, yvar_name, -1, -1, 1);
		    } else {
			readnetcdf(get_cg(), -1, netcdf_name, NULL, yvar_name, -1, -1, 1);
		    }
#endif				/* HAVE_NETCDF */
		} else if (argmatch(argv[i], "-timer", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for time delay\n");
			usage(stderr, argv[0]);
		    } else {
			timer_delay = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-maxblock", 9)) {
		    int itmp;			
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max size of block data\n");
			usage(stderr, argv[0]);
		    } else {
			itmp = atoi(argv[i]);
			if (itmp < maxblock) {
			    itmp = maxblock;
			}
			alloc_blockdata(itmp);
		    }
#ifndef NONE_GUI
		} else if (argmatch(argv[i], "-install", 7)) {
		    install_cmap = CMAP_INSTALL_ALWAYS;
		} else if (argmatch(argv[i], "-noinstall", 9)) {
		    install_cmap = CMAP_INSTALL_NEVER;
#endif
		} else if (argmatch(argv[i], "-timestamp", 10)) {
		    timestamp.active = TRUE;
		} else if (argmatch(argv[i], "-remove", 7)) {
		    remove_flag = TRUE;
		} else if (argmatch(argv[i], "-fixed", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for fixed canvas width\n");
			usage(stderr, argv[0]);
		    } else {
		        if (i == argc - 1) {
			    fprintf(stderr, "Missing argument for fixed canvas height\n");
			    usage(stderr, argv[0]);
		        } else {
		            pg = get_page_geometry();
                            pg.width = atoi(argv[i]);
		    	    i++;
		    	    pg.height = atoi(argv[i]);
                            set_page_geometry(pg);
#ifndef NONE_GUI
		    	    set_pagelayout(PAGE_FIXED);
#endif
			}
		    }
#ifndef NONE_GUI
		} else if (argmatch(argv[i], "-free", 5)) {
		    set_pagelayout(PAGE_FREE);
#endif
		} else if (argmatch(argv[i], "-noask", 5)) {
		    noask = TRUE;
		} else if (argmatch(argv[i], "-mono", 5)) {
		    monomode = TRUE;
		} else if (argmatch(argv[i], "-hdevice", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for hardcopy device select flag\n");
			usage(stderr, argv[0]);
		    } else {
		        if (set_printer_by_name(argv[i]) != GRACE_EXIT_SUCCESS) {
                            errmsg("Unknown or unsupported device");
                            exit(1);
                        }
                    }
		} else if (argmatch(argv[i], "-log", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for log plots flag\n");
			usage(stderr, argv[0]);
		    }
		    if (!strcmp("x", argv[i])) {
			set_graph_xscale(cur_graph, SCALE_LOG);
		    } else if (!strcmp("y", argv[i])) {
			set_graph_yscale(cur_graph, SCALE_LOG);
		    } else if (!strcmp("xy", argv[i])) {
			set_graph_xscale(cur_graph, SCALE_LOG);
			set_graph_yscale(cur_graph, SCALE_LOG);
		    } else {
			fprintf(stderr, "%s: Improper argument for -l flag; should be one of 'x', 'y', 'xy'\n", argv[0]);
		    }
		} else if (argmatch(argv[i], "-printfile", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing file name for printing\n");
			usage(stderr, argv[0]);
		    } else {
			set_ptofile(TRUE);
                        strcpy(print_file, argv[i]);
		    }
		} else if (argmatch(argv[i], "-hardcopy", 6)) {
		    gracebat = TRUE;
		} else if (argmatch(argv[i], "-pexec", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for exec\n");
			usage(stderr, argv[0]);
		    } else {
			read_param(argv[i]);
		    }
		} else if (argmatch(argv[i], "-graph", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter for graph select\n");
			usage(stderr, argv[0]);
		    } else {
			sscanf(argv[i], "%d", &gno);
			if (set_graph_active(gno, TRUE) == GRACE_EXIT_SUCCESS) {
			    cur_graph = gno;
                            select_graph(gno);
			} else {
			    fprintf(stderr, "Error activating graph %d\n", gno);
			}
		    }
		} else if (argmatch(argv[i], "-block", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter for block data\n");
			usage(stderr, argv[0]);
		    } else {
			getdata(cur_graph, argv[i], cursource, SET_BLOCK);
		    }
		} else if (argmatch(argv[i], "-bxy", 4)) {
		    char blocksetcols[32];
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter for block data set creation\n");
		    }
		    strcpy(blocksetcols, argv[i]);
		    create_set_fromblock(get_cg(), curtype, blocksetcols);
		} else if (argmatch(argv[i], "-nxy", 4)) {
		    curtype = SET_NXY;
		} else if (argmatch(argv[i], "-type", 2) ||
                           argmatch(argv[i], "-settype", 8)) {
		    /* set types */
		    i++;
                    curtype = get_settype_by_name(argv[i]);
                    if (curtype == -1) {
			fprintf(stderr, "%s: Unknown set type '%s'\n", argv[0], argv[i]);
			usage(stderr, argv[0]);
		    }
		} else if (argmatch(argv[i], "-graphtype", 7)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph type\n");
		    } else {
			if (!strcmp("xy", argv[i])) {
			    set_graph_type(cur_graph, GRAPH_XY);
			} else if (!strcmp("polar", argv[i])) {
			    set_graph_type(cur_graph, GRAPH_POLAR);
			} else if (!strcmp("bar", argv[i])) {
			    set_graph_type(cur_graph, GRAPH_CHART);
			} else if (!strcmp("smith", argv[i])) {
			    set_graph_type(cur_graph, GRAPH_SMITH);
			} else if (!strcmp("fixed", argv[i])) {
			    set_graph_type(cur_graph, GRAPH_FIXED);
			} else {
			    fprintf(stderr, "%s: Improper argument for -graphtype\n", argv[0]);
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-arrange", 7)) {
		    if (i + 2 >= argc) {
			fprintf(stderr, "Missing argument for graph arrangement\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
			grows = atoi(argv[i]);
			i++;
			gcols = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-cols", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph column arrangement\n");
			usage(stderr, argv[0]);
		    } else {
			gcols = atoi(argv[i]);
                        if (grows == 0) {
                            grows = 1;
                        }
		    }
		} else if (argmatch(argv[i], "-rows", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph row arrangement\n");
			usage(stderr, argv[0]);
		    } else {
			grows = atoi(argv[i]);
                        if (gcols == 0) {
                            gcols = 1;
                        }
		    }
		} else if (argmatch(argv[i], "-legend", 4)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for -legend\n");
			usage(stderr, argv[0]);
		    } else {
			if (!strcmp("load", argv[i])) {
			    loadlegend = TRUE;
			    set_graph_legend_active(cur_graph, TRUE);
			} else {
			    fprintf(stderr, "Improper argument for -legend\n");
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-rvideo", 7)) {
		    reverse_video();
		} else if (argmatch(argv[i], "-param", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter file name\n");
			usage(stderr, argv[0]);
		    } else {
			if (!getparms(argv[i])) {
			    fprintf(stderr, "Unable to read parameter file %s\n", argv[i]);
			}
		    }
		} else if (argmatch(argv[i], "-results", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing results file name\n");
			usage(stderr, argv[0]);
		    } else {
                        /*  open resfile if -results option given */
		        if ((resfp = grace_openw(argv[i])) == NULL) {
		            exit(1);
		        }
		    }
		} else if (argmatch(argv[i], "-saveall", 8)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing save file name\n");
			usage(stderr, argv[0]);
		    } else {
			save_project(s);
		    }
		} else if (argmatch(argv[i], "-wd", 3)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameters for working directory\n");
			usage(stderr, argv[0]);
		    } else {
			if (set_workingdir(argv[i]) != GRACE_EXIT_SUCCESS) {
			    fprintf(stderr, "Can't change to directory %s, fatal error", argv[i]);
			    exit(1);
			}
		    }
		} else if (argmatch(argv[i], "-source", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for data source parameter\n");
			usage(stderr, argv[0]);
		    }
		    if (argmatch(argv[i], "pipe", 4)) {
			cursource = SOURCE_PIPE;
		    } else if (argmatch(argv[i], "disk", 4)) {
			cursource = SOURCE_DISK;
		    }
		} else if (argmatch(argv[i], "-viewport", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameters for viewport setting\n");
			usage(stderr, argv[0]);
		    } else {
			v.xv1 = atof(argv[i++]);
			v.yv1 = atof(argv[i++]);
			v.xv2 = atof(argv[i++]);
			v.yv2 = atof(argv[i]);
			set_graph_viewport(cur_graph, v);
		    }
		} else if (argmatch(argv[i], "-world", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameters for world setting\n");
			usage(stderr, argv[0]);
		    } else {
			w.xg1 = atof(argv[i++]);
			w.yg1 = atof(argv[i++]);
			w.xg2 = atof(argv[i++]);
			w.yg2 = atof(argv[i]);
                        set_graph_world(cur_graph, w);
		    }
		} else if (argmatch(argv[i], "-seed", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing seed for srand48()\n");
			usage(stderr, argv[0]);
		    } else {
			srand48(atol(argv[i]));	/* note atol() */
		    }
		} else if (argmatch(argv[i], "-help", 2)) {
		    usage(stdout, argv[0]);
		} else if (argmatch(argv[i], "-usage", 5)) {
		    usage(stdout, argv[0]);
		} else {
		    fprintf(stderr, "No option %s\n", argv[i]);
		    usage(stderr, argv[0]);
		}
	    } else {
		if (i != argc) {
		    lock_dirtystate(TRUE);
		    if (getdata(cur_graph, argv[i], cursource, curtype) ==
                                                        GRACE_EXIT_SUCCESS) {
			strcpy(docname, argv[i]);
			if (remove_flag) {
			    unlink(argv[i]);
			}
			clear_dirtystate();
		    } else {
		        lock_dirtystate(FALSE);
			set_dirtystate();
		    }
		}		/* end if */
	    }			/* end else */
	}			/* end for */
    }				/* end if */

    
    /*
     * Process events.
     */
    if (sigcatch == TRUE) {
        installSignal();
    }

/*
 * load legend
 */
    if (loadlegend) {
	for (i = 0; i < number_of_graphs(); i++) {
	    if (is_graph_active(i) && (activeset(i))) {
		for (j = 0; j < number_of_sets(i); j++) {
		    if (is_set_active(i, j)) {
			load_comments_to_legend(i, j);
		    }
		}
	    }
	}
    }

/*
 * arrange graphs
 */
    if (grows > 0 || gcols > 0) {
        arrange_graphs(grows, gcols);
    }

/*
 * try to switch to the first graph
 */
    select_graph(0);  

/*
 * if -hardcopy on command line or executed as gracebat,
 * just plot the graph and quit
 */
    if (gracebat == TRUE) {
	if (hdevice == 0) {
	    errmsg("Terminal device can't be used for batch plotting");
	    exit(1);
	}
	if (inpipe == TRUE) {
	    getdata(get_cg(), "stdin", SOURCE_DISK, curtype);
	    inpipe = FALSE;
	}
	if (batchfile[0]) {
	    getparms(batchfile);
	}
        while (real_time_under_monitoring()) {
            monitor_input(ib_tbl, ib_tblsize, 0);
        }
	if (!noprint) {
	    do_hardcopy();
	}
        
	bailout();
    } else {
/*
 * go main loop
 */
#ifndef NONE_GUI
        if (cli == TRUE) {
            cli_loop();
        } else {
            startup_gui();
        }
#else
        cli_loop();
#endif        
    }
    /* never reaches */
    exit(0);
}

/*
 * command interface loop
 */
void cli_loop(void)
{
    char pstring[MAX_STRING_LENGTH] = "";
    int ilen;
    int i = 1;

    if (inpipe == TRUE) {
        getdata(get_cg(), "stdin", SOURCE_DISK, curtype);
        inpipe = FALSE;
    }
    if (batchfile[0]) {
        getparms(batchfile);
    }
    /* TODO: RTI */
    
    while (TRUE) {
        printf("grace:%d> ", i);
        fgets(pstring, MAX_STRING_LENGTH - 1, stdin);
        ilen = strlen(pstring);
        if (ilen < 2) {
            continue;
        }
        read_param(pstring);
        i++;
    }
}

static void usage(FILE *stream, char *progname)
{
/* We use alphabetial order */

    fprintf(stream, "Usage of %s command line arguments: \n", progname);

    fprintf(stream, "-arrange   [rows] [cols]              Arrange the graphs in a grid rows by cols\n");
    fprintf(stream, "-autoscale [x|y|xy|none]              Set autoscale type\n");
    fprintf(stream, "-batch     [batch_file]               Execute batch_file on start up\n");
    fprintf(stream, "-block     [block_data]               Assume data file is block data\n");
    fprintf(stream, "-bxy       [x:y:etc.]                 Form a set from the current block data set\n");
    fprintf(stream, "                                        using the current set type from columns\n");
    fprintf(stream, "                                        given in the argument\n");
    fprintf(stream, "-cols      [gcols]                    Arrange graphs in gcols columns\n");
#if defined(DEBUG)
    fprintf(stream, "-debug     [debug_level]              Set debugging options\n");
#endif
    fprintf(stream, "-dpipe     [descriptor]               Read data from descriptor on startup\n");
    fprintf(stream, "-hdevice   [hardcopy_device_name]     Set default hardcopy device\n");
    fprintf(stream, "-fixed     [width] [height]           Set canvas size fixed to width*height\n");
#ifndef NONE_GUI
    fprintf(stream, "-free                                 Use free page layout\n");
#endif
    fprintf(stream, "-graph     [graph_number]             Set the current graph number\n");
    fprintf(stream, "-graphtype [graph_type]               Set the type of the current graph\n");
    fprintf(stream, "-hardcopy                             No interactive session, just print and\n");
    fprintf(stream, "                                        quit\n");
    fprintf(stream, "-install                              Install private colormap\n");
    fprintf(stream, "-legend    [load]                     Turn the graph legend on\n");
    fprintf(stream, "-log       [x|y|xy]                   Set the axis scaling of the current graph\n");
    fprintf(stream, "                                        to logarithmic\n");
    fprintf(stream, "-logwindow                            Open the log window\n");
    fprintf(stream, "-maxblock  [number_of_columns]        Set the number of columns for block data\n");
    fprintf(stream, "                                        (default is %d)\n", MAXBLOCK);
    fprintf(stream, "-mono                                 Run %s in monochrome mode (affects\n", progname);
    fprintf(stream, "                                        the display only)\n");
#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
    fprintf(stream, "-netcdf    [netcdf file]              Assume data file is bnetCDF format\n");
    fprintf(stream, "-netcdfxy  [X var name] [Y var name]  If -netcdf was used previously, read from\n");
    fprintf(stream, "                                        the netCDF file, 'X var name' and 'Y\n");
    fprintf(stream, "                                        var name' and create a set. If 'X var\n");
    fprintf(stream, "                                        name' equals \"null\" then load the\n");
    fprintf(stream, "                                        index of Y to X\n");
#endif
    fprintf(stream, "-noask                                Assume the answer is yes to all requests -\n");
    fprintf(stream, "                                        if the operation would overwrite a file,\n");
    fprintf(stream, "                                        grace will do so without prompting\n");
    fprintf(stream, "-noinstall                            Don't use private colormap\n");
    fprintf(stream, "-nologwindow                          No log window, overrides resource setting\n");
    fprintf(stream, "-noprint                              In batch mode, do not print\n");
    fprintf(stream, "-nosigcatch                           Don't catch signals\n");
    fprintf(stream, "-npipe     [file]                     Read data from named pipe on startup\n");
    fprintf(stream, "-nxy       [nxy_file]                 Assume data file is in X Y1 Y2 Y3 ...\n");
    fprintf(stream, "                                        format\n");
    fprintf(stream, "-param     [parameter_file]           Load parameters from parameter_file to the\n");
    fprintf(stream, "                                        current graph\n");
    fprintf(stream, "-pexec     [parameter_string]         Interpret string as a parameter setting\n");
    fprintf(stream, "-pipe                                 Read data from stdin on startup\n");
    fprintf(stream, "-printfile [file for hardcopy output] Save print output to file \n");
    fprintf(stream, "-remove                               Remove data file after read\n");
    fprintf(stream, "-results   [results_file]             Write the results from regression to\n");
    fprintf(stream, "                                        results_file\n");
    fprintf(stream, "-rows      [grows]                    Arrange graphs in grows rows \n");
    fprintf(stream, "-rvideo                               Exchange the color indices for black and\n");
    fprintf(stream, "                                        white\n");
    fprintf(stream, "-saveall   [save_file]                Save all to save_file\n");
    fprintf(stream, "-seed      [seed_value]               Integer seed for random number generator\n");
    fprintf(stream, "-source    [disk|pipe]                Source type of next data file\n");
    fprintf(stream, "-timer     [delay]                    Set allowed time slice for real time inputs to delay ms\n");
    fprintf(stream, "-timestamp                            Add timestamp to plot\n");
    fprintf(stream, "-settype   [xy|xydx|...]              Set the type of the next data file\n");
    fprintf(stream, "-version                              Show the program version\n");
    fprintf(stream, "-viewport  [xmin ymin xmax ymax]      Set the viewport for the current graph\n");
    fprintf(stream, "-wd        [directory]                Set the working directory\n");
    fprintf(stream, "-world     [xmin ymin xmax ymax]      Set the world coordinates for the\n");
    fprintf(stream, "                                        current graph\n");

    fprintf(stream, "-usage|-help                          This message\n");
    fprintf(stream, "\n");
    fprintf(stream, " ** If it scrolls too fast, run `%s -help | more\' **\n", progname);
    exit(0);
}

void VersionInfo(void)
{
    fprintf(stdout, "%s\n", bi_version_string());

/* We don't want to reproduce the complete config.h,
   but those settings which may be related to problems on runtime */

    fprintf(stdout, "GUI toolkit: %s\n", BI_GUI);
    fprintf(stdout, "T1lib: %s\n", BI_T1LIB);

#ifdef DEBUG
    fprintf(stdout, "Debugging is enabled\n");
#endif
    fprintf(stdout, "Built %s on %s\n", BI_DATE, BI_SYSTEM);
    fprintf(stdout, "Compiler flags: %s\n", BI_CCOMPILER);
 
    fprintf(stdout, "\n");
    fprintf(stdout, "(C) Copyright 1991-1995 Paul J Turner\n");
    fprintf(stdout, "(C) Copyright 1996-1999 Grace Development Team\n");
    fprintf(stdout, "All Rights Reserved\n");

    return;
}
