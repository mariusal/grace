/*--------------------------------------------------------------------------
  ----- File:        t1enc.c 
  ----- Author:      Rainer Menzner (rmz@neuroinformatik.ruhr-uni-bochum.de)
  ----- Date:        1999-09-03
  ----- Description: This file is part of the t1-library. It contains
                     functions encoding handling at runtime.
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
		     independ from X11.
                     Thanks to all people who make free software living!
--------------------------------------------------------------------------*/
  
#define T1ENC_C


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>


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
#include "t1enc.h"
#include "t1env.h"
#include "t1base.h"
#include "t1finfo.h"


#define DEFAULTENCODINGNAME  "Unspecified"


/* ScanEncodingFile(): Read an encoding file of an appropriate format
   and prepare the read data for usage with the type1 rasterizer, i.e.
   generate an array char *enc[256]. Return the pointer to the data area
   or NULL in case of an error.
   */
char **ScanEncodingFile( char *FileName) 
{
  
  char *linebuf;
  char *charnames;
  char save_char;
  char **encoding;
  
  FILE *enc_fp;
  int filesize, i, j, k, l=0, charname_count;

  if ((enc_fp=fopen( FileName,"r"))==NULL){
    T1_errno=T1ERR_FILE_OPEN_ERR;
    return(NULL);  /* file could not be opened 
		      => no encoding read */
  }
  
  
  /* enc_fp points now to a (hopefully) valid encoding file */
  /* Get the file size */
  fseek( enc_fp, 0, SEEK_END);
  filesize=ftell(enc_fp);
  /* Reset fileposition to start */
  fseek( enc_fp, 0, SEEK_SET);
  
  if ((linebuf=(char *)calloc( filesize,
			       sizeof(char)))==NULL){
    T1_errno=T1ERR_ALLOC_MEM;
    return(NULL);
  }
  
  /* Allocate space for character names, assume the worst case and realloc
     later: */
  if ((charnames=(char *)calloc( filesize + strlen(DEFAULTENCODINGNAME),
				 sizeof(char)))==NULL){
    T1_errno=T1ERR_ALLOC_MEM;
    return(NULL);
  }
  
  fread((char *)linebuf, sizeof(char), filesize, enc_fp);
  fclose(enc_fp);

  i=0;
  
  while(i<filesize){
    j=i;     /* Save index of beginning of line */
    while (isspace((int)linebuf[i])==0)
      i++;
    save_char=linebuf[i];
    linebuf[i]=0;  /* replace ' ' by ASCII-0 */
    if (strncmp( "Encoding=", &linebuf[j], strlen("Encoding="))==0){
      /* We save the current file position to read the encoding name
	 later */
      l=j+strlen("Encoding=");
      /* set index to start of next line */
      if (save_char=='\n')
	i++;   
      else {
	while (linebuf[i]!='\n')
	  i++;
	i++;
      }
      /* keyword found => now, 256 lines should follow, each
	 specifying a character name and optionally some comment
	 to enhance readability: */
      break;
    }
    i++;
  }
  
  k=0;
  charname_count=0;

  while((i<filesize) && (charname_count<256)){
    j=i;     /* Save index of beginning of line */
    while (isspace((int)linebuf[i])==0)
      i++;
    save_char=linebuf[i];
    linebuf[i]=0;  /* replace whitespace by ASCII-0 */
    /* Copy Name to *char_names-array */
    while (linebuf[j])
      charnames[k++]=linebuf[j++];
    /* Append ASCII-0 */
    charnames[k++]=linebuf[j++];
    /* set index to start of next line */
    if (save_char=='\n')
      i++;   
    else {
      while (linebuf[i]!='\n')
	i++;
      i++;
    }
    /* Increment character counter */
    charname_count++;
  }

  /* Check if exactly 256 characters have been defined, if not,
     return NULL: */
  if (charname_count!=256){
    free(charnames);
    T1_errno=T1ERR_UNSPECIFIED;
    return(NULL);
  }

  /* Append the string for the encoding's name */
  i=l;
  while (isspace((int)linebuf[i])==0 && linebuf[i]!='\0'){
    charnames[k++]=linebuf[i++];
  }
  
  if (i==l){
    strcpy(&(charnames[k]), DEFAULTENCODINGNAME);
    k +=strlen(DEFAULTENCODINGNAME);
    charnames[k++]='\0';
  }
  else{
    charnames[k++]='\0';
  }

  free(linebuf);

  /* We alloc 257 to save the encoding's name at the 257th entry */
  if ((encoding=(char **)malloc(257*sizeof(char *)))==NULL){
    free(charnames);
    T1_errno=T1ERR_ALLOC_MEM;
    return(NULL);
  }
  /* k should still contain the number of characters copied, so let's
     now realloc charnames */
  charnames=(char *)realloc( charnames, k*sizeof(char));
  /* Now initialize the array with the start-addresses of the character
     name strings */
  i=0;
  j=0;
  
  while (i<257){
    encoding[i]=&charnames[j];
    while (charnames[j])
      j++;
    j++;
    i++;
  }
  return(encoding);
}



