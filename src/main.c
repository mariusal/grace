/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
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
#include "ssdata.h"

#include "dicts.h"
#include "plotone.h"

#include "devlist.h"
#include "parser.h"
#include "protos.h"


extern Input_buffer *ib_tbl;
extern int ib_tblsize;

static void usage(FILE *stream, char *progname);
static void VersionInfo(const Grace *grace);
static void cli_loop(Grace *grace);

#if defined(DEBUG)    
extern int yydebug;
#endif

int main(int argc, char *argv[])
{
    char *s;
    int i;
    int fd;
    Quark *cur_graph;	        /* default (current) graph */
    int gracebat;		/* if executed as 'gracebat' then TRUE */
    int cli = FALSE;            /* command line interface only */
    int noprint = FALSE;	/* if gracebat, then don't print if true */
    int sigcatch = TRUE;	/* we handle signals ourselves */

    char fd_name[GR_MAXPATHLEN];

    int wpp, hpp;
    
    RunTime *rt;
    GUI *gui;
    Canvas *canvas;
    
    grace = grace_new();
    if (!grace) {
        errmsg("Failed to allocate run-time structures");
        exit(1);
    }
    
    rt      = grace->rt;
    canvas  = rt->canvas;
    gui     = grace->gui;
    
    if (init_font_db(canvas)) {
        errmsg("Broken or incomplete installation - read the FAQ!");
        exit(1);
    }
    
    /* initialize the parser symbol table */
    init_symtab();
    
    /* initialize the rng */
    srand48(100L);
    
    /*
     * if program name is gracebat* then don't initialize the X toolkit
     */
    s = mybasename(argv[0]);

    if (strstr(s, "gracebat") == s) {
	gracebat = TRUE;
    } else {
	gracebat = FALSE;
        if (strstr(s, "grace") == s) {
            cli = TRUE;
        } else {
#ifndef NONE_GUI    
            cli = FALSE;
            if (initialize_gui(&argc, argv) != RETURN_SUCCESS) {
	        errmsg("Failed initializing GUI, exiting");
                exit(1);
            }
#endif
        }
    }

    /* initialize devices */
#ifndef NONE_GUI
    if (cli == TRUE || gracebat == TRUE) {
        rt->tdevice = register_dummy_drv(canvas);
    } else {
        rt->tdevice = register_x11_drv(canvas);
    }
#else
    rt->tdevice = register_dummy_drv(canvas);
#endif

    rt->hdevice = register_ps_drv(canvas);
    register_eps_drv(canvas);

#ifdef HAVE_LIBPDF
    register_pdf_drv(canvas);
#endif
    register_mif_drv(canvas);
    register_svg_drv(canvas);

#ifdef HAVE_LIBXMI
    register_pnm_drv(canvas);
#  ifdef HAVE_LIBJPEG
    register_jpg_drv(canvas);
#  endif
#  ifdef HAVE_LIBPNG
    register_png_drv(canvas);
#  endif
#endif

    register_mf_drv(canvas);

    /* check whether locale is correctly set */
    if (init_locale() != RETURN_SUCCESS) {
        errmsg("Invalid or unsupported locale");
    }
    /* default is POSIX */
    set_locale_num(FALSE);
    
    /* load startup file */
    getparms(grace, "gracerc");

    /* load default template */
    new_project(grace, NULL);
    
    cur_graph = graph_get_current(grace->project);

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-' && argv[i][1] != '\0') {
	    if (argmatch(argv[i], "-version", 2)) {
                VersionInfo(grace);
		exit(0);
	    }
#if defined(DEBUG)
	    if (argmatch(argv[i], "-debug", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for debug flag\n");
		    usage(stderr, argv[0]);
		} else {
		    set_debuglevel(grace, atoi(argv[i]));
		    if (get_debuglevel(grace) == 4) { 
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
			rt->autoscale_onread = AUTOSCALE_X;
		    } else if (!strcmp("y", argv[i])) {
			rt->autoscale_onread = AUTOSCALE_Y;
		    } else if (!strcmp("xy", argv[i])) {
			rt->autoscale_onread = AUTOSCALE_XY;
		    } else if (!strcmp("none", argv[i])) {
			rt->autoscale_onread = AUTOSCALE_NONE;
		    } else {
			errmsg("Improper argument for autoscale flag");
			usage(stderr, argv[0]);
		    }
		}
	    } else if (argmatch(argv[i], "-datehint", 5)) {
		i++;
		if (i == argc) {
		    errmsg("Missing argument for datehint flag");
		    usage(stderr, argv[0]);
		} else {
		    if (!strcmp("iso", argv[i])) {
			set_date_hint(FMT_iso);
		    } else if (!strcmp("european", argv[i])) {
			set_date_hint(FMT_european);
		    } else if (!strcmp("us", argv[i])) {
			set_date_hint(FMT_us);
		    } else if (!strcmp("nohint", argv[i])) {
			set_date_hint(FMT_nohint);
		    } else {
			errmsg("Improper argument for datehint flag");
			usage(stderr, argv[0]);
		    }
		}
	    } else if (argmatch(argv[i], "-noprint", 8)) {
		noprint = TRUE;
	    } else if (argmatch(argv[i], "-dpipe", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for descriptor pipe\n");
		    usage(stderr, argv[0]);
		} else {
                    fd = atoi(argv[i]);
                    sprintf(fd_name, "pipe<%d>", fd);
                    if (register_real_time_input(grace, fd, fd_name, FALSE) !=
                        RETURN_SUCCESS) {
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
                        if (register_real_time_input(grace, fd, argv[i], TRUE) !=
                            RETURN_SUCCESS) {
                            exit(1);
                        }
                    }
		}
	    } else if (argmatch(argv[i], "-timer", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for time delay\n");
		    usage(stderr, argv[0]);
		} else {
		    rt->timer_delay = atoi(argv[i]);
		}
#ifndef NONE_GUI
	    } else if (argmatch(argv[i], "-install", 7)) {
		gui->install_cmap = CMAP_INSTALL_ALWAYS;
	    } else if (argmatch(argv[i], "-noinstall", 9)) {
		gui->install_cmap = CMAP_INSTALL_NEVER;
	    } else if (argmatch(argv[i], "-barebones", 9)) {
		gui_set_barebones(gui);
#endif
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
                        wpp = atoi(argv[i]);
		    	i++;
		    	hpp = atoi(argv[i]);
                        set_page_dimensions(grace, wpp, hpp, FALSE);
#ifndef NONE_GUI
		    	gui_set_page_free(gui, FALSE);
#endif
		    }
		}
