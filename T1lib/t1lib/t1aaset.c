/*--------------------------------------------------------------------------
  ----- File:        t1aaset.c 
  ----- Author:      Rainer Menzner (rmz@neuroinformatik.ruhr-uni-bochum.de)
                     Subsampling-code by Raph Levien (raph@acm.org)
  ----- Date:        10/01/1998
  ----- Description: This file is part of the t1-library. It contains
                     functions for antialiased setting of characters
		     and strings of characters.
  ----- Copyright:   t1lib is copyrighted (c) Rainer Menzner, 1996-1998. 
                     As of version 0.5, t1lib is distributed under the
		     GNU General Public Library Lincense. The
		     conditions can be found in the files LICENSE and
		     LGPL, which should reside in the toplevel
		     directory of the distribution.  Please note that 
		     there are parts of t1lib that are subject to
		     other licenses:
		     The parseAFM-package is copyrighted by Adobe Systems
		     Inc.
		     The type1 rasterizer is copyrighted by IBM and the
		     X11-consortium.
  ----- Warranties:  Of course, there's NO WARRANTY OF ANY KIND :-)
  ----- Credits:     I want to thank IBM and the X11-consortium for making
                     their rasterizer freely available.
		     Also thanks to Piet Tutelaers for his ps2pk, from
		     which I took the rasterizer sources in a format
		     independent from X11.
                     Thanks to all people who make free software living!
--------------------------------------------------------------------------*/
  
#define T1AASET_C


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../type1/ffilest.h"
#include "../type1/types.h"
#include "parseAFM.h" 
#include "../type1/objects.h"
#include "../type1/spaces.h"
#include "../type1/util.h"
#include "../type1/fontfcn.h"
#include "../type1/regions.h"


#include "t1types.h"
#include "t1extern.h"
#include "t1aaset.h"
#include "t1set.h"
#include "t1load.h"
#include "t1finfo.h"
#include "t1misc.h"
#include "t1base.h"


#define DEFAULTBPP 8


/* As a fall back */
#ifndef T1_AA_TYPE16 
#define T1_AA_TYPE16    short
#endif
#ifndef T1_AA_TYPE32 
#define T1_AA_TYPE32    int
#endif


/* In the following arrays take the gray values. Entry 0 is associated
   with the white (background) value and the max entry is the
   black (foreground) value. */
