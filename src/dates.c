/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1999 Grace Development Team
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


/* date and time conversion functions */

/*
 * We use two calendars here : the one that was established in 532 by
 * Denys and lasted until 1582, and the one that was created by Luigi
 * Lilio (Alyosius Lilius) and Christoph Klau (Christophorus Clavius)
 * for pope Gregorius XIII. Both use the same months (they were
 * introduced under emperor Augustus, a few years after julian
 * calendar introduction, both Julius and Augustus were honored by a
 * month beeing named after each one).

 * The leap years occured regularly in Denys's calendar : once every
 * four years, there is no year 0 in this calendar (the leap year -1
 * was just before year 1). This calendar was not compliant with earth
 * motion and the dates were slowly shifting with regard to
 * astronomical events.

 * This was corrected in 1582 by introducing gregorian calendar. First
 * a ten days shift was introduced to reset correct dates (Thursday
 * October the 4th was followed by Friday October the 15th). The rules
 * for leap years were also changed : three leap years are removed
 * every four centuries. These years are those that are multiple of
 * 100 but not multiple of 400 : 1700, 1800, and 1900 were not leap
 * years, but 1600 and 2000 were (will be) leap years.

 * We still use gregorian calendar today, but we now have several time
 * scales for increased accuracy. The International Atomic Time is a
 * linear scale : the best scale to use for scientific reference. The
 * Universal Time Coordinate (often confused with Greenwhich Mean
 * Time) is a legal time that is almost synchronized with earth
 * motion. However, since the earth is slightly slowing down, leap
 * seconds are introduced from time to time in UTC (about one second
 * every 18 monthes). UTC is not a continuous scale ! When a leap
 * second is introduced by International Earth Rotation Service, this
 * is published in advance and the legal time sequence is as follows :
 * 23:59:59 followed one second later by 23:59:60 followed one second
 * later by 00:00:00. At the time of this writing (1999-01-05) the
 * difference between IAT and UTC was 32 seconds, and the last leap
 * second was introduced in 1998-12-31.

 * These calendars allow to represent any date from the mist of the
 * past to the fog of the future, but they are not convenient for
 * computation. Another time scale is of possible : counting only the
 * days from a reference. Such a time scale was introduced by
 * Joseph-Juste Scaliger (Josephus Justus Scaliger) in 1583. He
 * decided to use "-4713-01-01 12:00:00" as a reference date because
 * it was at the same time a monday, first of January of a leap year,
 * there was an exact number of 19 years Meton cycle between this date
 * and year 1 (for Easter computation), and it was at the beginning of
 * a 15 years "roman indiction" cycle. The day number is called
 * "julian day", but it has really nothing to do with the julian
 * calendar.

 * In Grace, we consider both julian days and calendar dates, and do
 * not consider leap seconds. The following routines are used to parse
 * the dates according to these formats and to convert between
 * them. If you find yourself in a situation were you need UTC, a very
 * precise scale, and should take into account leap seconds ... you
 * should convert your data yourself (for example using International
 * Atomic Time). But if you bother with that you probably already know
 * what to do ;-)
 *                                                       Luc */

#include <math.h>
#include <ctype.h>

#include "defines.h"
#include "protos.h"

static double ref_date = 0.0;

/*
 * store the reference date
 */
void set_ref_date(double ref)
{
    ref_date = ref;
}

/*
 * get the reference date
 */
double get_ref_date(void)
{
    return ref_date;
}

static Dates_format date_hint = FMT_nohint;

/*
 * store the user's preferrence (it is only an hint)
 */
void set_date_hint(Dates_format preferred)
{
    date_hint = preferred;
}

/*
 * get the user's preferrence (it is only an hint)
 */
Dates_format get_date_hint(void)
{
    return date_hint;
}

static int two_digits_years_flag = FALSE;

/*
 * set two digits years handling
 */
void allow_two_digits_years(int allowed)
{
    two_digits_years_flag = allowed ? TRUE : FALSE;
}

/*
 * get two digits years handling
 */
int two_digits_years_allowed(void)
{
    return two_digits_years_flag;
}


/*
 * reduce years according to the folowing rules :
 * [1950 ; 1999] -> [50 ; 99]
 * [2000 ; 2049] -> [00 ; 49]
 */
static int reduced_year(int y)
{
    if (two_digits_years_allowed()) {
        if (y < 1950) {
            return y;
        } else if (y < 2000) {
            return y - 1900;
        } else if (y < 2050) {
            return y - 2000;
        } else {
            return y;
        }
    } else {
        return y;
    }
}


/*
 * set of functions to convert julian calendar elements
 * with negative years to julian day
 */
static int neg_julian_non_leap (int year)
{
    /* one leap year every four years, leap years : -4713, -4709, ..., -5, -1 */
    return (3 - year) & 3;
}

