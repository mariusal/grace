/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2006 Grace Development Team
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
#include <sys/time.h>
#include <errno.h>
#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef HAVE_NETCDF
#  include <netcdf.h>
#endif

#include "graceapp.h"
#include "utils.h"
#include "files.h"
#include "ssdata.h"
#include "core_utils.h"

#include "xprotos.h"

#define MAXERR 5

/*
 * number of rows to allocate for each call to realloc
 */
#define BUFSIZE  512

/*
 * number of bytes in each line chunk
 * (should be related to system pipe size, typically 4K)
 */
#ifndef PIPE_BUF
#  define PIPE_BUF 4096
#endif
#define CHUNKSIZE 2*PIPE_BUF

char *close_input;		/* name of real-time input to close */

struct timeval read_begin = {0l, 0l};	/* used to check too long inputs */

static Input_buffer dummy_ib = {-1, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, 0l};

int nb_rt = 0;		        /* number of real time file descriptors */
Input_buffer *ib_tbl = 0;	/* table for each open input */
int ib_tblsize = 0;		/* number of elements in ib_tbl */

static int time_spent(void);
static int expand_ib_tbl(int delay);
static int expand_line_buffer(char **adrBuf, int *ptrSize, char **adrPtr);
static int reopen_real_time_input(GraceApp *gr, Input_buffer *ib);
static int read_real_time_lines(Input_buffer *ib);
static int process_complete_lines(GraceApp *gapp, Input_buffer *ib);

static int read_long_line(FILE *fp, char **linebuf, int *buflen);

/*
 * part of the time sliced already spent in milliseconds
 */
static int time_spent(void)
{
    struct timeval now;

    gettimeofday(&now, NULL);

    return 1000 * (now.tv_sec - read_begin.tv_sec)
        + (now.tv_usec - read_begin.tv_usec) / 1000;

}


/*
 * expand the table of monitored real time inputs
 */
static int expand_ib_tbl(int delay)
{
    int i, new_size;
    Input_buffer *new_tbl;

    new_size = (ib_tblsize > 0) ? 2*ib_tblsize : 5;
    new_tbl  = xcalloc(new_size, sizeof(Input_buffer));
    if (new_tbl == NULL) {
        return RETURN_FAILURE;
    }

    for (i = 0; i < new_size; i++) {
        new_tbl[i] = (i < ib_tblsize) ? ib_tbl[i] : dummy_ib;
        new_tbl[i].delay = delay;
    }

    if (ib_tblsize > 0) {
        xfree((void *) ib_tbl);
    }
    ib_tbl  = new_tbl;
    ib_tblsize = new_size;

    return RETURN_SUCCESS;
}


/*
 * expand a line buffer
 */
static int expand_line_buffer(char **adrBuf, int *ptrSize, char **adrPtr)
{
    char *newbuf;
    int   newsize;

    newsize = *ptrSize + CHUNKSIZE;
    newbuf = xmalloc(newsize);
    if (newbuf == 0) {
        return RETURN_FAILURE;
    }

    if (*ptrSize == 0) {
        /* this is the first time through */
        if (adrPtr) {
            *adrPtr = newbuf;
        }
    } else {
        /* we are expanding an existing line */
        strncpy(newbuf, *adrBuf, *ptrSize);
        if (adrPtr) {
            *adrPtr += newbuf - *adrBuf;
        }
        xfree(*adrBuf);
    }

    *adrBuf  = newbuf;
    *ptrSize = newsize;    

    return RETURN_SUCCESS;
}


/*
 * reopen an Input_buffer (surely a fifo)
 */
static int reopen_real_time_input(GraceApp *gapp, Input_buffer *ib)
{
    int fd;
    char buf[256];

    /* in order to avoid race conditions (broken pipe on the write
       side), we open a new file descriptor before closing the
       existing one */
    fd = open(ib->name, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        sprintf(buf, "Can't reopen real time input %s", ib->name);
        errmsg(buf);
        unregister_real_time_input(ib->name);
        return RETURN_FAILURE;
    }

#ifndef NONE_GUI
    xunregister_rti(ib);
#endif

    /* swapping the file descriptors */
    close(ib->fd);
    ib->fd = fd;

#ifndef NONE_GUI
    xregister_rti(ib);
#endif

    return RETURN_SUCCESS;

}


/*
 * unregister a file descriptor no longer monitored
 * (since Input_buffer structures dedicated to static inputs
 *  are not kept in the table, it is not an error to unregister
 *  an input not already registered)
 */
void unregister_real_time_input(const char *name)
{
    Input_buffer *ib;
    int           l1, l2;

    l1 = strlen(name);

    nb_rt = 0;
    for (ib = ib_tbl; ib < ib_tbl + ib_tblsize; ib++) {
        l2 = (ib->name == NULL) ? -1 : strlen(ib->name);
        if (l1 == l2 && strcmp (name, ib->name) == 0) {
            /* name is usually the same pointer as ib->name so we cannot */
            /* free the string and output the message using name afterwards */
#ifndef NONE_GUI
            xunregister_rti(ib);
#endif
            close(ib->fd);
            ib->fd = -1;
            xfree(ib->name);
            ib->name = NULL;
        } else 
        if (l2 > 0) {
            /* this descriptor (if not dummy!) is still in use */
            nb_rt++;
        }
    }
}

