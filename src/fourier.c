/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2000 Grace Development Team
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
 * Fourier transforms
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "grace/baseP.h"
#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "files.h"

#ifdef HAVE_FFTW

/* FFTW-based transforms (originally written by Marcus H. Mendenhall */

#include <fftw3.h>

static char *wisdom_file = NULL;
static char *initial_wisdom = NULL;
static int  using_wisdom = FALSE;

static void save_wisdom(void){
    char *final_wisdom;

    final_wisdom = fftw_export_wisdom_to_string();
    
    if (!initial_wisdom ||
        strings_are_equal(initial_wisdom, final_wisdom) != TRUE) {
        FILE *wf;
        wf = gapp_openw(gapp, wisdom_file);
        if (wf) {
            fftw_export_wisdom_to_file(wf);
            gapp_close(wf);
        }
    } 
    
    fftw_free(final_wisdom);
    if (initial_wisdom) {
        fftw_free(initial_wisdom);
    }
}

static void init_wisdom(void)
{
    static int wisdom_inited = FALSE;
    
    if (!wisdom_inited)  {
        char *ram_cache_wisdom;
        
        wisdom_inited = TRUE;
        wisdom_file      = getenv("GRACE_FFTW_WISDOM_FILE");
        ram_cache_wisdom = getenv("GRACE_FFTW_RAM_WISDOM");

        if (ram_cache_wisdom) {
            sscanf(ram_cache_wisdom, "%d", &using_wisdom);
        }

        /* turn on wisdom if it is requested even without persistent storage */
        if (wisdom_file && wisdom_file[0] ) {
            /* if a file was specified in GRACE_FFTW_WISDOM_FILE, try to read it */
            FILE *wf;
            
            wf = gapp_openr(gapp, wisdom_file, SOURCE_DISK);
            if (wf) {
	        fftw_import_wisdom_from_file(wf);
	        gapp_close(wf);
	        initial_wisdom = fftw_export_wisdom_to_string();
            } else {
                initial_wisdom = NULL;
            }
            
            atexit(save_wisdom);
            
            /* if a file is specified, always use wisdom */
            using_wisdom = TRUE;
        }
    }
}

int fourier(double *jr, double *ji, int n, int iflag)
{
    int i;
    int plan_flags;
    fftw_plan plan;
    fftw_complex *cbuf;
    
    init_wisdom();

    plan_flags = using_wisdom ? FFTW_MEASURE:FFTW_ESTIMATE;
    
    cbuf = xcalloc(n, sizeof(fftw_complex));
    if (!cbuf) {
        return RETURN_FAILURE;
    }
    for (i = 0; i < n; i++) {
        cbuf[i][0] = jr[i];
        cbuf[i][1] = ji[i];
    }
    
    plan = fftw_plan_dft_1d(n, cbuf, cbuf,
        iflag ? FFTW_BACKWARD:FFTW_FORWARD, plan_flags);
    
    fftw_execute(plan);
    
    fftw_destroy_plan(plan);

    for (i = 0; i < n; i++) {
        jr[i] = cbuf[i][0];
        ji[i] = cbuf[i][1];
    }

    xfree(cbuf);
    
    return RETURN_SUCCESS;
}

#else

/* Legacy FFT code */

static int bit_swap(int i, int nu);
static int ilog2(int n);
static int dft(double *jr, double *ji, int n, int iflag);
static int fft(double *jr, double *ji, int n, int nu, int iflag);

int fourier(double *jr, double *ji, int n, int iflag)
{
    int i2;
    
    if ((i2 = ilog2(n)) > 0) {
	return fft(jr, ji, n, i2, iflag);
    } else {
	return dft(jr, ji, n, iflag);
    }
}

