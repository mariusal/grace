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
 * read data files
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "graphs.h"
#include "graphutils.h"
#include "protos.h"

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
#  include "netcdf.h"
#endif

#define MAXERR 5
/*
 * number of doubles to allocate for each call to realloc
 */
#define BUFSIZE  512

static char buf[256];

static char *linebuf = 0;
static int   linelen = 0;

int change_gno;			/* if the graph number changes on read in */
static int cur_gno;		/* if the graph number changes on read in */
int change_type;		/* current set type */
static int cur_type;		/* current set type */

static int readerror = 0;	/* number of errors */
static int readline = 0;	/* line number in file */

static int readxyany(int gno, char *fn, FILE * fp, int type);
static int readxystring(int gno, char *fn, FILE * fp);
static int readnxy(int gno, char *fn, FILE * fp);

/*
 * read a line increasing buffer as necessary
 */
int expand_line_buffer(char **adrPtr)
{
    char *newbuf;
    int   newlen;

    newlen = linelen + BUFSIZE;
    newbuf = (char *) malloc (newlen);
    if (newbuf == 0) {
        sprintf(buf, "Insufficient memory for line");
        errmsg(buf);
        return 0;
    }

    if (linelen == 0) {
        /* this is the first time through */
        *adrPtr = newbuf;
    } else {
        /* we are expanding an existing line */
        strncpy (newbuf, linebuf, linelen);
        *adrPtr += newbuf - linebuf;
        free (linebuf);
    }

    linebuf = newbuf;
    linelen = newlen;    

    return 1;
}

/*
 * read a line increasing buffer as necessary
 * returns 0 on failure (memory or no character), and 1 on success
 */
int read_long_line(FILE * fp)
{
    char *cursor;
    int   available, nbread, retval;

    cursor    = linebuf;
    available = linelen;
    retval    = 0;
    do {
        /* have we enough space to store the characters ? */
        if (available < 2)
            if (expand_line_buffer (&cursor) == 0)
                return 0;
        available = linebuf + linelen - cursor;

        /* read as much as possible */
        if (fgets (cursor, available, fp) == NULL)
            return retval;
        nbread = strlen (cursor);
        if (nbread < 1)
            return retval;
        else
            retval = 1;

        /* prepare next read */
        cursor    += nbread;
        available -= nbread;

    } while (*(cursor - 1) != '\n');

    return retval;
}

/* open a file for write */
FILE *grace_openw(char *fn)
{
    struct stat statb;
    char buf[GR_MAXPATHLEN + 50];

    if (!fn[0]) {
        errmsg("No file name given");
	return NULL;
    } else if (strcmp(fn, "-") == 0 || strcmp(fn, "stdout") == 0) {
        return stdout;
    /* check to make sure this is a file and not a dir */
    } else if (stat(fn, &statb) == 0 && !S_ISREG(statb.st_mode)) {
        sprintf(buf, "%s is not a regular file", fn);
        errmsg(buf);
	return NULL;
    } else if (fexists(fn)) {
	return NULL;
    } else {
        return filter_write(fn);
    }
}

/* open a file for read */
FILE *grace_openr(char *fn, int src)
{
    struct stat statb;
    char buf[GR_MAXPATHLEN + 50];

    if (!fn[0]) {
        errmsg("No file name given");
	return NULL;
    }
    switch (src) {
    case SOURCE_DISK:
	/* check to make sure this is a file and not a dir */
	if (strcmp(fn, "-") == 0 || strcmp(fn, "stdin") == 0) {
            return stdin;
	} else if (stat(fn, &statb)) {
            sprintf(buf, "Can't stat file %s", fn);
            errmsg(buf);
	    return NULL;
	} else if (!S_ISREG(statb.st_mode)) {
            sprintf(buf, "%s is not a regular file", fn);
            errmsg(buf);
	    return NULL;
        } else {
            return filter_read(fn);
	}
        break;
    case SOURCE_PIPE:
	return popen(popen_path_translate(fn), "r");
	break;
    default:
        errmsg("Wrong call to grace_openr()");
	return NULL;
    }
}

/*
 * close either a pipe or a file pointer
 *
 */
void grace_close(FILE *fp)
{
    if (fp == stdin || fp == stderr || fp == stdout) {
        return;
    }
    if (pclose(fp) == -1) {
        fclose(fp);
    }
}


int getdata(int gno, char *fn, int src, int type)
{
    FILE *fp;
    int retval = -1;
    int save_version, cur_version;
    
    fp = grace_openr(fn, src);
    if (fp == NULL) {
	return GRACE_EXIT_FAILURE;
    }
    
    readline = 0;
    cur_gno = gno;
    change_type = cur_type = type;
    save_version = get_project_version();
    set_project_version(0);
    set_work_pending(TRUE);

    while (retval == -1) {
	retval = 0;
	switch (cur_type) {
	case SET_XYSTRING:
	    retval = readxystring(cur_gno, fn, fp);
	    break;
	case SET_NXY:
	    retval = readnxy(cur_gno, fn, fp);
	    break;
	case SET_BLOCK:
	    retval = readblockdata(cur_gno, fn, fp);
	    break;
	default:
	    retval = readxyany(cur_gno, fn, fp, cur_type);
	    break;
	}
    }

    grace_close(fp);
    
    cur_version = get_project_version();
    if (cur_version != 0) {         /* a complete project */
        postprocess_project(cur_version);
    } else if (type != SET_BLOCK) { /* just a few sets */
        autoscale_graph(cur_gno, autoscale_onread);
    }
    set_project_version(save_version);
    
    set_work_pending(FALSE);

#ifndef NONE_GUI
    update_status_popup(NULL, NULL, NULL);
#endif

    if (retval < 0) {
        return GRACE_EXIT_FAILURE;
    } else {
        return GRACE_EXIT_SUCCESS;
    }
}


/*
 * read a set from a file
 * assume the first column is x and take y form the col'th column
 * if only 1 column, use index for x
 */