static long neg_julian_cal_to_jul(int y, int m, int d)
{
    /* day 0       : -4713-01-01
     * day 1721423 :    -1-01-01
     */
    return (1461L*(y + 1L))/4L
        + (m*489)/16 - ((m > 2) ? (neg_julian_non_leap(y) ? 32L : 31L) : 30L)
        + d + 1721057L;

}

static int neg_julian_year_estimate(long n)
{
    /* year bounds : 4n - 6887153 <= 1461y <= 4n - 6885693
     * lower bound reached 31st December of leap years
     * upper bound reached 1st January of leap years
     * the lower bound gives a low estimate of the year
     */
    return (int) ((4L*n - 6887153L)/1461L);
}


/*
 * set of functions to convert julian calendar elements
 * with positive years to julian day
 */
static int pos_julian_non_leap(int year)
{
    /* one leap year every four years, leap years : 4, 8, ..., 1576, 1580 */
    return year & 3;
}

static long pos_julian_cal_to_jul(int y, int m, int d)
{
    /* day 1721424 :     1-01-01
     * day 2299160 :  1582-10-04
     */
    return (1461L*(y -1L))/4L
        + (m*489)/16 - ((m > 2) ? (pos_julian_non_leap(y) ? 32L : 31L) : 30L)
        + d + 1721423L;

}

static int pos_julian_year_estimate(long n)
{
    /* year bounds : 4n - 6885692 <= 1461y <= 4n - 6884232
     * lower bound reached 31st December of leap years
     * upper bound reached 1st January of leap years
     * the lower bound gives a low estimate of the year
     */
    int y = (int) ((4L*n - 6885692L)/1461L);

    /* make sure we stay in the positive model even with our underestimate */
    return (y < 1) ? 1 : y;

}


/*
 * set of functions to convert gregorian calendar elements to julian day
 */
static int gregorian_non_leap(int year)
{
    /* one leap year every four years, except for multiple of 100 that
     * are not also multiple of 400 (so 1600, 1896, 1904, &nd 2000 are
     * leap years, but 1700, 1800 and 1900 are non leap years
     */
    return (year & 3) || ((year % 100) == 0 && ((year/100 & 3)));
}

static long gregorian_cal_to_jul(int y, int m, int d)
{
    long c;

    /* day 2299161 : 1582-10-15 */
    c = (long) ((y - 1)/100);
    return (1461L*(y - 1))/4 + c/4 - c
        + (m*489)/16 - ((m > 2) ? (gregorian_non_leap(y) ? 32L : 31L) : 30L)
        + d + 1721425L;

}

static int gregorian_year_estimate(long n)
{
    /*
     * year bounds : 400n - 688569888 <= 146097y <= 400n - 688423312
     * lower bound reached on : 1696-12-31, 2096-12-31, 2496-12-31 ...
     * upper bound reached on : 1904-01-01, 2304-01-01, 2704-01-01 ...
     * the lower bound gives a low estimate of the year
     */
    return (int) ((400L*n - 688569888L)/146097L);
}


/*
 * convert calendar elements to Julian day
 */
long cal_to_jul(int y, int m, int d)
{

    long n;

    n = gregorian_cal_to_jul(y, m, d);

    if (n < 2299161L) {
        /* the date belongs to julian calendar */
        n = (y < 0)
            ? neg_julian_cal_to_jul(y, m, d)
            : pos_julian_cal_to_jul(y, m, d);
    }

    return n;

}


/*
 * convert julian day to calendar elements
 */
static void jul_to_some_cal(long n,
                            int (*some_non_leap) (int),
                            long (*some_cal_to_jul) (int, int, int),
                            int (*some_year_estimate) (long),
                            int *y, int *m, int *d)
{
    int non_leap, day_of_year, days_until_end_of_year;

    /* lower estimation of year */
    *y = some_year_estimate(n);
    non_leap = some_non_leap(*y);
    days_until_end_of_year = (int) (some_cal_to_jul(*y, 12, 31) - n);

    while (days_until_end_of_year < 0) {
        /* correction of the estimate */
        (*y)++;
        non_leap = some_non_leap(*y);
        days_until_end_of_year += non_leap ? 365 : 366;
    }

    day_of_year = (non_leap ? 365 : 366) - days_until_end_of_year;

    /* estimate of the month : one too high only on last days of January */
    *m = (16*(day_of_year + (non_leap ? 32 : 31))) / 489;

    /* day of month */
    *d = day_of_year
       - (*m*489)/16 + ((*m > 2) ? (non_leap ? 32 : 31) : 30);
    if (*d < 1) {
        /* no luck, our estimate is false near end of January */
        *m = 1;
        *d += 31;
    }

}


/*
 * convert julian day to calendar elements
 */