#ifndef NONE_GUI
	    } else if (argmatch(argv[i], "-free", 5)) {
		gui_set_page_free(gui, TRUE);
#endif
	    } else if (argmatch(argv[i], "-noask", 5)) {
		gui->noask = TRUE;
	    } else if (argmatch(argv[i], "-hdevice", 5)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for hardcopy device select flag\n");
		    usage(stderr, argv[0]);
		} else {
		    if (set_printer_by_name(grace, argv[i]) != RETURN_SUCCESS) {
                        errmsg("Unknown or unsupported device");
                        exit(1);
                    }
                }
	    } else if (argmatch(argv[i], "-hdevice-options", 12)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for hardcopy device options\n");
		    usage(stderr, argv[0]);
		} else {
		    if (parse_device_options(rt->canvas,
                        rt->hdevice, argv[i]) != RETURN_SUCCESS) {
                        errmsg("Failed parsing device options");
                        exit(1);
                    }
                }
	    } else if (argmatch(argv[i], "-printfile", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing file name for printing\n");
		    usage(stderr, argv[0]);
		} else {
		    set_ptofile(grace, TRUE);
                    strcpy(rt->print_file, argv[i]);
		}
	    } else if (argmatch(argv[i], "-hardcopy", 6)) {
		gracebat = TRUE;
	    } else if (argmatch(argv[i], "-pexec", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for exec\n");
		    usage(stderr, argv[0]);
		} else {
		    scanner(argv[i]);
		}
	    } else if (argmatch(argv[i], "-block", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing filename for block data\n");
		    usage(stderr, argv[0]);
		} else {
		    getdata(grace->project, argv[i], rt->cursource, LOAD_BLOCK);
		}
	    } else if (argmatch(argv[i], "-bxy", 4)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing parameter for block data set creation\n");
		    usage(stderr, argv[0]);
		} else {
                    int nc, *cols, scol;
                    if (field_string_to_cols(argv[i], &nc, &cols, &scol) !=
                        RETURN_SUCCESS) {
                        errmsg("Erroneous field specifications");
                        return 1;
                    }
		    create_set_fromblock(grace_set_new(cur_graph),
                        rt->curtype, nc, cols, scol, rt->autoscale_onread);
                    xfree(cols);
                }
	    } else if (argmatch(argv[i], "-nxy", 4)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing filename for nxy data\n");
		    usage(stderr, argv[0]);
		} else {
		    getdata(grace->project, argv[i], rt->cursource, LOAD_NXY);
		}
	    } else if (argmatch(argv[i], "-type", 2) ||
                       argmatch(argv[i], "-settype", 8)) {
		/* set types */
		i++;
                rt->curtype = get_settype_by_name(rt, argv[i]);
                if (rt->curtype == -1) {
		    fprintf(stderr, "%s: Unknown set type '%s'\n", argv[0], argv[i]);
		    usage(stderr, argv[0]);
		}
	    } else if (argmatch(argv[i], "-param", 2)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing parameter file name\n");
		    usage(stderr, argv[0]);
		} else {
		    if (!getparms(grace, argv[i])) {
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
		    if ((rt->resfp = grace_openw(grace, argv[i])) == NULL) {
		        exit(1);
		    }
		    setvbuf(rt->resfp, NULL, _IOLBF, 0);
		}
	    } else if (argmatch(argv[i], "-saveall", 8)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing save file name\n");
		    usage(stderr, argv[0]);
		} else {
		    save_project(grace->project, argv[i]);
		}
	    } else if (argmatch(argv[i], "-wd", 3)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing parameters for working directory\n");
		    usage(stderr, argv[0]);
		} else {
		    if (set_workingdir(grace, argv[i]) != RETURN_SUCCESS) {
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
		    rt->cursource = SOURCE_PIPE;
		} else if (argmatch(argv[i], "disk", 4)) {
		    rt->cursource = SOURCE_DISK;
		}
	    } else if (argmatch(argv[i], "-seed", 5)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing seed for srand48()\n");
		    usage(stderr, argv[0]);
		} else {
		    srand48(atol(argv[i]));	/* note atol() */
		}
            } else if (argmatch(argv[i], "-safe", 5)) {
                grace->rt->safe_mode = TRUE;
            } else if (argmatch(argv[i], "-nosafe", 7)) {
                grace->rt->safe_mode = FALSE;
	    } else if (argmatch(argv[i], "-help", 2)) {
		usage(stdout, argv[0]);
	    } else if (argmatch(argv[i], "-usage", 5)) {
		usage(stdout, argv[0]);
	    } else {
		fprintf(stderr, "No option %s\n", argv[i]);
		usage(stderr, argv[0]);
	    }
	} else {
	    if (strstr(argv[i], ".xgr") || strstr(argv[i], ".agr")) {
                load_project(grace, argv[i]);
            } else {
                getdata(grace->project, argv[i], rt->cursource, LOAD_SINGLE);
            }
	} /* end else */
    } /* end for */
    
    /*
     * Process events.
     */
    if (sigcatch == TRUE) {
        installSignal();
    }