int read_set_fromfile(int gno, int setno, char *fn, int src, int col)
{
    FILE *fp;
    int readline = 0;
    int i = 0, pstat, retval = 0;
    double *x, *y, tmp;
    char *scstr;                    /* scanf string */

    fp = grace_openr(fn, src);
    if (fp == NULL) {
	return 0;
    }

    scstr = (char *)malloc( (col+1)*5 );
    scstr[0] = '\0';
    for( i=0; i<=col; i++ )
        if( !i || i==col-1 || i==col )
            strcat( scstr, "%lf " );
        else
            strcat( scstr, "%*lf " );
    i = 0;
    killsetdata(gno, setno);
    x = calloc(BUFSIZE, SIZEOF_DOUBLE);
    y = calloc(BUFSIZE, SIZEOF_DOUBLE);
    if (x == NULL || y == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        goto breakout;
    }
    while (read_long_line(fp) > 0) {
        readline++;
        if (linebuf[strlen(linebuf) - 1] != '\n') { 
            /* must have a newline char at the end of line */
            readerror++;
            fprintf(stderr, "No newline at line #%1d: %s", readline, linebuf);
            continue;
        }
        if (linebuf[0] == '#') {
            continue;
        }
        if (linebuf[0] == '@') {
            continue;
        }
        convertchar(linebuf);
        /* count the number of items scanned */
        if( col==1 ) {
            if ((pstat = sscanf(linebuf, "%lf %lf", &x[i], &y[i])) >= 1) {
                /* supply x if missing (y winds up in x) */
                if (pstat == 1) {
                    y[i] = x[i];
                    x[i] = i;
                }
                i++;
                if (i % BUFSIZE == 0) {
                    x = realloc(x, (i + BUFSIZE) * SIZEOF_DOUBLE);
                    y = realloc(y, (i + BUFSIZE) * SIZEOF_DOUBLE);
                }
            }
        } else {
            if ((pstat = sscanf(linebuf, scstr, &x[i], &tmp, &y[i])) >= 2) {
                /* if there are only as many columns as the column
                 * specified, use the index for the x value
                 */
                if (pstat == 2) {
                    y[i] = tmp;
                    x[i] = i;
                }
                i++;
                if (i % BUFSIZE == 0) {
                    x = realloc(x, (i + BUFSIZE) * SIZEOF_DOUBLE);
                    y = realloc(y, (i + BUFSIZE) * SIZEOF_DOUBLE);
                }
            }
        }
    }
    activateset(gno, setno);
    setcol(gno, x, setno, i, 0);
    setcol(gno, y, setno, i, 1);
    if (!strlen(getcomment(gno, setno))) {
        setcomment(gno, setno, fn);
    }
    log_results(fn);
    retval = 1;

  breakout:;

    grace_close(fp);
    
    free(scstr);
    return retval;
}

/*
 * read x1 y1 y2 ... y30 formatted files
 * note that the maximum number of sets is 30
 */
#define MAXSETN 30

static int readnxy(int gno, char *fn, FILE * fp)
{
    int i, j, pstat, cnt, scnt[MAXSETN], setn[MAXSETN], retval = 0;
    double *x[MAXSETN], *y[MAXSETN], xval, yr[MAXSETN];
    char *s, buf[1024], *tmpbuf;
    int do_restart = 0;

/* if more than one set of nxy data is in the file,
 * leap to here after each is read - the goto is at the
 * bottom of this module.
 */
  restart:;

    while ((read_long_line(fp) > 0) && ((linebuf[0] == '#') || (linebuf[0] == '@'))) {
	readline++;
	if (linebuf[0] == '@') {
	    change_gno = -1;
	    read_param(linebuf + 1);
	    if (change_gno >= 0) {
		cur_gno = gno = change_gno;
	    }
	}
    }
    convertchar(linebuf);

    /*
     * count the columns
     */
    tmpbuf = copy_string(NULL, linebuf);
    s = tmpbuf;
    cnt = 0;
    while ((s = strtok(s, " \t\n")) != NULL) {
	cnt++;
	s = NULL;
    }
    if (cnt > MAXPLOT) {
	errmsg("Maximum number of columns exceeded, reading first 31");
	cnt = 31;
    }
    free(tmpbuf);
    s = linebuf;
    s = strtok(s, " \t\n");
    if (s == NULL) {
	errmsg("Read ended by a blank line at or near the beginning of file");
	return 0;
    }
    pstat = sscanf(s, "%lf", &xval);
    if (pstat == 0) {
	errmsg("Read ended, non-numeric found on line at or near beginning of file");
	return 0;
    }
    s = NULL;
    for (j = 0; j < cnt - 1; j++) {
	s = strtok(s, " \t\n");
	if (s == NULL) {
	    yr[j] = 0.0;
	    errmsg("Number of items in column incorrect");
	} else {
	    yr[j] = atof(s);
	}
	s = NULL;
    }
    if (cnt > 1) {
	for (i = 0; i < cnt - 1; i++) {
	    x[i] = calloc(BUFSIZE, SIZEOF_DOUBLE);
	    y[i] = calloc(BUFSIZE, SIZEOF_DOUBLE);
	    if (x[i] == NULL || y[i] == NULL) {
		errmsg("Insufficient memory for set");
		cxfree(x[i]);
		cxfree(y[i]);
		return (0);
	    }
	    *(x[i]) = xval;
	    *(y[i]) = yr[i];
	    scnt[i] = 1;
	}
	while (!do_restart && (read_long_line(fp) > 0)) {
	    readline++;
	    if (linebuf[0] == '#') {
		continue;
	    }
	    if (strlen(linebuf) < 2) {
		continue;
	    }
	    if (linebuf[0] == '@') {
		change_gno = -1;
		change_type = cur_type;
		read_param(linebuf + 1);
		if (change_gno >= 0) {
		    cur_gno = gno = change_gno;
		}
		if (change_type != cur_type) {
		    cur_type = change_type;
		    retval = -1;
		    break;	/* exit this module and store any set */
		}
		continue;
	    }
	    convertchar(linebuf);
	    s = linebuf;
	    s = strtok(s, " \t\n");
	    if (s == NULL) {
		continue;
	    }
/* check for set separator */
	    pstat = sscanf(s, "%lf", &xval);
	    if (pstat == 0) {
		do_restart = 1;
		continue;
	    } else {
		s = NULL;
		for (j = 0; j < cnt - 1; j++) {
		    s = strtok(s, " \t\n");
		    if (s == NULL) {
			yr[j] = 0.0;
			errmsg("Number of items in column incorrect");
		    } else {
			yr[j] = atof(s);
		    }
		    s = NULL;
		}
		for (i = 0; i < cnt - 1; i++) {
		    *(x[i] + scnt[i]) = xval;
		    *(y[i] + scnt[i]) = yr[i];
		    scnt[i]++;
		    if (scnt[i] % BUFSIZE == 0) {
			x[i] = realloc(x[i], (scnt[i] + BUFSIZE) * SIZEOF_DOUBLE);
			y[i] = realloc(y[i], (scnt[i] + BUFSIZE) * SIZEOF_DOUBLE);
		    }
		}
	    }
	}
	for (i = 0; i < cnt - 1; i++) {
	    setn[i] = nextset(gno);
            if ((setn[i]) == -1) {
		errmsg("Can't allocate more sets in readnxy()");
                for (j = 0; j < i; j++) {
		    killsetdata(gno, setn[j]);
		}
                return 0;
	    }
	    set_set_hidden(gno, setn[i], FALSE);

	    setcol(gno, x[i], setn[i], scnt[i], 0);
	    setcol(gno, y[i], setn[i], scnt[i], 1);
	    sprintf( buf, "%s:%d", fn, i+1 );	/* identify column # in comment */
	    setcomment(gno, setn[i], buf);
	    log_results(fn);
	}
	if (!do_restart) {
	    if (retval == -1) {
		return retval;
	    } else {
		return 1;
	    }
	} else {
	    do_restart = 0;
	    goto restart;
	}
    }
    return 0;
}


