/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
 *
 * curve fitting, and other numerical routines used in compose.
 *
 * Contents:
 *
 * void stasum() - compute mean and variance
 * void linearconv() - convolve one set with another
 * int crosscorr() - cross/auto correlation
 * void spline() - compute a spline fit
 * int seval() - evaluate the spline computed in spline()
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "utils.h"
#include "protos.h"


/*
	compute mean and standard dev
*/
void stasum(double *x, int n, double *xbar, double *sd)
{
    int i;

    *xbar = 0;
    *sd = 0;
    
    if (x == NULL) {
        return;
    }
    
    if (n < 1) {
	return;
    }
    
    for (i = 0; i < n; i++) {
        *xbar = (*xbar) + x[i];
    }
    *xbar = (*xbar)/n;
    
    for (i = 0; i < n; i++) {
        *sd = (*sd) + (x[i] - *xbar) * (x[i] - *xbar);
    }
    *sd = sqrt(*sd/n);
}

/*
 * linear convolution of set x (length n) with h (length m) and
 * result to y (length n + m - 1).
 */
void linearconv(double *x, int n, double *h, int m, double *y)
{
    int i, j, itmp;

    for (i = 0; i < n + m - 1; i++) {
	y[i] = 0.0;
        for (j = 0; j < m; j++) {
	    itmp = i - j;
	    if ((itmp >= 0) && (itmp < n)) {
		y[i] = y[i] + h[j] * x[itmp];
	    }
	}
    }
}

/*
 * cross correlation
 */
int crosscorr(double *x, double *y, int n, int maxlag, double *xcor)
{
    double xbar, xsd;
    double ybar, ysd;
    int i, j;

    if (maxlag + 2 > n)
	return 1;
    for (i = 0; i <= maxlag; i++) {
        stasum(&x[i], n - i, &xbar, &xsd);
        if (xsd == 0.0)
	    return 2;
        stasum(y, n - i, &ybar, &ysd);
        if (ysd == 0.0)
	    return 3;
	xcor[i] = 0.0;
	for (j = 0; j < n - i; j++) {
	    xcor[i] += (y[j] - ybar) * (x[j + i] - xbar);
	}
	xcor[i] /= (n - i)*xsd*ysd;
    }
    return 0;
}