/*
	DFT by definition
*/
static int dft(double *jr, double *ji, int n, int iflag)
{
    int i, j, sgn;
    double sumr, sumi, tpi, w, *xr, *xi, on = 1.0 / n;
    double *cov, *siv, co, si;
    int iwrap;

    sgn = iflag ? 1:-1;
    tpi = 2*M_PI;
    xr  = xcalloc(n, SIZEOF_DOUBLE);
    xi  = xcalloc(n, SIZEOF_DOUBLE);
    cov = xcalloc(n, SIZEOF_DOUBLE);
    siv = xcalloc(n, SIZEOF_DOUBLE);
    if (xr == NULL || xi == NULL || cov == NULL || siv == NULL) {
	xfree(xr);
	xfree(xi);
	xfree(cov);
	xfree(siv);
	return RETURN_FAILURE;
    }
    for (i = 0; i < n; i++) {
	w = i*tpi*on;
	
        cov[i] = cos(w);
	siv[i] = sin(w)*sgn;
	
        xr[i]  = jr[i];
	xi[i]  = ji[i];
    }
    for (j = 0; j < n; j++) {
	sumr = 0.0;
	sumi = 0.0;
	for (i = 0, iwrap=0; i < n; i++, iwrap += j) {
	    if (iwrap >= n) {
                iwrap -= n;
            }
	    co = cov[iwrap];
	    si = siv[iwrap];
	    sumr = sumr + xr[i]*co + sgn*xi[i]*si;
	    sumi = sumi + xi[i]*co - sgn*xr[i]*si;
	}
	jr[j] = sumr;
	ji[j] = sumi;
    }

    xfree(xr);
    xfree(xi);
    xfree(cov);
    xfree(siv);
    
    return RETURN_SUCCESS;
}


/*
   real_data ... ptr. to real part of data to be transformed
   imag_data ... ptr. to imag  "   "   "   "  "      "
   inv ..... Switch to flag normal or inverse transform
   n_pts ... Number of real data points
   nu ...... logarithm in base 2 of n_pts e.g. nu = 5 if n_pts = 32.
*/

int fft(double *real_data, double *imag_data, int n_pts, int nu, int inv)
{
    int n2, i, ib, mm, k;
    int sgn, tstep;
    double tr, ti, arg; /* intermediate values in calcs. */
    double c, s;        /* cosine & sine components of Fourier trans. */
    static double *sintab = NULL;
    static int last_n = 0;

    n2 = n_pts / 2;
    
    if (n_pts != last_n) { /* allocate new sin table */
        arg = 2*M_PI/n_pts;
        last_n = 0;
        sintab = xrealloc(sintab, n_pts*SIZEOF_DOUBLE);
        if (sintab == NULL) {
            return RETURN_FAILURE;
        }
        for (i = 0; i < n_pts; i++) {
            sintab[i] = sin(arg*i);
        }
        last_n = n_pts;
    }

/*
 * sign change for inverse transform
 */
    sgn = inv ? 1:-1;

    /* do bit reversal of data in advance */
    for (k = 0; k != n_pts; k++) {
	ib = bit_swap(k, nu);
	if (ib > k) {
	    fswap((real_data + k), (real_data + ib));
	    fswap((imag_data + k), (imag_data + ib));
	}
    }
/*
* Calculate the componets of the Fourier series of the function
*/
    tstep = n2;
    for (mm = 1; mm < n_pts; mm *= 2) {
        int sinidx = 0, cosidx = n_pts/4;
        for (k=0; k<mm; k++) {
	    c = sintab[cosidx];
	    s = sgn*sintab[sinidx];
	    sinidx += tstep;
            cosidx += tstep;
	    if (sinidx >= n_pts) {
              sinidx -= n_pts;
            }
	    if (cosidx >= n_pts) {
                cosidx -= n_pts;
            }
	    for (i = k; i < n_pts; i += mm*2) {
	        double re1, re2, im1, im2;  
	        re1 = real_data[i];
	        im1 = imag_data[i];
                re2 = real_data[i + mm];
                im2 = imag_data[i + mm];

	        tr = re2*c + im2*s;
	        ti = im2*c - re2*s;
	        real_data[i+mm] = re1 - tr;
	        imag_data[i+mm] = im1 - ti;
	        real_data[i] = re1 + tr;
	        imag_data[i] = im1 + ti;
	    }
        }
        tstep /= 2;
    }
    
    return RETURN_SUCCESS;
}

/*
* Bit swapping routine in which the bit pattern of the integer i is reordered.
* See Brigham's book for details
*/
static int bit_swap(int i, int nu)
{
    int ib, i1, i2;

    ib = 0;

    for (i1 = 0; i1 != nu; i1++) {
	i2 = i/2;
	ib = ib*2 + i - 2*i2;
	i = i2;
    }
    return (ib);
}

/*
 * log base 2
 */
static int ilog2(int n)
{
    int i = 0;
    int n1 = n;

    while (n1 >>= 1) {
	i++;
    }
    if (1 << i != n) {
	return -1;
    } else {
	return i;
    }
}

#endif
