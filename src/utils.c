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

#ifdef HAVE_SETLOCALE
#  include <locale.h>
#endif

#include "globals.h"
#include "utils.h"
#include "files.h"
#include "dicts.h"
#include "protos.h"

static void rereadConfig(Grace *grace);
static RETSIGTYPE actOnSignal(int signo);

/*
 * swap doubles and ints
 */
void fswap(double *x, double *y)
{
    double tmp;

    tmp = *x;
    *x = *y;
    *y = tmp;
}

void iswap(int *x, int *y)
{
    int tmp;

    tmp = *x;
    *x = *y;
    *y = tmp;
}

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

char *dayofweekstrs[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *dayofweekstrl[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char *monthl[] = {"January", "February", "March", "April", "May", "June",
"July", "August", "September", "October", "November", "December"};

int dayofweek(double j)
{
    int i = (int) floor(j + 1.5);
    return (i <= 0) ? 6 - (6 - i)%7 : i%7;
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

/* create format string */
char *create_fstring(const Quark *q, int form, int prec, double loc, int type)
{
    char format[64], eng_prefix[6];
    static char s[MAX_STRING_LENGTH];
    double tmp;
    int m, d, y, h, mm, sec;
    double arcmin, arcsec;
    int exponent;
    double mantissa;
    int yprec;
    Project *pr = project_get_data(q);
    
    if (pr->two_digits_years) {
        yprec = 2;
    } else {
        yprec = 4;
    }

    /* for locale decimal points */
    set_locale_num(TRUE);

    strcpy(format, "%.*lf");
    switch (form) {
    case FORMAT_DECIMAL:
	sprintf(s, format, prec, loc);
	tmp = atof(s);		/* fix reverse axes problem when loc == -0.0 */
	if (tmp == 0.0) {
	    strcpy(format, "%.*lf");
	    loc = 0.0;
	    sprintf(s, format, prec, loc);
	}
	break;
    case FORMAT_EXPONENTIAL:
	strcpy(format, "%.*le");
	sprintf(s, format, prec, loc);
	tmp = atof(s);		/* fix reverse axes problem when loc == -0.0 */
	if (tmp == 0.0) {
	    strcpy(format, "%.*le");
	    loc = 0.0;
	    sprintf(s, format, prec, loc);
	}
	break;
    case FORMAT_SCIENTIFIC:
	if (loc != 0.0) {
            exponent = (int) floor(log10(fabs(loc)));
            mantissa = loc/pow(10.0, (double) exponent);
            if (type == LFORMAT_TYPE_EXTENDED) {
	        strcpy(format, "%.*f\\x\\c4\\C\\f{}10\\S%d\\N");
	    } else {
	        strcpy(format, "%.*fx10(%d)");
            }
	    sprintf(s, format, prec, mantissa, exponent);
        } else {
	    strcpy(format, "%.*f");
	    sprintf(s, format, prec, 0.0);
        }
	break;
    case FORMAT_ENGINEERING:
	if (loc != 0.0) {
            exponent = (int) floor(log10(fabs(loc)));
            if (exponent < -18) {
                exponent = -18;
            } else if (exponent > 18) {
                exponent = 18;
            } else {
                exponent = (int) floor((double) exponent/3)*3;
            }
        } else {
            exponent = 0;
        }
        switch (exponent) {
        case -18: /* atto */
            strcpy(eng_prefix, "a");
            break;
        case -15: /* fempto */
            strcpy(eng_prefix, "f");
            break;
        case -12: /* pico */
            strcpy(eng_prefix, "p");
            break;
        case -9: /* nano */
            strcpy(eng_prefix, "n");
            break;
        case -6: /* micro */
            if (type == LFORMAT_TYPE_EXTENDED) {
                strcpy(eng_prefix, "\\xm\\f{}");
            } else {
                strcpy(eng_prefix, "mk");
            }
            break;
        case -3: /* milli */
            strcpy(eng_prefix, "m");
            break;
        case 3: /* kilo */
            strcpy(eng_prefix, "k");
            break;
        case 6: /* Mega */
            strcpy(eng_prefix, "M");
            break;
        case 9: /* Giga */
            strcpy(eng_prefix, "G");
            break;
        case 12: /* Tera */
            strcpy(eng_prefix, "T");
            break;
        case 15: /* Peta */
            strcpy(eng_prefix, "P");
            break;
        case 18: /* Exza (spelling?) */
            strcpy(eng_prefix, "E");
            break;
        default:
            strcpy(eng_prefix, "");
            break;
        }
	strcpy(format, "%.*f %s");
	sprintf(s, format, prec, loc/(pow(10.0, exponent)), eng_prefix);
	break;
    case FORMAT_POWER:
        if (loc < 0.0) {
            loc = log10(-loc);
            if (type == LFORMAT_TYPE_EXTENDED) {
                strcpy(format, "-10\\S%.*lf\\N");
            } else {
                strcpy(format, "-10(%.*lf)\\N");
            }
        } else if (loc == 0.0) {
            sprintf(format, "%.*f", prec, 0.0);
        } else {
            loc = log10(loc);
            if (type == LFORMAT_TYPE_EXTENDED) {
                strcpy(format, "10\\S%.*lf\\N");
            } else {
                strcpy(format, "10(%.*lf)\\N");
            }
        }
        sprintf(s, format, prec, loc);
        break;
    case FORMAT_GENERAL:
	strcpy(format, "%.*lg");
	sprintf(s, format, prec, loc);
	tmp = atof(s);
	if (tmp == 0.0) {
	    strcpy(format, "%lg");
	    loc = 0.0;
	    sprintf(s, format, loc);
	}
	break;
    case FORMAT_DDMMYY:
	strcpy(format, "%02d-%02d-%0*d");
	jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, d, m, yprec, y);
	break;
    case FORMAT_MMDDYY:
	strcpy(format, "%02d-%02d-%0*d");
	jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d, yprec, y);
	break;
    case FORMAT_YYMMDD:
	strcpy(format, "%0*d-%02d-%02d");
	jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, yprec, y, m, d);
	break;
    case FORMAT_MMYY:
	strcpy(format, "%02d-%0*d");
	jul_to_cal_and_time(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, yprec, y);
	break;
    case FORMAT_MMDD:
	strcpy(format, "%02d-%02d");
	jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d);
	break;
    case FORMAT_MONTHDAY:
	strcpy(format, "%s-%02d");
	jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, months[m - 1], d);
	}
	break;
    case FORMAT_DAYMONTH:
	strcpy(format, "%02d-%s");
	jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, d, months[m - 1]);
	}
	break;
    case FORMAT_MONTHS:
	strcpy(format, "%s");
	jul_to_cal_and_time(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, months[m - 1]);
	}
	break;
    case FORMAT_MONTHSY:
	strcpy(format, "%s-%0*d");
	jul_to_cal_and_time(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, months[m - 1], yprec, y);
	}
	break;
    case FORMAT_MONTHL:
	strcpy(format, "%s");
	jul_to_cal_and_time(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, monthl[m - 1]);
	}
	break;
    case FORMAT_DAYOFWEEKS:
	strcpy(format, "%s");
	sprintf(s, format, dayofweekstrs[dayofweek(loc + pr->ref_date)]);
	break;
    case FORMAT_DAYOFWEEKL:
	strcpy(format, "%s");
	sprintf(s, format, dayofweekstrl[dayofweek(loc + pr->ref_date)]);
	break;
    case FORMAT_DAYOFYEAR:
	strcpy(format, "%d");
        jul_to_cal_and_time(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format,
                1 + (int) (cal_to_jul(y, m, d) - cal_to_jul(y, 1, 1)));
	break;
    case FORMAT_HMS:
	strcpy(format, "%02d:%02d:%02d");
	jul_to_cal_and_time(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, h, mm, sec);
	break;
    case FORMAT_MMDDHMS:
	strcpy(format, "%02d-%02d %02d:%02d:%02d");
	jul_to_cal_and_time(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d, h, mm, sec);
	break;
    case FORMAT_MMDDYYHMS:
	strcpy(format, "%02d-%02d-%d %02d:%02d:%02d");
	jul_to_cal_and_time(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d, y, h, mm, sec);
	break;
    case FORMAT_YYMMDDHMS:
	strcpy(format, "%0*d-%02d-%02d %02d:%02d:%02d");
	jul_to_cal_and_time(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, yprec, y, m, d, h, mm, sec);
	break;
    case FORMAT_DEGREESLON:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%.*lfW");
	} else if (loc > 0.0) {
	    strcpy(format, "%.*lfE");
	} else {
	    strcpy(format, "0");
	}
	sprintf(s, format, prec, loc);
	break;
    case FORMAT_DEGREESMMLON:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%d %.*lf' W");
	} else if (loc > 0.0) {
	    strcpy(format, "%d %.*lf' E");
	} else {
	    strcpy(format, "0 0'");
	}
	y = loc;
	arcmin = (loc - y) * 60.0;
	sprintf(s, format, y, prec, arcmin);
	break;
    case FORMAT_DEGREESMMSSLON:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%d %d' %.*lf\" W");
	} else if (loc > 0.0) {
	    strcpy(format, "%d %d' %.*lf\" E");
	} else {
	    strcpy(format, "0 0' 0\"");
	}
	y = loc;
	arcsec = (loc - y) * 3600.0;
	m = arcsec / 60.0;
	arcsec = (arcsec - m * 60);
	sprintf(s, format, y, m, prec, arcsec);
	break;
    case FORMAT_MMSSLON:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%d' %.*lf\" W");
	} else if (loc > 0.0) {
	    strcpy(format, "%d' %.*lf\" E");
	} else {
	    strcpy(format, "0 0' 0\"");
	}
	y = loc;
	arcsec = (loc - y) * 3600.0;
	m = arcsec / 60.0;
	arcsec = (arcsec - m * 60);
	sprintf(s, format, m, prec, arcsec);
	break;
    case FORMAT_DEGREESLAT:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%.*lfS");
	} else if (loc > 0.0) {
	    strcpy(format, "%.*lfN");
	} else {
	    strcpy(format, "0");
	}
	sprintf(s, format, prec, loc);
	break;
    case FORMAT_DEGREESMMLAT:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%d %.*lf' S");
	} else if (loc > 0.0) {
	    strcpy(format, "%d %.*lf' N");
	} else {
	    strcpy(format, "0 0'");
	}
	y = loc;
	arcsec = (loc - y) * 60.0;
	sprintf(s, format, y, prec, arcsec);
	break;
    case FORMAT_DEGREESMMSSLAT:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%d %d' %.*lf\" S");
	} else if (loc > 0.0) {
	    strcpy(format, "%d %d' %.*lf\" N");
	} else {
	    strcpy(format, "0 0' 0\"");
	}
	y = loc;
	arcsec = (loc - y) * 3600.0;
	m = arcsec / 60.0;
	arcsec = (arcsec - m * 60);
	sprintf(s, format, y, m, prec, arcsec);
	break;
    case FORMAT_MMSSLAT:
	if (loc < 0.0) {
	    loc *= -1.0;
	    strcpy(format, "%d' %.*lf\" S");
	} else if (loc > 0.0) {
	    strcpy(format, "%d' %.*lf\" N");
	} else {
	    strcpy(format, "0 0' 0\"");
	}
	y = loc;
	arcsec = (loc - y) * 3600.0;
	m = arcsec / 60.0;
	arcsec = (arcsec - m * 60);
	sprintf(s, format, m, prec, arcsec);
	break;
    default:
	sprintf(s, format, prec, loc);
	break;
    }

    /* revert to POSIX */
    set_locale_num(FALSE);
    
    return(s);
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