/*
 * if -hardcopy on command line or executed as gracebat,
 * just plot the graph and quit
 */
    if (gracebat == TRUE) {
	if (rt->hdevice == 0) {
	    errmsg("Terminal device can't be used for batch plotting");
	    exit(1);
	}
        while (real_time_under_monitoring()) {
            monitor_input(grace, ib_tbl, ib_tblsize, 0);
        }
	if (!noprint) {
	    do_hardcopy(grace->project);
	}
        
	bailout(grace);
    } else {
/*
 * go main loop
 */
#ifndef NONE_GUI
        if (cli == TRUE) {
            cli_loop(grace);
        } else {
            startup_gui(grace);
        }
#else
        cli_loop(grace);
#endif        
    }
    /* never reaches */
    exit(0);
}

/*
 * command interface loop
 */
static void cli_loop(Grace *grace)
{
    Input_buffer *ib_stdin;
    int previous = -1;

    if (register_real_time_input(grace, STDIN_FILENO, "stdin", 0)
        != RETURN_SUCCESS) {
        exit(1);
    }
    for (ib_stdin = ib_tbl; ib_stdin->fd != STDIN_FILENO; ib_stdin++) {
        ;
    }

    while (ib_stdin->fd == STDIN_FILENO) {
        /* the standard input is still under monitoring */
        if (ib_stdin->lineno != previous) {
            printf("grace:%d> ", ib_stdin->lineno + 1);
            fflush(stdout);
            previous = ib_stdin->lineno;
        }
        monitor_input(grace, ib_tbl, ib_tblsize, 0);
    }

}

