/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 * misc utilities
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif
#include <limits.h>

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "dicts.h"
#include "protos.h"

static void rereadConfig(Grace *grace);
static RETSIGTYPE actOnSignal(int signo);

int isoneof(int c, char *s)
{
    while (*s) {
	if (c == *s) {
	    return 1;
	} else {
	    s++;
	}
    }
    return 0;
}

int argmatch(char *s1, char *s2, int atleast)
{
    int l1 = strlen(s1);
    int l2 = strlen(s2);

    if (l1 < atleast) {
	return 0;
    }
    if (l1 > l2) {
	return 0;
    }
    return (strncmp(s1, s2, l1) == 0);
}

/*
 * convert a string from lower to upper case
 * leaving quoted strings alone
 */
void lowtoupper(char *s)
{
    unsigned int i, quoteon = FALSE;

    for (i = 0; i < strlen(s); i++) {
	if (s[i] == '"') {
	    if (!quoteon) {
		quoteon = TRUE;
	    } else if ((i > 0) && (s[i-1] != '\\')) {
		quoteon = FALSE;
	    }
	}
	if (quoteon == FALSE) {
            if (!isprint(s[i])) {
                s[i] = ' ';
            } else if (s[i] >= 'a' && s[i] <= 'z') {
	        s[i] -= ' ';
	    }
        }
    }
}

/*
 * remove all that fortran nastiness
 */
void convertchar(char *s)
{
    while (*s++) {
	if (*s == ',')
	    *s = ' ';
	if (*s == 'D' || *s == 'd')
	    *s = 'e';
    }
}

/*
 * escape quotes
 */
char *escapequotes (char *s)
{
    static char *es = NULL;
    int i, k, n, len, elen;
    
    if (s == NULL)
        return NULL;
    
    len = strlen(s);
    es = xrealloc(es, (len + 1)*SIZEOF_CHAR);
    strcpy(es, s);
    n = 0;
    while ((es = strchr(es, '\"'))) {
    	es++;
    	n++;
    }
    
    elen = len + n + 1;
    es = xrealloc(es, elen*SIZEOF_CHAR);
    
    i = k = 0;
    while (i < len) {
        if (s[i] == '\"') {
            es[k] = '\\';
            k++;
        }
        es[k] = s[i];
        i++; k++;
    }
    es[elen-1] = '\0';
    return es;
}

double mytrunc(double a)
{
    if (a > 0.0) {
        return floor(a);
    } else {
        return ceil(a);
    }
}

/*
 * exit grace
 */
void bailout(Grace *grace)
{
    if ((grace->project && !quark_dirtystate_get(grace->project)) ||
        yesno("Exit losing unsaved changes?", NULL, NULL, NULL)) {
        grace_free(grace);
        exit(0);
    }
}

/*
 * Reread config (TODO)
 */
static void rereadConfig(Grace *grace)
{
}

static void please_report_the_bug(void)
{
    fprintf(stderr, "\nPlease use \"Help/Comments\" to report the bug.\n");
#ifdef HAVE_LESSTIF
    fprintf(stderr, "NB: This version of Grace was compiled with LessTif.\n");
    fprintf(stderr, "    Make sure to read the FAQ carefully prior to\n");
    fprintf(stderr, "    reporting the bug, ESPECIALLY if the problem might\n");
    fprintf(stderr, "    be related to the graphical interface.\n");
#endif
}

/*
 * Warn about a possible bug displaying the passed message, try to save
 * any unsaved work and abort
 */
void emergency_exit(Grace *grace, int is_my_bug, char *msg)
{
/*
 *  Since we got so far, memory is probably corrupted so it's better to use
 *  a static storage
 */
    static char buf[GR_MAXPATHLEN];
    
    if (grace->rt->emergency_save != FALSE) {
        /* don't mind signals anymore: we're in emergency save mode already */
        grace->rt->interrupts++;
        if (grace->rt->interrupts > 10) {
            fprintf(stderr, "oh, no luck :-(\n");
            please_report_the_bug();
            abort();
        }
        return;
    } else {
        grace->rt->emergency_save = TRUE;
        grace->rt->interrupts = 0;
        fprintf(stderr, "\a\nOops! %s\n", msg);
        if (quark_dirtystate_get(grace->project)) {
            strcpy(buf, project_get_docname(grace->project));
            strcat(buf, "$");
            fprintf(stderr, "Trying to save your work into file \"%s\"... ", buf);
            fflush(stderr);
            grace->gui->noask = TRUE;
            if (save_project(grace->project, buf) == RETURN_SUCCESS) {
                fprintf(stderr, "ok!\n");
            } else {
                fprintf(stderr, "oh, no luck :-(\n");
            }
        }
        if (is_my_bug) {
            please_report_the_bug();
        }
        abort();
    }
}

/*
 * Signal-handling routines
 */
 