#ifdef HAVE_SETLOCALE
static int need_locale = FALSE;
static char *system_locale_string, *posix_locale_string;
#endif

int init_locale(void)
{
#ifdef HAVE_SETLOCALE
    char *s;
    s = setlocale(LC_NUMERIC, "");
    if (s == NULL) {
        /* invalid/unsupported locale */
        return RETURN_FAILURE;
    } else if (!strcmp(s, "C")) {
        /* don't enable need_locale, since the system locale is C */
        return RETURN_SUCCESS;
    } else {
        system_locale_string = copy_string(NULL, s);
        s = setlocale(LC_NUMERIC, "C");
        posix_locale_string = copy_string(NULL, s);
        need_locale = TRUE;
        return RETURN_SUCCESS;
    }
#else
    return RETURN_SUCCESS;
#endif
}

void set_locale_num(int flag)
{
#ifdef HAVE_SETLOCALE
    if (need_locale) {
        if (flag == TRUE) {
            setlocale(LC_NUMERIC, system_locale_string);
        } else {
            setlocale(LC_NUMERIC, posix_locale_string);
        }
    }
#endif
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
    
    buf = xmalloc(strlen(QIDSTR(q)) + 128);
    if (!buf) {
        return NULL;
    }
    
    switch (quark_fid_get(q)) {
    case QFlavorProject:
        sprintf(buf, "Project \"%s%s\"", QIDSTR(q),
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
    case QFlavorAxis:
        t = axis_get_data(q);
        
        sprintf(buf, "%c Axis \"%s%s\"",
            t->type == AXIS_TYPE_X ? 'X':'Y', QIDSTR(q),
            quark_dirtystate_get(q) ? "*":"");

        break;
    case QFlavorDObject:
        o = object_get_data(q);

        sprintf(buf, "DObject \"%s%s\" (%s)",
            QIDSTR(q), quark_dirtystate_get(q) ? "*":"",
            object_types(o->type));
        
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