void jul_to_cal(long n, int *y, int *m, int *d)
{
    if (n < 1721424L) {
       jul_to_some_cal(n, neg_julian_non_leap,
                       neg_julian_cal_to_jul, neg_julian_year_estimate,
                       y, m, d);
    } else if (n < 2299161L) {
       jul_to_some_cal(n, pos_julian_non_leap,
                       pos_julian_cal_to_jul, pos_julian_year_estimate,
                       y, m, d);
    } else {
       jul_to_some_cal(n, gregorian_non_leap,
                       gregorian_cal_to_jul, gregorian_year_estimate,
                       y, m, d);
    }
}


/*
 * convert julian day and hourly elements to julian day
 */
double jul_and_time_to_jul(long jul, int hour, int min, double sec)
{
    return ((double) jul)
        + (((double) (((hour + 12)*60 + min)*60)) + sec)/86400.0;

}


/*
 * convert calendar and hourly elements to julian day
 */
double cal_and_time_to_jul(int y, int m, int d,
                           int hour, int min, double sec)
{
    return jul_and_time_to_jul (cal_to_jul(y, m, d), hour, min, sec);
}


/*
 * convert julian day to calendar and hourly elements
 * rounding_tol allows to say 1999-12-31 23:59:59.501
 * should be rounded to 2000-01-01 00:00:00.000 assuming
 * it is set to 0.5 second. It is wise to set it according
 * to the display accuracy of seconds.
 */
void jul_to_cal_and_time(double jday, double rounding_tol,
                         int *y, int *m, int *d,
                         int *hour, int *min, double *sec)
{
    long n;
    double tmp;

    /* compensate for the reference date */
    jday += get_ref_date();
    
    /* find the time of the day */
    n = (long) floor(jday - 0.5);
    tmp = 24.0*(jday - 0.5 - n);
    *hour = (int) floor(tmp);
    tmp = 60.0*(tmp - *hour);
    *min = (int) floor(tmp);
    *sec = 60.0*(tmp - *min);
    if (*sec + rounding_tol >= 60.0) {
        /* we should round to next minute */
        *sec = 0.0;
        *min += 1;
        if (*min == 60) {
            *min = 0;
            *hour += 1;
            if (*hour == 24) {
                *hour = 0;
                n++;
            }
        }
    }

    /* now find the date */
    jul_to_cal(n, y, m, d);
    
    /* introduce the y2k bug for those who want it :) */
    *y = reduced_year(*y);
}

/*
 * check the existence of given calendar elements
 * this includes either number of day in the month
 * and calendars pecularities (year 0 and October 1582)
 */
static int check_date(Int_token y, Int_token m, Int_token d, long *jul)
{
    int y_expand, y_check, m_check, d_check;

    if (two_digits_years_allowed()) {
        /* expands years written with two digits only */
        if (y.value >= 0 && y.value < 50 && y.digits <= 2) {
            y_expand = 2000 + y.value;
        } else if (y.value >= 50 && y.value < 100 && y.digits <= 2) {
            y_expand = 1900 + y.value;
        } else {
            y_expand = y.value;
        }
    } else {
        y_expand = y.value;
    }

    if (m.digits > 2 || d.digits > 2) {
        /* this should be the year instead of either the month or the day */
        return GRACE_EXIT_FAILURE;
    }

    *jul = cal_to_jul(y_expand, m.value, d.value);
    jul_to_cal(*jul, &y_check, &m_check, &d_check);
    if (y_expand != y_check || m.value != m_check || d.value != d_check) {
        return GRACE_EXIT_FAILURE;
    } else {
        return GRACE_EXIT_SUCCESS;
    }

}


/*
 * lexical analyzer for float data. Knows about fortran exponent
 * markers. Store address of following data in *after, only in case of
 * success (thus it is allowed to have after == &s)
 */
int parse_float(const char* s, double *value, const char **after)
{
    int neg_mant, neg_exp, digits, dot_exp, raw_exp;
    const char *after_dot;

    /* we skip leading whitespace */
    while (isspace(*s)) {
        s++;
    }

    /* sign */
    if (*s == '-') {
       neg_mant = 1;
       s++;
    } else {
        neg_mant = 0;
    }

    /* mantissa */
    digits = 0;
    *value = 0.0;
    while (isdigit(*s)) {
        *value = *value*10.0 + (*s++ - '0');
        digits++;
    }
    if (*s == '.') {
        after_dot = ++s;
        while (isdigit(*s)) {
            *value = *value*10.0 + (*s++ - '0');
            digits++;
        }
        dot_exp = after_dot - s;
    } else {
        dot_exp = 0;
    }
    if (digits == 0) {
        /* there should be at least one digit (either before or after dot) */
        return GRACE_EXIT_FAILURE;
    }

    /* exponent (d and D are fortran exponent markers) */
    raw_exp = 0;
    if (*s == 'e' || *s == 'E' || *s == 'd' || *s == 'D') {
        s++;
        if (*s == '-') {
            neg_exp = 1;
            s++;
        } else {
            neg_exp = 0;
            if (*s == '+') {
                s++;
            }
        }
        while (isdigit(*s)) {
            raw_exp = raw_exp*10 + (*s++ - '0');
        }
        if (neg_exp) {
            raw_exp = -raw_exp;
        }
    }

    /* read float */
    *value = (neg_mant ? -(*value) : (*value)) * pow (10.0, dot_exp + raw_exp);

    if (after != NULL) {
        /* the caller wants to know what follows the float number */
        *after = s;
    }

    return GRACE_EXIT_SUCCESS;

}