static void usage(FILE *stream, char *progname)
{
    /* We use alphabetical order */

    fprintf(stream, "Usage of %s command line arguments: \n", progname);

    fprintf(stream, "-autoscale [x|y|xy|none]              Set autoscale type\n");
#ifndef NONE_GUI
    fprintf(stream, "-barebones                            Turn off all toolbars\n");
#endif
    fprintf(stream, "-block     [block_data]               Assume data file is block data\n");
    fprintf(stream, "-bxy       [x:y:etc.]                 Form a set from the current block data set\n");
    fprintf(stream, "                                        using the current set type from columns\n");
    fprintf(stream, "                                        given in the argument\n");
    fprintf(stream, "-datehint  [iso|european|us\n");
    fprintf(stream, "            |days|seconds|nohint]     Set the hint for dates analysis\n");
    fprintf(stream, "                                        (it is only a hint for the parser)\n");
#if defined(DEBUG)
    fprintf(stream, "-debug     [debug_level]              Set debugging options\n");
#endif
    fprintf(stream, "-dpipe     [descriptor]               Read data from descriptor on startup\n");
    fprintf(stream, "-fixed     [width] [height]           Set canvas size fixed to width*height\n");
#ifndef NONE_GUI
    fprintf(stream, "-free                                 Use free page layout\n");
#endif
    fprintf(stream, "-hardcopy                             No interactive session, just print and\n");
    fprintf(stream, "                                        quit\n");
    fprintf(stream, "-hdevice   [hardcopy_device_name]     Set default hardcopy device\n");
    fprintf(stream, "-hdevice-options [option_string]      Set options for default hardcopy device\n");
#ifndef NONE_GUI
    fprintf(stream, "-install                              Install private colormap\n");
#endif
    fprintf(stream, "-noask                                Assume the answer is yes to all requests -\n");
    fprintf(stream, "                                        if the operation would overwrite a file,\n");
    fprintf(stream, "                                        grace will do so without prompting\n");
#ifndef NONE_GUI
    fprintf(stream, "-noinstall                            Don't use private colormap\n");
#endif
    fprintf(stream, "-noprint                              In batch mode, do not print\n");
    fprintf(stream, "-nosafe                               Disable safe mode\n");
    fprintf(stream, "-nosigcatch                           Don't catch signals\n");
    fprintf(stream, "-npipe     [file]                     Read data from named pipe on startup\n");
    fprintf(stream, "-nxy       [nxy_file]                 Assume data file is in X Y1 Y2 Y3 ...\n");
    fprintf(stream, "                                        format\n");
    fprintf(stream, "-param     [parameter_file]           Load parameters from parameter_file to the\n");
    fprintf(stream, "                                        current graph\n");
    fprintf(stream, "-pexec     [parameter_string]         Interpret string as a parameter setting\n");
    fprintf(stream, "-printfile [file for hardcopy output] Save print output to file \n");
    fprintf(stream, "-results   [results_file]             Write results of some data manipulations\n");
    fprintf(stream, "                                        to results_file\n");
    fprintf(stream, "-safe                                 Safe mode (default)\n");
    fprintf(stream, "-saveall   [save_file]                Save all to save_file\n");
    fprintf(stream, "-seed      [seed_value]               Integer seed for random number generator\n");
    fprintf(stream, "-source    [disk|pipe]                Source type of next data file\n");
    fprintf(stream, "-timer     [delay]                    Set allowed time slice for real time\n");
    fprintf(stream, "                                        inputs to delay ms\n");
    fprintf(stream, "-settype   [xy|xydx|...]              Set the type of the next data file\n");
    fprintf(stream, "-version                              Show the program version\n");
    fprintf(stream, "-wd        [directory]                Set the working directory\n");

    fprintf(stream, "-usage|-help                          This message\n");
    fprintf(stream, "\n");
    fprintf(stream, " ** If it scrolls too fast, run `%s -help | more\' **\n", progname);
    exit(0);
}

static void VersionInfo(const Grace *grace)
{
    int i;
    
    fprintf(stdout, "\n%s\n\n", bi_version_string());

/* We don't want to reproduce the complete config.h,
   but those settings which may be related to problems on runtime */

    fprintf(stdout, "GUI toolkit: %s\n", bi_gui());
#ifdef MOTIF_GUI
    fprintf(stdout, "Xbae version: %s\n", bi_gui_xbae());
#endif
    fprintf(stdout, "T1lib: %s\n", bi_t1lib());
#ifdef HAVE_FFTW
    fprintf(stdout, "FFT: FFTW\n");
#else
    fprintf(stdout, "FFT: built-in\n");
#endif
#ifdef HAVE_NETCDF
    fprintf(stdout, "NetCDF support: on\n");
#else
    fprintf(stdout, "NetCDF support: off\n");
#endif

#ifdef DEBUG
    fprintf(stdout, "Debugging: enabled\n");
#endif
    fprintf(stdout, "Built: %s on %s\n", bi_date(), bi_system());
    fprintf(stdout, "Compiler flags: %s\n", bi_ccompiler());
 
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Registered devices:\n");
    for (i = 0; i < number_of_devices(grace->rt->canvas); i++) {
        fprintf(stdout, "%s ", get_device_name(grace->rt->canvas, i));
    }
    fprintf(stdout, "\n\n");
    
    fprintf(stdout, "(C) Copyright 1991-1995 Paul J Turner\n");
    fprintf(stdout, "(C) Copyright 1996-2004 Grace Development Team\n");
    fprintf(stdout, "All Rights Reserved\n");

    return;
}
