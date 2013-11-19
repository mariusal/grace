/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2007 Grace Development Team
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

#include "core_utils.h"
#include "utils.h"
#include "files.h"
#include "ssdata.h"

#include "xprotos.h"


extern Input_buffer *ib_tbl;
extern int ib_tblsize;

static void usage(FILE *stream, char *progname);
static void VersionInfo(const GraceApp *gapp);
static void cli_loop(GraceApp *gapp);

int main(int argc, char *argv[])
{
    char *s;
    int i;
    int fd;
    int gracebat;		/* if executed as 'gracebat' then TRUE */
    int cli = FALSE;            /* command line interface only */
    int noprint = FALSE;	/* if gracebat, then don't print if true */
    int sigcatch = TRUE;	/* we handle signals ourselves */
    int nogui = FALSE;
    
    int curtype = SET_XY;

    char fd_name[GR_MAXPATHLEN];

    Grace *grace;
    RunTime *rt;
    GUI *gui;
    Canvas *canvas;
    
    int device_id;
    
    gapp = gapp_new();
    if (!gapp) {
        errmsg("Failed to allocate run-time structures");
        exit(1);
    }
    
    grace   = gapp->grace;
    rt      = gapp->rt;
    canvas  = grace_get_canvas(grace);
    gui     = gapp->gui;
    
    /* check whether locale is correctly set */
    if (init_locale() != RETURN_SUCCESS) {
        errmsg("Invalid or unsupported locale");
    }
    /* default is POSIX */
    set_locale_num(FALSE);
    
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

#ifndef NONE_GUI
    if (cli == TRUE || gracebat == TRUE) {
        nogui = TRUE;
    }
#else
    nogui = TRUE;
#endif

    /* initialize devices */
#ifndef NONE_GUI
    if (nogui) {
        rt->tdevice = register_dummy_drv(canvas);
    } else {
        Device_entry *d;
        int pixel_size;

        rt->tdevice = register_x11_drv(canvas);
        
        d = get_device_props(canvas, rt->tdevice);
        pixel_size = x11_get_pixelsize(gui);
        
        switch (pixel_size) {
        case 0:
            /* disable font AA in mono mode */
            d->fontrast = FONT_RASTER_MONO;
            break;
        case 1:
            /* low AA in pseudocolor mode */
            d->fontrast = FONT_RASTER_AA_LOW;
            break;
        default:
            d->fontrast = FONT_RASTER_AA_SMART;
            break;
        }
    
    }
#else
    rt->tdevice = register_dummy_drv(canvas);
#endif

    rt->hdevice = register_ps_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_ps_drv_setup(canvas, rt->hdevice);
    }
#endif
    device_id = register_eps_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_eps_drv_setup(canvas, device_id);
    }
#endif

#ifdef HAVE_LIBPDF
    device_id = register_pdf_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_pdf_drv_setup(canvas, device_id);
    }
#endif
#endif
#ifdef HAVE_HARU
    device_id = register_hpdf_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_hpdf_drv_setup(canvas, device_id);
    }
#endif
#endif
    register_mif_drv(canvas);
    register_svg_drv(canvas);

#ifdef HAVE_LIBXMI
    device_id = register_pnm_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_pnm_drv_setup(canvas, device_id);
    }
#endif
#  ifdef HAVE_LIBJPEG
    device_id = register_jpg_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_jpg_drv_setup(canvas, device_id);
    }
#endif
#  endif
#  ifdef HAVE_LIBPNG
    device_id = register_png_drv(canvas);
#ifndef NONE_GUI
    if (!nogui) {
        attach_png_drv_setup(canvas, device_id);
    }
