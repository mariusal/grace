/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
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
 * misc utils
 */

#include <stdlib.h>
#include <string.h>

#include "grace/baseP.h"

#ifdef HAVE_SETLOCALE
#  include <locale.h>
#endif

int strings_are_equal(const char *s1, const char *s2)
{
    if (s1 == NULL && s2 == NULL) {
        return TRUE;
    } else if (s1 == NULL || s2 == NULL) {
        return FALSE;
    } else {
        return (strcmp(s1, s2) == 0);
    }
}

int string_is_empty(const char *s)
{
    if (!s || s[0] == '\0') {
        return TRUE;
    } else {
        return FALSE;
    }
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

/*
 * nicenum: find a "nice" number approximately equal to x
 */

double nicenum(double x, int nrange, int round)
{
    int xsign;
    double f, y, fexp, rx, sx;
    
    if (x == 0.0) {
        return(0.0);
    }

    xsign = sign(x);
    x = fabs(x);

    fexp = floor(log10(x)) - nrange;
    sx = x/pow(10.0, fexp)/10.0;            /* scaled x */
    rx = floor(sx);                         /* rounded x */
    f = 10*(sx - rx);                       /* fraction between 0 and 10 */

    if ((round == NICE_FLOOR && xsign == +1) ||
        (round == NICE_CEIL  && xsign == -1)) {
        y = (int) floor(f);
    } else if ((round == NICE_FLOOR && xsign == -1) ||
               (round == NICE_CEIL  && xsign == +1)) {
        y = (int) ceil(f);
    } else {    /* round == NICE_ROUND */
        if (f < 1.5)
            y = 1;
        else if (f < 3.)
            y = 2;
        else if (f < 7.)
            y = 5;
        else
            y = 10;
    }
    
    sx = rx + (double) y/10.0;
    
    return (xsign*sx*10.0*pow(10.0, fexp));
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

void uswap(unsigned int *x, unsigned int *y)
{
    unsigned int tmp;

    tmp = *x;
    *x = *y;
    *y = tmp;
}

void sswap(char **x, char **y)
{
    char *tmp;
    tmp = *x;
    *x = *y;
    *y = tmp;
}

/*
 * compute the mins and maxes of a vector x
 */
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax)
{
    int i;
    
    *imin = 0;
    *imax = 0;

    if (x == NULL) {
        *xmin = 0.0;
        *xmax = 0.0;
        return;
    }
    
    *xmin = x[0];
    *xmax = x[0];
    
    for (i = 1; i < n; i++) {
        if (x[i] < *xmin) {
            *xmin = x[i];
            *imin = i;
        }
        if (x[i] > *xmax) {
            *xmax = x[i];
            *imax = i;
        }
    }
}

int isoneof(int c, char *s)
{
    if (strchr(s, c)) {
        return TRUE;
    } else {
        return FALSE;
    }
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

int is_locale_utf8(void)
{
#ifdef HAVE_SETLOCALE
    char *s = system_locale_string;
    if (s && (strstr(s, "utf8") || strstr(s, "UTF-8"))) {
        return TRUE;
    } else {
        return FALSE;
    }
#else
    return FALSE;
#endif
}