static unsigned T1_AA_TYPE32 gv[5]={0,0,0,0,0};
static unsigned T1_AA_TYPE32 gv_h[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static int T1aa_level=T1_AA_LOW;  /* The default value */
static T1_AA_TYPE32 T1aa_lut[625];
static int T1aa_count[256];
static T1_AA_TYPE32 T1aa_h_lut[289];
static int T1aa_h_count[256];

/* This global is for querying the current bg from other parts
   of t1lib */
unsigned T1_AA_TYPE32 T1aa_bg=0;



/* T1_AAInit: This function must be called whenever the T1aa_gray_val
   or T1aa_bpp variables change, or the level changes. */
static int T1_AAInit ( int level )
{
  int i;
  int i0, i1, i2, i3;
  int movelow=0, movehigh=0, indlow=0, indhigh=0;
  

  /* Note: movelow, movehigh, indlow and indhigh take care for proper
     byte swapping in dependence of endianess for level=4 */
  if (level==T1_AA_HIGH){
    
    if (T1aa_bpp==8){
      if (pFontBase->endian){
	indlow=17;
	indhigh=1;
	movelow=3;
	movehigh=2;
      }
      else{
	indlow=1;
	indhigh=17;
	movelow=0;
	movehigh=1;
      }
    }
    else if (T1aa_bpp==16){
      if (pFontBase->endian){
	indlow=17;
	indhigh=1;
	movelow=1;
	movehigh=0;
      }
      else{
	indlow=1;
	indhigh=17;
	movelow=0;
	movehigh=1;
      }
    }
    if (T1aa_bpp==32){
      indlow=1;
      indhigh=17;
    }
    for (i = 0; i < 256; i++) {
      T1aa_h_count[i] = 0;
      if (i & 0x80) T1aa_h_count[i] += indhigh; 
      if (i & 0x40) T1aa_h_count[i] += indhigh;
      if (i & 0x20) T1aa_h_count[i] += indhigh;
      if (i & 0x10) T1aa_h_count[i] += indhigh;
      if (i & 0x08) T1aa_h_count[i] += indlow;
      if (i & 0x04) T1aa_h_count[i] += indlow;
      if (i & 0x02) T1aa_h_count[i] += indlow;
      if (i & 0x01) T1aa_h_count[i] += indlow;
    }
  }
  
  
  if (level == 2 && T1aa_bpp == 8) {
    for (i0 = 0; i0 < 5; i0++)
      for (i1 = 0; i1 < 5; i1++)
	for (i2 = 0; i2 < 5; i2++)
	  for (i3 = 0; i3 < 5; i3++) {
	    ((char *)T1aa_lut)[(((i0 * 5 + i1) * 5 + i2) * 5 + i3) * 4] = gv[i3];
	    ((char *)T1aa_lut)[(((i0 * 5 + i1) * 5 + i2) * 5 + i3) * 4 + 1] = gv[i2];
	    ((char *)T1aa_lut)[(((i0 * 5 + i1) * 5 + i2) * 5 + i3) * 4 + 2] = gv[i1];
	    ((char *)T1aa_lut)[(((i0 * 5 + i1) * 5 + i2) * 5 + i3) * 4 + 3] = gv[i0];
	  }
    for (i = 0; i < 256; i++) {
      T1aa_count[i] = 0;
      if (i & 0x80) T1aa_count[i] += 125;
      if (i & 0x40) T1aa_count[i] += 125;
      if (i & 0x20) T1aa_count[i] += 25;
      if (i & 0x10) T1aa_count[i] += 25;
      if (i & 0x08) T1aa_count[i] += 5;
      if (i & 0x04) T1aa_count[i] += 5;
      if (i & 0x02) T1aa_count[i] += 1;
      if (i & 0x01) T1aa_count[i] += 1;
    }
    return(0);
  } else if (level == 2 && T1aa_bpp == 16) {
    for (i0 = 0; i0 < 5; i0++)
      for (i1 = 0; i1 < 5; i1++) {
	((T1_AA_TYPE16 *)T1aa_lut)[(i0 * 5 + i1) * 2] = gv[i1];
	((T1_AA_TYPE16 *)T1aa_lut)[(i0 * 5 + i1) * 2 + 1] = gv[i0];
      }
    for (i = 0; i < 256; i++) {
      T1aa_count[i] = 0;
      if (i & 0x80) T1aa_count[i] += 160;
      if (i & 0x40) T1aa_count[i] += 160;
      if (i & 0x20) T1aa_count[i] += 32;
      if (i & 0x10) T1aa_count[i] += 32;
      if (i & 0x08) T1aa_count[i] += 5;
      if (i & 0x04) T1aa_count[i] += 5;
      if (i & 0x02) T1aa_count[i] += 1;
      if (i & 0x01) T1aa_count[i] += 1;
    }
    return(0);
  } else if (level == 2 && T1aa_bpp == 32) {
    for (i0 = 0; i0 < 5; i0++)
      ((T1_AA_TYPE32 *)T1aa_lut)[i0] = gv[i0];
    for (i = 0; i < 256; i++) {
      T1aa_count[i] = 0;
      if (i & 0x80) T1aa_count[i] += 512;
      if (i & 0x40) T1aa_count[i] += 512;
      if (i & 0x20) T1aa_count[i] += 64;
      if (i & 0x10) T1aa_count[i] += 64;
      if (i & 0x08) T1aa_count[i] += 8;
      if (i & 0x04) T1aa_count[i] += 8;
      if (i & 0x02) T1aa_count[i] += 1;
      if (i & 0x01) T1aa_count[i] += 1;
    }
    return(0);
  } else if (level == 4 && T1aa_bpp == 8) {
    for (i0 = 0; i0 < 17; i0++){ /* i0 indexes higher nibble */
      for (i1 = 0; i1 < 17; i1++){ /* i1 indixes lower nibble */
	((char *)T1aa_h_lut)[(i0 * 17 + i1) * 4 + movelow] = gv_h[i1];
	((char *)T1aa_h_lut)[(i0 * 17 + i1) * 4 + movehigh] = gv_h[i0];
      }
    }
    return(0);
  } else if (level == 4 && T1aa_bpp == 16) {
    for (i0 = 0; i0 < 17; i0++){ /* i0 indexes higher nibble */
      for (i1 = 0; i1 < 17; i1++){ /* i1 indixes lower nibble */
	((T1_AA_TYPE16 *)T1aa_h_lut)[(i0 * 17 + i1) * 2 + movelow] = gv_h[i1];
	((T1_AA_TYPE16 *)T1aa_h_lut)[(i0 * 17 + i1) * 2 + movehigh] = gv_h[i0];
      }
    }
    return(0);
  } else if (level == 4 && T1aa_bpp == 32) {
    for (i0 = 0; i0 < 17; i0++){ /* i0 indexes higher nibble */
      for (i1 = 0; i1 < 17; i1++){ /* i1 indixes lower nibble */
	((T1_AA_TYPE32 *)T1aa_h_lut)[(i0 * 17 + i1)] = gv_h[i1];
      }
    }
    return(0);
  }
  else {
    /* unsupported combination of level and bpp -> we set T1_errno and
       put an entry into the logfile! */
    T1_errno=T1ERR_INVALID_PARAMETER;
    sprintf( err_warn_msg_buf,
	     "Unsupported AA specification: level=%d, bpp=%d",
	     level, T1aa_bpp);
    T1_PrintLog( "T1_AAInit()", err_warn_msg_buf, T1LOG_WARNING);
  }
  return(1);
}


/* T1_AADoLine: Create a single scanline of antialiased output. The
   (x, y) arguments refer to the number of pixels in the input image
   to convert down. The width argument is the number of bytes
   separating scanlines in the input. */
static void T1_AADoLine ( int level, int x, int y, int width,
			  char *c_in_ptr,
			  char *target_ptr )
{
  int i=0;
  int size;
  char buf[8];
  int count=0;
  int mod;
  
  
  static char *align_buf = NULL;
  static int align_buf_size = 0;
  unsigned char *in_ptr;
  
  int new_size=55;
  char *optr;

  

  /* Perhaps we should mask the last byte when the line end isn't aligned
     with a byte, but it always seems to be zero, so we don't bother. */

  /* We convert the input pointer to unsigned since we use it as index! */
  in_ptr=(unsigned char*)c_in_ptr;
  

  if ((long)target_ptr & 3){
    /* calculate new_size (size in bytes of output buffer */
    if (level == T1_AA_LOW)
      new_size = ((x + 1) >> 1) * (T1aa_bpp >> 3);
    else /* T1_AA_HIGH */
      new_size = ((x + 3) >> 2) * (T1aa_bpp >> 3);
    if (new_size > align_buf_size)
      {
	if (align_buf)
	  free (align_buf);
	align_buf = (char *)malloc(new_size);
	align_buf_size = new_size;
      }
    optr = align_buf;
  }
  else
    optr = target_ptr;

  if (level == T1_AA_LOW) {
    /* Note: (x+1)&6 is an identical statement to ((x+1)>>1)&3 which
       in turn is the horizontal pixelnumber in the aaglyph */
    size = (x + 1) >> 3; 
    mod = ((x+1)>>1)&3;
    
    if (T1aa_bpp == 8) {
      if (y == 2){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE32 *)optr)[i] = T1aa_lut[(T1aa_count[in_ptr[i]] +
						T1aa_count[in_ptr[i + width]])];
	}
      }
      else if (y == 1){
	for (i = 0; i < size; i++) {
	  /* could collapse the luts here, but it would be a marginal
	     performance gain at best. */
	  ((T1_AA_TYPE32 *)optr)[i] = T1aa_lut[(T1aa_count[in_ptr[i]])];
	}
      }
      if (mod) {
	if (y == 2)
	  ((T1_AA_TYPE32 *)buf)[0] = T1aa_lut[(T1aa_count[in_ptr[i]] +
					       T1aa_count[in_ptr[i + width]])];
	else if (y == 1)
	  ((T1_AA_TYPE32 *)buf)[0] = T1aa_lut[(T1aa_count[in_ptr[i]])];
	memcpy (optr + i * 4, buf, mod );
      }
      
    } else if (T1aa_bpp == 16) {
      if (y == 2)
	for (i = 0; i < size; i++) {
	  count = T1aa_count[in_ptr[i]] + T1aa_count[in_ptr[i + width]];
	  ((T1_AA_TYPE32 *)optr)[i * 2] = T1aa_lut[count & 31];
	  ((T1_AA_TYPE32 *)optr)[i * 2 + 1] = T1aa_lut[count >> 5];
	}
      else if (y == 1)
	for (i = 0; i < size; i++) {
	  count = T1aa_count[in_ptr[i]];
	  ((T1_AA_TYPE32 *)optr)[i * 2] = T1aa_lut[count & 31];
	  ((T1_AA_TYPE32 *)optr)[i * 2 + 1] = T1aa_lut[count >> 5];
	}
      if (mod){
	if (y == 2)
	  count = T1aa_count[in_ptr[i]] + T1aa_count[in_ptr[i + width]];
	else if (y == 1)
	  count = T1aa_count[in_ptr[i]];
	((T1_AA_TYPE32 *)buf)[0] = T1aa_lut[count & 31];
	((T1_AA_TYPE32 *)buf)[1] = T1aa_lut[count >> 5];
	memcpy (optr + i * 8, buf, mod<<1 );
      }
    } else if (T1aa_bpp == 32) {
      if (y == 2)
	for (i = 0; i < size; i++) {
	  count = T1aa_count[in_ptr[i]] + T1aa_count[in_ptr[i + width]];
	  ((T1_AA_TYPE32 *)optr)[i * 4] = T1aa_lut[count & 7];
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 1] = T1aa_lut[(count >> 3) & 7];
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 2] = T1aa_lut[(count >> 6) & 7];
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 3] = T1aa_lut[(count >> 9) & 7];
	}
      else if (y == 1)
	for (i = 0; i < size; i++) {
	  count = T1aa_count[in_ptr[i]];
	  ((T1_AA_TYPE32 *)optr)[i * 4] = T1aa_lut[count & 7];
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 1] = T1aa_lut[(count >> 3) & 7];
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 2] = T1aa_lut[(count >> 6) & 7];
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 3] = T1aa_lut[(count >> 9) & 7];
	}
      if(mod){
	if (y == 2)
	  count = T1aa_count[in_ptr[i]] + T1aa_count[in_ptr[i + width]];
	else if (y == 1)
	  count = T1aa_count[in_ptr[i]];
	((T1_AA_TYPE32 *)optr)[i * 4] = T1aa_lut[count & 7];
	if (((x + 1) & 6) > 2) {
	  ((T1_AA_TYPE32 *)optr)[i * 4 + 1] = T1aa_lut[(count >> 3) & 7];
	  if (((x + 1) & 6) > 4) {
	    ((T1_AA_TYPE32 *)optr)[i * 4 + 2] = T1aa_lut[(count >> 6) & 7];
	  }
	}
      }
    }
  }
  else if (level==T1_AA_HIGH){ 
    size = (x+3)>>3; 
    mod =  ((x+3)>>2)&1;
    if (T1aa_bpp == 8) {
      if (y == 4){
	for (i = 0; i < size; i++) {
	  count=T1aa_h_count[in_ptr[i]]+
	    T1aa_h_count[in_ptr[i + width]] +
	    T1aa_h_count[in_ptr[i + 2*width]] +
	    T1aa_h_count[in_ptr[i + 3*width]];
	  ((T1_AA_TYPE16 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] + 
						  T1aa_h_count[in_ptr[i + width]] + 
						  T1aa_h_count[in_ptr[i + 2*width]] + 
						  T1aa_h_count[in_ptr[i + 3*width]])];
	}
      }
      else if (y == 3){
	for (i = 0; i < size; i++) {
	  count=T1aa_h_count[in_ptr[i]]+
	    T1aa_h_count[in_ptr[i + width]] +
	    T1aa_h_count[in_ptr[i + 2*width]];
	  ((T1_AA_TYPE16 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						  T1aa_h_count[in_ptr[i + width]] +
						  T1aa_h_count[in_ptr[i + 2*width]])];
	}
      }
      else if (y == 2){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE16 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						  T1aa_h_count[in_ptr[i + width]])];
	}
      }
      else if (y == 1){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE16 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]])];
	}
      }
      if (mod) {
	if (y == 4)
	  ((T1_AA_TYPE16 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						 T1aa_h_count[in_ptr[i + width]] + 
						 T1aa_h_count[in_ptr[i + 2*width]] +
						 T1aa_h_count[in_ptr[i + 3*width]])];
	else if (y == 3)
	  ((T1_AA_TYPE16 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						 T1aa_h_count[in_ptr[i + width]] + 
						 T1aa_h_count[in_ptr[i + 2*width]])];
	else if (y == 2)
	  ((T1_AA_TYPE16 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						 T1aa_h_count[in_ptr[i + width]])];
	else if (y == 1)
	  ((T1_AA_TYPE16 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]])];
	memcpy (optr + i * 2, buf, mod );
      }
      
    } else if (T1aa_bpp == 16) {
      if (y == 4){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE32 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] + 
						  T1aa_h_count[in_ptr[i + width]] + 
						  T1aa_h_count[in_ptr[i + 2*width]] + 
						  T1aa_h_count[in_ptr[i + 3*width]])];
	}
      }
      else if (y == 3){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE32 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						  T1aa_h_count[in_ptr[i + width]] +
						  T1aa_h_count[in_ptr[i + 2*width]])];
	}
      }
      else if (y == 2){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE32 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						  T1aa_h_count[in_ptr[i + width]])];
	}
      }
      else if (y == 1){
	for (i = 0; i < size; i++) {
	  ((T1_AA_TYPE32 *)optr)[i] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]])];
	}
      }
      if (mod) {
	if (y == 4)
	  ((T1_AA_TYPE32 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						 T1aa_h_count[in_ptr[i + width]] + 
						 T1aa_h_count[in_ptr[i + 2*width]] +
						 T1aa_h_count[in_ptr[i + 3*width]])];
	else if (y == 3)
	  ((T1_AA_TYPE32 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						 T1aa_h_count[in_ptr[i + width]] + 
						 T1aa_h_count[in_ptr[i + 2*width]])];
	else if (y == 2)
	  ((T1_AA_TYPE32 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]] +
						 T1aa_h_count[in_ptr[i + width]])];
	else if (y == 1)
	  ((T1_AA_TYPE32 *)buf)[0] = T1aa_h_lut[(T1aa_h_count[in_ptr[i]])];
	memcpy (optr + i * 4, buf, mod << 1);
      }
    } else if (T1aa_bpp == 32) {
      if (y == 4){
	for (i = 0; i < size; i++) {
	  count=T1aa_h_count[in_ptr[i]] +
	    T1aa_h_count[in_ptr[i + width]] +
	    T1aa_h_count[in_ptr[i + 2*width]] +
	    T1aa_h_count[in_ptr[i + 3*width]];
	  ((T1_AA_TYPE32 *)optr)[2*i] = T1aa_h_lut[count % 17];
	  ((T1_AA_TYPE32 *)optr)[2*i+1] = T1aa_h_lut[count / 17];
	}
      }
      else if (y == 3){
	for (i = 0; i < size; i++) {
	  count=T1aa_h_count[in_ptr[i]] +
	    T1aa_h_count[in_ptr[i + width]] +
	    T1aa_h_count[in_ptr[i + 2*width]];
	  ((T1_AA_TYPE32 *)optr)[2*i] = T1aa_h_lut[count % 17];
	  ((T1_AA_TYPE32 *)optr)[2*i+1] = T1aa_h_lut[count / 17];
	}
      }
      else if (y == 2){
	for (i = 0; i < size; i++) {
	  count=T1aa_h_count[in_ptr[i]] +
	    T1aa_h_count[in_ptr[i + width]];
	  ((T1_AA_TYPE32 *)optr)[2*i] = T1aa_h_lut[count % 17];
	  ((T1_AA_TYPE32 *)optr)[2*i+1] = T1aa_h_lut[count / 17];
	}
      }
      else if (y == 1){
	for (i = 0; i < size; i++) {
	  count=T1aa_h_count[in_ptr[i]];
	  ((T1_AA_TYPE32 *)optr)[2*i] = T1aa_h_lut[count % 17];
	  ((T1_AA_TYPE32 *)optr)[2*i+1] = T1aa_h_lut[count / 17];
	}
      }
      if (mod) {
	if (y == 4)
	  count=T1aa_h_count[in_ptr[i]] +
	    T1aa_h_count[in_ptr[i + width]] +
	    T1aa_h_count[in_ptr[i + 2*width]] +
	    T1aa_h_count[in_ptr[i + 3*width]];
	else if (y == 3)
	  count=T1aa_h_count[in_ptr[i]] +
	    T1aa_h_count[in_ptr[i + width]] +
	    T1aa_h_count[in_ptr[i + 2*width]];
	else if (y == 2)
	  count=T1aa_h_count[in_ptr[i]] +
	    T1aa_h_count[in_ptr[i + width]];
	else if (y == 1)
	  count=T1aa_h_count[in_ptr[i]];
	((T1_AA_TYPE32 *)buf)[0] = T1aa_h_lut[count % 17];
	memcpy (optr + 8*i, buf, mod << 2);
      }
    }
  }
  
  /* Copy to target if necessary */
  if ((long)target_ptr & 3){
    memcpy (target_ptr, align_buf, new_size);
  }
  
}