/*
 * register a file descriptor for monitoring
 */
int register_real_time_input(GraceApp *gapp, int fd, const char *name, int reopen)
{
    Input_buffer *ib;
    char buf[256];

    /* some safety checks */
    if (fd < 0) {
        sprintf(buf, "%s : internal error, wrong file descriptor", name);
        errmsg(buf);
        return RETURN_FAILURE;
    }

#ifdef HAVE_FCNTL
    if (fcntl(fd, F_GETFL) & O_WRONLY) {
        fprintf(stderr,
                "Descriptor %d not open for reading\n",
                fd);
        return RETURN_FAILURE;
    }
#endif

    /* remove previous entry for the same set if any */
    unregister_real_time_input(name);

    /* find an empty slot in the table */
    for (ib = ib_tbl; ib < ib_tbl + ib_tblsize; ib++) {
        if (ib->fd == fd) {
            sprintf(buf, "%s : internal error, file descriptor already in use",
                    name);
            errmsg(buf);
            return RETURN_FAILURE;
        } else if (ib->fd < 0) {
            break;
        }
    }

    if (ib == ib_tbl + ib_tblsize) {
        /* the table was full, we expand it */
        int old_size = ib_tblsize;
        if (expand_ib_tbl(gapp->rt->timer_delay) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
        ib = ib_tbl + old_size;
    }

    /* we keep the current buffer (even if 0),
       and only say everything is available */
    ib->fd     = fd;
    ib->errors = 0;
    ib->lineno = 0;
    ib->zeros  = 0;
    ib->reopen = reopen;
    ib->name   = copy_string(ib->name, name);
    ib->used   = 0;
#ifndef NONE_GUI
    xregister_rti (ib);
#endif

    nb_rt++;

    return RETURN_SUCCESS;
}

/*
 * read a real-time line (but do not process it)
 */
static int read_real_time_lines(Input_buffer *ib)
{
    char *cursor;
    int   available, nbread;
    char buf[256];

    cursor     = ib->buf  + ib->used;
    available  = ib->size - ib->used;

    /* have we enough space to store the characters ? */
    if (available < 2) {
        if (expand_line_buffer(&(ib->buf), &(ib->size), &cursor)
            != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
        available = ib->buf + ib->size - cursor;
    }

    /* read as much as possible */
    nbread = read(ib->fd, (void *) cursor, available - 1);

    if (nbread < 0) {
        sprintf(buf, "%s : read error on real time input",
                ib->name);
        errmsg(buf);
        return RETURN_FAILURE;
    } else {
        if (nbread == 0) {
            ib->zeros++;
        } else {
            ib->zeros = 0;
            ib->used += nbread;
            ib->buf[ib->used] = '\0';
        }
    }

    return RETURN_SUCCESS;
}


/*
 * process complete lines that have already been read
 */
static int process_complete_lines(GraceApp *gapp, Input_buffer *ib)
{
    int line_corrupted;
    char *begin_of_line, *end_of_line;
    char buf[256];

    if (ib->used <= 0) {
        return RETURN_SUCCESS;
    }

    end_of_line = NULL;
    do {
        /* loop over the embedded lines */
        begin_of_line  = (end_of_line == NULL) ? ib->buf : (end_of_line + 1);
        end_of_line    = begin_of_line;
        line_corrupted = 0;
        while (end_of_line != NULL && *end_of_line != '\n') {
            /* trying to find a complete line */
            if (end_of_line == ib->buf + ib->used) {
                end_of_line = NULL;
            } else {
                if (*end_of_line == '\0') {
                    line_corrupted = 1;
                }
                ++end_of_line;
            }
        }

        if (end_of_line != NULL) {
            /* we have a whole line */

            ++(ib->lineno);
            *end_of_line = '\0';
            close_input = NULL;

            if (line_corrupted ||
                graal_parse_line(grace_get_graal(gapp->grace),
                    begin_of_line, gproject_get_top(gapp->gp)) != RETURN_SUCCESS) {
                sprintf(buf, "Error at line %d", ib->lineno);
                errmsg(buf);
                ++(ib->errors);
                if (ib->errors > MAXERR) {

#ifndef NONE_GUI
                    /* this prevents from being called recursively by
                       the inner X loop of yesno */
                    xunregister_rti(ib);
#endif
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        close_input = copy_string(close_input, "");
                    }
#ifndef NONE_GUI
                    xregister_rti(ib);
#endif
                    ib->errors = 0;

                }
            }

            if (close_input != NULL) {
                /* something should be closed */
                if (close_input[0] == '\0') {
                    unregister_real_time_input(ib->name);
                } else {
                    unregister_real_time_input(close_input);
                }

                xfree(close_input);
                close_input = NULL;

                if (ib->fd < 0) {
                    /* we have closed ourselves */
                    return RETURN_SUCCESS;
                }

            }

        }

    } while (end_of_line != NULL);

    if (end_of_line != NULL) {
        /* the line has just been processed */
        begin_of_line = end_of_line + 1;
    }

    if (begin_of_line > ib->buf) {
        /* move the remaining data to the beginning */
        ib->used -= begin_of_line - ib->buf;
        memmove(ib->buf, begin_of_line, ib->used);
        ib->buf[ib->used] = '\0';

    }

    return RETURN_SUCCESS;

}

int real_time_under_monitoring(void)
{
    return nb_rt > 0;
}

/*
 * monitor the set of registered file descriptors for pending input
 */
int monitor_input(GraceApp *gapp, Input_buffer *tbl, int tblsize, int no_wait)
{

    Input_buffer *ib;
    fd_set rfds;
    int remaining;
    struct timeval timeout;
    int highest, first_time, retsel;

    /* we don't want to get stuck here, we memorize the start date
       and will check we do not exceed our allowed time slice */
    gettimeofday(&read_begin, NULL);
    first_time    = 1;
    retsel        = 1;
    while (((time_spent() < tbl->delay) || first_time) && retsel > 0) {

        /* register all the monitored descriptors */
        highest = -1;
        FD_ZERO(&rfds);
        for (ib = tbl; ib < tbl + tblsize; ib++) {
            if (ib->fd >= 0) {
                FD_SET(ib->fd, &rfds);
                if (ib->fd > highest) {
                    highest = ib->fd;
                }
            }
        }

        if (highest < 0) {
            /* there's nothing to do */
            return RETURN_SUCCESS;
        }

        if (no_wait) {
            /* just check for available data without waiting */
            remaining = 0;
        } else {
            /* wait until data or end of time slice arrive */
            remaining = tbl->delay - time_spent();
            if (remaining < 0) {
                remaining = 0;
            }
        }
        timeout.tv_sec = remaining / 1000;
        timeout.tv_usec = 1000l * (remaining % 1000);
        retsel = select(highest + 1, &rfds, NULL, NULL, &timeout);

        for (ib = tbl;
             ((time_spent() < tbl->delay) || first_time) && ib < tbl + tblsize;
             ib++) {
            if (ib->fd >= 0 && FD_ISSET(ib->fd, &rfds)) {
                /* there is pending input */
                if (read_real_time_lines(ib) != RETURN_SUCCESS
                    || process_complete_lines(gapp, ib) != RETURN_SUCCESS) {
                    return RETURN_FAILURE;
                }

                if (ib->zeros >= 5) {
                    /* we were told five times something happened, but
                       never got any byte : we assume the pipe (or
                       whatever) has been closed by the peer */
                    if (ib->reopen) {
                        /* we should reset the input buffer, in case
                           the peer also reopens it */
                        if (reopen_real_time_input(gapp, ib) != RETURN_SUCCESS) {
                            return RETURN_FAILURE;
                        }
                    } else {
                        unregister_real_time_input(ib->name);
                    }

                    /* we have changed the table, we should end the loop */
                    break;
                }
            }
        }

        /* after one pass, we obey timeout */
        first_time = 0;
    }

    return RETURN_SUCCESS;
}

/*
 * read a line increasing buffer as necessary
 */
static int read_long_line(FILE * fp, char **linebuf, int *buflen)
{
    char *cursor;
    int  available;
    int  nbread, retval;

    cursor    = *linebuf;
    available = *buflen;
    retval    = RETURN_FAILURE;
    do {
        /* do we have enough space to store the characters ? */
        if (available < 2) {
            if (expand_line_buffer(linebuf, buflen, &cursor)
                != RETURN_SUCCESS) {
                return RETURN_FAILURE;
            }
        }
        available = (int)(*linebuf-cursor) + *buflen;

        /* read as much as possible */
        if (grace_fgets(cursor, available, fp) == NULL) {
            return retval;
        }
        nbread = strlen(cursor);
        if (nbread < 1) {
            return retval;
        } else {
            retval = RETURN_SUCCESS;
        }

        /* prepare next read */
        cursor    += nbread;
        available -= nbread;

    } while (*(cursor - 1) != '\n');

    return retval;
}


/* open a file for write */
FILE *gapp_openw(GraceApp *gapp, const char *fn)
{
    struct stat statb;
    char buf[GR_MAXPATHLEN + 50];
    FILE *retval;

    if (!fn || !fn[0]) {
        errmsg("No file name given");
	return NULL;
    } else if (strcmp(fn, "-") == 0 || strcmp(fn, "stdout") == 0) {
        return stdout;
    } else {
        if (stat(fn, &statb) == 0) {
            /* check to make sure this is a file and not a dir */
            if (S_ISREG(statb.st_mode)) {
	        sprintf(buf, "Overwrite %s?", fn);
	        if (!yesno(buf, NULL, NULL, NULL)) {
	            return NULL;
	        }
            } else {
                sprintf(buf, "%s is not a regular file!", fn);
                errmsg(buf);
	        return NULL;
            }
        }
        retval = filter_write(gapp, fn);
        if (!retval) {
	    sprintf(buf, "Can't write to file %s, check permissions!", fn);
            errmsg(buf);
        }
        return retval;
    }
}

char *gapp_exe_path(GraceApp *gapp, const char *fn)
{
    static char buf[GR_MAXPATHLEN], *epath;
    char *cp;
    
    if (fn == NULL) {
        return NULL;
    } else {
        cp = strchr(fn, ' ');
        if (cp == NULL) {
            epath = grace_path(gapp->grace, fn);
            strcpy(buf, exe_path_translate(epath));
            xfree(epath);
            return buf;
        } else {
            strcpy(buf, fn);
            buf[cp - fn] = '\0';
            epath = grace_path(gapp->grace, buf);
            strcpy(buf, epath);
            xfree(epath);
            strcat(buf, " ");
            strcat(buf, cp);
            return exe_path_translate(buf);
        }
    }
}

/* open a file for read */
FILE *gapp_openr(GraceApp *gapp, const char *fn, int src)
{
    struct stat statb;
    char *tfn;
    char buf[GR_MAXPATHLEN + 50];
    FILE *fp;

    if (!fn || !fn[0]) {
        errmsg("No file name given");
	return NULL;
    }
    switch (src) {
    case SOURCE_DISK:
        tfn = grace_path(gapp->grace, fn);
	if (strcmp(tfn, "-") == 0 || strcmp(tfn, "stdin") == 0) {
            xfree(tfn);
            return stdin;
	} else if (stat(tfn, &statb)) {
            sprintf(buf, "Can't stat file %s", tfn);
            errmsg(buf);
            xfree(tfn);
	    return NULL;
	/* check to make sure this is a file and not a dir */
	} else if (!S_ISREG(statb.st_mode)) {
            sprintf(buf, "%s is not a regular file", tfn);
            errmsg(buf);
            xfree(tfn);
	    return NULL;
        } else {
            fp = filter_read(gapp, tfn);
            xfree(tfn);
            return fp;
	}
        break;
    case SOURCE_PIPE:
        tfn = gapp_exe_path(gapp, fn);
	fp = popen(tfn, "r");
        xfree(tfn);
        return fp;
	break;
    default:
        errmsg("Wrong call to gapp_openr()");
	return NULL;
    }
}

/*
 * close either a pipe or a file pointer
 *
 */
void gapp_close(FILE *fp)
{
    if (fp == stdin || fp == stderr || fp == stdout) {
        return;
    }
    if (pclose(fp) == -1) {
        fclose(fp);
    }
}

FILE *gapp_tmpfile(char *templateval)
{
    FILE *fp;
#if defined(HAVE_MKSTEMP) && defined(HAVE_FDOPEN)
    int fd;
    
    fd = mkstemp(templateval);
    if (fd < 0) {
        fp = NULL;
    } else {
        fp = fdopen(fd, "wb");
    }
#else
    tmpnam(templateval);
    fp = fopen(templateval, "wb");
#endif

    if (!fp) {
        errmsg("Can't open temporary file");
    }
    
    return fp;
}


int uniread(Quark *pr, FILE *fp,
    DataParser parse_cb, DataStore store_cb, void *udata)
{
    int nrows, nrows_allocated;
    int ok, readerror;
    Quark *q = NULL;
    char *linebuf = NULL;
    int linebuflen = 0;
    int linecount;

    linecount = 0;
    readerror = 0;
    nrows = 0;
    nrows_allocated = 0;
    
    ok = TRUE;
    while (ok) {
        char *s;
	int maybe_data;
        int ncols, nncols, nscols;
        int *formats;
        
        if (read_long_line(fp, &linebuf, &linebuflen) == RETURN_SUCCESS) {
            linecount++;
            s = linebuf;

	    /* skip leading whitespaces */
            while (*s == ' ' || *s == '\t') {
                s++;
            }

	    /* skip comments */
            if (*s == '#') {
                continue;
            }

            /*     EOL           EOD    */
            if (*s == '\n' || *s == '\0') {
                maybe_data = FALSE;
            } else
            if (parse_cb && parse_cb(s, udata) == RETURN_SUCCESS) {
                maybe_data = FALSE;
            } else {
                maybe_data = TRUE;
            }
        } else {
            ok = FALSE;
            maybe_data = FALSE;
        }
        
        if (maybe_data) {
	    if (!nrows) {
		/* parse the data line */
                if (parse_ss_row(pr, s, &nncols, &nscols, &formats) != RETURN_SUCCESS) {
		    errmsg("Can't parse data");
		    xfree(linebuf);
		    return RETURN_FAILURE;
                }
                ncols = nncols + nscols;

                /* init the SSD */
                q = gapp_ssd_new(pr);
                if (!q || ssd_set_ncols(q, ncols, formats) != RETURN_SUCCESS) {
		    errmsg("Malloc failed in uniread()");
		    quark_free(q);
                    xfree(formats);
		    xfree(linebuf);
                    return RETURN_FAILURE;
                }
                xfree(formats);
	    }
            
	    if (nrows >= nrows_allocated) {
		if (!nrows_allocated) {
                    nrows_allocated = BUFSIZE;
                } else {
                    nrows_allocated *= 2;
                }
                if (ssd_set_nrows(q, nrows_allocated) != RETURN_SUCCESS) {
		    errmsg("Malloc failed in uniread()");
                    quark_free(q);
		    xfree(linebuf);
		    return RETURN_FAILURE;
                }
	    }

            if (insert_data_row(q, nrows, s) != RETURN_SUCCESS) {
                char tbuf[128];
                
                sprintf(tbuf, "Error parsing line %d, skipped", linecount);
                errmsg(tbuf);
                readerror++;
                if (readerror > MAXERR) {
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        quark_free(q);
		        xfree(linebuf);
                        return RETURN_FAILURE;
                    } else {
                        readerror = 0;
                    }
                }
            } else {
	        nrows++;
            }
	} else
        if (nrows) {
            /* free excessive storage */
            ssd_set_nrows(q, nrows);

            /* store accumulated data */
            if (store_cb && store_cb(q, udata) != RETURN_SUCCESS) {
		quark_free(q);
                xfree(linebuf);
                return RETURN_FAILURE;
            }

            /* reset state registers */
            nrows = 0;
            nrows_allocated = 0;
            readerror = 0;
        }
    }

    xfree(linebuf);
    
    return RETURN_SUCCESS;
}


