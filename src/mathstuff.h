/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2001 Grace Development Team
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

/* Wrappers for some math functions */

#ifndef _MATHSTUFF_H__
#define _MATHSTUFF_H__

double ai_wrap(double x);
double bi_wrap(double x);
double chi_wrap(double x);
double ci_wrap(double x);
double ellpe_wrap(double x);
double ellpk_wrap(double x);
double fresnlc_wrap(double x);
double fresnls_wrap(double x);
double fx(double x);
double iv_wrap(double v, double x);
double jv_wrap(double v, double x);
double kn_wrap(int n, double x);
double max_wrap(double x, double y);
double min_wrap(double x, double y);
double irand_wrap(int x);
double rnorm(double mean, double sdev);
double shi_wrap(double x);
double si_wrap(double x);
double sqr_wrap(double x);
double yv_wrap(double v, double x);

/* constants */
double pi_const(void);
double deg_uconst(void);
double rad_uconst(void);

#endif /* _MATHSTUFF_H__ */