/* T1_AASetChar(...): Generate the anti-aliased bitmap for a character */
GLYPH *T1_AASetChar( int FontID, char charcode, float size,
		     T1_TMATRIX *transform)
{
  
  GLYPH *glyph;   /* pointer to bitmap glyph */
  static GLYPH aaglyph={NULL,{0,0,0,0,0,0},NULL,DEFAULTBPP};/* The anti-aliased glyph */
  long asc, dsc, ht, wd;
  long i, k;
  long n_horz, n_horz_pad, n_vert, n_asc, n_dsc;
  long v_start, v_end;
  char *target_ptr;
  long offset;
  char *ptr;
  int y;

  int memsize;
  LONG paddedW;
 

  /* Reset character glyph, if necessary */
  if (aaglyph.bits!=NULL){
    free(aaglyph.bits);
    aaglyph.bits=NULL;
  }
  aaglyph.metrics.leftSideBearing=0;
  aaglyph.metrics.rightSideBearing=0;
  aaglyph.metrics.advanceX=0;
  aaglyph.metrics.advanceY=0;
  aaglyph.metrics.ascent=0;
  aaglyph.metrics.descent=0;
  aaglyph.pFontCacheInfo=NULL;
  aaglyph.bpp=T1aa_bpp;
  
  /* First, call routine to rasterize character, all error checking is
     done in this function: */ 
  if ((glyph=T1_SetChar( FontID, charcode, T1aa_level*size, transform))==NULL)
    return(NULL); /* An error occured */
  
  /* Get dimensions of bitmap: */
  asc=glyph->metrics.ascent;
  dsc=glyph->metrics.descent;
  ht=asc-dsc;
  wd=glyph->metrics.rightSideBearing-glyph->metrics.leftSideBearing;
  
  /* Set some looping parameters for subsampling */
  /* The horizontal number of steps: */
  n_horz=(wd+T1aa_level-1)/T1aa_level;
  /* And the padded value */
  n_horz_pad=PAD( n_horz*T1aa_bpp, pFontBase->bitmap_pad )>>3;

  /* vertical number of steps: */
  if (asc % T1aa_level){ /* not aligned */
    if ( asc > 0){
      n_asc=asc/T1aa_level+1;
      v_start=asc % T1aa_level;
    }
    else{
      n_asc=asc/T1aa_level;
      v_start=T1aa_level + (asc % T1aa_level); 
    }
  }
  else{
    n_asc=asc/T1aa_level;
    v_start=T1aa_level;
  }
  if (dsc % T1aa_level){ /* not aligned */
    if ( dsc < 0){
      n_dsc=dsc/T1aa_level-1;
      v_end=-(dsc % T1aa_level);
    }
    else{
      n_dsc=dsc/T1aa_level;
      v_end=T1aa_level - (dsc % T1aa_level);
    }
  }
  else{
    n_dsc=dsc/T1aa_level;
    v_end=T1aa_level;
  }
  /* the total number of lines: */
  n_vert=n_asc-n_dsc;
  
  /* Allocate memory for glyph */
  memsize = n_horz_pad*n_vert;
  
  aaglyph.bits = (char *)malloc(memsize*sizeof( char));
  if (aaglyph.bits == NULL) {
    T1_errno=T1ERR_ALLOC_MEM;
    return(NULL);
  }
  

  paddedW=PAD(wd,pFontBase->bitmap_pad)/8;
  offset=0;
  target_ptr=aaglyph.bits;
  
  /* We must check for n_vert==1 because the computation above is not
     valid in this case */
  if (n_vert==1)
    v_start=v_start < v_end ? v_start : v_end;

  ptr = glyph->bits;
  for (i = 0; i < n_vert; i++) {
    if (i==0)
      y=v_start;
    else if (i==n_vert-1)
      y=v_end;
    else
      y=T1aa_level;
    T1_AADoLine ( T1aa_level, wd, y, paddedW, ptr, target_ptr );
    ptr += y * paddedW;
    target_ptr += n_horz_pad;
  }
  k = n_horz;
  
  /* .. and set them in aaglyph */
  aaglyph.metrics.leftSideBearing=(int) floor(glyph->metrics.leftSideBearing/(float)T1aa_level+0.5);
  aaglyph.metrics.rightSideBearing=aaglyph.metrics.leftSideBearing + k;
  aaglyph.metrics.advanceX=(int) floor(glyph->metrics.advanceX/(float)T1aa_level+0.5);
  aaglyph.metrics.advanceY=(int) floor(glyph->metrics.advanceY/(float)T1aa_level+0.5);
  aaglyph.metrics.ascent=n_asc;
  aaglyph.metrics.descent=n_dsc;
  aaglyph.pFontCacheInfo=NULL;

  return(&aaglyph);
}



