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
 * misc utilities
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#include "globals.h"
#include "utils.h"
#include "protos.h"

static void rereadConfig(void);
static RETSIGTYPE actOnSignal(int signo);
static void bugwarn(char *signame);

/*
 * free and check for NULL pointer
 */
void cxfree(void *ptr)
{
    if (ptr != NULL) {
	free(ptr);
        ptr = NULL;
    }
}

void *xrealloc(void *ptr, size_t size)
{
    void *retval;

#if defined(REALLOC_IS_BUGGY)
    if (ptr == NULL) {
        retval = malloc(size);
    } else if (size == 0) {
        free(ptr);
        retval = NULL;
    } else {
        retval = realloc(ptr, size); 
    }
#else
    retval = realloc(ptr, size);
    if (size == 0) {
        retval = NULL;
    }
#endif
    
    if (retval == NULL && size != 0) {
        errmsg("Memory storage exceeded!");
    }
    return retval;
}

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
    int i, quoteon = FALSE;

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
 * log base 2
 */
int ilog2(int n)
{
    int i = 0;
    int n1 = n;

    while (n1 >>= 1)
	i++;
    if (1 << i != n)
	return -1;
    else
	return i;
}

/*
 * compute the area bounded by the polygon (xi,yi)
 */
double comp_area(int n, double *x, double *y)
{
    int i;
    double sum = 0.0;

    for (i = 0; i < n; i++) {
	sum = sum + x[i] * y[(i + 1) % n] - y[i] * x[(i + 1) % n];
    }
    return sum * 0.5;
}

/*
 * compute the perimeter bounded by the polygon (xi,yi)
 */
double comp_perimeter(int n, double *x, double *y)
{
    int i;
    double sum = 0.0;

    for (i = 0; i < n - 1; i++) {
	sum = sum + hypot(x[i] - x[(i + 1) % n], y[i] - y[(i + 1) % n]);
    }
    return sum;
}

/*
 * Time and date routines
 */