/* T1_LoadEncoding(): Load an encoding file to have a new encoding
   available. If successful, the pointer to the encoding array is
   returned. In case of an error, the return value is NULL.
   */
char **T1_LoadEncoding( char *FileName)
{
  char **Encoding;
  char *EncFileName;
  
  if( FileName==NULL){
    T1_errno=T1ERR_INVALID_PARAMETER;
    return(NULL);
  }
  
  if ((EncFileName=Env_GetCompletePath( FileName, T1_ENC_ptr))==NULL){
    T1_errno=T1ERR_FILE_OPEN_ERR;
    return(NULL);
  }
  Encoding=ScanEncodingFile(EncFileName);
  free(EncFileName);

  return(Encoding);
}

  

/* T1_DeleteEncoding() free a previously loaded encoding */
int T1_DeleteEncoding( char **encoding)
{
  if (encoding){
    /* First free character names memory */
    free( encoding[0]);
    /* then, free pointer array */
    free( encoding);
  }
  return(0);
  
}



/* T1_ReencodeFont(): Assign a new encoding to an existent font. This is
   only allowed if no size dependent data exists for the font in question.
   Moreover, the font must be loaded already since must get the position
   of the space-character. Function returns 0 if successful, and -1 otherwise.
   */
int T1_ReencodeFont( int FontID, char **Encoding)
{
  int i, j, k;
  char *charname;
  PairKernData *pkd;
  METRICS_ENTRY *kern_tbl;
  int char1, char2;
  
  
  /* First, check for valid font ID residing in memory: */
  if (CheckForFontID(FontID)!=1){
    T1_errno=T1ERR_INVALID_FONTID;
    return(-1);
  }
  
  /* Second, check whether size-dependent data exists: */
  if (pFontBase->pFontArray[FontID].pFontSizeDeps != NULL){
    T1_errno=T1ERR_OP_NOT_PERMITTED;
    return(-1);
  }
  
  pFontBase->pFontArray[FontID].pFontEnc=Encoding;


  /* We have to update the space_position-entry in the FONTPRIVATE. 
     If space is not found (not encoded), set it to -1: */
  pFontBase->pFontArray[FontID].space_position=-1;
  i=0;
  if (Encoding){ /* external encoding */
    while (i<256){
      if (strcmp( (char *)pFontBase->pFontArray[FontID].pFontEnc[i],
		  "space")==0){
	/* space found at position i: */
	pFontBase->pFontArray[FontID].space_position=i;
	break;
      }
      i++;
    }
  }
  else{ /* reencoding to internal encoding */
    while (i<256){
      if (strcmp( (char *)pFontBase->pFontArray[FontID].pType1Data->fontInfoP[ENCODING].value.data.arrayP[i].data.arrayP,
		  "space")==0){
	/* space found at position i: */
	pFontBase->pFontArray[FontID].space_position=i;
	break;
      }
      i++;
    }
  }

  /* Now update afm index mapping: */
  if (pFontBase->pFontArray[FontID].pAFMData != NULL){
    for (i=0; i<256; i++){
      pFontBase->pFontArray[FontID].pEncMap[i]=-1;
      charname=T1_GetCharName( FontID, i);
      if (strcmp( charname, ".notdef")==0) {
	/* This is because not all AFM files have the .notdef, so we can't
	   rely on it */
	pFontBase->pFontArray[FontID].pEncMap[i]=-2;
	continue;
      }
      charname=T1_GetCharName( FontID, i);
      for ( j=0; j<pFontBase->pFontArray[FontID].pAFMData->numOfChars; j++){
	if (strcmp( charname,
		    pFontBase->pFontArray[FontID].pAFMData->cmi[j].name)==0){
	  pFontBase->pFontArray[FontID].pEncMap[i]=j;
	}
      }
    }
    
    /* Update kerning table */
    k=pFontBase->pFontArray[FontID].pAFMData->numOfPairs;
    if (k>0){ /* i.e., there are any pairs */ 
      kern_tbl=pFontBase->pFontArray[FontID].pKernMap;
      pkd=pFontBase->pFontArray[FontID].pAFMData->pkd;
      j=0;
      for ( i=0; i<k; i++){
	if ((char1=T1_GetEncodingIndex( FontID, pkd[i].name1))==-1){
	  /* pair is not relevant in current encoding */
	  continue;
	}
	if ((char2=T1_GetEncodingIndex( FontID, pkd[i].name2))==-1){
	  /* pair is not relevant in current encoding */
	  continue;
	}
	/* Since we get here we have a relevant pair -->
	   Put char1 in higher byte and char2 in LSB:
	   */
	kern_tbl[j].chars=(char1 << 8) | char2;
	
	/* We only make use of horizontal kerning */
	kern_tbl[j].hkern=pkd[i].xamt;
	j++;
      }
      /* Reset the remaining of the table in case there were
	 irrelevant pairs: */
      for ( ; j<k; j++){
	kern_tbl[j].chars=0;
	kern_tbl[j].hkern=0;
      }
      /* We now sort the kerning array with respect to char indices */
      qsort( kern_tbl, (size_t) j, sizeof(METRICS_ENTRY),
	     &cmp_METRICS_ENTRY );
    }
    else
      pFontBase->pFontArray[FontID].pKernMap=NULL;
    
  }
  return(0);
}