/*
	an almost literal translation of the spline routine in
	Forsyth, Malcolm, and Moler
*/
void spline(int n, double *x, double *y, double *b, double *c, double *d)
{
/*
c
c  the coefficients b(i), c(i), and d(i), i=1,2,...,n are computed
c  for a cubic interpolating spline
c
c    s(x) = y(i) + b(i)*(x-x(i)) + c(i)*(x-x(i))**2 + d(i)*(x-x(i))**3
c
c    for  x(i) .le. x .le. x(i+1)
c
c  input..
c
c    n = the number of data points or knots (n.ge.2)
c    x = the abscissas of the knots in strictly increasing order
c    y = the ordinates of the knots
c
c  output..
c
c    b, c, d  = arrays of spline coefficients as defined above.
c
c  using  p  to denote differentiation,
c
c    y(i) = s(x(i))
c    b(i) = sp(x(i))
c    c(i) = spp(x(i))/2
c    d(i) = sppp(x(i))/6  (derivative from the right)
c
c  the accompanying function subprogram  seval	can be used
c  to evaluate the spline.
c
c
*/

    int nm1, ib, i;
    double t;

/*
Gack!
*/
    x--;
    y--;
    b--;
    c--;
    d--;

/*
Fortran 66
*/
    if (n < 2) {
        return;
    }
    
    if (n < 3) {
        b[1] = (y[2] - y[1]) / (x[2] - x[1]);
        c[1] = 0.0;
        d[1] = 0.0;
        b[2] = b[1];
        c[2] = 0.0;
        d[2] = 0.0;
        return;
    }

    nm1 = n - 1;
/*
c
c  set up tridiagonal system
c
c  b = diagonal, d = offdiagonal, c = right hand side.
c
*/
    d[1] = x[2] - x[1];
    c[2] = (y[2] - y[1]) / d[1];
    for (i = 2; i <= nm1; i++) {
	d[i] = x[i + 1] - x[i];
	b[i] = 2.0 * (d[i - 1] + d[i]);
	c[i + 1] = (y[i + 1] - y[i]) / d[i];
	c[i] = c[i + 1] - c[i];
    }
/*
c
c  end conditions.  third derivatives at  x(1)	and  x(n)
c  obtained from divided differences
c
*/
    b[1] = -d[1];
    b[n] = -d[n - 1];
    c[1] = 0.0;
    c[n] = 0.0;

    if (n != 3) { /* i.e. n > 3 here */
	c[1] = c[3] / (x[4] - x[2]) - c[2] / (x[3] - x[1]);
	c[n] = c[n - 1] / (x[n] - x[n - 2]) - c[n - 2] / (x[n - 1] - x[n - 3]);
	c[1] = c[1] * d[1] * d[1] / (x[4] - x[1]);
	c[n] = -c[n] * d[n - 1] * d[n - 1] / (x[n] - x[n - 3]);
    }
/*
c
c  forward elimination
c
*/
    for (i = 2; i <= n; i++) {
	t = d[i - 1] / b[i - 1];
	b[i] = b[i] - t * d[i - 1];
	c[i] = c[i] - t * c[i - 1];
    }
/*
c
c  back substitution
c
*/
    c[n] = c[n] / b[n];
    for (ib = 1; ib <= nm1; ib++) {
	i = n - ib;
	c[i] = (c[i] - d[i] * c[i + 1]) / b[i];
    }
/*
c
c  c(i) is now the sigma(i) of the text
c
c  compute polynomial coefficients
c
*/
    b[n] = (y[n] - y[nm1]) / d[nm1] + d[nm1] * (c[nm1] + 2.0 * c[n]);
    for (i = 1; i <= nm1; i++) {
	b[i] = (y[i + 1] - y[i]) / d[i] - d[i] * (c[i + 1] + 2.0 * c[i]);
	d[i] = (c[i + 1] - c[i]) / d[i];
	c[i] = 3.0 * c[i];
    }
    c[n] = 3.0 * c[n];
    d[n] = d[n - 1];
    return;

}

/***************************************************************************
 * aspline - modified version of David Frey's spline.c                     *
 *                                                                         *    
 * aspline does an Akima spline interpolation.                             *
 ***************************************************************************/

