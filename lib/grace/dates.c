/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1999-2006 Grace Development Team
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

#include <ctype.h>

#include "grace/graceP.h"

/*
 * expand years according to the following rules :
 * [wy ; 99] -> [ wrap_year ; 100*(1 + wrap_year/100) - 1 ]
 * [00 ; wy-1] -> [ 100*(1 + wrap_year/100) ; wrap_year + 99]
 */
static int expanded_year(const Project *pr, Int_token y)
{
    if (pr->two_digits_years) {
        int century = 100*(1 + pr->wrap_year/100);
        int wy      = pr->wrap_year - (century - 100);
        if (y.value >= 0 && y.value < wy && y.digits <= 2) {
            return century + y.value;
        } else if (y.value >= wy && y.value < 100 && y.digits <= 2) {
            return century - 100 + y.value;
        } else {
            return y.value;
        }
    } else {
        return y.value;
    }
}

/*
 * check the existence of given calendar elements
 * this includes either number of day in the month
 * and calendars pecularities (year 0 and October 1582)
 */
static int check_date(const Project *pr,
    Int_token y, Int_token m, Int_token d, long *jul)
{
    int y_expand, y_check, m_check, d_check;

    y_expand = expanded_year(pr, y);

    if (m.digits > 2 || d.digits > 2) {
        /* this should be the year instead of either the month or the day */
        return RETURN_FAILURE;
    }

    *jul = cal_to_jul(y_expand, m.value, d.value);
    jul_to_cal(*jul, &y_check, &m_check, &d_check);
    if (y_expand != y_check || m.value != m_check || d.value != d_check) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
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
        if (*s == '+') {
            s++;
        }
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
        return RETURN_FAILURE;
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

    return RETURN_SUCCESS;
}


/*
 * lexical analyzer for calendar dates
 * return the number of read elements, or -1 on failure
 */
static int parse_calendar_date(const char* s, Int_token tab[5], double *sec)
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

          case '/' : case ':' : case '.' : case 'T' : /* non-repeatable separator */
              if (waiting_separator) {
                  if ((*s == 'T') && (i != 3)) {
                      /* the T separator is only allowed between date
                         and time (mainly for iso8601) */
                      return -1;
                  }
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
        if (parse_float(s, sec, &s) == RETURN_SUCCESS) {
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
int parse_date(const Quark *q, const char* s, Dates_format preferred, int absolute,
               double *jul, Dates_format *recognized)
{
    Project *pr = project_get_data(q);
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

              if (check_date(pr, tab[ky], tab[km], tab[kd], &j)
                  == RETURN_SUCCESS) {
                  *jul =
                      jul_and_time_to_jul(j, tab[3].value, tab[4].value, sec);
                  if (!absolute) {
                      *jul -= pr->ref_date;
                  }

                  *recognized = trials[i];
                  return RETURN_SUCCESS;
              }
          }
          break;

      default :
          /* probably a julian date */
          break;

    }

    return RETURN_FAILURE;
}

int parse_date_or_number(const Quark *q, const char* s,
    int absolute, Dates_format hint, double *value)
{
    Dates_format dummy;
    const char *sdummy;
    
    if (parse_date(q, s, hint, absolute, value, &dummy) == RETURN_SUCCESS) {
        return RETURN_SUCCESS;
    } else if (parse_float(s, value, &sdummy) == RETURN_SUCCESS) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