/*
 * lexical analyzer for calendar dates
 * return the number of read elements, or -1 on failure
 */
static int parse_calendar_date(const char* s,
                               Int_token tab [5], double *sec)
{
    int i, waiting_separator, negative;

    negative = 0;
    waiting_separator = 0;
    i = 0;
    while (i < 5) {
        /* loop from year to minute elements : all integers */

        switch (*s) {
          case '\0': /* end of string */
              return i;

          case ' ' : /* repeatable separator */
              s++;
              negative = 0;
              break;

          case '/' : case ':' : case '.' : /* non-repeatable separator */
              if (waiting_separator) {
                  s++;
                  negative = 0;
                  waiting_separator = 0;
              } else {
                  return -1;
              }
              break;

          case '-' : /* either separator or minus sign */
              s++;
              if (waiting_separator) {
                  negative = 0;
                  waiting_separator = 0;
              } else if ((*s >= '0') && (*s <= '9')) {
                  negative = 1;
              } else {
                  return -1;
              }
              break;

          case '0' : case '1' : case '2' : case '3' : case '4' :
          case '5' : case '6' : case '7' : case '8' : case '9' : /* digit */
              tab[i].value  = ((int) *s) - '0';
              tab[i].digits = 1;
              while (isdigit(*++s)) {
                  tab[i].value = tab[i].value*10 + (((int) *s) - '0');
                  tab[i].digits++;
              }
              if (negative) {
                  tab[i].value = -tab[i].value;
              }
              i++;
              negative = 0;
              waiting_separator = 1;
              break;

          default  :
              return -1;

        }

    }

    while (isspace(*s)) {
        s++;
    }
    if (*s == '\0') {
        return 5;
    }

    if ((*s == '/') || (*s == ':') || (*s == '.') || (*s == '-')) {
        /* this was the seconds separator */
        s++;

        /* seconds are read in float format */
        if (parse_float(s, sec, &s) == GRACE_EXIT_SUCCESS) {
            while (isspace(*s)) {
                s++;
            }
            if (*s == '\0') {
                return 6;
            }
        }

    }

    /* something is wrong */
    return -1;

}


/*
 * parse a date given either in calendar or numerical format
 */
int parse_date(const char* s, Dates_format preferred,
               double *jul, Dates_format *recognized)
{
    int i, n;
    int ky, km, kd;
    static Dates_format trials [] = {FMT_nohint, FMT_iso, FMT_european, FMT_us};
    Int_token tab [5];
    long j;
    double sec;

    /* first guess : is it a date in calendar format ? */
    n = parse_calendar_date(s, tab, &sec);
    switch (n) {
        /* we consider hours, minutes and seconds as optional items */
      case -1 : /* parse error */
          break;

      case 3 :
          tab[3].value  = 0; /* adding hours */
          tab[3].digits = 1;

      case 4 :
          tab[4].value  = 0; /* adding minutes */
          tab[4].digits = 1;

      case 5 :
          sec = 0.0;  /* adding seconds */

      case 6 :
          /* we now have a complete date */

          /* try the user's choice first */
          trials[0] = preferred;

          for (i = 0; i < 4; i++) {
              if (trials[i] == FMT_iso) {
                  /* YYYY-MM-DD */
                  ky = 0;
                  km = 1;
                  kd = 2;
              } else if (trials[i] == FMT_european) {
                  /* DD/MM/(YY)YY */
                  ky = 2;
                  km = 1;
                  kd = 0;
              } else if (trials[i] == FMT_us) {
                  /* MM/DD/(YY)YY */
                  ky = 2;
                  km = 0;
                  kd = 1;
              } else {
                  /* the user didn't choose a calendar format */
                  continue;
              }

              if (check_date(tab[ky], tab[km], tab[kd], &j)
                  == GRACE_EXIT_SUCCESS) {
                  *jul = jul_and_time_to_jul(j, tab[3].value, tab[4].value,
                                             sec);
                  *recognized = trials[i];
                  return GRACE_EXIT_SUCCESS;
              }
          }
          break;

      default :
          /* probably a julian date */
          break;

    }

    return GRACE_EXIT_FAILURE;
}