typedef struct {
    char *label;
    int load_type;
    int settype;
} ascii_data;

static int store_cb(Quark *q, void *udata)
{
    ascii_data *adata = (ascii_data *) udata;

    /* label the SSD */
    quark_idstr_set(q, adata->label);

    return store_data(q, adata->load_type, adata->settype);
}

int getdata(Quark *gr, char *fn, int settype, int load_type)
{
    FILE *fp;
    int retval;
    GraceApp *gapp = gapp_from_quark(gr);
    ascii_data adata;
    
    fp = gapp_openr(gapp, fn, SOURCE_DISK);
    if (fp == NULL) {
	return RETURN_FAILURE;
    }

    adata.label = fn;
    adata.load_type = load_type;
    adata.settype = settype;
    
    retval = uniread(gr, fp, NULL, store_cb, &adata);

    gapp_close(fp);
    
    if (load_type != LOAD_BLOCK) {
        autoscale_graph(gr, gapp->rt->autoscale_onread);
    }

    return retval;
}


int write_ssd(const Quark *ssd, unsigned int ncols, const int *cols, FILE *fp)
{
    char *sep = "\t";
    unsigned int nrows = ssd_get_nrows(ssd), i, j;

    unsigned int prec = project_get_prec(get_parent_project(ssd));

    fputs("# ", fp);
    for (j = 0; j < ncols; j++) {
        char *lab = ssd_get_col_label(ssd, cols[j]);
        if (j != 0) {
            fputs(sep, fp);
        }
        fputs(lab ? lab:"?", fp);
    }
    fputs("\n", fp);

    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++) {
            unsigned int col = cols[j];
            ss_column *scol = ssd_get_col(ssd, col);
            if (!scol) {
                return RETURN_FAILURE;
            }
            
            if (j != 0) {
                fputs(sep, fp);
            }
            
            if (scol->format == FFORMAT_STRING) {
                char **s = ((char **) scol->data);
                fprintf(fp, " \"%s\"", escapequotes(s[i]));
            } else {
                double *x = ((double *) scol->data);
                fprintf(fp, "%.*g", prec, x[i]);
            }
        }
        fputs("\n", fp);
    }
    fputs("\n", fp);
    return RETURN_SUCCESS;
}