/* T1_AASetString(...): Generate the antialiased bitmap for a
   string of characters */
GLYPH *T1_AASetString( int FontID, char *string, int len, 
		       long spaceoff, int modflag, float size,
		       T1_TMATRIX *transform)
{
  GLYPH *glyph;   /* pointer to bitmap glyph */
  static GLYPH aastring_glyph={NULL,{0,0,0,0,0,0},NULL,DEFAULTBPP};/* The anti-aliased glyph */
  long asc, dsc, ht, wd;
  long i, k;
  long n_horz, n_horz_pad, n_vert, n_asc, n_dsc;
  long v_start, v_end;
  char *target_ptr;
  long offset;
  char *ptr;
  int y;
  
  int memsize;
  LONG paddedW;

  
  /* Reset character glyph, if necessary */
  if (aastring_glyph.bits!=NULL){
    free(aastring_glyph.bits);
    aastring_glyph.bits=NULL;
  }
  aastring_glyph.metrics.leftSideBearing=0;
  aastring_glyph.metrics.rightSideBearing=0;
  aastring_glyph.metrics.advanceX=0;
  aastring_glyph.metrics.advanceY=0;
  aastring_glyph.metrics.ascent=0;
  aastring_glyph.metrics.descent=0;
  aastring_glyph.pFontCacheInfo=NULL;
  aastring_glyph.bpp=T1aa_bpp;
  
  /* First, call routine to rasterize character, all error checking is
     done in this function: */ 
  if ((glyph=T1_SetString( FontID, string, len, spaceoff,
			   modflag, T1aa_level*size, transform))==NULL)
    return(NULL); /* An error occured */
  

  /* Get dimensions of bitmap: */
  asc=glyph->metrics.ascent;
  dsc=glyph->metrics.descent;
  ht=asc-dsc;
  wd=glyph->metrics.rightSideBearing-glyph->metrics.leftSideBearing;
  
  /* Set some looping parameters for subsampling */
  /* The horizontal number of steps: */
  n_horz=(wd+T1aa_level-1)/T1aa_level;
  /* And the padded value */
  n_horz_pad=PAD( n_horz*T1aa_bpp, pFontBase->bitmap_pad )>>3;

  /* vertical number of steps: */
  if (asc % T1aa_level){ /* not aligned */
    if ( asc > 0){
      n_asc=asc/T1aa_level+1;
      v_start=asc % T1aa_level;
    }
    else{
      n_asc=asc/T1aa_level;
      v_start=T1aa_level + (asc % T1aa_level); 
    }
  }
  else{
    n_asc=asc/T1aa_level;
    v_start=T1aa_level;
  }
  if (dsc % T1aa_level){ /* not aligned */
    if ( dsc < 0){
      n_dsc=dsc/T1aa_level-1;
      v_end=-(dsc % T1aa_level);
    }
    else{
      n_dsc=dsc/T1aa_level;
      v_end=T1aa_level - (dsc % T1aa_level);
    }
  }
  else{
    n_dsc=dsc/T1aa_level;
    v_end=T1aa_level;
  }
  /* the total number of lines: */
  n_vert=n_asc-n_dsc;
  
  /* Allocate memory for glyph */
  memsize = n_horz_pad*n_vert;
  
  aastring_glyph.bits = (char *)malloc(memsize*sizeof( char));
  if (aastring_glyph.bits == NULL) {
    T1_errno=T1ERR_ALLOC_MEM;
    return(NULL);
  }
  
  paddedW=PAD(wd,pFontBase->bitmap_pad)/8;
  offset=0;
  target_ptr=aastring_glyph.bits;
  
  /* We must check for n_vert==1 because the computation above is not
     valid in this case */
  if (n_vert==1)
    v_start=v_start < v_end ? v_start : v_end;
  
  ptr = glyph->bits;
  for (i = 0; i < n_vert; i++) {
    if (i==0)
      y=v_start;
    else if (i==n_vert-1)
      y=v_end;
    else
      y=T1aa_level;
    T1_AADoLine ( T1aa_level, wd, y, paddedW, ptr, target_ptr );
    ptr += y * paddedW;
    target_ptr += n_horz_pad;
  }
  k = n_horz;
  
  /* .. and set them in aastring_glyph */
  aastring_glyph.metrics.leftSideBearing=(int)floor(glyph->metrics.leftSideBearing/(float)T1aa_level+0.5);
  aastring_glyph.metrics.rightSideBearing=aastring_glyph.metrics.leftSideBearing + k;
  aastring_glyph.metrics.advanceX=(int)floor(glyph->metrics.advanceX/(float)T1aa_level+0.5);
  aastring_glyph.metrics.advanceY=(int)floor(glyph->metrics.advanceY/(float)T1aa_level+0.5);
  aastring_glyph.metrics.ascent=n_asc;
  aastring_glyph.metrics.descent=n_dsc;
  aastring_glyph.pFontCacheInfo=NULL;

  return(&aastring_glyph);
}



