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
#include "xprotos.h"

static void rereadConfig(GraceApp *gapp);
static RETSIGTYPE actOnSignal(int signo);

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
 * escape quotes
 */
char *escapequotes(char *s)
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

/*
 * exit gapp
 */
void bailout(GraceApp *gapp)
{
    if ((gapp->gp && !quark_dirtystate_get(gproject_get_top(gapp->gp))) ||
        yesno("Exit losing unsaved changes?", NULL, NULL, NULL)) {
        gapp_free(gapp);
#ifdef QT_GUI
	return;
#else
        exit(0);
#endif
    }
}

/*
 * Reread config (TODO)
 */
static void rereadConfig(GraceApp *gapp)
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
void emergency_exit(GraceApp *gapp, int is_my_bug, char *msg)
{
/*
 *  Since we got so far, memory is probably corrupted so it's better to use
 *  a static storage
 */
    static char buf[GR_MAXPATHLEN];
    
    if (gapp->rt->emergency_save != FALSE) {
        /* don't mind signals anymore: we're in emergency save mode already */
        gapp->rt->interrupts++;
        if (gapp->rt->interrupts > 10) {
            fprintf(stderr, "oh, no luck :-(\n");
            please_report_the_bug();
            abort();
        }
        return;
    } else {
        gapp->rt->emergency_save = TRUE;
        gapp->rt->interrupts = 0;
        fprintf(stderr, "\a\nOops! %s\n", msg);
        if (gapp->gp && quark_dirtystate_get(gproject_get_top(gapp->gp))) {
            strcpy(buf, gproject_get_docname(gapp->gp));
            strcat(buf, "$");
            fprintf(stderr, "Trying to save your work into file \"%s\"... ", buf);
            fflush(stderr);
            gapp->gui->noask = TRUE;
            if (save_project(gapp->gp, buf) == RETURN_SUCCESS) {
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
    	rereadConfig(gapp);
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
        bailout(gapp);
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
        emergency_exit(gapp, TRUE, buf);
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

int get_print_dest(const GraceApp *gapp)
{
    return gapp->rt->print_dest;
}

void set_print_dest(GraceApp *gapp, int dest)
{
    if (dest >= 0 && dest < gapp->rt->num_print_dests) {
        gapp->rt->print_dest = dest;
    }
}

char *get_print_cmd(const GraceApp *gapp)
{
    return gapp->rt->print_cmd;
}

void set_print_cmd(GraceApp *gapp, const char *cmd)
{
    gapp->rt->print_cmd = copy_string(gapp->rt->print_cmd, cmd);
}

char *get_editor(const GraceApp *gapp)
{
    return gapp->rt->gapp_editor;
}

void set_editor(GraceApp *gapp, const char *cmd)
{
    gapp->rt->gapp_editor = copy_string(gapp->rt->gapp_editor, cmd);
}

char *get_help_viewer(const GraceApp *gapp)
{
    return gapp->rt->help_viewer;
}

void set_help_viewer(GraceApp *gapp, const char *dir)
{
    gapp->rt->help_viewer = copy_string(gapp->rt->help_viewer, dir);
}


void set_date_hint(GraceApp *gapp, Dates_format preferred)
{
    gapp->rt->date_hint = preferred;
}

Dates_format get_date_hint(const GraceApp *gapp)
{
    return gapp->rt->date_hint;
}


void errmsg(const char *buf)
{
#ifdef NONE_GUI
    fprintf(stderr, "%s\n", buf);
#else
    if (gapp && gapp->gui && gapp->gui->inwin) {
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
    if (gapp->gui->noask) {
	return TRUE;
    }
#ifdef NONE_GUI
    return (yesnoterm(msg));
#else
    if (gapp->gui->inwin) {
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
    if (gapp->gui->inwin) {
        stufftextwin(s);
    } else {
        printf(s);
    }
#endif
    /* log results to file */
    if (gapp->rt->resfp != NULL) {
	fprintf(gapp->rt->resfp, s);
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

int set_workingdir(GraceApp *gapp, const char *wd)
{
    char buf[GR_MAXPATHLEN], *epath;
    int retval;
    
    epath = grace_path(gapp->grace, wd);
    
    if (chdir(epath) >= 0) {
        gapp->rt->workingdir = copy_string(gapp->rt->workingdir, buf);
        if (gapp->rt->workingdir[strlen(gapp->rt->workingdir) - 1] != '/') {
            gapp->rt->workingdir = concat_strings(gapp->rt->workingdir, "/");
        }
	retval = RETURN_SUCCESS;
    } else {
        retval = RETURN_FAILURE;
    }
    
    xfree(epath);
    
    return retval;
}

char *get_workingdir(const GraceApp *gapp)
{
    return gapp->rt->workingdir;
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

char *q_labeling(Quark *q)
{
    Grace *grace = grace_from_quark(q);
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
            graph_types(grace, graph_get_type(q)));

        break;
    case QFlavorSet:
        sprintf(buf, "Set \"%s%s\" (%s)",
            QIDSTR(q), quark_dirtystate_get(q) ? "*":"",
            set_types(grace, set_get_type(q)));

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
            object_type_descr(grace, o->type),
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