#if 0
/*
 * read data to the set from a file overriding the current contents
 */
int update_set_from_file(Quark *pset)
{
    int retval;
    Dataset *dsp;
    
    dsp = set_get_dataset(pset);
    
    if (!dsp) {
        retval = RETURN_FAILURE;
    } else {
        FILE *fp;
        RunTime *rt = rt_from_quark(pset);
        
        fp = gapp_openr(gapp_from_quark(pset), dsp->hotfile, dsp->hotsrc);
        
        killsetdata(pset);
        rt->curtype = set_get_type(pset);
        retval = uniread(get_parent_project(pset), fp, LOAD_SINGLE, dsp->hotfile);

        gapp_close(fp);
    }
    
    return retval;
}
#endif

static int project_cb(Quark *pr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        GraceApp *gapp = gapp_from_quark(pr);
        if (pr == gproject_get_top(gapp->gp)) {
            gapp->gp = NULL;
        }
    } else
    if (etype == QUARK_ETYPE_MODIFY) {
#if 0
        /* TODO: */
	if ((dirtystate > SOME_LIMIT) || 
            (current_time - autosave_time > ANOTHER_LIMIT) ) {
	    autosave();
	}
#endif
    }
#ifndef NONE_GUI
    clean_graph_selectors(pr, etype, data);
    clean_frame_selectors(pr, etype, data);