void aspline(int n, double *x, double *y, double *b, double *c, double *d)
{
  int i;
 	
  double num, den;
  double m_m1, m_m2, m_p1, m_p2;
  double x_m1, x_m2, x_p1, x_p2;
  double y_m1, y_m2, y_p1, y_p2;

#define dx(i) (x[i+1]-x[i])
#define dy(i) (y[i+1]-y[i])
#define  m(i) (dy(i)/dx(i))

  if (n > 0)		     /* we have data to process */
  {

      /*
       * calculate the coefficients of the spline 
       * (the Akima interpolation itself)                      
       */

      /* b) interpolate the missing points: */

      x_m1 = x[0] + x[1] - x[2]; 
      y_m1 = (x[0]-x_m1) * (m(1) - 2 * m(0)) + y[0];

      m_m1 = (y[0]-y_m1)/(x[0]-x_m1);
       
      x_m2 = 2 * x[0] - x[2];
      y_m2 = (x_m1-x_m2) * (m(0) - 2 * m_m1) + y_m1;
       
      m_m2 = (y_m1-y_m2)/(x_m1-x_m2);

      x_p1 = x[n-1] + x[n-2] - x[n-3];
      y_p1 = (2 * m(n-2) - m(n-3)) * (x_p1 - x[n-1]) + y[n-1];

      m_p1 = (y_p1-y[n-1])/(x_p1-x[n-1]);
      
      x_p2 = 2 * x[n-1] - x[n-3];
      y_p2 = (2 * m_p1 - m(n-2)) * (x_p2 - x_p1) + y_p1;
      
      m_p2 = (y_p2-y_p1)/(x_p2-x_p1);
           
      /* i = 0 */
      num=fabs(m(1) - m(0)) * m_m1 + fabs(m_m1 - m_m2) * m(0);
      den=fabs(m(1) - m(0)) + fabs(m_m1 - m_m2);
    	
      if (den != 0.0) b[0]=num / den;
      else            b[0]=0.0;
		
      /* i = 1 */
      num=fabs(m(2) - m(1)) * m(0) + fabs(m(0) - m_m1) * m(1);
      den=fabs(m(2) - m(1)) + fabs(m(0) - m_m1);

      if (den != 0.0) b[1]=num / den;
      else            b[1]=0.0;
			
      for (i=2; i < n-2; i++)
      {

	num=fabs(m(i+1) - m(i)) * m(i-1) + fabs(m(i-1) - m(i-2)) * m(i);
	den=fabs(m(i+1) - m(i)) + fabs(m(i-1) - m(i-2));

	if (den != 0.0) b[i]=num / den;
	else            b[i]=0.0;
      }

      /* i = n - 2 */
      num=fabs(m_p1 - m(n-2)) * m(n-3) + fabs(m(n-3) - m(n-4)) * m(n-2);
      den=fabs(m_p1 - m(n-2)) + fabs(m(n-3) - m(n-4));

      if (den != 0.0) b[n-2]=num / den;
      else	      b[n-2]=0.0;
 
      /* i = n - 1 */
      num=fabs(m_p2 - m_p1) * m(n-2) + fabs(m(n-2) - m(n-3)) * m_p1;
      den=fabs(m_p2 - m_p1) + fabs(m(n-2) - m(n-3));

      if (den != 0.0) b[n-1]=num / den;
      else	      b[n-1]=0.0;
 
      for (i=0; i < n-1; i++)
      {
  	   double dxv = dx(i);
  	   c[i]=(3 * m(i) - 2 * b[i] - b[i+1]) / dxv;
	   d[i]=(b[i] + b[i+1] - 2 * m(i)) / (dxv * dxv);
      }
  }
#undef dx
#undef dy
#undef  m
}

int seval(double *u, double *v, int ulen,
    double *x, double *y, double *b, double *c, double *d, int n)
{

/*
 * 
 *  this subroutine evaluates the cubic spline function on a mesh
 * 
 *    seval = y(i) + b(i)*(u-x(i)) + c(i)*(u-x(i))**2 + d(i)*(u-x(i))**3
 * 
 *    where  x(i) .lt. u .lt. x(i+1), using horner's rule
 * 
 *  if  u .lt. x(1) then  i = 1  is used.
 *  if  u .ge. x(n) then  i = n  is used.
 * 
 *  input..
 * 
 *    u = the array of abscissas at which the spline is to be evaluated
 *    ulen = length of the mesh
 * 
 *    x,y = the arrays of data abscissas and ordinates
 *    b,c,d = arrays of spline coefficients computed by spline
 *    n = the number of data points
 * 
 *  output..
 * 
 *    v = the array of evaluated values
 */

    int j, m;

    m = monotonicity(x, n, FALSE);
    if (m == 0) {
        errmsg("seval() called with a non-monotonic array");
        return RETURN_FAILURE;
    }
    
    for (j = 0; j < ulen; j++) {
        double dx;
        int ifound;
        
        ifound = find_span_index(x, n, m, u[j]);
        if (ifound < 0) {
            ifound = 0;
        } else if (ifound > n - 2) {
            ifound = n - 1;
        }
        dx = u[j] - x[ifound];
        v[j] = y[ifound] + dx*(b[ifound] + dx*(c[ifound] + dx*d[ifound]));
    }
    
    return RETURN_SUCCESS;
}