/*
 * read in any "plain" (but NXY, XYSTRING and BLOCK) set types
 */
static int readxyany(int gno, char *fn, FILE * fp, int type)
{
    int i = 0, j = 0, readset = 0, pstat, retval = 0;
    double *x, *y, *dx, *dy, *dz, *dw;
    double xtmp, ytmp, dxtmp, dytmp, dztmp, dwtmp;
    int ncols;  /* columns of numbers */
    long alloclen;

    retval = 0;
    
    alloclen = 0;
    x = y = dx = dy = dz = dw = NULL;
    
    ncols = settype_cols(type);
   
    while (read_long_line (fp) > 0) {
	readline++;
	/* ignore comments or empty lines: */
        if (linebuf[0] == '#' || strlen(linebuf) < 2) {
	    continue;
	}
        
	if (linebuf[0] == '@') {
	    change_gno = -1;
	    change_type = type;
	    if (read_param(linebuf + 1) != 0) {
                retval = -2;
            }
	    if (change_gno >= 0) {
		cur_gno = gno = change_gno;
	    }
	    if (change_type != type) {
	        cur_type = change_type;
                retval = -1;
	        break;      /* exit this module and store any set */
	    }
	    continue;
	}
        
	convertchar(linebuf);
        
	/* count the number of items scanned */
	pstat = sscanf(linebuf, "%lf %lf %lf %lf %lf %lf", 
                                &xtmp, &ytmp, &dxtmp, &dytmp, &dztmp, &dwtmp);
        /* pstat == ncols - 1 corresponds to dummy X (index) */
        if (pstat >= ncols - 1) {
            if (i >= alloclen) {
                switch (ncols) {
                case 2:
                    x = xrealloc(x, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    y = xrealloc(y, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    if (x == NULL || y == NULL) {
                        errmsg("Insufficient memory for set");
                        cxfree(x);
                        cxfree(y);
                        return -2;
                    }
                    break;
                case 3:
                    x  = xrealloc(x,  (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    y  = xrealloc(y,  (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    dx = xrealloc(dx, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    if (x == NULL || y == NULL || dx == NULL) {
                        errmsg("Insufficient memory for set");
                        cxfree(x);
                        cxfree(y);
                        cxfree(dx);
                        return -2;
                    }
                    break;
                case 4:
                    x  = xrealloc(x,  (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    y  = xrealloc(y,  (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    dx = xrealloc(dx, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    dy = xrealloc(dy, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    if (x == NULL || y == NULL || dx == NULL || dy == NULL) {
                        errmsg("Insufficient memory for set");
                        cxfree(x);
                        cxfree(y);
                        cxfree(dx);
                        cxfree(dy);
                        return -2;
                    }
                    break;
                case 5:
                    x  = xrealloc(x,  (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    y  = xrealloc(y,  (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    dx = xrealloc(dx, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    dy = xrealloc(dy, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    dz = xrealloc(dz, (alloclen + BUFSIZE)*SIZEOF_DOUBLE);
                    if (x == NULL || y == NULL || dx == NULL || dy == NULL || dz == NULL) {
                        errmsg("Insufficient memory for set");
                        cxfree(x);
                        cxfree(y);
                        cxfree(dx);
                        cxfree(dy);
                        cxfree(dz);
                        return -2;
                    }
                    break;
                default:
                    errmsg("Internal error in readxyany");
                    return -3;
                }
                alloclen += BUFSIZE;
            }

	    if (pstat == ncols - 1) {
                dwtmp = dztmp;
                dztmp = dytmp;
                dytmp = dxtmp;
                dxtmp = ytmp;
                ytmp  = xtmp;
                xtmp  = i;
            }
            
            /* got x and y so increment */
	    switch (ncols) {
            case 2:
	        x[i] = xtmp;
	        y[i] = ytmp;
                break;
            case 3:
	        x[i] = xtmp;
	        y[i] = ytmp;
		dx[i] = dxtmp;
                break;
            case 4:
	        x[i] = xtmp;
	        y[i] = ytmp;
		dx[i] = dxtmp;
		dy[i] = dytmp;
                break;
            case 5:
	        x[i] = xtmp;
	        y[i] = ytmp;
		dx[i] = dxtmp;
		dy[i] = dytmp;
		dz[i] = dztmp;
                break;
            }
	    i++;
	} else {
	    if (i != 0) {
		if ((j = nextset(gno)) == -1) {
		    cxfree(x);
		    cxfree(y);
		    cxfree(dx);
		    cxfree(dy);
		    cxfree(dz);
		    cxfree(dw);
		    return -2;
		} else {
		    activateset(gno, j);
		    set_dataset_type(gno, j, type);
		    setcol(gno, x, j, i, 0);
		    setcol(gno, y, j, i, 1);
		    setcol(gno, dx, j, i, 2);
		    setcol(gno, dy, j, i, 3);
		    setcol(gno, dz, j, i, 4);
		    setcol(gno, dw, j, i, 5);
		    if (!strlen(getcomment(gno, j))) {
		        setcomment(gno, j, fn);
		    }
		    log_results(fn);
		    readset++;

                    i = 0;
                    alloclen = 0;
                    x = y = dx = dy = dz = dw = NULL;
                }
	    } else {
		readerror++;
		sprintf(buf, "Error at line #%1d: %s", readline, linebuf);
		errmsg(buf);
		if (readerror > MAXERR) {
		    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
			cxfree(x);
			cxfree(y);
			cxfree(dx);
			cxfree(dy);
			cxfree(dz);
			cxfree(dw);
			return -2;
		    } else {
			readerror = 0;
		    }
		}
	    }
	}
    }

    if (i != 0) {
	if ((j = nextset(gno)) == -1) {
	    cxfree(x);
	    cxfree(y);
	    cxfree(dx);
	    cxfree(dy);
	    cxfree(dz);
	    cxfree(dw);
	    return -2;
	}
	activateset(gno, j);
	set_dataset_type(gno, j, type);
	setcol(gno, x, j, i, 0);
	setcol(gno, y, j, i, 1);
	setcol(gno, dx, j, i, 2);
	setcol(gno, dy, j, i, 3);
	setcol(gno, dz, j, i, 4);
	setcol(gno, dw, j, i, 5);
	if (!strlen(getcomment(gno, j))) {
	    setcomment(gno, j, fn);
	}
	log_results(fn);
	readset++;
    } else {
	cxfree(x);
	cxfree(y);
	cxfree(dx);
	cxfree(dy);
	cxfree(dz);
	cxfree(dw);
    }
    
    return retval;
}

static int readxystring(int gno, char *fn, FILE * fp)
{
    int i = 0, ll, j, pstat, readset = 0, retval = 0;
    double *x, *y;
    char *s, *s1, *s2, **strs;

    x = calloc(BUFSIZE, SIZEOF_DOUBLE);
    y = calloc(BUFSIZE, SIZEOF_DOUBLE);
    strs = calloc(BUFSIZE, sizeof(char *));
    if (x == NULL || y == NULL || strs == NULL) {
	errmsg("Insufficient memory for set");
	cxfree(x);
	cxfree(y);
	cxfree(strs);
	return retval;
    }
    while (read_long_line (fp) > 0) {
	readline++;
	ll = strlen(linebuf);
	if ((ll > 0) && (linebuf[ll - 1] != '\n')) {	/* must have a newline
							 * char at end of line */
	    readerror++;
	    fprintf(stderr, "No newline at line #%1d: %s\n", readline, linebuf);
	    if (readerror > MAXERR) {
		if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
		    cxfree(x);
		    cxfree(y);
		    cxfree(strs);
		    return retval;
		} else {
		    readerror = 0;
		}
	    }
	    continue;
	}
	if (linebuf[0] == '#') {
	    continue;
	}
	if (strlen(linebuf) < 2) {	/* blank line */
	    continue;
	}
	if (linebuf[0] == '@') {
	    change_gno = -1;
	    change_type = cur_type;
	    read_param(linebuf + 1);
	    if (change_gno >= 0) {
		cur_gno = gno = change_gno;
	    }
	    if (change_type != cur_type) {
		cur_type = change_type;
		retval = -1;
		break;		/* exit this module and store any set */
	    }
	    continue;
	}
	/* count the number of items scanned */
	if ((pstat = sscanf(linebuf, "%lf %lf", &x[i], &y[i])) >= 1) {
	    /* supply x if missing (y winds up in x) */
	    if (pstat == 1) {
		y[i] = x[i];
		x[i] = i;
	    }
	    /* get the string portion */
	    linebuf[strlen(linebuf) - 1] = 0;	/* remove newline */
	    s1 = strrchr(linebuf, '"');	/* find last quote */
	    s2 = strchr(linebuf, '"');	/* find first quote */
	    if (s1 != s2) { /* a quoted string */
	        s = s1;
	        s[0] = 0;           /* terminate the string here */
	        s = s2;
	        s++;                /* increment to the first char */
	        strs[i] = malloc((strlen(s) + 1) * sizeof(char));
	        strcpy(strs[i], s);
	        /* got x and y so increment */
	        i++;
	        if (i % BUFSIZE == 0) {
	            x = realloc(x, (i + BUFSIZE) * SIZEOF_DOUBLE);
	            y = realloc(y, (i + BUFSIZE) * SIZEOF_DOUBLE);
	            strs = realloc(strs, (i + BUFSIZE) * sizeof(char *));
	        }
	    } else {
	        readerror++;
		sprintf(buf, "Error at line #%1d: %s", readline, linebuf);
		errmsg(buf);
	        if (readerror > MAXERR) {
	            if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
	                cxfree(x);
	                cxfree(y);
	                cxfree(strs);
	                return (0);
	            } else {
	                readerror = 0;
	            }
	        }
	    }
	} else {
	    if (i != 0) {
		if ((j = nextset(gno)) == -1) {
		    cxfree(x);
		    cxfree(y);
		    return (readset);
		}
		activateset(gno, j);
                set_dataset_type(gno, j, SET_XYSTRING);
		setcol(gno, x, j, i, 0);
		setcol(gno, y, j, i, 1);
		set_set_strings(gno, j, i, strs);
		if (!strlen(getcomment(gno, j))) {
		    setcomment(gno, j, fn);
		}
		log_results(fn);
		readset++;
	    } else {
		readerror++;
		sprintf(buf, "Error at line #%1d: %s", readline, linebuf);
		errmsg(buf);
		if (readerror > MAXERR) {
		    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
			cxfree(x);
			cxfree(y);
			cxfree(strs);
			return (0);
		    } else {
			readerror = 0;
		    }
		}
	    }
	    i = 0;
	    x = calloc(BUFSIZE, SIZEOF_DOUBLE);
	    y = calloc(BUFSIZE, SIZEOF_DOUBLE);
	    strs = calloc(BUFSIZE, sizeof(char *));
	    if (x == NULL || y == NULL) {
		errmsg("Insufficient memory for set");
		cxfree(x);
		cxfree(y);
		return (readset);
	    }
	}
    }

    if (i != 0) {
	if ((j = nextset(gno)) == -1) {
	    cxfree(x);
	    cxfree(y);
	    cxfree(strs);
	    return (readset);
	}
	activateset(gno, j);
        set_dataset_type(gno, j, SET_XYSTRING);
	setcol(gno, x, j, i, 0);
	setcol(gno, y, j, i, 1);
	set_set_strings(gno, j, i, strs);
	if (!strlen(getcomment(gno, j))) {
	    setcomment(gno, j, fn);
	}
	log_results(fn);
	readset++;
    } else {
	cxfree(x);
	cxfree(y);
	cxfree(strs);
    }

    if (retval == -1) {
	return retval;
    } else {
	return readset;
    }
}

/*
 * read block data
 */
int readblockdata(int gno, char *fn, FILE * fp)
{
    int i = 0, j, k, ncols = 0, pstat;
    int first = 1, readerror = 0;
    double **data = NULL;
    char *tmpbuf, *s, tbuf[256];
    int linecount = 0;

    i = 0;
    pstat = 0;
    while (read_long_line (fp) > 0) {
        s = linebuf;
	readline++;
	linecount++;
	if (linebuf[0] == '#') {
	    continue;
	}
	if (linebuf[0] == '@') {
	    read_param(linebuf + 1);
	    continue;
	}
	if (strlen(linebuf) > 1) {
	    convertchar(linebuf);
	    if (first) {	/* count the number of columns */
		ncols = 0;
		tmpbuf = copy_string(NULL, linebuf);
                if (tmpbuf == NULL) {
                    errmsg("Insufficient memory for string");
                    return 0;
                }
		s = tmpbuf;
		while (*s == ' ' || *s == '\t' || *s == '\n') {
		    s++;
		}
		while ((s = strtok(s, " \t\n")) != NULL) {
		    ncols++;
		    s = NULL;
		}
		if (ncols < 1 || ncols > maxblock) {
		    errmsg("Column count incorrect");
                    free (tmpbuf);
		    return 0;
		}
		data = malloc(sizeof(double *) * maxblock);
		if (data == NULL) {
		    errmsg("Can't allocate memory for block data");
                    free (tmpbuf);
		    return 0;
		}
		for (j = 0; j < ncols; j++) {
		    data[j] = calloc(BUFSIZE, SIZEOF_DOUBLE);
		    if (data[j] == NULL) {
			errmsg("Insufficient memory for block data");
			for (k = 0; k < j; k++) {
			    cxfree(data[k]);
			}
			cxfree(data);
                        free (tmpbuf);
			return 0;
		    }
		}
                free (tmpbuf);
		first = 0;
	    }
	    s = linebuf;
	    while (*s == ' ' || *s == '\t' || *s == '\n') {
		s++;
	    }
	    for (j = 0; j < ncols; j++) {
		s = strtok(s, " \t\n");
		if (s == NULL) {
		    data[j][i] = 0.0;
		    sprintf(tbuf, "Number of items in column incorrect at line %d, line skipped", linecount);
		    errmsg(tbuf);
		    readerror++;
		    if (readerror > MAXERR) {
			if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
			    for (k = 0; k < ncols; k++) {
				cxfree(data[k]);
			    }
			    cxfree(data);
			    return (0);
			} else {
			    readerror = 0;
			}
		    }
		    /* skip the rest */
		    goto bustout;
		} else {
		    data[j][i] = atof(s);
		}
		s = NULL;
	    }
	    i++;
	    if (i % BUFSIZE == 0) {
		for (j = 0; j < ncols; j++) {
		    data[j] = realloc(data[j], (i + BUFSIZE) * SIZEOF_DOUBLE);
		    if (data[j] == NULL) {
			errmsg("Insufficient memory for block data");
			for (k = 0; k < j; k++) {
			    cxfree(data[k]);
			}
			cxfree(data);
			return 0;
		    }
		}
	    }
	}
      bustout:;
    }
    for (j = 0; j < ncols; j++) {
	blockdata[j] = data[j];
    }
    cxfree(data);
    blocklen = i;
    blockncols = ncols;
    return 1;
}

void create_set_fromblock(int gno, int type, char *cols)
{
    int i;
    int setno;
    int cx, cy, c1, c2, c3, c4;
    double *tx, *ty, *t2, *t3, *t4;
    int nc, *coli;
    char *s, buf[256];

    if (blockncols <= 0) {
        errmsg("No block data read");
        return;
    }
    
    strcpy(buf, cols);
    s = buf;
    c1 = c2 = c3 = c4 = nc = 0;
    coli = malloc(maxblock * sizeof(int *));
    while ((s = strtok(s, ":")) != NULL) {
	coli[nc] = atoi(s);
	coli[nc]--;
	nc++;
	s = NULL;
    }
    if (nc == 0) {
	errmsg("No columns scanned in column string");
	free(coli);
	return;
    }
    for (i = 0; i < nc; i++) {
	if (coli[i] < -1) {
	    errmsg("Incorrect column specification");
	    free(coli);
	    return;
	}
    }

    cx = coli[0];
    cy = coli[1];
    if (cx >= blockncols) {
	errmsg("Column for X exceeds the number of columns in block data");
	free(coli);
	return;
    }
    if (cy >= blockncols) {
	errmsg("Column for Y exceeds the number of columns in block data");
	free(coli);
	return;
    }
    switch (settype_cols(type)) {
    case 2:
	break;
    case 3:
	c1 = coli[2];
	if (c1 >= blockncols) {
	    errmsg("Column for E1 exceeds the number of columns in block data");
	    free(coli);
	    return;
	}
	break;
    case 4:
	c1 = coli[2];
	c2 = coli[3];
	if (c1 >= blockncols) {
	    errmsg("Column for E1 exceeds the number of columns in block data");
	    free(coli);
	    return;
	}
	if (c2 >= blockncols) {
	    errmsg("Column for E2 exceeds the number of columns in block data");
	    free(coli);
	    return;
	}
	break;
    case 5:
	c1 = coli[2];
	c2 = coli[3];
	c3 = coli[4];
	if (c1 >= blockncols) {
	    errmsg("Column for E1 exceeds the number of columns in block data");
	    free(coli);
	    return;
	}
	if (c2 >= blockncols) {
	    errmsg("Column for E2 exceeds the number of columns in block data");
	    free(coli);
	    return;
	}
	if (c3 >= blockncols) {
	    errmsg("Column for E3 exceeds the number of columns in block data");
	    free(coli);
	    return;
	}
	break;
    }

    if (!is_graph_active(gno)) {
	set_graph_active(gno, TRUE);
    }
    setno = nextset(gno);
    if (setno == -1) {
	return;
    }
    
    activateset(gno, setno);
    set_dataset_type(gno, setno, type);

    tx = calloc(blocklen, SIZEOF_DOUBLE);
    ty = calloc(blocklen, SIZEOF_DOUBLE);
    for (i = 0; i < blocklen; i++) {
        if (cx == -1) {
            tx[i] = i + 1;
        }
        else {
            tx[i] = blockdata[cx][i];
        }
        if (cy == -1) {
            ty[i] = i + 1;
        }
        else {
            ty[i] = blockdata[cy][i];
        }
    }
    setcol(gno, tx, setno, blocklen, 0);
    setcol(gno, ty, setno, blocklen, 1);

    switch (settype_cols(type)) {
    case 2:
	sprintf(buf, "Cols %d %d", cx + 1, cy + 1);
	break;
    case 3:
	sprintf(buf, "Cols %d %d %d", cx + 1, cy + 1, c1 + 1);
	t2 = calloc(blocklen, SIZEOF_DOUBLE);
	for (i = 0; i < blocklen; i++) {
	    if (c1 == -1) {
		t2[i] = i + 1;
	    }
	    else {
		t2[i] = blockdata[c1][i];
	    }
	}
	setcol(gno, t2, setno, blocklen, 2);
	break;
    case 4:
	sprintf(buf, "Cols %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1);
	t2 = calloc(blocklen, SIZEOF_DOUBLE);
	t3 = calloc(blocklen, SIZEOF_DOUBLE);
	for (i = 0; i < blocklen; i++) {
	    if (c1 == -1) {
		t2[i] = i + 1;
	    }
	    else {
		t2[i] = blockdata[c1][i];
	    }
	    if (c2 == -1) {
		t3[i] = i + 1;
	    }
	    else {
		t3[i] = blockdata[c2][i];
	    }
	}
	setcol(gno, t2, setno, blocklen, 2);
	setcol(gno, t3, setno, blocklen, 3);
	break;
    case 5:
	sprintf(buf, "Cols %d %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1, c3 + 1);
	t2 = calloc(blocklen, SIZEOF_DOUBLE);
	t3 = calloc(blocklen, SIZEOF_DOUBLE);
	t4 = calloc(blocklen, SIZEOF_DOUBLE);
	for (i = 0; i < blocklen; i++) {
	    if (c1 == -1) {
		t2[i] = i + 1;
	    }
	    else {
		t2[i] = blockdata[c1][i];
	    }
	    if (c2 == -1) {
		t3[i] = i + 1;
	    }
	    else {
		t3[i] = blockdata[c2][i];
	    }
	    if (c3 == -1) {
		t4[i] = i + 1;
	    }
	    else {
		t4[i] = blockdata[c3][i];
	    }
	}
	setcol(gno, t2, setno, blocklen, 2);
	setcol(gno, t3, setno, blocklen, 3);
	setcol(gno, t4, setno, blocklen, 4);
	break;
    }

    free(coli);
    setcomment(gno, setno, buf);
    log_results(buf);
    
#ifndef NONE_GUI
    update_status_popup(NULL, NULL, NULL);
#endif
}

void outputset(int gno, int setno, char *fname, char *dformat)
{
    FILE *cp;
    
    if ((cp = grace_openw(fname)) == NULL) {
	return;
    } else {
        write_set(gno, setno, cp, dformat, TRUE);
	grace_close(cp);
    }
}

int save_project(char *fn)
{
    FILE *cp;
    int gno, setno;
    
    if ((cp = grace_openw(fn)) == NULL) {
	return GRACE_EXIT_FAILURE;
    }
    
    putparms(-1, cp, TRUE);

    for (gno = 0; gno < number_of_graphs(); gno++) {
        for (setno = 0; setno < number_of_sets(gno); setno++) {
            if (is_set_active(gno, setno) == TRUE &&
                is_set_hidden(gno, setno) == FALSE) {
                write_set(gno, setno, cp, sformat, FALSE);
            }
        }
    }

    grace_close(cp);
    return GRACE_EXIT_SUCCESS;
}

/*
 * write out a set
 */
int write_set(int gno, int setno, FILE *cp, char *format, int rawdata)
{
    int i, n, ncols;
    double *x, *y, *dx, *dy, *dz;
    char **s;
    char *format_string;

    if (cp == NULL) {
	return GRACE_EXIT_FAILURE;
    }
    
    if (format == NULL) {
        format = sformat;
    }

    if (is_set_active(gno, setno) == TRUE) {
        if (!rawdata) {
            fprintf(cp, "@target G%d.S%d\n", gno, setno);
            fprintf(cp, "@type %s\n", set_types(dataset_type(gno, setno)));
        }
        x = getx(gno, setno);
        y = gety(gno, setno);
        n = getsetlength(gno, setno);
        ncols = dataset_cols(gno, setno);
        /* 2 for "\n" and 4 for "%s" in xystring */
        format_string = malloc((ncols + 1)*strlen(format) + 6);
        strcpy(format_string, format);
        for (i = 1; i < ncols; i++) {
            strcat(format_string, " ");
            strcat(format_string, format);
        }
        switch (dataset_type(gno, setno)) {
        case SET_XYSTRING:
            strcat(format_string, " ");
            strcat(format_string, "\"%s\"");
            strcat(format_string, "\n");
            s = get_set_strings(gno, setno);
            for (i = 0; i < n; i++) {
                fprintf(cp, format_string, x[i], y[i], s[i]);
            }
            break;
        default:    
            strcat(format_string, "\n");
            switch (ncols) {
            case 2:
                for (i = 0; i < n; i++) {
                    fprintf(cp, format_string, x[i], y[i]);
                }
                break;
            case 3:
                dx = getcol(gno, setno, 2);
                for (i = 0; i < n; i++) {
                    fprintf(cp, format_string, x[i], y[i], dx[i]);
                }
                break;
            case 4:
                dx = getcol(gno, setno, 2);
                dy = getcol(gno, setno, 3);
                for (i = 0; i < n; i++) {
                    fprintf(cp, format_string, x[i], y[i], dx[i], dy[i]);
                }
                break;
            case 5:
                dx = getcol(gno, setno, 2);
                dy = getcol(gno, setno, 3);
                dz = getcol(gno, setno, 4);
                for (i = 0; i < n; i++) {
                    fprintf(cp, format_string, x[i], y[i], dx[i], dy[i], dz[i]);
                }
                break;
            }
        }
        if (rawdata) {
            fprintf(cp, "\n");
        } else {
            fprintf(cp, "&\n");
        }
        free(format_string);
    }
    return GRACE_EXIT_SUCCESS;
}


#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)

/*
 * read a variable from netcdf file into a set in graph gno
 * xvar and yvar are the names for x, y in the netcdf file resp.
 * return 0 on fail, return 1 if success.
 *
 * if xvar == NULL, then load the index of the point to x
 *
 */
int readnetcdf(int gno,
	       int setno,
	       char *netcdfname,
	       char *xvar,
	       char *yvar,
	       int nstart,
	       int nstop,
	       int nstride)
{
    int cdfid;			/* netCDF id */
    int i, n;
    double *x, *y;
    float *xf, *yf;
    short *xs, *ys;
    long *xl, *yl;

    /* variable ids */
    int x_id = -1, y_id;

    /* variable shapes */
    long start[2];
    long count[2];

    nc_type xdatatype = 0;
    nc_type ydatatype = 0;
    int xndims, xdim[10], xnatts;
    int yndims, ydim[10], ynatts;
    long nx, ny;

    ncopts = 0;			/* no crash on error */

/*
 * get a set if on entry setno == -1, if setno=-1, then fail
 */
    if (setno == -1) {
	if ((setno = nextset(gno)) == -1) {
	    return 0;
	}
    } else {
	if (is_set_active(gno, setno)) {
	    killset(gno, setno);
	}
    }
/*
 * open the netcdf file and locate the variable to read
 */
    if ((cdfid = ncopen(netcdfname, NC_NOWRITE)) == -1) {
	errmsg("Can't open file.");
	return 0;
    }
    if (xvar != NULL) {
	if ((x_id = ncvarid(cdfid, xvar)) == -1) {
	    char ebuf[256];
	    sprintf(ebuf, "readnetcdf(): No such variable %s for X", xvar);
	    errmsg(ebuf);
	    return 0;
	}
	ncvarinq(cdfid, x_id, NULL, &xdatatype, &xndims, xdim, &xnatts);
	ncdiminq(cdfid, xdim[0], NULL, &nx);
	if (xndims != 1) {
	    errmsg("Number of dimensions for X must be 1.");
	    return 0;
	}
    }
    if ((y_id = ncvarid(cdfid, yvar)) == -1) {
	char ebuf[256];
	sprintf(ebuf, "readnetcdf(): No such variable %s for Y", yvar);
	errmsg(ebuf);
	return 0;
    }
    ncvarinq(cdfid, y_id, NULL, &ydatatype, &yndims, ydim, &ynatts);
    ncdiminq(cdfid, ydim[0], NULL, &ny);
    if (yndims != 1) {
	errmsg("Number of dimensions for Y must be 1.");
	return 0;
    }
    if (xvar != NULL) {
	n = nx < ny ? nx : ny;
    } else {
	n = ny;
    }
    if (n <= 0) {
	errmsg("Length of dimension == 0.");
	return 0;
    }
/*
 * allocate for this set
 */
    x = calloc(n, SIZEOF_DOUBLE);
    y = calloc(n, SIZEOF_DOUBLE);
    if (x == NULL || y == NULL) {
	errmsg("Insufficient memory for set");
	cxfree(x);
	cxfree(y);
	ncclose(cdfid);
	return 0;
    }
    start[0] = 0;
    count[0] = n;		/* This will retrieve whole file, modify
				 * these values to get subset. This will only
				 * work for single-dimension vars.  You need
				 * to add dims to start & count for
				 * multi-dimensional. */

/*
 * read the variables from the netcdf file
 */
    if (xvar != NULL) {
/* TODO should check for other data types here */
/* TODO should check for NULL on the callocs() */
/* TODO making assumptions about the sizes of shorts and longs */
	switch (xdatatype) {
	case NC_SHORT:
	    xs = calloc(n, SIZEOF_SHORT);
	    ncvarget(cdfid, x_id, start, count, (void *) xs);
	    for (i = 0; i < n; i++) {
		x[i] = xs[i];
	    }
	    free(xs);
	    break;
	case NC_LONG:
	    xl = calloc(n, SIZEOF_LONG);
	    ncvarget(cdfid, x_id, start, count, (void *) xl);
	    for (i = 0; i < n; i++) {
		x[i] = xl[i];
	    }
	    free(xl);
	    break;
	case NC_FLOAT:
	    xf = calloc(n, SIZEOF_FLOAT);
	    ncvarget(cdfid, x_id, start, count, (void *) xf);
	    for (i = 0; i < n; i++) {
		x[i] = xf[i];
	    }
	    free(xf);
	    break;
	case NC_DOUBLE:
	    ncvarget(cdfid, x_id, start, count, (void *) x);
	    break;
	default:
	    errmsg("Data type not supported");
	    cxfree(x);
	    cxfree(y);
	    ncclose(cdfid);
	    return 0;
	    break;
	}
    } else {			/* just load index */
	for (i = 0; i < n; i++) {
	    x[i] = i + 1;
	}
    }
    switch (ydatatype) {
    case NC_SHORT:
	ys = calloc(n, SIZEOF_SHORT);
	ncvarget(cdfid, y_id, start, count, (void *) ys);
	for (i = 0; i < n; i++) {
	    y[i] = ys[i];
	}
	break;
    case NC_LONG:
	yl = calloc(n, SIZEOF_LONG);
	ncvarget(cdfid, y_id, start, count, (void *) yl);
	for (i = 0; i < n; i++) {
	    y[i] = yl[i];
	}
	break;
    case NC_FLOAT:
/* TODO should check for NULL here */
	yf = calloc(n, SIZEOF_FLOAT);
	ncvarget(cdfid, y_id, start, count, (void *) yf);
	for (i = 0; i < n; i++) {
	    y[i] = yf[i];
	}
	free(yf);
	break;
    case NC_DOUBLE:
	ncvarget(cdfid, y_id, start, count, (void *) y);
	break;
    default:
	errmsg("Data type not supported");
	cxfree(x);
	cxfree(y);
	ncclose(cdfid);
	return 0;
	break;
    }
    ncclose(cdfid);

/*
 * initialize stuff for the newly created set
 */
    activateset(gno, setno);
    set_dataset_type(gno, setno, SET_XY);
    setcol(gno, x, setno, n, 0);
    setcol(gno, y, setno, n, 1);

    sprintf(buf, "File %s x = %s y = %s", netcdfname, xvar == NULL ? "Index" : xvar, yvar);
    setcomment(gno, setno, buf);
    
    autoscale_graph(gno, autoscale_onread);
    
    log_results(buf);
    
    return 1;
}

int write_netcdf(char *fname)
{
    char buf[512];
    int ncid;			/* netCDF id */
    int i, j;
    /* dimension ids */
    int n_dim;
    /* variable ids */
    int x_id, y_id;
    int dims[1];
    long len[1];
    long it = 0;
    double *x, *y, x1, x2, y1, y2;
    ncid = nccreate(fname, NC_CLOBBER);
    ncattput(ncid, NC_GLOBAL, "Contents", NC_CHAR, 11, (void *) "grace sets");
    for (i = 0; i < number_of_graphs(); i++) {
	if (is_graph_active(i)) {
	    for (j = 0; j < number_of_sets(i); j++) {
		if (is_set_active(i, j)) {
		    char s[64];

		    sprintf(buf, "g%d_s%d_comment", i, j);
		    ncattput(ncid, NC_GLOBAL, buf, NC_CHAR,
		    strlen(getcomment(i, j)), (void *) getcomment(i, j));

		    sprintf(buf, "g%d_s%d_type", i, j);
		    strcpy(s, set_types(dataset_type(i, j)));
		    ncattput(ncid, NC_GLOBAL, buf, NC_CHAR, strlen(s), (void *) s);

		    sprintf(buf, "g%d_s%d_n", i, j);
		    n_dim = ncdimdef(ncid, buf, getsetlength(i, j));
		    dims[0] = n_dim;
		    getsetminmax(i, j, &x1, &x2, &y1, &y2);
		    sprintf(buf, "g%d_s%d_x", i, j);
		    x_id = ncvardef(ncid, buf, NC_DOUBLE, 1, dims);
		    ncattput(ncid, x_id, "min", NC_DOUBLE, 1, (void *) &x1);
		    ncattput(ncid, x_id, "max", NC_DOUBLE, 1, (void *) &x2);
		    dims[0] = n_dim;
		    sprintf(buf, "g%d_s%d_y", i, j);
		    y_id = ncvardef(ncid, buf, NC_DOUBLE, 1, dims);
		    ncattput(ncid, y_id, "min", NC_DOUBLE, 1, (void *) &y1);
		    ncattput(ncid, y_id, "max", NC_DOUBLE, 1, (void *) &y2);
		}
	    }
	}
    }
    ncendef(ncid);
    ncclose(ncid);
    if ((ncid = ncopen(fname, NC_WRITE)) == -1) {
	errmsg("Can't open file.");
	return 1;
    }
    for (i = 0; i < number_of_graphs(); i++) {
	if (is_graph_active(i)) {
	    for (j = 0; j < number_of_sets(i); j++) {
		if (is_set_active(i, j)) {
		    len[0] = getsetlength(i, j);
		    x = getx(i, j);
		    y = gety(i, j);
		    sprintf(buf, "g%d_s%d_x", i, j);
		    x_id = ncvarid(ncid, buf);
		    sprintf(buf, "g%d_s%d_y", i, j);
		    y_id = ncvarid(ncid, buf);
		    ncvarput(ncid, x_id, &it, len, (void *) x);
		    ncvarput(ncid, y_id, &it, len, (void *) y);
		}
	    }
	}
    }

    ncclose(ncid);
    return 0;
}

#endif				/* HAVE_NETCDF */