#endif
    return RETURN_SUCCESS;
}


static int project_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    switch (quark_fid_get(q)) {
    case QFlavorProject:
        quark_cb_add(q, project_cb, NULL);
        break;
    case QFlavorSSD:
        quark_cb_add(q, kill_ssd_cb, NULL);
        break;
    }
    
    return TRUE;
}

GProject *load_any_project(GraceApp *gapp, const char *fn)
{
    GProject *gp;
    
    /* FIXME: A temporary hack */
    if (fn && strstr(fn, ".xgr")) {
        gp = load_xgr_project(gapp, fn);
    } else {
        gp = load_agr_project(gapp, fn);
    }

    if (gp) {
        quark_traverse(gproject_get_top(gp), project_hook, NULL);
    }

    return gp;
}

static int load_project_file(GraceApp *gapp, const char *fn, int as_template)
{    
    GProject *gp;
    Quark *project, *gr, **graphs;
    int i, ngraphs;
    AMem *amem;

    if (gapp->gp && gproject_get_top(gapp->gp) &&
        quark_dirtystate_get(gproject_get_top(gapp->gp)) &&
        !yesno("Abandon unsaved changes?", NULL, NULL, NULL)) {
        return RETURN_FAILURE;
    }
    
    gp = load_any_project(gapp, fn);
    if (!gp) {
        errmsg("Failed loading project file");
        return RETURN_FAILURE;
    }
    
    project = gproject_get_top(gp);

    gapp->rt->print_file[0] = '\0';

    gapp_set_project(gapp, gp);

    if (as_template) {
        grfile_free(gp->grf);
        gp->grf = NULL;
    }

    amem = quark_get_amem(project);

    /* Set undo limit of 16MB */
    amem_set_undo_limit(amem, 0x1000000L);
    /* Get initial memory snapshot */
    amem_snapshot(amem);

    /* try to switch to the first active graph */
    ngraphs = project_get_graphs(project, &graphs);
    for (i = 0; i < ngraphs; i++) {
        gr = graphs[i];
        if (select_graph(gr) == RETURN_SUCCESS) {
            break;
        }
    }
    xfree(graphs);

#ifndef NONE_GUI
    update_all();
#endif
    if (project) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int load_project(GraceApp *gapp, char *fn)
{
    return load_project_file(gapp, fn, FALSE);
}

int new_project(GraceApp *gapp, char *template)
{
    int retval;
    char *s;
    
    if (string_is_empty(template)) {
        retval = load_project_file(gapp, "templates/Default.xgr", TRUE);
    } else if (template[0] == '/') {
        retval = load_project_file(gapp, template, TRUE);
    } else {
        s = xmalloc(strlen("templates/") + strlen(template) + 1);
        if (s == NULL) {
            retval = RETURN_FAILURE;
        } else {
            sprintf(s, "templates/%s", template);
            retval = load_project_file(gapp, s, TRUE);
            xfree(s);
        }
    }
    
    return retval;
}


int save_project(GProject *gp, char *fn)
{
    GrFILE *grf;
    Quark *project = gproject_get_top(gp);
    GUI *gui = gui_from_quark(project);
    int noask_save;
    static int save_unsupported = FALSE;
    int retval;

    if (!project || !fn) {
        return RETURN_FAILURE;
    }
    
    if (fn && strstr(fn, ".agr")) {
        errmsg("Cowardly refusing to overwrite an agr file");
        return RETURN_FAILURE;
    }
    if (!save_unsupported &&
        !yesno("The current format may be unsupported by the final release. Continue?",
            "Yeah, I'm brave!", NULL, "doc/UsersGuide.html#unsupported_format")) {
        return RETURN_FAILURE;
    }
    save_unsupported = TRUE;

    noask_save = gui->noask;
    if (strings_are_equal(gproject_get_docname(gp), fn)) {
        /* If saving under the same name, don't warn about overwriting */
        gui->noask = TRUE;
    }
    
    grf = grfile_openw(fn);
    if (!grf) {
        return RETURN_FAILURE;
    }
    
    gui->noask = noask_save;

    retval = gproject_save(gp, grf);

    grfile_free(grf);
    
    return retval;
}

GProject *load_xgr_project(GraceApp *gapp, const char *fn)
{
    GrFILE *grf;
    GProject *gp;
    char *epath;
    
    epath = grace_path(gapp->grace, fn);
    grf = grfile_openr(epath);
    xfree(epath);
    if (grf == NULL) {
        return NULL;
    }
    
    gp = gproject_load(gapp->grace, grf, AMEM_MODEL_LIBUNDO);

    grfile_free(grf);
    
    return gp;
}   

#ifdef HAVE_NETCDF
				
/*  
 * Read scaling attribute such ass add_offset or scale_factor 
 * with default value
*/

double read_scale_attr(int cdfid, int varid, char *attrname, double default_value) {
  double attribute;
  int status;
  nc_type attr_type;
  size_t length;
  status =  nc_inq_att(cdfid, varid, attrname, &attr_type, &length);
  if((status!=NC_NOERR) || (length != 1)) {
    return default_value;
  }
  status= nc_get_att_double (cdfid, varid, attrname, &attribute); 
  if(status!=NC_NOERR) {
    return default_value;
  }
  return attribute;
}


/*
 * read a variable from netcdf file into a set in graph gr
 * xvar and yvar are the names for x, y in the netcdf file resp.
 * return 0 on fail, return 1 if success.
 *
 * if xvar == NULL, then load the index of the point to x
 *
 */
int readnetcdf(Quark *pset,
	       char *netcdfname,
	       char *xvar,
	       char *yvar,
	       int nstart,
	       int nstop,
	       int nstride)
{
    Quark *gr = get_parent_graph(pset);
    int cdfid;			/* netCDF id */
    int i, n;
    double *x, *y;
    float *xf, *yf;
    short *xs, *ys;
    long *xl, *yl;
    char *xb,*yb;
    char buf[256];

    /* variable ids */
    int x_id = -1, y_id;

    /* variable shapes */
    long start[2];
    long count[2];
    /* scaling attributes */
    double scale_factor, add_offset;

    nc_type xdatatype = 0;
    nc_type ydatatype = 0;
    int xndims, xdim[10], xnatts;
    int yndims, ydim[10], ynatts;
    long nx, ny;

    RunTime *rt = rt_from_quark(pset);
    
    ncopts = 0;			/* no crash on error */

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
    x = xcalloc(n, SIZEOF_DOUBLE);
    y = xcalloc(n, SIZEOF_DOUBLE);
    if (x == NULL || y == NULL) {
	XCFREE(x);
	XCFREE(y);
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
      scale_factor = read_scale_attr(cdfid,x_id, "scale_factor", 1.0);
      add_offset = read_scale_attr(cdfid,x_id, "add_offset", 0.0);
/* TODO should check for other data types here */
/* TODO should check for NULL on the xcallocs() */
/* TODO making assumptions about the sizes of shorts and longs */
	switch (xdatatype) {
	case NC_BYTE:
	    xb = xcalloc(n, 1);	
	    /* Theoretically I should add macro to autoconf
	     file to figure out sizeof(byte) 
	    */
	    ncvarget(cdfid, x_id, start, count, (void *) xb);
	    for (i = 0; i < n; i++) {
		x[i] = scale_factor*xb[i]+add_offset;
	    }
	    xfree(xb);
	    break;
	case NC_SHORT:
	    xs = xcalloc(n, SIZEOF_SHORT);
	    ncvarget(cdfid, x_id, start, count, (void *) xs);
	    for (i = 0; i < n; i++) {
		x[i] = x[i] = scale_factor*xs[i]+add_offset;
	    }
	    xfree(xs);
	    break;
	case NC_LONG:
	    xl = xcalloc(n, SIZEOF_LONG);
	    ncvarget(cdfid, x_id, start, count, (void *) xl);
	    for (i = 0; i < n; i++) {
		x[i] = scale_factor*xl[i]+add_offset;
	    }
	    xfree(xl);
	    break;
	case NC_FLOAT:
	    xf = xcalloc(n, SIZEOF_FLOAT);
	    ncvarget(cdfid, x_id, start, count, (void *) xf);
	    for (i = 0; i < n; i++) {
		x[i] = scale_factor*xf[i]+add_offset;
	    }
	    xfree(xf);
	    break;
	case NC_DOUBLE:
	    ncvarget(cdfid, x_id, start, count, (void *) x);
	    for (i = 0; i < n; i++) {
		x[i] = scale_factor*x[i]+add_offset;
	    }
	    break;
	default:
	    errmsg("Data type not supported");
	    XCFREE(x);
	    XCFREE(y);
	    ncclose(cdfid);
	    return 0;
	    break;
	}
    } else {			/* just load index */
	for (i = 0; i < n; i++) {
	    x[i] = i + 1;
	}
    }
    scale_factor = read_scale_attr(cdfid,y_id, "scale_factor", 1.0);
    add_offset = read_scale_attr(cdfid,y_id, "add_offset", 0.0);
    switch (ydatatype) {
    case NC_BYTE:
      yb = xcalloc(n, 1);	
      ncvarget(cdfid, y_id, start, count, (void *) yb);
      for (i = 0; i < n; i++) {
	y[i] = scale_factor*yb[i]+add_offset;
      }
      xfree(yb);
      break;

    case NC_SHORT:
	ys = xcalloc(n, SIZEOF_SHORT);
	ncvarget(cdfid, y_id, start, count, (void *) ys);
	for (i = 0; i < n; i++) {
	    y[i] = scale_factor*ys[i]+add_offset;
	}
	break;
    case NC_LONG:
	yl = xcalloc(n, SIZEOF_LONG);
	ncvarget(cdfid, y_id, start, count, (void *) yl);
	for (i = 0; i < n; i++) {
	    y[i] = scale_factor*yl[i]+add_offset;
	}
	break;
    case NC_FLOAT:
/* TODO should check for NULL here */
	yf = xcalloc(n, SIZEOF_FLOAT);
	ncvarget(cdfid, y_id, start, count, (void *) yf);
	for (i = 0; i < n; i++) {
	    y[i] = scale_factor*yf[i]+add_offset;
	}
	xfree(yf);
	break;
    case NC_DOUBLE:
	ncvarget(cdfid, y_id, start, count, (void *) y);
	for (i = 0; i < n; i++) {
	    y[i] = scale_factor*y[i]+add_offset;
	}
	break;
    default:
	errmsg("Data type not supported");
	XCFREE(x);
	XCFREE(y);
	ncclose(cdfid);
	return 0;
	break;
    }
    ncclose(cdfid);

/*
 * initialize stuff for the newly created set
 */
    /* TODO!!! */
    set_set_type(pset, SET_XY);
    sprintf(buf, "File %s x = %s y = %s", netcdfname, xvar == NULL ? "Index" : xvar, yvar);
#if 0
    set_set_col(pset, DATA_X, x, n);
    set_set_col(pset, DATA_Y, y, n);
    set_set_comment(pset, buf);
#endif

    xfree(x);
    xfree(y);
    
    autoscale_graph(gr, rt->autoscale_onread);
    
    return 1;
}

int write_netcdf(Quark *pr, char *fname)
{
    Quark **graphs;
    int i, ngraphs;
    Quark *gr; 
    char buf[512];
    int ncid;			/* netCDF id */
    int j;
    /* dimension ids */
    int n_dim;
    /* variable ids */
    int x_id, y_id;
    int dims[1];
    long len[1];
    long it = 0;
    double *x, *y, x1, x2, y1, y2;
    Grace *grace = grace_from_quark(pr);
    ncid = nccreate(fname, NC_CLOBBER);
    ncattput(ncid, NC_GLOBAL, "Contents", NC_CHAR, 11, (void *) "gapp sets");
    ngraphs = project_get_graphs(pr, &graphs);
    for (i = 0; i < ngraphs; i++) {
        gr = graphs[i];
        if (gr) {
            Quark **psets;
            int nsets = quark_get_descendant_sets(gr, &psets);
	    for (j = 0; j < nsets; j++) {
		Quark *pset = psets[j];
                if (!set_is_dataless(pset)) {
		    char s[64];

		    sprintf(buf, "type");
		    strcpy(s, set_types(grace, set_get_type(pset)));
		    ncattput(ncid, NC_GLOBAL, buf, NC_CHAR, strlen(s), (void *) s);

		    sprintf(buf, "n");
		    n_dim = ncdimdef(ncid, buf, set_get_length(pset));
		    dims[0] = n_dim;
		    set_get_minmax(pset, &x1, &x2, &y1, &y2);
		    sprintf(buf, "x");
		    x_id = ncvardef(ncid, buf, NC_DOUBLE, 1, dims);
		    ncattput(ncid, x_id, "min", NC_DOUBLE, 1, (void *) &x1);
		    ncattput(ncid, x_id, "max", NC_DOUBLE, 1, (void *) &x2);
		    dims[0] = n_dim;
		    sprintf(buf, "y");
		    y_id = ncvardef(ncid, buf, NC_DOUBLE, 1, dims);
		    ncattput(ncid, y_id, "min", NC_DOUBLE, 1, (void *) &y1);
		    ncattput(ncid, y_id, "max", NC_DOUBLE, 1, (void *) &y2);
		}
	    }
            xfree(psets);
	}
    }
    ncendef(ncid);
    ncclose(ncid);
    if ((ncid = ncopen(fname, NC_WRITE)) == -1) {
	errmsg("Can't open file.");
        xfree(graphs);
	return 1;
    }
    for (i = 0; i < ngraphs; i++) {
        gr = graphs[i];
	if (gr) {
            Quark **psets;
            int nsets = quark_get_descendant_sets(gr, &psets);
	    for (j = 0; j < nsets; j++) {
		Quark *pset = psets[j];
                if (!set_is_dataless(pset)) {
		    len[0] = set_get_length(pset);
		    x = getx(pset);
		    y = gety(pset);
		    sprintf(buf, "x");
		    x_id = ncvarid(ncid, buf);
		    sprintf(buf, "y");
		    y_id = ncvarid(ncid, buf);
		    ncvarput(ncid, x_id, &it, len, (void *) x);
		    ncvarput(ncid, y_id, &it, len, (void *) y);
		}
	    }
            xfree(psets);
	}
    }

    xfree(graphs);
    ncclose(ncid);
    return 0;
}

#endif				/* HAVE_NETCDF */