#endif
#  endif
#endif

    register_mf_drv(canvas);

    /* TODO: load prefs */

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-' && argv[i][1] != '\0') {
	    if (argmatch(argv[i], "-version", 2)) {
                VersionInfo(gapp);
		exit(0);
	    }
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
			set_date_hint(gapp, FMT_iso);
		    } else if (!strcmp("european", argv[i])) {
			set_date_hint(gapp, FMT_european);
		    } else if (!strcmp("us", argv[i])) {
			set_date_hint(gapp, FMT_us);
		    } else if (!strcmp("nohint", argv[i])) {
			set_date_hint(gapp, FMT_nohint);
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
                    if (register_real_time_input(gapp, fd, fd_name, FALSE) !=
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
                        if (register_real_time_input(gapp, fd, argv[i], TRUE) !=
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
	    } else if (argmatch(argv[i], "-free", 5)) {
		gui_set_page_free(gui, TRUE);
#endif
	    } else if (argmatch(argv[i], "-maxpath", 8)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for max drawing path\n");
		    usage(stderr, argv[0]);
		} else {
		    set_max_path_limit(canvas, atoi(argv[i]));
		}
	    } else if (argmatch(argv[i], "-noask", 5)) {
		gui->noask = TRUE;
	    } else if (argmatch(argv[i], "-hdevice", 5)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing argument for hardcopy device select flag\n");
		    usage(stderr, argv[0]);
		} else {
		    if (set_printer_by_name(gapp, argv[i]) != RETURN_SUCCESS) {
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
		    if (parse_device_options(canvas,
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
		    set_ptofile(gapp, TRUE);
                    strcpy(rt->print_file, argv[i]);
		}
	    } else if (argmatch(argv[i], "-hardcopy", 6)) {
		gracebat = TRUE;
	    } else if (argmatch(argv[i], "-block", 6)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing filename for block data\n");
		    usage(stderr, argv[0]);
		} else {
		    if (!gapp->gp) {
                        new_project(gapp, NULL);
                    }
                    getdata(graph_get_current(gproject_get_top(gapp->gp)), argv[i], curtype, LOAD_BLOCK);
		}
	    } else if (argmatch(argv[i], "-nxy", 4)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing filename for nxy data\n");
		    usage(stderr, argv[0]);
		} else {
		    if (!gapp->gp) {
                        new_project(gapp, NULL);
                    }
		    getdata(graph_get_current(gproject_get_top(gapp->gp)), argv[i], curtype, LOAD_NXY);
		}
	    } else if (argmatch(argv[i], "-type", 2) ||
                       argmatch(argv[i], "-settype", 8)) {
		/* set types */
		i++;
                curtype = get_settype_by_name(grace, argv[i]);
                if (curtype == -1) {
		    fprintf(stderr, "%s: Unknown set type '%s'\n", argv[0], argv[i]);
		    usage(stderr, argv[0]);
		}
	    } else if (argmatch(argv[i], "-results", 2)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing results file name\n");
		    usage(stderr, argv[0]);
		} else {
                    /*  open resfile if -results option given */
		    if ((rt->resfp = gapp_openw(gapp, argv[i])) == NULL) {
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
		    save_project(gapp->gp, argv[i]);
		}
	    } else if (argmatch(argv[i], "-wd", 3)) {
		i++;
		if (i == argc) {
		    fprintf(stderr, "Missing parameters for working directory\n");
		    usage(stderr, argv[0]);
		} else {
		    if (set_workingdir(gapp, argv[i]) != RETURN_SUCCESS) {
			fprintf(stderr, "Can't change to directory %s, fatal error", argv[i]);
			exit(1);
		    }
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
                rt->safe_mode = TRUE;
            } else if (argmatch(argv[i], "-nosafe", 7)) {
                rt->safe_mode = FALSE;
	    } else if (argmatch(argv[i], "-help", 2)) {
		usage(stdout, argv[0]);
	    } else {
		fprintf(stderr, "No option %s\n", argv[i]);
		usage(stderr, argv[0]);
	    }
	} else {
	    if (strstr(argv[i], ".xgr") || strstr(argv[i], ".agr")) {
                load_project(gapp, argv[i]);
            } else {
		if (!gapp->gp) {
                    new_project(gapp, NULL);
                }
                getdata(graph_get_current(gproject_get_top(gapp->gp)), argv[i], curtype, LOAD_SINGLE);
            }
	} /* end else */
    } /* end for */

    if (!gapp->gp) {
        new_project(gapp, NULL);
    }
    
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
            if (monitor_input(gapp, ib_tbl, ib_tblsize, 0) != RETURN_SUCCESS) {
                break;
            }
        }
	if (!noprint) {
	    do_hardcopy(gapp->gp);
	}
        
	bailout(gapp);
    } else {
/*
 * go main loop
 */
#ifndef NONE_GUI
        if (cli == TRUE) {
            cli_loop(gapp);
        } else {
            startup_gui(gapp);
        }
#else
        cli_loop(gapp);
#endif        
    }
    /* never reaches */
    exit(0);
}