static RETSIGTYPE actOnSignal(int signo)
{
    char *signame, buf[32];
     
    installSignal();
    
    switch (signo) {
#ifdef SIGHUP
    case SIGHUP:
    	rereadConfig(grace);
    	break;
#endif
#ifdef SIGINT
    case SIGINT:
#endif
#ifdef SIGQUIT
    case SIGQUIT:
#endif
#ifdef SIGTERM
    case SIGTERM:
#endif
        bailout(grace);
        break;
#ifdef SIGILL
    case SIGILL:
        signame = "SIGILL";
#endif
#ifdef SIGFPE
    case SIGFPE:
        signame = "SIGFPE";
#endif
#ifdef SIGBUS
    case SIGBUS:
        signame = "SIGBUS";
#endif
#ifdef SIGSEGV
    case SIGSEGV:
        signame = "SIGSEGV";
#endif
#ifdef SIGSYS
    case SIGSYS:
        signame = "SIGSYS";
#endif
        sprintf(buf, "Got fatal signal %s!", signame);
        emergency_exit(grace, TRUE, buf);
        break;
    default:
        /* ignore the rest */
        break;
    }
}

void installSignal(void){
#ifdef SIGHUP
    signal(SIGHUP,  actOnSignal);   /* hangup */
#endif
#ifdef SIGINT
    signal(SIGINT,  actOnSignal);   /* interrupt */
#endif
#ifdef SIGQUIT
    signal(SIGQUIT, actOnSignal);   /* quit */
#endif
#ifdef SIGILL
    signal(SIGILL,  actOnSignal);   /* illegal instruction */
#endif
#ifdef SIGFPE
    signal(SIGFPE,  actOnSignal);   /* floating point exception */
#endif
#ifdef SIGBUS
    signal(SIGBUS,  actOnSignal);   /* bus error */
#endif
#ifdef SIGSEGV
    signal(SIGSEGV, actOnSignal);   /* segmentation violation */
#endif
#ifdef SIGSYS
    signal(SIGSYS,  actOnSignal);   /* bad argument to system call */
#endif
#ifdef SIGTERM
    signal(SIGTERM, actOnSignal);   /* software termination signal */
#endif
#ifdef SIGALRM
    signal(SIGALRM, actOnSignal);   /* timer */
#endif
#ifdef SIGIO
    signal(SIGIO, actOnSignal);     /* input/output ready */
#endif
}

char *get_grace_home(const Grace *grace)
{
    return grace->rt->grace_home;
}

int get_print_dest(const Grace *grace)
{
    return grace->rt->print_dest;
}

void set_print_dest(Grace *grace, int dest)
{
    if (dest >= 0 && dest < grace->rt->num_print_dests) {
        grace->rt->print_dest = dest;
    }
}

char *get_print_cmd(const Grace *grace)
{
    return grace->rt->print_cmd;
}

void set_print_cmd(Grace *grace, const char *cmd)
{
    grace->rt->print_cmd = copy_string(grace->rt->print_cmd, cmd);
}

char *get_editor(const Grace *grace)
{
    return grace->rt->grace_editor;
}

void set_editor(Grace *grace, const char *cmd)
{
    grace->rt->grace_editor = copy_string(grace->rt->grace_editor, cmd);
}

char *get_help_viewer(const Grace *grace)
{
    return grace->rt->help_viewer;
}

void set_help_viewer(Grace *grace, const char *dir)
{
    grace->rt->help_viewer = copy_string(grace->rt->help_viewer, dir);
}

char *get_docbname(const Quark *q)
{
    return QIDSTR(q);
}


void errmsg(const char *buf)
{
#ifdef NONE_GUI
    fprintf(stderr, "%s\n", buf);
#else
    if (grace->gui->inwin) {
        errwin(buf);
    } else {
        fprintf(stderr, "%s\n", buf);
    }
#endif
}

int yesnoterm(char *msg)
{
    return TRUE;
}

int yesno(char *msg, char *s1, char *s2, char *help_anchor)
{
    if (grace->gui->noask) {
	return TRUE;
    }
#ifdef NONE_GUI
    return (yesnoterm(msg));
#else
    if (grace->gui->inwin) {
        return (yesnowin(msg, s1, s2, help_anchor));
    } else {
        return (yesnoterm(msg));
    }
#endif
}
 
void stufftext(char *s)
{
#ifdef NONE_GUI
    printf(s);
#else
    if (grace->gui->inwin) {
        stufftextwin(s);
    } else {
        printf(s);
    }
#endif
    /* log results to file */
    if (grace->rt->resfp != NULL) {
	fprintf(grace->rt->resfp, s);
    }
}


char *mybasename(const char *s)
{
    int start, end;
    static char basename[GR_MAXPATHLEN];
    
    if (!s) {
        return NULL;
    }
    
    s = path_translate(s);
    if (s == NULL) {
        errmsg("Could not translate basename:");
        return "???";
    }
    
    end = strlen(s) - 1;
    
    /* root is a special case */
    if (end == 0 && *s == '/'){
        basename[0] = '/';
        return basename;
    }

    /* strip trailing white space and slashes */
    while (s[end] == '/' || s[end] == ' ' || s[end] == '\t') {
        end--;
    }
    /* find start of basename */
    start = end;
    do {
        start--;
    } while (start >= 0 && s[start] != '/');

    strncpy(basename, s + (start + 1), end - start);
    basename[end - start] = '\0';
    return basename;
}