/* T1_AASetGrayValues(): Sets the byte values that are put into the
   pixel position for the respective entries:
   Returns 0 if successfull.
   */
int T1_AASetGrayValues(unsigned long white,
		       unsigned long gray75,
		       unsigned long gray50,
		       unsigned long gray25,
		       unsigned long black)
{
  
  if (CheckForInit()){
    T1_errno=T1ERR_OP_NOT_PERMITTED;
    return(-1);
  }
  
  gv[4]=(unsigned T1_AA_TYPE32)black;    /* black value */
  gv[3]=(unsigned T1_AA_TYPE32)gray25;   /* gray 25% value */
  gv[2]=(unsigned T1_AA_TYPE32)gray50;   /* gray 50% value */   
  gv[1]=(unsigned T1_AA_TYPE32)gray75;   /* gray 75% value */
  gv[0]=(unsigned T1_AA_TYPE32)white;    /* white value */
  
  T1aa_bg=white;
  
  if ((T1_AAInit( T1_AA_LOW)))
    return(-1);
  return(0);
  
}

		     

/* T1_AAHSetGrayValues(): Sets the byte values that are put into the
   pixel position for the respective entries (for 17 gray levels):
   Returns 0 if successfull.
   */
int T1_AAHSetGrayValues( unsigned long *grayvals)
{
  int i;
  
  if (CheckForInit()){
    T1_errno=T1ERR_OP_NOT_PERMITTED;
    return(-1);
  }

  /* 0==white(background) ... 16==black(foreground) */
  for (i=0; i<17; i++){
    gv_h[i]=(unsigned T1_AA_TYPE32)grayvals[i];
  }
  

  T1aa_bg=grayvals[0];
  
  if ((T1_AAInit( T1_AA_HIGH)))
    return(-1);
  return(0);
  
}


