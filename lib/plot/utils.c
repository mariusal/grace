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
 *
 * plot utils
 *
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "grace/plotP.h"

#define RES_NONE    0
#define RES_DEG     1
#define RES_MIN     2
#define RES_SEC     3

/* FIXME: check for max size indeed! */
size_t strfgeo(char *str, size_t max, const char *format, double value, int prec)
{
    char *s = str, *fmt = (char *) format;
    size_t out_size = 0;
    int min_resolution = RES_NONE, sgn, np;
    double deg_value, min_value, sec_value;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch(*fmt) {
            case 'D':
                min_resolution = RES_DEG;
                break;
            case 'M':
                min_resolution = RES_MIN;
                break;
            case 'S':
                min_resolution = RES_SEC;
                break;
            }
        }
        fmt++;
    }
    
    sgn = sign(value);
    value = fabs(value);
    
    fmt = (char *) format;
    while (*fmt && out_size < max) {
        if (*fmt == '%') {
            fmt++;
            switch(*fmt) {
            case 'D':
                if (min_resolution == RES_DEG) {
                    deg_value = value;
                    np = sprintf(s, "%.*f", prec, deg_value);
                } else {
                    deg_value = floor(value);
                    np = sprintf(s, "%d", (int) deg_value);
                }
                break;
            case 'M':
                deg_value = floor(value);
                if (min_resolution == RES_MIN) {
                    min_value = (value - deg_value)*60.0;
                    np = sprintf(s, "%.*f", prec, min_value);
                } else {
                    min_value = floor(value - deg_value);
                    np = sprintf(s, "%d", (int) min_value);
                }
                break;
            case 'S':
                min_value = (value - floor(value))*60.0;
                sec_value = (min_value - floor(min_value))*60.0;
                np = sprintf(s, "%.*f", prec, sec_value);
                break;
            case 'X':
                if (sgn < 0) {
                    np = sprintf(s, "%c", 'W');
                } else
                if (sgn > 0) {
                    np = sprintf(s, "%c", 'E');
                } else {
                    np = 0;
                }
                break;
            case 'Y':
                if (sgn < 0) {
                    np = sprintf(s, "%c", 'S');
                } else
                if (sgn > 0) {
                    np = sprintf(s, "%c", 'N');
                } else {
                    np = 0;
                }
                break;
            default:
                np = 0;
                break;
            }
            out_size += np;
            s += np;
        } else {
            *s = *fmt;
            out_size++;
            s++;
        }
        fmt++;
    }
    return out_size;
}

/*
 * reduce years according to the following rules :
 * [ wrap_year ; 100*(1 + wrap_year/100) - 1 ] -> [wy ; 99]
 * [ 100*(1 + wrap_year/100) ; wrap_year + 99] -> [00 ; wy-1]
 */
static int reduced_year(const Project *pr, int y)
{
    if (pr->two_digits_years) {
        int century = 100*(1 + pr->wrap_year/100);
        if (y < pr->wrap_year) {
            return y;
        } else if (y < century) {
            return y - (century - 100);
        } else if (y < (pr->wrap_year + 100)) {
            return y - century;
        } else {
            return y;
        }
    } else {
        return y;
    }
}

void jdate_to_datetime(const Quark *q, double jday, int rounding,
                         int *y, int *m, int *d,
                         int *hour, int *min, int *sec)
{
    Project *pr = project_get_data(q);
    
    if (pr) {
        /* compensate for the reference date */
        jday += pr->ref_date;

        jul_to_cal_and_time(jday, rounding, y, m, d, hour, min, sec);

        /* introduce the y2k bug for those who want it :) */
        *y = reduced_year(pr, *y);
    }
}

static int dayofweek(double j)
{
    int i = (int) floor(j + 1.5);
    return (i <= 0) ? 6 - (6 - i)%7 : i%7;
}

/* create format string */
char *create_fstring(const Quark *q, const Format *form, double loc, int type)
{
    char format[64], eng_prefix[6];
    static char s[MAX_STRING_LENGTH];
    double tmp;
    int m, d, y, h, mm, sec;
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
    switch (form->type) {
    case FORMAT_DECIMAL:
        sprintf(s, format, form->prec1, loc);
        tmp = atof(s);          /* fix reverse axes problem when loc == -0.0 */
        if (tmp == 0.0) {
            strcpy(format, "%.*lf");
            loc = 0.0;
            sprintf(s, format, form->prec1, loc);
        }
        break;
    case FORMAT_EXPONENTIAL:
        strcpy(format, "%.*le");
        sprintf(s, format, form->prec1, loc);
        tmp = atof(s);          /* fix reverse axes problem when loc == -0.0 */
        if (tmp == 0.0) {
            strcpy(format, "%.*le");
            loc = 0.0;
            sprintf(s, format, form->prec1, loc);
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
            sprintf(s, format, form->prec1, mantissa, exponent);
        } else {
            strcpy(format, "%.*f");
            sprintf(s, format, form->prec1, 0.0);
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
        sprintf(s, format, form->prec1, loc/(pow(10.0, exponent)), eng_prefix);
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
            sprintf(format, "%.*f", form->prec1, 0.0);
        } else {
            loc = log10(loc);
            if (type == LFORMAT_TYPE_EXTENDED) {
                strcpy(format, "10\\S%.*lf\\N");
            } else {
                strcpy(format, "10(%.*lf)\\N");
            }
        }
        sprintf(s, format, form->prec1, loc);
        break;
    case FORMAT_GENERAL:
        strcpy(format, "%.*lg");
        sprintf(s, format, form->prec1, loc);
        tmp = atof(s);
        if (tmp == 0.0) {
            strcpy(format, "%lg");
            loc = 0.0;
            sprintf(s, format, loc);
        }
        break;
    case FORMAT_DATETIME:
        if (!string_is_empty(form->fstring)) {
            struct tm datetime_tm;
            
            jdate_to_datetime(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
            
            memset(&datetime_tm, 0, sizeof(datetime_tm));
            datetime_tm.tm_year = y - 1900;
            datetime_tm.tm_mon  = m - 1;
            datetime_tm.tm_mday = d;
            datetime_tm.tm_hour = h;
            datetime_tm.tm_min  = mm;
            datetime_tm.tm_sec  = sec;
            datetime_tm.tm_wday = dayofweek(loc + pr->ref_date);
            datetime_tm.tm_yday = (int) (cal_to_jul(y, m, d) -
                                         cal_to_jul(y, 1, 1));
            
            if (strftime(s,
                MAX_STRING_LENGTH - 1, form->fstring, &datetime_tm) <= 0) {
                s[0] = '\0';
            }
        } else {
            s[0] = '\0';
        }
        break;
    case FORMAT_GEOGRAPHIC:
        if (!string_is_empty(form->fstring)) {
            if (strfgeo(s, MAX_STRING_LENGTH - 1, form->fstring, loc,
                form->prec1) <= 0) {
                s[0] = '\0';
            }
        } else {
            s[0] = '\0';
        }
        break;
    default:
        sprintf(s, format, form->prec1, loc);
        break;
    }

    /* revert to POSIX */
    set_locale_num(FALSE);
    
    return(s);
}