int set_workingdir(Grace *grace, const char *wd)
{
    char buf[GR_MAXPATHLEN];
    
    strncpy(buf, wd, GR_MAXPATHLEN - 1);
    if (buf[0] == '~') {
        expand_tilde(grace, buf);
    }
    if (chdir(buf) >= 0) {
        grace->rt->workingdir = copy_string(grace->rt->workingdir, buf);
        if (grace->rt->workingdir[strlen(grace->rt->workingdir) - 1] != '/') {
            grace->rt->workingdir = concat_strings(grace->rt->workingdir, "/");
        }
	return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *get_workingdir(const Grace *grace)
{
    return grace->rt->workingdir;
}

char *get_username(const Grace *grace)
{
    return grace->rt->username;
}

char *get_userhome(const Grace *grace)
{
    return grace->rt->userhome;
}

/* TODO this needs some work */
void expand_tilde(const Grace *grace, char *buf)
{
    char buf2[GR_MAXPATHLEN];

    if (buf[0] == '~') {
	if (strlen(buf) == 1) {
            strcpy(buf, get_userhome(grace));
	} else if (buf[1] == '/') {
            if (strlen(buf) > 2) {
                strcpy(buf2, get_userhome(grace));
	        strcat(buf2, buf + 1);
	        strcpy(buf, buf2);
            } else {
                strcpy(buf, get_userhome(grace));
            }
	} else {
	    char tmp[128], *pp = tmp, *q = buf + 1;
	    struct passwd *pent;

	    while (*q && (*q != '/')) {
		*pp++ = *q++;
	    }
	    *pp = 0;
	    if ((pent = getpwnam(tmp)) != NULL) {
		strcpy(buf2, pent->pw_dir);
		strcat(buf2, "/");
		strcat(buf2, q);
		strcpy(buf, buf2);
	    } else {
		errmsg("No user by that name");
	    }
	}
    }
}

void echomsg(char *msg)
{
    stufftext(msg);
    stufftext("\n");
}

int system_wrap(const char *string)
{
    return system(string);
}

void msleep_wrap(unsigned int msec)
{
    struct timeval timeout;
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = 1000 * (msec % 1000);
    select(0, NULL, NULL, NULL, &timeout);    
}

#ifdef DEBUG
void set_debuglevel(Grace *grace, int level)
{
    grace->rt->debuglevel = level;
}

int get_debuglevel(Grace *grace)
{
    return grace->rt->debuglevel;
}
#endif

char *q_labeling(Quark *q)
{
    RunTime *rt = rt_from_quark(q);
    char *buf;
    tickmarks *t;
    DObject *o;
    region *r;
    
    if (!q) {
        return NULL;
    }
    
    buf = xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }
    
    switch (quark_fid_get(q)) {
    case QFlavorProject:
        sprintf(buf, "Project \"%s%s\"", QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");

        break;
    case QFlavorSSD:
        sprintf(buf, "SpreadSheet \"%s%s\"", QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");

        break;
    case QFlavorFrame:
        sprintf(buf, "Frame \"%s%s\"", QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");

        break;
    case QFlavorGraph:
        sprintf(buf, "Graph \"%s%s\" (type: %s)",
            QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"",
            graph_types(rt, graph_get_type(q)));

        break;
    case QFlavorSet:
        sprintf(buf, "Set \"%s%s\" (%s)",
            QIDSTR(q), quark_dirtystate_get(q) ? "*":"",
            set_types(rt, set_get_type(q)));

        break;
    case QFlavorAGrid:
        t = axisgrid_get_data(q);
        
        sprintf(buf, "%c AGrid \"%s%s\"",
            t->type == AXIS_TYPE_X ? 'X':'Y', QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");

        break;
    case QFlavorAxis:
        t = axisgrid_get_data(q);
        
        sprintf(buf, "Axis \"%s%s\"", QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");

        break;
    case QFlavorDObject:
        o = object_get_data(q);

        sprintf(buf, "%s \"%s%s\"",
            object_type_descr(rt, o->type),
            QIDSTR(q), quark_dirtystate_get(q) ? "*":"");
        
        break;
    case QFlavorAText:
        sprintf(buf, "AText \"%s%s\"",
            QIDSTR(q), quark_dirtystate_get(q) ? "*":"");
        
        break;
    case QFlavorRegion:
        r = region_get_data(q);

        sprintf(buf, "Region \"%s%s\" (%d pts)",
            QIDSTR(q), quark_dirtystate_get(q) ? "*":"",
            r->n);
        
        break;
    default:
        sprintf(buf, "??? \"%s%s\"", QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");
        break;
    }
    
    return buf;
}