char *dayofweekstrs[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *dayofweekstrl[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char *monthl[] = {"January", "February", "March", "April", "May", "June",
"July", "August", "September", "October", "November", "December"};

static int days1[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static int days2[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

/*
 * return the Julian day + hms as a real number
 */

/*
** Takes a date, and returns a Julian day. A Julian day is the number of
** days since some base date  (in the very distant past).
** Handy for getting date of x number of days after a given Julian date
** (use jdate to get that from the Gregorian date).
** Author: Robert G. Tantzen, translator: Nat Howard
** Translated from the algol original in Collected Algorithms of CACM
** (This and jdate are algorithm 199).
*/
double julday(int mon, int day, int year, int h, int mi, double se)
{
    long m = mon, d = day, y = year;
    long c, ya, j;
    double seconds = h * 3600.0 + mi * 60 + se;

    if (m > 2)
	m -= 3;
    else {
	m += 9;
	--y;
    }
    c = y / 100L;
    ya = y - (100L * c);
    j = (146097L * c) / 4L + (1461L * ya) / 4L + (153L * m + 2L) / 5L + d + 1721119L;
    if (seconds < 12 * 3600.0) {
	j--;
	seconds += 12.0 * 3600.0;
    } else {
	seconds = seconds - 12.0 * 3600.0;
    }
    return (j + (seconds / 3600.0) / 24.0);
}

/* Julian date converter. Takes a julian date (the number of days since
** some distant epoch or other), and returns an int pointer to static space.
** ip[0] = month;
** ip[1] = day of month;
** ip[2] = year (actual year, like 1977, not 77 unless it was  77 a.d.);
** ip[3] = day of week (0->Sunday to 6->Saturday)
** These are Gregorian.
** Copied from Algorithm 199 in Collected algorithms of the CACM
** Author: Robert G. Tantzen, Translator: Nat Howard
*/
void calcdate(double jd, int *m, int *d, int *y, int *h, int *mi, double *sec)
{
    static int ret[4];

    long j = jd;
    double tmp, frac = jd - j;

    if (frac >= 0.5) {
	frac = frac - 0.5;
	j++;
    } else {
	frac = frac + 0.5;
    }

    ret[3] = (j + 1L) % 7L;
    j -= 1721119L;
    *y = (4L * j - 1L) / 146097L;
    j = 4L * j - 1L - 146097L * *y;
    *d = j / 4L;
    j = (4L * *d + 3L) / 1461L;
    *d = 4L * *d + 3L - 1461L * j;
    *d = (*d + 4L) / 4L;
    *m = (5L * *d - 3L) / 153L;
    *d = 5L * *d - 3 - 153L * *m;
    *d = (*d + 5L) / 5L;
    *y = 100L * *y + j;
    if (*m < 10)
	*m += 3;
    else {
	*m -= 9;
	*y += 1;
    }
    tmp = 3600.0 * (frac * 24.0);
    *h = (int) (tmp / 3600.0);
    tmp = tmp - *h * 3600.0;
    *mi = (int) (tmp / 60.0);
    *sec = tmp - *mi * 60.0;
}

int dayofweek(double j)
{
    j += 0.5;
    return (int) (j + 1) % 7;
}

int leapyear(int year)
{
    if (year % 400 == 0)
        return (TRUE);
    else if (year % 100 == 0)
        return (FALSE);
    else if (year % 4 == 0)
        return (TRUE);
    else
        return (FALSE);
}

/*
   get the month and day given the number of days
   from the beginning of the year 'yr'
*/
void getmoday(int days, int yr, int *mo, int *da)
{
    int i;

    if (leapyear(yr)) {
	for (i = 0; i < 13; i++) {
	    if (days <= days2[i]) {
		*mo = i;
		*da = (days - days2[i - 1]);
		goto out1;
	    }
	}
    } else {
	for (i = 0; i < 13; i++) {
	    if (days <= days1[i]) {
		*mo = i;
		*da = (days - days1[i - 1]);
		goto out1;
	    }
	}
    }
out1:;
}

/*
   return the number of days from the beginning of the year 'yr'
*/
int getndays(double j)
{
    int m, d, y, hh, mm;
    double ss;

    calcdate(j, &m, &d, &y, &hh, &mm, &ss);
    if (leapyear(y)) {
	return days2[m - 1] + d;
    } else {
	return days1[m - 1] + d;
    }
}

/*
 * strip special chars from a string
 */
void stripspecial(char *s, char *cs)
{
    int i, slen = strlen(s), curcnt = 0;

    for (i = 0; i < slen; i++) {
	if (s[i] == '\\' && isdigit(s[i + 1])) {
	    i++;
	} else if (s[i] == '\\' && isoneof(s[i + 1], "cCbxsSNuU+-")) {
	    i++;
	} else if (s[i] == '\\' && s[i + 1] == '\\') {
	    i++;
	} else {
	    cs[curcnt++] = s[i];
	}
    }
    cs[curcnt] = 0;
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
    es = (char *) realloc (es, (len + 1)*sizeof(char));
    strcpy(es, s);
    n = 0;
    while ((es = strchr(es, '\"'))) {
    	es++;
    	n++;
    }
    
    elen = len + n + 1;
    es = (char *) realloc (es, elen*sizeof(char));
    
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

int sign(double a)
{
    if (a > 0.0) {
        return +1;
    } else if (a < 0.0) {
        return -1;
    } else {
        return 0;
    }
}

/*
 * exit grace
 */
void bailout(void)
{
    if (!is_dirtystate() || yesno("Exit losing unsaved changes?", NULL, NULL, "file.html#exit")) {
         if (resfp) {
             filter_close(resfp);
         }
         exit(0);
    }
}

/*
 * Reread config (TODO)
 */
static void rereadConfig(void)
{
}

/*
 * Warn about bug (TODO X message)
 */
static void bugwarn(char *signame)
{
    fprintf(stderr, "\a\nOops! Got %s. Please use \"Help/Comments\" to report the bug.\n", signame);
    exit(GRACE_EXIT_FAILURE);
}

static void timerUpdates(void)
{
    static int reading_now = FALSE;
    static int fd = -1;
    static char buf[2*PIPE_BUF + 1];
    static char *s = buf;
    char *sstart, *sstop;
    int nread, nread_max, len;
#ifndef NONE_GUI
    int cursor_set;
#endif

    
    if (named_pipe == TRUE && fd < 0) {
        fd = open(pipe_name, O_NONBLOCK | O_RDONLY);
        if (fd < 0) {
            errmsg("Can't open fifo");
            named_pipe = FALSE;
        }
    }
    
    if (fd < 0 || reading_now == TRUE) {
        return;
    } else {
        reading_now = TRUE;
        cursor_set = FALSE;

        while (TRUE) {
            nread_max = 2*PIPE_BUF - (s - buf);
            if ((nread = read(fd, s, nread_max)) <= 0) {
                break;
            }
            /* make sure there will be no overflow */
            s[nread] = '\0';
            
#ifndef NONE_GUI
            if (cursor_set == FALSE) {
                set_wait_cursor();
                cursor_set = TRUE;
            }
#endif
            for (sstop = buf, sstart = buf; *sstop != '\0'; sstop++) {
                if (*sstop == '\n') {
                    *sstop = '\0';
                    read_param(sstart);
                    sstart = sstop + 1;
                } 
            }
            /* move rest of the string to the beginning of the buffer */
            len = strlen(sstart);
            memmove(buf, sstart, len + 1);
            /* continue reading in at the point where we stopped */
            s = buf + len;
        }
        
#ifndef NONE_GUI
        if (cursor_set == TRUE) {
            unset_wait_cursor();
        }
#endif
        reading_now = FALSE;
    }
}

/*
 * Signal-handling routines
 */
 
static RETSIGTYPE actOnSignal(int signo)
{
    char signame[16];
     
    installSignal();
    
    switch (signo) {
#ifdef SIGHUP
    case SIGHUP:
    	rereadConfig();
    	break;
#endif
    case SIGALRM:
    	timerUpdates();
    	break;
#ifdef SIGINT
    case SIGINT:
#endif
#ifdef SIGQUIT
    case SIGQUIT:
#endif
#ifdef SIGTERM
    case SIGTERM:
#endif
        bailout();
        break;
#ifdef SIGILL
    case SIGILL:
        strcpy(signame, "SIGILL");
#endif
#ifdef SIGABRT
    case SIGABRT:
        strcpy(signame, "SIGABRT");
#endif
#ifdef SIGFPE
    case SIGFPE:
        strcpy(signame, "SIGFPE");
#endif
#ifdef SIGBUS
    case SIGBUS:
        strcpy(signame, "SIGBUS");
#endif
#ifdef SIGSEGV
    case SIGSEGV:
        strcpy(signame, "SIGSEGV");
#endif
#ifdef SIGSYS
    case SIGSYS:
        strcpy(signame, "SIGSYS");
#endif
        bugwarn(signame);
        break;
    default:
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
#ifdef SIGABRT
    signal(SIGABRT, actOnSignal);   /* abort */
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
    signal(SIGALRM, actOnSignal);  /* timer */
    alarm((int) ceil((double) timer_delay/1000));
}

/* create format string */
char *create_fstring(int form, int prec, double loc, int type)
{
    char format[64], eng_prefix[6];
    static char s[MAX_STRING_LENGTH];
    double tmp;
    int m, d, y, h, mm;
    double sec;
    int itmp;
    int exponent;
    double mantissa;

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
	strcpy(format, "%d-%d-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, d, m, y);
	break;
    case FORMAT_MMDDYY:
	strcpy(format, "%d-%d-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, m, d, y);
	break;
    case FORMAT_YYMMDD:
	strcpy(format, "%d-%d-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, y, m, d);
	break;
    case FORMAT_MMYY:
	strcpy(format, "%d-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, m, y);
	break;
    case FORMAT_MMDD:
	strcpy(format, "%d-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	sprintf(s, format, m, d);
	break;
    case FORMAT_MONTHDAY:
	strcpy(format, "%s-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	} else {
	    sprintf(s, format, months[m - 1], d);
	}
	break;
    case FORMAT_DAYMONTH:
	strcpy(format, "%d-%s");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	} else {
	    sprintf(s, format, d, months[m - 1]);
	}
	break;
    case FORMAT_MONTHS:
	strcpy(format, "%s");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	} else {
	    sprintf(s, format, months[m - 1]);
	}
	break;
	break;
    case FORMAT_MONTHSY:
	strcpy(format, "%s-%d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	} else {
	    sprintf(s, format, months[m - 1], y);
	}
	break;
	break;
    case FORMAT_MONTHL:
	strcpy(format, "%s");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	} else {
	    sprintf(s, format, monthl[m - 1]);
	}
	break;
	break;
    case FORMAT_DAYOFWEEKS:
	strcpy(format, "%s");
	itmp = dayofweek(loc);
	if ((itmp < 0) | (itmp > 6)) {
	} else {
	    sprintf(s, format, dayofweekstrs[dayofweek(loc)]);
	}
	break;
    case FORMAT_DAYOFWEEKL:
	strcpy(format, "%s");
	itmp = dayofweek(loc);
	if ((itmp < 0) | (itmp > 6)) {
	} else {
	    sprintf(s, format, dayofweekstrl[dayofweek(loc)]);
	}
	break;
    case FORMAT_DAYOFYEAR:
	strcpy(format, "%d");
	sprintf(s, format, getndays(loc));
	break;
    case FORMAT_HMS:
	strcpy(format, "%02d:%02d:%02d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	sprintf(s, format, h, mm, (int) sec);
	break;
    case FORMAT_MMDDHMS:
	strcpy(format, "%d-%d %02d:%02d:%02d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, m, d, h, mm, (int) sec);
	break;
    case FORMAT_MMDDYYHMS:
	strcpy(format, "%d-%d-%d %02d:%02d:%02d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, m, d, y, h, mm, (int) sec);
	break;
    case FORMAT_YYMMDDHMS:
	strcpy(format, "%d-%d-%d %02d:%02d:%02d");
	calcdate(loc, &m, &d, &y, &h, &mm, &sec);
	if (y >= 1900 && y < 2000) {
	    y -= 1900;
	}
	sprintf(s, format, y, m, d, h, mm, (int) sec);
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
	sec = (loc - y) * 60.0;
	sprintf(s, format, y, prec, sec);
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
	sec = (loc - y) * 3600.0;
	m = sec / 60.0;
	sec = (sec - m * 60);
	sprintf(s, format, y, m, prec, sec);
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
	sec = (loc - y) * 3600.0;
	m = sec / 60.0;
	sec = (sec - m * 60);
	sprintf(s, format, m, prec, sec);
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
	sec = (loc - y) * 60.0;
	sprintf(s, format, y, prec, sec);
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
	sec = (loc - y) * 3600.0;
	m = sec / 60.0;
	sec = (sec - m * 60);
	sprintf(s, format, y, m, prec, sec);
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
	sec = (loc - y) * 3600.0;
	m = sec / 60.0;
	sec = (sec - m * 60);
	sprintf(s, format, m, prec, sec);
	break;
    default:
	sprintf(s, format, prec, loc);
	break;
    }
    
    return(s);
}

int bin_dump(char *value, int i, int pad)
{
    char *word;
    
    if (i > pad - 1) {
        return 0;
    }
    
    word = value;
    
#ifdef WORDS_BIGENDIAN
    return (((*word)>>i)&0x01);
#else
    switch (pad) {
    case 8:
        return (((*word)>>i)&0x01);
        break;
    case 16:
        if (i < 8) {
            word++;
            return (((*word)>>i)&0x01);
        } else {
            return (((*word)>>(8 - i))&0x01);
        }
        break;
    case 32:
        if (i < 8) {
            word += 2;
            return (((*word)>>i)&0x01);
        } else if (i < 16) {
            word++;
            return (((*word)>>(8 - i))&0x01);
        } else {
            return (((*word)>>(16 - i))&0x01);
        }
        break;
    default:
        return 0;
    }
#endif
}

unsigned char reversebits(unsigned char inword)
{
    int i;
    unsigned char result = 0;
    
    for (i = 0; i <= 7; i++) {
        result |= (((inword)>>i)&0x01)<<(7 - i);
    }
    
    return (result);
}

char *copy_string(char *dest, char *src)
{
    if (src == dest) {
        ;
    } else if (src == NULL) {
        free(dest);
        dest = NULL;
    } else {
        dest = xrealloc(dest, (strlen(src) + 1)*SIZEOF_CHAR);
        strcpy(dest, src);
    }
    return(dest);
}

void reverse_string(char *s)
{
    char cbuf;
    int i, len;
    
    if (s == NULL) {
        return;
    }
    
    len = strlen(s);
    for (i = 0; i < len/2; i++) {
        cbuf = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = cbuf;
    }
}

/* location of Grace home directory */
#ifndef GRACE_HOME
#  define GRACE_HOME "/usr/local/grace"
#endif
static char grace_home[GR_MAXPATHLEN] = GRACE_HOME;	

char *get_grace_home(void)
{
    return grace_home;
}

void set_grace_home(char *dir)
{
    strncpy(grace_home, dir, GR_MAXPATHLEN - 1);
}

#ifndef GRACE_HELPVIEWER
#  define GRACE_HELPVIEWER "netscape -noraise -remote openURL\\(%s,newwindow\\) >>/dev/null 2>&1 || netscape %s"
#endif
static char help_viewer[GR_MAXPATHLEN] = GRACE_HELPVIEWER;	

char *get_help_viewer(void)
{
    return help_viewer;
}

void set_help_viewer(char *dir)
{
    strncpy(help_viewer, dir, GR_MAXPATHLEN - 1);
}

/*
 * stuff results, etc. into a text window
 */
void log_results(char *buf)
{
    char tmpbuf[512];
    if (logwindow) {
        strcpy(tmpbuf, buf);
        if (tmpbuf[strlen(tmpbuf) - 1] != '\n') {
            strcat(tmpbuf, "\n");
        }
        stufftext(tmpbuf, 1);
    }
}


void errmsg(char *buf)
{
#ifdef NONE_GUI
    printf("%s\n", buf);
#else
    if (inwin) {
        errwin(buf);
    } else {
        fprintf(stderr, "%s\n", buf);
    }
#endif
}

int yesnoterm(char *msg)
{
    return 1;
}

int yesno(char *msg, char *s1, char *s2, char *help_anchor)
{
    if (noask) {
	return 1;
    }
#ifdef NONE_GUI
    return (yesnoterm(msg));
#else
    if (inwin) {
        return (yesnowin(msg, s1, s2, help_anchor));
    } else {
        return (yesnoterm(msg));
    }
#endif
}
 

int fexists(char *to)
{
    struct stat stto;
    char tbuf[256];

    if (stat(to, &stto) == 0) {
	sprintf(tbuf, "Overwrite %s?", to);
	if (!yesno(tbuf, NULL, NULL, NULL)) {
	    return (1);
	}
	return (0);
    }
    return (0);
}

void stufftext(char *s, int sp)
{
#ifdef NONE_GUI
    printf("%s", buf);
#else
    stufftextwin(s, sp);
#endif
}


char *mybasename(char *s)
{
	int start, end;
	static char basename[256];
	
	end = strlen( s )-1;
	if( end==0 && *s=='/' ){	/* root is a special case */
		basename[0] = '/';
		return basename;
	}
	
	/* strip trailing white space and slashes */
	while( s[end]=='/' || s[end]==' ' || s[end]=='\t' )
		end--;
	/* find start of basename */
	start = end;
	do{
		start--;
	} while( start>=0 && s[start]!='/' );
	
	strncpy( basename, s+(start+1), end-start );
	basename[end-start] = '\0';
	return basename;
}

static char workingdir[GR_MAXPATHLEN];

int set_workingdir(char *wd)
{
    char buf[GR_MAXPATHLEN];
    
    if (wd == NULL) {
        getcwd(workingdir, GR_MAXPATHLEN - 1);
        if (workingdir[strlen(workingdir)-1] != '/') {
            strcat(workingdir, "/");
        }
        return GRACE_EXIT_SUCCESS;
    }
    
    strncpy(buf, wd, GR_MAXPATHLEN - 1);
    if (buf[0] == '~') {
        expand_tilde(buf);
    }
    if (chdir(buf) >= 0) {
        strncpy(workingdir, buf, GR_MAXPATHLEN - 1);
        if (workingdir[strlen(workingdir)-1] != '/') {
            strcat(workingdir, "/");
        }
	return GRACE_EXIT_SUCCESS;
    } else {
        return GRACE_EXIT_FAILURE;
    }
}

char *get_workingdir(void)
{
    return workingdir;
}

/* TODO this needs some work */
void expand_tilde(char *buf)
{
    char buf2[GR_MAXPATHLEN];
    char *home;
    if (buf[0] == '~') {
	if (strlen(buf) == 1) {
	    home = getenv("HOME");
	    if (home == NULL) {
		errmsg("Couldn't find $HOME!");
		return;
	    } else {
		strcpy(buf, home);
		strcat(buf, "/");
	    }
	} else if (buf[1] == '/') {
	    home = getenv("HOME");
	    if (home == NULL) {
		errmsg("Couldn't find $HOME!");
		return;
	    }
	    strcpy(buf2, home);
	    strcat(buf2, "/");
	    strcat(buf2, buf + 1);
	    strcpy(buf, buf2);
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
    if (inwin) {
#ifndef NONE_GUI
        set_left_footer(msg);
#endif
    } else {
        printf("%s\n", msg);
    }
}

static void update_timestamp(void)
{
    struct tm tm;
    time_t time_value;
    char *str;

    (void) time(&time_value);
    tm = *localtime(&time_value);
    str = asctime(&tm);
    if (str[strlen(str) - 1] == '\n') {
        str[strlen(str) - 1]= '\0';
    }
    set_plotstr_string(&timestamp, str);
}

/*
 * dirtystate routines
 */

static int dirtystate = 0;

void set_dirtystate(void)
{
    if (dirtystate >= 0) {
        dirtystate++;
        update_timestamp();
/*
 * TODO:
 * 	if ( (dirtystate > SOME_LIMIT) || 
 *           (current_time - autosave_time > ANOTHER_LIMIT) ) {
 * 	    autosave();
 * 	}
 */
    }
}

void clear_dirtystate(void)
{
    dirtystate = 0;
}

void lock_dirtystate(void)
{
    dirtystate = -1;
}

int is_dirtystate(void)
{
    return (dirtystate);
}
