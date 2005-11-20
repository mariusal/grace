/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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
 * procedures for performing transformations from the command
 * line interpreter and the GUI.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cephes.h"

#include "core_utils.h"
#include "ssdata.h"
#include "parser.h"
#include "numerics.h"
#include "protos.h"

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

/* compute mean and standard dev */
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
    
    if (n > 1) {
        for (i = 0; i < n; i++) {
            *sd = (*sd) + (x[i] - *xbar) * (x[i] - *xbar);
        }
        *sd = sqrt(*sd/(n - 1));
    }
}

/*
 * trapezoidal rule
 */
double trapint(double *x, double *y, double *resx, double *resy, int n)
{
    int i;
    double sum = 0.0;
    double h;

    if (n < 2) {
        return 0.0;
    }
    
    if (resx != NULL) {
        resx[0] = x[0];
    }
    if (resy != NULL) {
        resy[0] = 0.0;
    }
    for (i = 1; i < n; i++) {
	h = (x[i] - x[i - 1]);
	if (resx != NULL) {
	    resx[i] = x[i];
	}
	sum = sum + h * (y[i - 1] + y[i]) * 0.5;
	if (resy != NULL) {
	    resy[i] = sum;
	}
    }
    return sum;
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

int vbarycenter(double *x, double *y, int n, double *barycenter)
{
    int i;
    double wsum = 0.0, xsum = 0.0;

    if (n < 1 || !x || !y) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < n; i++) {
        wsum += x[i]*y[i];
        xsum += x[i];
    }
    *barycenter = wsum/xsum;
    return RETURN_SUCCESS;
}