/* T1_AASetBitsPerPixel(): Sets the depths of the antialiased glyph
   pixel. Returns 0 if bpp is valid and -1 otherwise. If 24 is
   specified, meaning to be the depth rather than the bpp-value,
   automatically 32 bpp is chosen. */
int  T1_AASetBitsPerPixel( int bpp)
{
  
  if (CheckForInit()){
    T1_errno=T1ERR_OP_NOT_PERMITTED;
    return(-1);
  }
  

  /* T1aa_level = 0; */

  if (bpp==8){
    T1aa_bpp=8;
    return(0);
  }
  if (bpp==16){
    T1aa_bpp=16;
    return(0);
  }
  if ((bpp==32)|(bpp==24)){
    T1aa_bpp=32;
    return(0);
  }

  T1_errno=T1ERR_INVALID_PARAMETER;
  return(-1);
}


/* Set the Subsampling level for subsequent operations: */
int T1_AASetLevel( int level)
{
  
   if (CheckForInit()){
     T1_errno=T1ERR_OP_NOT_PERMITTED;
     return(-1);
   }

   if (level==T1_AA_LOW){
     T1aa_level=T1_AA_LOW;
     return(0);
   }
   else if (level==T1_AA_HIGH){
     T1aa_level=T1_AA_HIGH;
     return(0);
   }
   
   T1_errno=T1ERR_INVALID_PARAMETER;
   return(-1);
   
}


/* Get the current subsampling level */
int T1_AAGetLevel( void)
{
  return( T1aa_level);
}

