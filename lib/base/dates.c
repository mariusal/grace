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
 * decided to use "-4713-01-01T12:00:00" as a reference date because
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

#include "grace/baseP.h"

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
     * day 1721423 :    -1-12-31
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
     * are not also multiple of 400 (so 1600, 1896, 1904, and 2000 are
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
     * year bounds : 400n - 688570288 <= 146097y <= 400n - 688423712
     * lower bound reached on : 1696-12-31, 2096-12-31, 2496-12-31 ...
     * upper bound reached on : 1904-01-01, 2304-01-01, 2704-01-01 ...
     * the lower bound gives a low estimate of the year
     */
    return (int) ((400L*n - 688570288L)/146097L);
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
        + (((double) (((hour - 12)*60 + min)*60)) + sec)/86400.0;

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
 */
void jul_to_cal_and_time(double jday, int rounding,
                         int *y, int *m, int *d,
                         int *hour, int *min, int *sec)
{
    long n;
    double tmp;
    
    /* find the time of the day */
    n = (long) floor(jday + 0.5);
    tmp = 24.0*(jday + 0.5 - n);
    *hour = (int) floor(tmp);
    tmp = 60.0*(tmp - *hour);
    *min = (int) floor(tmp);
    tmp  = 60.0*(tmp - *min);
    *sec = (int) floor(tmp + 0.5);

    /* perform some rounding */
    if (*sec >= 60 || rounding > ROUND_SECOND) {
        /* we should round to at least nearest minute */
        if (*sec >= 30) {
            (*min)++;
        }
        *sec = 0;
        if (*min == 60 || rounding > ROUND_MINUTE) {
            /* we should round to at least nearest hour */
            if (*min >= 30) {
                (*hour)++;
            }
            *min = 0;
            if (*hour == 24 || rounding > ROUND_HOUR) {
                /* we should round to at least nearest day */
                if (*hour >= 12) {
                    n++;
                }
                *hour = 0;
            }
        }
    }

    /* now find the date */
    jul_to_cal(n, y, m, d);

    /* perform more rounding */
    if (rounding == ROUND_MONTH) {

        int m2, y2;
        if (*m < 12) {
            m2 = *m + 1;
            y2 = *y;
        } else {
            m2 = 1;
            y2 = *y + 1;
        }

        if ((cal_to_jul(y2, m2, 1) - n) <= (n - cal_to_jul(*y, *m, 1))) {
            *m = m2;
            *y = y2;
        }
        *d = 1;

    }
}