/*
 * an almost literal translation of the spline routine in
 * Forsyth, Malcolm, and Moler
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

    int ib, i;
    double t;

    if (n < 2) {
        return;
    }
    
    if (n < 3) {
        b[0] = (y[1] - y[0]) / (x[1] - x[0]);
        c[0] = 0.0;
        d[0] = 0.0;
        b[1] = b[0];
        c[1] = 0.0;
        d[1] = 0.0;
        return;
    }

/*
c
c  set up tridiagonal system
c
c  b = diagonal, d = offdiagonal, c = right hand side.
c
*/
    d[0] = x[1] - x[0];
    c[1] = (y[1] - y[0]) / d[0];
    for (i = 1; i < n - 1; i++) {
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
    b[0] = -d[0];
    b[n - 1] = -d[n - 2];
    c[0] = 0.0;
    c[n - 1] = 0.0;

    if (n != 3) { /* i.e. n > 3 here */
	c[0] = c[2] / (x[3] - x[1]) - c[1] / (x[2] - x[0]);
	c[n - 1] = c[n - 2] / (x[n - 1] - x[n - 3]) - c[n - 3] / (x[n - 2] - x[n - 4]);
	c[0] = c[0] * d[0] * d[0] / (x[3] - x[0]);
	c[n - 1] = -c[n - 1] * d[n - 2] * d[n - 2] / (x[n - 1] - x[n - 4]);
    }
/*
c
c  forward elimination
c
*/
    for (i = 1; i < n; i++) {
	t = d[i - 1] / b[i - 1];
	b[i] = b[i] - t * d[i - 1];
	c[i] = c[i] - t * c[i - 1];
    }
/*
c
c  back substitution
c
*/
    c[n - 1] = c[n - 1] / b[n - 1];
    for (ib = 1; ib <= n - 1; ib++) {
	i = n - ib - 1;
	c[i] = (c[i] - d[i] * c[i + 1]) / b[i];
    }
/*
c
c  c(i) is now the sigma(i) of the text
c
c  compute polynomial coefficients
c
*/
    b[n - 1] = (y[n - 1] - y[n - 2]) / d[n - 2] + d[n - 2] * (c[n - 2] + 2.0 * c[n - 1]);
    for (i = 0; i < n - 1; i++) {
	b[i] = (y[i + 1] - y[i]) / d[i] - d[i] * (c[i + 1] + 2.0 * c[i]);
	d[i] = (c[i + 1] - c[i]) / d[i];
	c[i] = 3.0 * c[i];
    }
    c[n - 1] = 3.0 * c[n - 1];
    d[n - 1] = d[n - 2];
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


/*
 * compute the min and max of vector vec calculated for indices such that
 * bvec values lie within [bmin, bmax] range
 * returns RETURN_FAILURE if none found
 */
int minmaxrange(double *bvec, double *vec, int n, double bvmin, double bvmax,
              	   double *vmin, double *vmax)
{
    int i, first = TRUE;
    
    if ((vec == NULL) || (bvec == NULL)) {
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < n; i++) {
        if ((bvec[i] >= bvmin) && (bvec[i] <= bvmax)) {
	    if (first == TRUE) {
                *vmin = vec[i];
                *vmax = vec[i];
                first = FALSE;
            } else {
                if (vec[i] < *vmin) {
                    *vmin = vec[i];
  	        } else if (vec[i] > *vmax) {
                    *vmax = vec[i];
       	        }
            }
        }
    }
    
    if (first == FALSE) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


/*
 * compute the mins and maxes of a vector x
 */
double vmin(double *x, int n)
{
    int i;
    double xmin;
    if (n <= 0) {
	return 0.0;
    }
    xmin = x[0];
    for (i = 1; i < n; i++) {
	if (x[i] < xmin) {
	    xmin = x[i];
	}
    }
    return xmin;
}

double vmax(double *x, int n)
{
    int i;
    double xmax;
    if (n <= 0) {
	return 0.0;
    }
    xmax = x[0];
    for (i = 1; i < n; i++) {
	if (x[i] > xmax) {
	    xmax = x[i];
	}
    }
    return xmax;
}

static int dbl_comp(const void *a, const void *b)
{
    if (*(double *)a < *(double *)b) {
        return -1;
    } else if (*(double *)a > *(double *)b) {
        return 1;
    } else {
        return 0;
    }
}

/* get the median of a vector */
int vmedian(double *x, int n, double *med)
{
    double *d;

    if (n < 1) {
        return RETURN_FAILURE;
    } else if (n == 1) {
        *med = x[0];
    } else {
        if (monotonicity(x, n, FALSE) == 0) {
            d = copy_data_column_simple(x, n);
            if (!d) {
                return RETURN_FAILURE;
            }
            qsort(d, n, SIZEOF_DOUBLE, dbl_comp);
        } else {
            d = x;
        }
        if (n % 2) {
            /* odd length */
            *med = d[(n + 1)/2 - 1];
        } else {
            *med = (d[n/2 - 1] + d[n/2])/2;
        }

        if (d != x) {
            /* FIXME (no direct use of *alloc/free) !!! */
            xfree(d);
        }
    }
    
    return RETURN_SUCCESS;
}

int find_span_index(double *array, int len, int m, double x)
{
    int ind, low = 0, high = len - 1;
    
    if (len < 2 || m == 0) {
        errmsg("find_span_index() called with a non-monotonic array");
        return -2;
    } else if (m > 0) {
        /* ascending order */
        if (x < array[0]) {
            return -1;
        } else if (x > array[len - 1]) {
            return len - 1;
        } else {
            while (low <= high) {
	        ind = (low + high) / 2;
	        if (x < array[ind]) {
	            high = ind - 1;
	        } else if (x > array[ind + 1]) {
	            low = ind + 1;
	        } else {
	            return ind;
	        }
            }
        }
    } else {
        /* descending order */
        if (x > array[0]) {
            return -1;
        } else if (x < array[len - 1]) {
            return len - 1;
        } else {
            while (low <= high) {
	        ind = (low + high) / 2;
	        if (x > array[ind]) {
	            high = ind - 1;
	        } else if (x < array[ind + 1]) {
	            low = ind + 1;
	        } else {
	            return ind;
	        }
            }
        }
    }

    /* should never happen */
    errmsg("internal error in find_span_index()");
    return -2;
}

int dump_dc(double *v, int len)
{
    int i;
    double avg, dummy;
    
    if (len < 1 || !v) {
        return RETURN_FAILURE;
    }
    
    stasum(v, len, &avg, &dummy);
    for (i = 0; i < len; i++) {
        v[i] -= avg;
    }
    
    return RETURN_SUCCESS;
}

int apply_window(double *v, int len, int window, double beta)
{
    int i;

    if (len < 2 || !v) {
        return RETURN_FAILURE;
    }
    
    if (window == FFT_WINDOW_NONE) {
        return RETURN_SUCCESS;
    }
    
    for (i = 0; i < len; i++) {
	double c, w, tmp;
        w = 2*M_PI*i/(len - 1);
        switch (window) {
	case FFT_WINDOW_TRIANGULAR:
	    c = 1.0 - fabs((i - 0.5*(len - 1))/(0.5*(len - 1)));
	    break;
	case FFT_WINDOW_PARZEN:
	    c = 1.0 - fabs((i - 0.5*(len - 1))/(0.5*(len + 1)));
	    break;
	case FFT_WINDOW_WELCH:
	    tmp = (i - 0.5*(len - 1))/(0.5*(len + 1));
            c = 1.0 - tmp*tmp;
	    break;
	case FFT_WINDOW_HANNING:
	    c = 0.5 - 0.5*cos(w);
	    break;
	case FFT_WINDOW_HAMMING:
	    c = 0.54 - 0.46*cos(w);
	    break;
	case FFT_WINDOW_BLACKMAN:
	    c = 0.42 - 0.5*cos(w) + 0.08*cos(2*w);
	    break;
	case FFT_WINDOW_FLATTOP:
	    c = 0.2810639 - 0.5208972*cos(w) + 0.1980399*cos(2*w);
	    break;
	case FFT_WINDOW_KAISER:
	    tmp = (i - 0.5*(len - 1))/(0.5*(len - 1));
            c = i0(beta*sqrt(1.0 - tmp*tmp))/i0(beta);
	    break;
	default:	/* should never happen */
            c = 0.0;
            return RETURN_FAILURE;
	    break;
        }
    
        v[i] *= c;
    }
    
    return RETURN_SUCCESS;
}

int histogram(int ndata, double *data, int nbins, double *bins, int *hist)
{
    int i, j, bsign;
    double minval, maxval;
    
    if (nbins < 1) {
        errmsg("Number of bins < 1");
        return RETURN_FAILURE;
    }
    
    bsign = monotonicity(bins, nbins + 1, TRUE);
    if (bsign == 0) {
        errmsg("Non-monotonic bins");
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nbins; i++) {
        hist[i] = 0;
    }
    
    /* TODO: binary search */
    for (i = 0; i < ndata; i++) {
        for (j = 0; j < nbins; j++) {
            if (bsign > 0) {
                minval = bins[j];
                maxval = bins[j + 1];
            } else {
                minval = bins[j + 1];
                maxval = bins[j];
            }
            if (data[i] >= minval && data[i] <= maxval) {
                hist[j] += 1;
                break;
            }
        }
    }
    return RETURN_SUCCESS;
}

static int inside_dxdy(double ddx, double ddy, double dx, double dy, int elliptic)
{
    if (dx <= 0.0 || dy <= 0.0) {
        return FALSE;
    }
    
    if (elliptic) {
        if (hypot(ddx/dx, ddy/dy) <= 1.0) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if (fabs(ddx) <= dx && fabs(ddy) <= dy) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

static int prune_plain_inside(double old_x, double old_y, double x, double y,
    int elliptic, double dx, int reldx, double dy, int reldy)
{
    double ddx, ddy;
    
    ddx = x - old_x;
    ddy = y - old_y;
    
    if (reldx) {
        if (old_x == 0.0) {
            return FALSE;
        } else {
            ddx /= old_x;
        }
    }
    if (reldy) {
        if (old_y == 0.0) {
            return FALSE;
        } else {
            ddy /= old_y;
        }
    }
    
    return inside_dxdy(ddx, ddy, dx, dy, elliptic);
}

static int prune_interp_inside(double *x, double *y, int len,
    int elliptic, double dx, int reldx, double dy, int reldy)
{
    if (len < 3) {
        return TRUE;
    } else {
        int j;
        double x10, y10, x20, y20;
        x20 = x[len - 1] - x[0];
        y20 = y[len - 1] - y[0];
        for (j = 1; j < len - 1; j++) {
            double t, den, ddx, ddy;
            
            x10 = x[j] - x[0];
            y10 = y[j] - y[0];
            
            den = x20*x20 + y20*y20;
            if (den == 0.0) {
                return FALSE;
            }
            
            t = (x10*y20 - x20*y10)/den;
            ddx =   y20*t;
            ddy = - x20*t;
            
            if (reldx) {
                if (x[j] == 0.0) {
                    return FALSE;
                } else {
                    ddx /= x[j];
                }
            }
            if (reldy) {
                if (y[j] == 0.0) {
                    return FALSE;
                } else {
                    ddy /= y[j];
                }
            }

            if (!inside_dxdy(ddx, ddy, dx, dy, elliptic)) {
                return FALSE;
            }
        }
    }
    
    return TRUE;
}

int interpolate(double *mesh, double *yint, int meshlen,
    double *x, double *y, int len, int method)
{
    double *b, *c, *d;
    double dx;
    int i, ifound;
    int m;

    /* For linear interpolation, non-strict monotonicity is fine */
    m = monotonicity(x, len, method == INTERP_LINEAR ? FALSE:TRUE);
    if (m == 0) {
        errmsg("Can't interpolate a set with non-monotonic abscissas");
        return RETURN_FAILURE;
    }

    switch (method) {
    case INTERP_SPLINE:
    case INTERP_ASPLINE:
        b = xcalloc(len, SIZEOF_DOUBLE);
        c = xcalloc(len, SIZEOF_DOUBLE);
        d = xcalloc(len, SIZEOF_DOUBLE);
        if (b == NULL || c == NULL || d == NULL) {
            xfree(b);
            xfree(c);
            xfree(d);
            return RETURN_FAILURE;
        }
        if (method == INTERP_ASPLINE){
            /* Akima spline */
            aspline(len, x, y, b, c, d);
        } else {
            /* Plain cubic spline */
            spline(len, x, y, b, c, d);
        }

	seval(mesh, yint, meshlen, x, y, b, c, d, len);

        xfree(b);
        xfree(c);
        xfree(d);
        break;
    default:
        /* linear interpolation */

        for (i = 0; i < meshlen; i++) {
            ifound = find_span_index(x, len, m, mesh[i]);
            if (ifound < 0) {
                ifound = 0;
            } else if (ifound > len - 2) {
                ifound = len - 2;
            }
            dx = x[ifound + 1] - x[ifound];
            if (dx) {
                yint[i] = y[ifound] + (mesh[i] - x[ifound])*
                    ((y[ifound + 1] - y[ifound])/dx);
            } else {
                yint[i] = (y[ifound] + y[ifound + 1])/2;
            }
        }
        break;
    }
    
    return RETURN_SUCCESS;
}

int monotonicity(double *array, int len, int strict)
{
    int i;
    int s0, s1;
    
    if (len < 2) {
        errmsg("Monotonicity of an array of length < 2 is meaningless");
        return 0;
    }
    
    s0 = sign(array[1] - array[0]);
    for (i = 2; i < len; i++) {
        s1 = sign(array[i] - array[i - 1]);
        if (s1 != s0) {
            if (strict) {
                return 0;
            } else if (s0 == 0) {
                s0 = s1;
            } else if (s1 != 0) {
                return 0;
            }
        }
    }
    
    return s0;
}

int monospaced(double *array, int len, double *space)
{
    int i;
    double eps;
    
    if (len < 2) {
        errmsg("Monospacing of an array of length < 2 is meaningless");
        return FALSE;
    }
    
    *space = array[1] - array[0];
    eps = fabs((array[len - 1] - array[0]))*1.0e-6; /* FIXME */
    for (i = 2; i < len; i++) {
        if (fabs(array[i] - array[i - 1] - *space) > eps) {
            return FALSE;
        }
    }
    
    return TRUE;
}

int filter_set(Quark *pset, char *rarray)
{
#if 0
    int i, ip, j, ncols;
    Dataset *dsp;
    
    if (!pset) {
        return RETURN_FAILURE;
    }
    if (rarray == NULL) {
        return RETURN_SUCCESS;
    }
    ncols = set_get_ncols(pset);
    dsp = set_get_dataset(pset);
    ip = 0;
    for (i = 0; i < dsp->len; i++) {
        if (rarray[i]) {
            for (j = 0; j < ncols; j++) {
                dsp->ex[j][ip] = dsp->ex[j][i];
            }
            if (dsp->s != NULL) {
                dsp->s[ip] = copy_string(dsp->s[ip], dsp->s[i]);
            }
            ip++;
        }
    }
    set_set_length(pset, ip);
#endif
    return RETURN_SUCCESS;
}

/*
 * evaluate a formula
 */
int do_compute(Quark *psrc, Quark *pdest, char *rarray, char *fstr)
{
    if (!set_is_dataless(psrc)) {
	if (psrc != pdest) {
	    if (copysetdata(psrc, pdest) != RETURN_SUCCESS) {
	        return RETURN_FAILURE;
            }
        }
	filter_set(pdest, rarray);
        set_parser_setno(pdest);
        if (scanner(fstr) != RETURN_SUCCESS) {
	    if (psrc != pdest) {
		quark_free(pdest);
	    }
	    return RETURN_FAILURE;
	} else {
	    quark_dirtystate_set(pdest, TRUE);
            return RETURN_SUCCESS;
        }
    } else {
        return RETURN_FAILURE;
    }
}

/*
 * difference a set
 */
int do_differ(Quark *psrc, Quark *pdest,
    int derivative, int xplace, int period)
{
    int i, ncols, nc, len, newlen;
    double *x1, *x2;
    char *stype, pbuf[32], buf[256];
    
    if (set_is_dataless(psrc)) {
	errmsg("Set not active");
	return RETURN_FAILURE;
    }
    
    if (period < 1) {
	errmsg("Non-positive period");
	return RETURN_FAILURE;
    }
    
    len = set_get_length(psrc);
    newlen = len - period;
    if (newlen <= 0) {
	errmsg("Source set length <= differentiation period");
	return RETURN_FAILURE;
    }
    
    x1 = set_get_col(psrc, DATA_X);
    if (derivative) {
        for (i = 0; i < newlen; i++) {
            if (x1[i + period] - x1[i] == 0.0) {
	        char buf[256];
                sprintf(buf, "Can't evaluate derivative, x1[%d] = x1[%d]",
                    i, i + period);
                errmsg(buf);
                return RETURN_FAILURE;
            }
        }
    }
    
    if (set_set_length(pdest, newlen) != RETURN_SUCCESS) {
	return RETURN_FAILURE;
    }
    
    ncols = set_get_ncols(psrc);
    if (set_get_ncols(pdest) != ncols) {
        set_set_type(pdest, set_get_type(psrc));
    }
    
    for (nc = 1; nc < ncols; nc++) {
        double h, *d1, *d2;
        d1 = set_get_col(psrc, nc);
        d2 = set_get_col(pdest, nc);
        for (i = 0; i < newlen; i++) {
            d2[i] = d1[i + period] - d1[i];
            if (derivative) {
                h = x1[i + period] - x1[i];
                d2[i] /= h;
            }
        }
    }
    
    x2 = set_get_col(pdest, DATA_X);
    for (i = 0; i < newlen; i++) {
        switch (xplace) {
        case DIFF_XPLACE_LEFT:
            x2[i] = x1[i];
	    break;
        case DIFF_XPLACE_RIGHT:
            x2[i] = x1[i + period];
	    break;
        case DIFF_XPLACE_CENTER:
            x2[i] = (x1[i + period] + x1[i])/2;
	    break;
        }
    }
    
    /* Prepare set comments */
    switch (xplace) {
    case DIFF_XPLACE_LEFT:
	stype = "Left";
	break;
    case DIFF_XPLACE_RIGHT:
	stype = "Right";
	break;
    case DIFF_XPLACE_CENTER:
	stype = "Centered";
	break;
    default:
	errmsg("Wrong parameters passed to do_differ()");
	return RETURN_FAILURE;
        break;
    }
    
    if (period != 1) {
        sprintf(pbuf, " (period = %d)", period);
    } else {
        pbuf[0] = '\0';
    }
    sprintf(buf, "%s %s%s of set %s", stype, pbuf,
       derivative ? "derivative":"difference", QIDSTR(psrc));
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}

/*
 * linear convolution
 */
int do_linearc(Quark *psrc, Quark *pdest,
    Quark *pconv)
{
    int srclen, convlen, destlen, i, ncols, nc;
    double xspace1, xspace2, *xsrc, *xconv, *xdest, *yconv;
    char buf[256];

    srclen  = set_get_length(psrc);
    convlen = set_get_length(pconv);
    if (srclen < 2 || convlen < 2) {
	errmsg("Set length < 2");
	return RETURN_FAILURE;
    }
    
    destlen = srclen + convlen - 1;

    xsrc  = set_get_col(psrc, DATA_X);
    if (monospaced(xsrc, srclen, &xspace1) != TRUE) {
        errmsg("Abscissas of the set are not monospaced");
        return RETURN_FAILURE;
    } else {
        if (xspace1 == 0.0) {
            errmsg("The set spacing is 0");
            return RETURN_FAILURE;
        }
    }

    xconv = set_get_col(pconv, DATA_X);
    if (monospaced(xconv, convlen, &xspace2) != TRUE) {
        errmsg("Abscissas of the set are not monospaced");
        return RETURN_FAILURE;
    } else {
        if (fabs(xspace2/xspace1 - 1) > 0.01/(srclen + convlen)) {
            errmsg("The source and convoluting functions are not equally sampled");
            return RETURN_FAILURE;
        }
    }
    
    if (set_set_length(pdest, destlen) != RETURN_SUCCESS) {
	return RETURN_FAILURE;
    }

    ncols = set_get_ncols(psrc);
    if (set_get_ncols(pdest) != ncols) {
        set_set_type(pdest, set_get_type(psrc));
    }
    
    yconv = set_get_col(pconv, DATA_Y);
    
    for (nc = 1; nc < ncols; nc++) {
        double *d1, *d2;
        
        d1 = set_get_col(psrc, nc);
        d2 = set_get_col(pdest, nc);
        
        linearconv(d1, srclen, yconv, convlen, d2);
        for (i = 0; i < destlen; i++) {
            d2[i] *= xspace1;
        }
    }

    xdest = set_get_col(pdest, DATA_X);
    for (i = 0; i < destlen; i++) {
	xdest[i] = (xsrc[0] + xconv[0]) + i*xspace1;
    }
    
    sprintf(buf, "Linear convolution of set %s with set %s",
        QIDSTR(psrc), QIDSTR(pconv));
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}

/*
 * cross correlation/covariance
 */
int do_xcor(Quark *psrc, Quark *pdest,
    Quark *pcor, int maxlag, int covar)
{
    int autocor;
    int len, i, ncols1, ncols2, ncols, nc;
    double xspace1, xspace2, *xsrc, *xcor, *xdest, xoffset;
    char *fname, buf[256];

    if (maxlag < 0) {
	errmsg("Negative max lag");
	return RETURN_FAILURE;
    }

    if (set_is_dataless(psrc)) {
	errmsg("Set not active");
	return RETURN_FAILURE;
    }
    
    len = set_get_length(psrc);
    if (len < 2) {
	errmsg("Set length < 2");
	return RETURN_FAILURE;
    }

    xsrc = set_get_col(psrc, DATA_X);
    if (monospaced(xsrc, len, &xspace1) != TRUE) {
        errmsg("Abscissas of the source set are not monospaced");
        return RETURN_FAILURE;
    } else {
        if (xspace1 == 0.0) {
            errmsg("The source set spacing is 0");
            return RETURN_FAILURE;
        }
    }

    if (psrc == pcor) {
        autocor = TRUE;
    } else {
        autocor = FALSE;
    }

    if (!autocor) {
        if (set_is_dataless(pcor)) {
	    errmsg("Set not active");
	    return RETURN_FAILURE;
        }
        
        if (set_get_length(pcor) != len) {
	    errmsg("The correlating sets are of different length");
	    return RETURN_FAILURE;
        }

        xcor = set_get_col(pcor, DATA_X);
        if (monospaced(xcor, len, &xspace2) != TRUE) {
            errmsg("Abscissas of the set are not monospaced");
            return RETURN_FAILURE;
        } else {
            if (fabs(xspace2/xspace1 - 1) > 0.01/(2*len)) {
                errmsg("The source and correlation functions are not equally sampled");
                return RETURN_FAILURE;
            }
        }
        
        xoffset = xcor[0] - xsrc[0];
    } else {
        xoffset = 0.0;
    }

    if (set_set_length(pdest, maxlag) != RETURN_SUCCESS) {
	return RETURN_FAILURE;
    }

    ncols1 = set_get_ncols(psrc);
    ncols2 = set_get_ncols(pcor);
    ncols = MIN2(ncols1, ncols2);
    if (set_get_ncols(pdest) != ncols) {
        set_set_type(pdest, set_get_type(psrc));
    }

    for (nc = 1; nc < ncols; nc++) {
        int buflen;
        double *d1_re, *d1_im, *d2_re, *d2_im, *dres;
        double xbar, sd;
        double cnorm = 1.0;
        
        buflen = len + maxlag;
        
        /* reallocate input to pad with zeros */
        d1_re = copy_data_column_simple(set_get_col(psrc, nc), len);
        d1_re = xrealloc(d1_re, SIZEOF_DOUBLE*buflen);
        d1_im = xcalloc(buflen, SIZEOF_DOUBLE);
        if (!d1_re || !d1_im) {
            xfree(d1_re);
            xfree(d1_im);
            return RETURN_FAILURE;
        }
        if (covar) {
            /* substract mean value if doing covariance */
            stasum(d1_re, len, &xbar, &sd);
            for (i = 0; i < len; i++) {
                d1_re[i] -= xbar;
            }
        }
        /* pad with zeros */
        for (i = len; i < buflen; i++) {
            d1_re[i] = 0.0;
        }
        
        /* perform FT */
        fourier(d1_re, d1_im, buflen, FALSE);
        
        /* do the same with the second input if not autocorrelating */
        if (!autocor) {
            d2_re = copy_data_column_simple(set_get_col(pcor, nc), len);
            d2_re = xrealloc(d2_re, SIZEOF_DOUBLE*buflen);
            d2_im = xcalloc(buflen, SIZEOF_DOUBLE);
            if (!d1_im || !d2_im) {
                xfree(d1_re);
                xfree(d2_re);
                xfree(d1_im);
                xfree(d2_im);
                return RETURN_FAILURE;
            }
            if (covar) {
                stasum(d2_re, len, &xbar, &sd);
                for (i = 0; i < len; i++) {
                    d2_re[i] -= xbar;
                }
            }
            for (i = len; i < buflen; i++) {
                d2_re[i] = 0.0;
            }
            
            fourier(d2_re, d2_im, buflen, FALSE);
        } else {
            d2_re = NULL;
            d2_im = NULL;
        }
        
        /* multiply fourier */
        for (i = 0; i < buflen; i++) {
            if (autocor) {
                d1_re[i] = d1_re[i]*d1_re[i] + d1_im[i]*d1_im[i];
                d1_im[i] = 0.0;
            } else {
                double bufc;
                bufc     = d1_re[i]*d2_re[i] + d1_im[i]*d2_im[i];
                d1_im[i] = d2_re[i]*d1_im[i] - d1_re[i]*d2_im[i];
                d1_re[i] = bufc;
            }
        }
        
        /* these are no longer used */
        xfree(d2_re);
        xfree(d2_im);

        /* perform backward FT */
        fourier(d1_re, d1_im, buflen, TRUE);
        
        /* the imaginary part must be zero */
        xfree(d1_im);
        
        dres = set_get_col(pdest, nc);
        for (i = 0; i < maxlag; i++) {
            dres[i] = d1_re[i]/buflen*xspace1;
            if (i == 0 && dres[0] != 0.0) {
                cnorm = fabs(dres[0]);
            }
            dres[i] /= cnorm;
        }
        
        /* free the remaining buffer storage */
        xfree(d1_re);
    }

    xdest = set_get_col(pdest, DATA_X);
    for (i = 0; i < maxlag; i++) {
	xdest[i] = xoffset + i*xspace1;
    }

    if (covar) {
        fname = "covariance";
    } else {
        fname = "correlation";
    }
    if (autocor) {
	sprintf(buf,
            "Auto-%s of set %s at maximum lag %d",
            fname, QIDSTR(psrc), maxlag);
    } else {
	sprintf(buf,
            "Cross-%s of sets %s and %s at maximum lag %d",
            fname, QIDSTR(psrc), QIDSTR(pcor), maxlag);
    }
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}


/*
 * numerical integration
 */
int do_int(Quark *psrc, Quark *pdest,
    int disponly, double *sum)
{
    int len;
    double *x, *y;
    
    *sum = 0.0;

    if (set_is_dataless(psrc)) {
	errmsg("Set not active");
	return RETURN_FAILURE;
    }
    
    len = set_get_length(psrc);
    if (len < 3) {
	errmsg("Set length < 3");
	return RETURN_FAILURE;
    }
    
    x = set_get_col(psrc, DATA_X);
    y = set_get_col(psrc, DATA_Y);
    if (!disponly) {
	char buf[256];
        if (set_set_length(pdest, len) != RETURN_SUCCESS) {
	    errmsg("Can't activate target set");
	    return RETURN_FAILURE;
        } else {
	    *sum = trapint(x, y, getx(pdest), gety(pdest), len);
	    sprintf(buf, "Integral of set %s", QIDSTR(psrc));
	    // set_set_comment(pdest, buf);
	}
    } else {
	*sum = trapint(x, y, NULL, NULL, len);
    }
    return RETURN_SUCCESS;
}

/*
 * running properties
 */
int do_runavg(Quark *psrc, Quark *pdest,
    int runlen, char *formula, int xplace)
{
    int i, nc, ncols, len, newlen;
    double *x1, *x2;
    grarr *t;
    char buf[256];

    if (runlen < 1) {
	errmsg("Length of running average < 1");
	return RETURN_FAILURE;
    }

    len = set_get_length(psrc);
    if (runlen > len) {
	errmsg("Length of running average > set length");
	return RETURN_FAILURE;
    }
    
    if (string_is_empty(formula)) {
	errmsg("Empty formula");
	return RETURN_FAILURE;
    }

    newlen = len - runlen + 1;
    if (set_set_length(pdest, newlen) != RETURN_SUCCESS) {
	return RETURN_FAILURE;
    }
    
    ncols = set_get_ncols(psrc);
    if (set_get_ncols(pdest) != ncols) {
        set_set_type(pdest, set_get_type(psrc));
    }
    
    t = get_parser_arr_by_name("$t");
    if (t == NULL) {
        t = define_parser_arr("$t");
        if (t == NULL) {
            errmsg("Internal error");
            return RETURN_FAILURE;
        }
    }
    
    if (t->length != 0) {
        XCFREE(t->data);
    }
    t->length = runlen;

    set_parser_setno(psrc);
    for (nc = 1; nc < ncols; nc++) {
        double *d1, *d2;
        d1 = set_get_col(psrc, nc);
        d2 = set_get_col(pdest, nc);
        for (i = 0; i < newlen; i++) {
            t->data = &(d1[i]);
            if (s_scanner(formula, &(d2[i])) != RETURN_SUCCESS) {
                t->length = 0;
                t->data = NULL;
                return RETURN_FAILURE;
            }
        }
    }
    
    /* undefine the virtual array */
    t->length = 0;
    t->data = NULL;

    x1 = set_get_col(psrc, DATA_X);
    x2 = set_get_col(pdest, DATA_X);
    for (i = 0; i < newlen; i++) {
        double dummy;
        switch (xplace) {
        case RUN_XPLACE_LEFT:
            x2[i] = x1[i];
            break;
        case RUN_XPLACE_RIGHT:
            x2[i] = x1[i + runlen - 1];
            break;
        default:
            stasum(&(x1[i]), runlen, &(x2[i]), &dummy);
            break;
        }
    }
    
    sprintf(buf, "%d-pt. running %s on %s", runlen, formula, QIDSTR(psrc));
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}


/*
 * Fourier transform
 */
int do_fourier(Quark *psrc, Quark *pdest,
    int invflag, int xscale, int norm, int complexin, int dcdump,
    double oversampling, int round2n, int window, double beta, int halflen,
    int output)
{
    int i, inlen, buflen, outlen, ncols, settype;
    double *in_x, *in_re, *in_im, *buf_re, *buf_im, *out_x, *out_y, *out_y1;
    double xspace, amp_correction;
    char buf[256];

    inlen = set_get_length(psrc);
    if (inlen < 2) {
	errmsg("Set length < 2");
	return RETURN_FAILURE;
    }

    /* get input */
    in_re = set_get_col(psrc, DATA_Y);
    if (!in_re) {
        /* should never happen */
        return RETURN_FAILURE;
    }
    if (!complexin) {
        in_im = NULL;
    } else {
        in_im = set_get_col(psrc, DATA_Y1);
    }
    
    in_x = set_get_col(psrc, DATA_X);
    if (monospaced(in_x, inlen, &xspace) != TRUE) {
        errmsg("Abscissas of the set are not monospaced, can't use for sampling");
        return RETURN_FAILURE;
    } else {
        if (xspace == 0.0) {
            errmsg("The set spacing is 0, can't continue");
            return RETURN_FAILURE;
        }
    }
    
    /* copy input data to buffer storage which will be used then to hold out */
    
    buf_re = copy_data_column_simple(in_re, inlen);
    if (in_im) {
        buf_im = copy_data_column_simple(in_im, inlen);
    } else {
        buf_im = xcalloc(inlen, SIZEOF_DOUBLE);
    }
    if (!buf_re || !buf_im) {
        xfree(buf_re);
        xfree(buf_im);
        return RETURN_FAILURE;
    }
    
    /* dump the DC component */
    if (dcdump) {
        dump_dc(buf_re, inlen);
    }
    
    /* apply data window */
    apply_window(buf_re, inlen, window, beta);
    if (in_im) {
        apply_window(buf_im, inlen, window, beta);
    }
    
    /* a safety measure */
    oversampling = MAX2(1.0, oversampling);
    
    buflen = (int) rint(inlen*oversampling);
    if (round2n) {
        /* round to the closest 2^N, but NOT throw away any data */
        int i2 = (int) rint(log2((double) buflen));
        buflen = (int) pow(2.0, (double) i2);
        if (buflen < inlen) {
            buflen *= 2;
        }
    }
    
    if (buflen != inlen) {
        buf_re = xrealloc(buf_re, SIZEOF_DOUBLE*buflen);
        buf_im = xrealloc(buf_im, SIZEOF_DOUBLE*buflen);
        
        if (!buf_re || !buf_im) {
            xfree(buf_re);
            xfree(buf_im);
            return RETURN_FAILURE;
        }
        
        /* stuff the added data with zeros */
        for (i = inlen; i < buflen; i++) {
            buf_re[i] = 0.0;
            buf_im[i] = 0.0;
        }
    }
    
    if (fourier(buf_re, buf_im, buflen, invflag) != RETURN_SUCCESS) {
        xfree(buf_re);
        xfree(buf_im);
        return RETURN_FAILURE;
    }
    
    /* amplitude correction due to the zero padding etc */
    amp_correction = (double) buflen/inlen;
    if ((invflag  && norm == FFT_NORM_BACKWARD) ||
        (!invflag && norm == FFT_NORM_FORWARD)) {
        amp_correction /= buflen;
    } else if (norm == FFT_NORM_SYMMETRIC) {
        amp_correction /= sqrt((double) buflen);
    }

    if (halflen && !complexin) {
	outlen = buflen/2 + 1;
        /* DC component */
        buf_re[0] = buf_re[0];
        buf_im[0] = 0.0;
        for (i = 1; i < outlen; i++) {
            /* carefully get amplitude of complex form: 
               use abs(a[i]) + abs(a[-i]) except for zero term */
            buf_re[i] += buf_re[buflen - i];
            buf_im[i] -= buf_im[buflen - i];
        }
    } else {
	outlen = buflen;
    }
    
    switch (output) {
    case FFT_OUTPUT_REIM:
    case FFT_OUTPUT_APHI:
        ncols = 3;
        // settype = SET_XYZ;
        break;
    default:
        ncols = 2;
        settype = SET_XY;
        break;
    }
    
    if (set_get_ncols(pdest) != ncols) {
        if (set_set_type(pdest, settype) != RETURN_SUCCESS) {
            xfree(buf_re);
            xfree(buf_im);
            return RETURN_FAILURE;
        } 
    }
    if (set_set_length(pdest, outlen) != RETURN_SUCCESS) {
        xfree(buf_re);
        xfree(buf_im);
        return RETURN_FAILURE;
    }
    
    out_y  = set_get_col(pdest, DATA_Y);
    out_y1 = set_get_col(pdest, DATA_Y1);
    
    for (i = 0; i < outlen; i++) {
        switch (output) {
        case FFT_OUTPUT_MAGNITUDE:
            out_y[i]  = amp_correction*hypot(buf_re[i], buf_im[i]);
            break;
        case FFT_OUTPUT_RE:
            out_y[i]  = amp_correction*buf_re[i];
            break;
        case FFT_OUTPUT_IM:
            out_y[i]  = amp_correction*buf_im[i];
            break;
        case FFT_OUTPUT_PHASE:
            out_y[i]  = atan2(buf_im[i], buf_re[i]);
            break;
        case FFT_OUTPUT_REIM:
            out_y[i]  = amp_correction*buf_re[i];
            out_y1[i] = amp_correction*buf_im[i];
            break;
        case FFT_OUTPUT_APHI:
            out_y[i]  = amp_correction*hypot(buf_re[i], buf_im[i]);
            out_y1[i] = atan2(buf_im[i], buf_re[i]);
            break;
        }
    }
    
    out_x  = set_get_col(pdest, DATA_X);
    for (i = 0; i < outlen; i++) {
        switch (xscale) {
	case FFT_XSCALE_NU:
	    out_x[i] = (double) i/(xspace*buflen);
	    break;
	case FFT_XSCALE_OMEGA:
	    out_x[i] = 2*M_PI*i/(xspace*buflen);
	    break;
	default:
	    out_x[i] = (double) i;
	    break;
	}
    }
    
    xfree(buf_re);
    xfree(buf_im);
    
    sprintf(buf, "FFT of set %s", QIDSTR(psrc));
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}


/*
 * histograms
 */
int do_histo(Quark *psrc, Quark *pdest,
	      double *bins, int nbins, int cumulative, int normalize)
{
    int i, ndata;
    int *hist;
    double *x, *y, *data;
    set *p;
    char buf[256];
    
    if (nbins <= 0) {
	errmsg("Number of bins <= 0");
	return RETURN_FAILURE;
    }
    
    ndata = set_get_length(psrc);
    data = gety(psrc);
    
    hist = xmalloc(nbins*SIZEOF_INT);
    if (hist == NULL) {
        errmsg("xmalloc failed in do_histo()");
        return RETURN_FAILURE;
    }

    if (histogram(ndata, data, nbins, bins, hist) == RETURN_FAILURE){
        xfree(hist);
        return RETURN_FAILURE;
    }
    
    set_set_length(pdest, nbins + 1);
    x = getx(pdest);
    y = gety(pdest);
    
    x[0] = bins[0];
    y[0] = 0.0;
    for (i = 1; i < nbins + 1; i++) {
        x[i] = bins[i];
        y[i] = hist[i - 1];
        if (cumulative) {
            y[i] += y[i - 1];
        }
    }
    
    if (normalize) {
        for (i = 1; i < nbins + 1; i++) {
            double factor;
            if (cumulative) {
                factor = 1.0/ndata;
            } else {
                factor = 1.0/((bins[i] - bins[i - 1])*ndata);
            }
            y[i] *= factor;
        }
    }
    
    xfree(hist);

    p = set_get_data(pdest);
    p->sym.type = SYM_NONE;
    p->line.type = LINE_TYPE_LEFTSTAIR;
    p->line.droplines = TRUE;
    p->line.baseline = FALSE;
    p->line.baseline_type = BASELINE_TYPE_0;
    p->line.line.style = 1;
    p->sym.line.style = 1;
    sprintf(buf, "Histogram from %s", QIDSTR(psrc));
    // set_set_comment(pdest, buf);

    return RETURN_SUCCESS;
}


/*
 * sample a set by a logical expression
 */
int do_sample(Quark *psrc, Quark *pdest, char *formula)
{
    int len, newlen, ncols, i, nc;
    int reslen;
    double *result;
    char buf[256];

    if (string_is_empty(formula)) {
	errmsg("Empty formula");
	return RETURN_FAILURE;
    }

    if (set_parser_setno(psrc) != RETURN_SUCCESS) {
	errmsg("Bad set");
	return RETURN_FAILURE;
    }
    
    len = set_get_length(psrc);
    
    if (v_scanner(formula, &reslen, &result) != RETURN_SUCCESS) {
	return RETURN_FAILURE;
    }
    if (reslen != len) {
	errmsg("Internal error");
	xfree(result);
        return RETURN_FAILURE;
    }
    
    newlen = 0;
    for (i = 0; i < len; i++) {
	if ((int) rint(result[i])) {
	    newlen++;
	}
    }

    ncols = set_get_ncols(psrc);
    if (set_get_ncols(pdest) != ncols) {
        set_set_type(pdest, set_get_type(psrc));
    }
    
    if (set_set_length(pdest, newlen) != RETURN_SUCCESS) {
        xfree(result);
        return RETURN_FAILURE;
    }

    for (nc = 0; nc < ncols; nc++) {
        double *d1, *d2;
        int j;
        j = 0;
        d1 = set_get_col(psrc, nc);
        d2 = set_get_col(pdest, nc);
        for (i = 0; i < len; i++) {
	    if ((int) rint(result[i])) {
	        d2[j] = d1[i];
                j++;
	    }
        }
    }
    
    xfree(result);
    
    sprintf(buf, "Sample from %s, using '%s'", QIDSTR(psrc), formula);
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}

/*
 * Prune data
 */
int do_prune(Quark *psrc, Quark *pdest, 
    int interp, int elliptic, double dx, int reldx, double dy, int reldy)
{
    int len, newlen, ncols, i, old_i, nc;
    char *iprune;
    double *x, *y, old_x, old_y;
    char buf[256];

    if (dx <= 0.0) {
        errmsg("DX <= 0");
	return RETURN_FAILURE;
    }
    if (dy <= 0.0) {
        errmsg("DY <= 0");
	return RETURN_FAILURE;
    }
    
    if (set_is_dataless(psrc)) {
        errmsg("Set not active");
        return RETURN_FAILURE;
    }
    
    len = set_get_length(psrc);
    if (len < 3) {
	errmsg("Set length < 3");
	return RETURN_FAILURE;
    }
    
    x = getx(psrc);
    y = gety(psrc);

    if (interp && monotonicity(x, len, FALSE) == 0) {
	errmsg("Can't prune a non-monotonic set using interpolation");
	return RETURN_FAILURE;
    }

    ncols = set_get_ncols(psrc);
    if (set_get_ncols(pdest) != ncols) {
        set_set_type(pdest, set_get_type(psrc));
    }
    
    iprune = xmalloc(len*SIZEOF_CHAR);
    if (!iprune) {
        return RETURN_FAILURE;
    }
    
    newlen = 1;
    iprune[0] = FALSE;
    old_i = 0;
    old_x = x[0];
    old_y = y[0];
    for (i = 1; i < len; i++) {
        int prune;
        
        if (interp) {
            prune = prune_interp_inside(&x[old_i], &y[old_i], i - old_i + 1,
                elliptic, dx, reldx, dy, reldy);
        } else {
            prune = prune_plain_inside(x[old_i], y[old_i], x[i], y[i],
                elliptic, dx, reldx, dy, reldy);
        }
        
        if (prune) {
            iprune[i] = TRUE;
        } else {
            iprune[i] = FALSE;
            newlen++;
            old_i = i;
            old_x = x[i];
            old_y = y[i];
        }
    }

    if (set_set_length(pdest, newlen) != RETURN_SUCCESS) {
        xfree(iprune);
        return RETURN_FAILURE;
    }
    
    for (nc = 0; nc < ncols; nc++) {
        double *d1, *d2;
        int j;
        j = 0;
        d1 = set_get_col(psrc, nc);
        d2 = set_get_col(pdest, nc);
        for (i = 0; i < len; i++) {
	    if (iprune[i] == FALSE) {
	        d2[j] = d1[i];
                j++;
	    }
        }
    }
    
    xfree(iprune);

    sprintf(buf, "Prune from %s, method: %s, area: %s (dx = %g, dy = %g)",
        QIDSTR(psrc),
        interp ? "interpolation":"plain",
        elliptic ? "elliptic":"rectangular",
        dx, dy);
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}


/* interpolate a set at abscissas from mesh
 * method - type of spline (or linear interpolation)
 * if strict is set, perform interpolation only within source set bounds
 * (i.e., no extrapolation)
 */
int do_interp(Quark *psrc, Quark *pdest,
    double *mesh, int meshlen, int method, int strict)
{
    int len, n, ncols;
    double *x, *xint;
    char *s;
    char buf[256];
	
    if (mesh == NULL || meshlen < 1) {
        errmsg("NULL sampling mesh");
        return RETURN_FAILURE;
    }
    
    len = set_get_length(psrc);
    ncols = set_get_ncols(psrc);

    if (set_get_ncols(pdest) != ncols) {
        copysetdata(psrc, pdest);
    }
    
    set_set_length(pdest, meshlen);
    
    x = set_get_col(psrc, DATA_X);
    for (n = 1; n < ncols; n++) {
        int res;
        double *y, *yint;
        
        y    = set_get_col(psrc, n);
        yint = set_get_col(pdest, n);
        
        res = interpolate(mesh, yint, meshlen, x, y, len, method);
        if (res != RETURN_SUCCESS) {
            quark_free(pdest);
            return RETURN_FAILURE;
        }
    }

    xint = set_get_col(pdest, DATA_X);
    memcpy(xint, mesh, meshlen*SIZEOF_DOUBLE);

    if (strict) {
        double xmin, xmax;
        int i, imin, imax;
        minmax(x, len, &xmin, &xmax, &imin, &imax);
        for (i = meshlen - 1; i >= 0; i--) {
            if (xint[i] < xmin || xint[i] > xmax) {
                del_point(pdest, i);
            }
        }
    }
    
    switch (method) {
    case INTERP_SPLINE:
        s = "cubic spline";
        break;
    case INTERP_ASPLINE:
        s = "Akima spline";
        break;
    default:
        s = "linear interpolation";
        break;
    }
    sprintf(buf, "Interpolated from %s using %s", QIDSTR(psrc), s);
    // set_set_comment(pdest, buf);
    
    return RETURN_SUCCESS;
}

int get_restriction_array(Quark *pset, Quark *r, int negate, char **rarray)
{
    int i, n;
    double *x, *y;
    WPoint wp;
    
    if (!r) {
        *rarray = NULL;
        return RETURN_SUCCESS;
    }
    
    n = set_get_length(pset);
    if (n <= 0) {
        *rarray = NULL;
        return RETURN_FAILURE;
    }
    
    *rarray = xmalloc(n*SIZEOF_CHAR);
    if (*rarray == NULL) {
        return RETURN_FAILURE;
    }
    
    x = set_get_col(pset, DATA_X);
    y = set_get_col(pset, DATA_Y);
    
    for (i = 0; i < n; i++) {
        wp.x = x[i];
        wp.y = y[i];
        (*rarray)[i] = region_contains(r, &wp) ? !negate : negate;
    }

    return RETURN_SUCCESS;
}

/* feature extraction */
int featext(Quark **sets, int nsets, Quark *pdest,
    char *formula)
{
    int i;
    char *tbuf;
    double *x, *y;

    if (set_get_ncols(pdest) != 2) {
        set_set_type(pdest, SET_XY);
    }
    
    if (set_set_length(pdest, nsets) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    x = set_get_col(pdest, DATA_X);
    y = set_get_col(pdest, DATA_Y);
    for (i = 0; i < nsets; i++) {
        Quark *pset = sets[i];
	set_parser_setno(pset);
        x[i] = (double) i;
        if (s_scanner(formula, &y[i]) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }

    /* set comment */
    tbuf = copy_string(NULL, "Feature extraction by formula ");
    tbuf = concat_strings(tbuf, formula);
    // set_set_comment(pdest, tbuf);
    xfree(tbuf);
    
    return RETURN_SUCCESS;
}

/* cumulative properties */
int num_cumulative(DArray *src_arrays, unsigned int nsrc,
    DArray *dst_array, int type)
{
    int i, is, k, maxlen, maxncols, settype;
    DArray *slice;
    double *y = dst_array->x;

    maxlen = 0; maxncols = 0; settype = SET_XY;
    for (is = 0; is < nsrc; is++) {
        unsigned int len = src_arrays[is].size;
        if (maxlen < len) {
            maxlen = len;
        }
    }
    
    if (maxlen > dst_array->size) {
        return RETURN_FAILURE;
    }
    
    slice = darray_new(nsrc);
    if (!slice) {
        return RETURN_FAILURE;
    }
    
    for (k = 0; k < maxlen; k++) {
        double val;
        for (is = 0, i = 0; is < nsrc; is++) {
            DArray *da = &src_arrays[is];
            if (k < da->size) {
                slice->x[i++] = da->x[k];
            }
        }
        slice->size = i;
        switch (type) {
        case RUN_AVG:
            darray_avg(slice, &val);
            break;
        case RUN_STD:
            darray_std(slice, &val);
            break;
        case RUN_MIN:
            darray_min(slice, &val);
            break;
        case RUN_MAX:
            darray_max(slice, &val);
            break;
        default:
            errmsg("Wrong type in num_cumulative()");
            break;
        }
        y[k] = val;
    }

    darray_free(slice);
    
    return RETURN_SUCCESS;
} 
