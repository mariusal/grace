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

#include "grace/plotP.h"

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

static char *dayofweekstrs[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char *dayofweekstrl[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char *monthl[] = {"January", "February", "March", "April", "May", "June",
"July", "August", "September", "October", "November", "December"};

static int dayofweek(double j)
{
    int i = (int) floor(j + 1.5);
    return (i <= 0) ? 6 - (6 - i)%7 : i%7;
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
	jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, d, m, yprec, y);
	break;
    case FORMAT_MMDDYY:
	strcpy(format, "%02d-%02d-%0*d");
	jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d, yprec, y);
	break;
    case FORMAT_YYMMDD:
	strcpy(format, "%0*d-%02d-%02d");
	jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, yprec, y, m, d);
	break;
    case FORMAT_MMYY:
	strcpy(format, "%02d-%0*d");
	jdate_to_datetime(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, yprec, y);
	break;
    case FORMAT_MMDD:
	strcpy(format, "%02d-%02d");
	jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d);
	break;
    case FORMAT_MONTHDAY:
	strcpy(format, "%s-%02d");
	jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, months[m - 1], d);
	}
	break;
    case FORMAT_DAYMONTH:
	strcpy(format, "%02d-%s");
	jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, d, months[m - 1]);
	}
	break;
    case FORMAT_MONTHS:
	strcpy(format, "%s");
	jdate_to_datetime(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, months[m - 1]);
	}
	break;
    case FORMAT_MONTHSY:
	strcpy(format, "%s-%0*d");
	jdate_to_datetime(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
	if (m - 1 < 0 || m - 1 > 11) {
	    sprintf(s, format, "???");
	} else {
	    sprintf(s, format, months[m - 1], yprec, y);
	}
	break;
    case FORMAT_MONTHL:
	strcpy(format, "%s");
	jdate_to_datetime(q, loc, ROUND_MONTH, &y, &m, &d, &h, &mm, &sec);
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
        jdate_to_datetime(q, loc, ROUND_DAY, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format,
                1 + (int) (cal_to_jul(y, m, d) - cal_to_jul(y, 1, 1)));
	break;
    case FORMAT_HMS:
	strcpy(format, "%02d:%02d:%02d");
	jdate_to_datetime(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, h, mm, sec);
	break;
    case FORMAT_MMDDHMS:
	strcpy(format, "%02d-%02d %02d:%02d:%02d");
	jdate_to_datetime(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d, h, mm, sec);
	break;
    case FORMAT_MMDDYYHMS:
	strcpy(format, "%02d-%02d-%d %02d:%02d:%02d");
	jdate_to_datetime(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
	sprintf(s, format, m, d, y, h, mm, sec);
	break;
    case FORMAT_YYMMDDHMS:
	strcpy(format, "%0*d-%02d-%02d %02d:%02d:%02d");
	jdate_to_datetime(q, loc, ROUND_SECOND, &y, &m, &d, &h, &mm, &sec);
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