/* T1_SetDefaultEncoding(): Set the default encoding vector that's
   used when fonts are loaded.
   */
int T1_SetDefaultEncoding( char **encoding)
{
  
  if (CheckForInit()){
    T1_errno=T1ERR_OP_NOT_PERMITTED;
    return(-1);
  }
  
  pFontBase->default_enc=encoding;
  return(0);
}


/* T1_GetEncodingScheme(): Get the name associated with the current
   encoding vector of font FontID */
char *T1_GetEncodingScheme( int FontID)
{
  
  static char enc_scheme[256];
  
  /* First, check for valid font ID residing in memory: */
  if (CheckForFontID(FontID)!=1){
    T1_errno=T1ERR_INVALID_FONTID;
    return(NULL);
  }

  if (pFontBase->pFontArray[FontID].pFontEnc==NULL){
    if (pFontBase->pFontArray[FontID].info_flags & USES_STANDARD_ENCODING){
      strcpy( enc_scheme, "StandardEncoding");
    }
    else {
      strcpy( enc_scheme, "FontSpecific");
    }
  }
  else
    strcpy( enc_scheme, pFontBase->pFontArray[FontID].pFontEnc[256]);
  
  return(enc_scheme);
  
}


/* A function for comparing METRICS_ENTRY structs */
static int cmp_METRICS_ENTRY( const void *entry1, const void *entry2)
{
  if (((METRICS_ENTRY *)entry1)->chars <
      ((METRICS_ENTRY *)entry2)->chars)
    return(-1);
  if (((METRICS_ENTRY *)entry1)->chars >
      ((METRICS_ENTRY *)entry2)->chars)
    return(1);
  return(0); /* This should not happen */
}
    