/*
 * command interface loop
 */
static void cli_loop(GraceApp *gapp)
{
    Input_buffer *ib_stdin;
    int previous = -1;

    if (register_real_time_input(gapp, STDIN_FILENO, "stdin", 0)
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
        if (monitor_input(gapp, ib_tbl, ib_tblsize, 0) != RETURN_SUCCESS) {
            break;
        }
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
    fprintf(stream, "-datehint  [iso|european|us\n");
    fprintf(stream, "            |days|seconds|nohint]     Set the hint for dates analysis\n");
    fprintf(stream, "                                        (it is only a hint for the parser)\n");
    fprintf(stream, "-dpipe     [descriptor]               Read data from descriptor on startup\n");
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
    fprintf(stream, "-maxpath   [length]                   Set the maximal drawing path length\n");
    fprintf(stream, "-noask                                Assume the answer is yes to all requests -\n");
    fprintf(stream, "                                        if the operation would overwrite a file,\n");
    fprintf(stream, "                                        gapp will do so without prompting\n");
#ifndef NONE_GUI
    fprintf(stream, "-noinstall                            Don't use private colormap\n");
#endif
    fprintf(stream, "-noprint                              In batch mode, do not print\n");
    fprintf(stream, "-nosafe                               Disable safe mode\n");
    fprintf(stream, "-nosigcatch                           Don't catch signals\n");
    fprintf(stream, "-npipe     [file]                     Read data from named pipe on startup\n");
    fprintf(stream, "-nxy       [nxy_file]                 Assume data file is in X Y1 Y2 Y3 ...\n");
    fprintf(stream, "                                        format\n");
    fprintf(stream, "-printfile [file for hardcopy output] Save print output to file \n");
    fprintf(stream, "-results   [results_file]             Write results of some data manipulations\n");
    fprintf(stream, "                                        to results_file\n");
    fprintf(stream, "-safe                                 Safe mode (default)\n");
    fprintf(stream, "-saveall   [save_file]                Save all to save_file\n");
    fprintf(stream, "-seed      [seed_value]               Integer seed for random number generator\n");
    fprintf(stream, "-timer     [delay]                    Set allowed time slice for real time\n");
    fprintf(stream, "                                        inputs to delay ms\n");
    fprintf(stream, "-settype   [xy|xydx|...]              Set the type of the next data file\n");
    fprintf(stream, "-version                              Show the program version\n");
    fprintf(stream, "-wd        [directory]                Set the working directory\n");

    fprintf(stream, "-help                                 This message\n");
    fprintf(stream, "\n");
    fprintf(stream, " ** If it scrolls too fast, run `%s -help | more\' **\n", progname);
    exit(0);
}

static void VersionInfo(const GraceApp *gapp)
{
    int i;
    Canvas *canvas = grace_get_canvas(gapp->grace);
    
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

    fprintf(stdout, "Built: %s on %s\n", bi_date(), bi_system());
    fprintf(stdout, "Compiler flags: %s\n", bi_ccompiler());
 
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Registered devices:\n");
    for (i = 0; i < number_of_devices(canvas); i++) {
        fprintf(stdout, "%s ", get_device_name(canvas, i));
    }
    fprintf(stdout, "\n\n");
    
    fprintf(stdout, "(C) Copyright 1996-2007 Grace Development Team\n");
    fprintf(stdout, "(C) Portions Copyright 1991-1995 Paul J Turner\n");
    fprintf(stdout, "All Rights Reserved\n");

    return;
}
