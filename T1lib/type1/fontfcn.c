/* $XConsortium: fontfcn.c,v 1.8 92/03/27 18:15:45 eswu Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* Author: Katherine A. Hitchcock    IBM Almaden Research Laboratory */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "t1imager.h" 
#include "util.h"
#include "fontfcn.h"
#include "fontmisc.h"
#include "paths_rmz.h" 

#include "../t1lib/parseAFM.h" 
#include "../t1lib/t1types.h"
#include "../t1lib/t1extern.h"
#include "../t1lib/t1misc.h"


extern xobject Type1Char(psfont *env, struct XYspace *S,
			 psobj *charstrP, psobj *subrsP,
			 psobj *osubrsP,
			 struct blues_struct *bluesP,
			 int *modeP);
extern xobject Type1Line(psfont *env, struct XYspace *S,
				 float line_position,
				 float line_thickness,
				 float line_length);
extern void T1io_reset(void);


/***================================================================***/
/*   GLOBALS                                                          */
/***================================================================***/
char CurFontName[120];
char *CurFontEnv;
char *vm_base = NULL;

static char notdef[]=".notdef";


/* the following is inserted by RMz for VM checking and reallocating: */
char *vm_used = NULL;
extern int vm_init_count;
extern int vm_init_amount;


psfont *FontP = NULL;
psfont TheCurrentFont;
 
 
/***================================================================***/
/*   SearchDict - look for  name                                      */
/*              - compare for match on len and string                 */
/*                return 0 - not found.                               */
/*                return n - nth element in dictionary.               */
/***================================================================***/
int SearchDictName(dictP,keyP)
 psdict *dictP;
 psobj  *keyP;
{
  int i,n;
 
 
  n =  dictP[0].key.len;
  for (i=1;i<=n;i++) {          /* scan the intire dictionary */
    if (
        (dictP[i].key.len  == keyP->len )
        &&
        (strncmp(dictP[i].key.data.valueP,
                 keyP->data.valueP,
                 keyP->len) == 0
        )
       ) return(i);
  }
  return(0);
}
/***================================================================***/
/* assignment of &TheCurrentFont removed by RMz:
 */
boolean initFont()
{
  extern  boolean Init_BuiltInEncoding();

  
  if (!(vm_init())) return(FALSE);
  vm_base = vm_next_byte();
  if (!(Init_BuiltInEncoding())) return(FALSE);
  strcpy(CurFontName, "");    /* iniitialize to none */
  FontP->vm_start = vm_next_byte();
  FontP->FontFileName.len = 0;
  FontP->FontFileName.data.valueP = CurFontName;
  return(TRUE);
}
/***================================================================***/
int resetFont(env)
char *env;
{
 
  vm_next =  FontP->vm_start;
  vm_free = vm_size - ( vm_next - vm_base);
  FontP->Subrs.len = 0;
  FontP->Subrs.data.stringP = NULL;
  FontP->CharStringsP = NULL;
  FontP->Private = NULL;
  FontP->fontInfoP = NULL;
  FontP->BluesP = NULL;
  /* This will load the font into the FontP */
  strcpy(CurFontName,env);
  FontP->FontFileName.len = strlen(CurFontName);
  FontP->FontFileName.data.nameP = CurFontName;
  T1io_reset();

  return(0);
  
}
/***================================================================***/
/* Read font used to attempt to load the font and, upon failure, 
   try a second time with twice as much memory.  Unfortunately, if
   it's a really complex font, simply using 2*vm_size may be insufficient.
   I've modified it so that the program will try progressively larger
   amounts of memory until it really runs out or the font loads
   successfully. (ndw)
*/
int readFont(env)
char *env;
{
  int rcode;
  /* int memscale = 2; */ /* initially, try twice just like we used to ... */
 
  /* restore the virtual memory and eliminate old font */
  
  resetFont(env);
  /* This will load the font into the FontP */
 
  rcode = scan_font(FontP);
  return(rcode);
}


/***================================================================***/
/* RMz: instead of code, which is a character pointer to the name
        of the character, we use "ev" which is a pointer to a desired
	encoding vector (or NULL if font-internal encoding should be
	used) and "index" as an index into the desired encoding vector!
	The user thus has the opportunity of supplying whatever encoding
	he wants. Font_Ptr is the pointer to the local psfont-structure. 
	*/

xobject fontfcnB(int FontID, int modflag,
		 struct XYspace *S, char **ev,
		 unsigned char index, int *mode,
		 psfont *Font_Ptr,
		 int do_raster)
{
  path updateWidth();
 
  psobj *charnameP; /* points to psobj that is name of character*/
  int   N;
  psdict *CharStringsDictP; /* dictionary with char strings     */
  psobj   CodeName;   /* used to store the translation of the name*/
  psobj  *SubrsArrayP;
  psobj  *theStringP;
 
  struct segment *charpath;   /* the path for this character   */           
  struct segment *tmppath1;   /* For concatenation */
  int acc_width;
  
  
  /* set the global font pointer to the address of already allocated
     structure */
  FontP=Font_Ptr;

  if (ev==NULL){  /* font-internal encoding should be used */
    charnameP = &CodeName;
    charnameP->len = FontP->fontInfoP[ENCODING].value.data.arrayP[index].len;
    charnameP->data.stringP = (unsigned char *) FontP->fontInfoP[ENCODING].value.data.arrayP[index].data.arrayP;
  }
  else{           /* some user-supplied encoding is yo be used */
    charnameP = &CodeName;
    charnameP->len = strlen(ev[index]);
    charnameP->data.stringP = (unsigned char *) ev[index];
  }
  
  CharStringsDictP =  FontP->CharStringsP;
 
  /* search the chars string for this charname as key */
  N = SearchDictName(CharStringsDictP,charnameP);
  if (N<=0) {
    /* Instead of returning an error, we substitute .notdef (RMz) */
    charnameP = &CodeName;
    charnameP->len = 7;
    charnameP->data.stringP = (unsigned char *) &notdef;
    N = SearchDictName(CharStringsDictP,charnameP);
    /* Font must be completely damaged if it doesn't define a .notdef */
    if (N<=0){
      *mode=FF_PARSE_ERROR;
      return(NULL);
    }
  }
  /* ok, the nth item is the psobj that is the string for this char */
  theStringP = &(CharStringsDictP[N].value);
 
  /* get the dictionary pointers to the Subrs  */
 
  SubrsArrayP = &(FontP->Subrs);

  /* call the type 1 routine to rasterize the character     */
  charpath = (struct segment *) Type1Char(FontP,S,theStringP,SubrsArrayP,NULL,
					  FontP->BluesP , mode);
  
  /* for debugging path elements */
  /*  tmppath1=charpath;
#define   LINETYPE    (0x10)
#define   CONICTYPE   (0x11)
#define   BEZIERTYPE  (0x12)
#define   HINTTYPE    (0x13)
 			
#define   MOVETYPE    (0x15)
#define   TEXTTYPE    (0x16)
  
  while (tmppath1->link!=NULL){
    printf("tmppath1=%p\n", tmppath1);
    
    if (tmppath1->type==LINETYPE)
      printf("path->type: LINETYPE\n");
    if (tmppath1->type==CONICTYPE)
      printf("path->type: CONICTYPE\n");
    if (tmppath1->type==MOVETYPE)
      printf("path->type: MOVETYPE\n");
    if (tmppath1->type==BEZIERTYPE)
      printf("path->type: BEZIERTYPE\n");
    if (tmppath1->type==HINTTYPE)
      printf("path->type: HINTTYPE\n");
    if (tmppath1->type==TEXTTYPE)
      printf("path->type: TEXTTYPE\n");
    tmppath1=tmppath1->link;
    }
  */
    
  /* Get width of char in charspace coordinates. We do not multiply with
     extension factor because extension is already done in the space matrix */
  acc_width = (int) (pFontBase->pFontArray[FontID].pAFMData->cmi[pFontBase->pFontArray[FontID].pEncMap[(int) index]].wx);
  
  /* Take care for underlining and such */
  if (modflag & T1_UNDERLINE){
    tmppath1=(struct segment *)Type1Line(FontP,S,
					 pFontBase->pFontArray[FontID].UndrLnPos,
					 pFontBase->pFontArray[FontID].UndrLnThick,
					 (float) acc_width);
    charpath=(struct segment *)Join(charpath,tmppath1);
  }
  if (modflag & T1_OVERLINE){
    tmppath1=(struct segment *)Type1Line(FontP,S,
					 pFontBase->pFontArray[FontID].OvrLnPos,
					 pFontBase->pFontArray[FontID].OvrLnThick,
					 (float) acc_width);
    charpath=(struct segment *)Join(charpath,tmppath1);
  }
  if (modflag & T1_OVERSTRIKE){
    tmppath1=(struct segment *)Type1Line(FontP,S,
					 pFontBase->pFontArray[FontID].OvrStrkPos,
					 pFontBase->pFontArray[FontID].OvrStrkThick,
					 (float) acc_width);
    charpath=(struct segment *)Join(charpath,tmppath1);
  }
  
  /* if Type1Char reported an error, then return */

  if ( *mode == FF_PARSE_ERROR)  return(NULL);
  if ( *mode == FF_PATH_ERROR)  return(NULL);
  if (do_raster) { 
    /* fill with winding rule unless path was requested */
    if (*mode != FF_PATH) {
      charpath =  (struct segment *)Interior(charpath,WINDINGRULE+CONTINUITY);
    }
  }
  
  return((xobject) charpath);
}




/***================================================================***/
/*   fontfcnA(env, mode)                                              */
/*                                                                    */
/*          env is a pointer to a string that contains the fontname.  */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/*                                                                    */
/* This function has been modified by RMz. It now takes a pointer which
   already contains the address of a valid type1 font structure as the
   third argument. The value of this pointer is first handed to FontP
   so that most other routines may be used without changes */

#define MAXTRIAL 4  

/***================================================================***/
Bool fontfcnA(env,mode,Font_Ptr)
char *env;
int  *mode;
psfont *Font_Ptr;

{
  int i, result;
  
  /* set the global font pointer to the address of already allocated
     structure */
  FontP=Font_Ptr;

  InitImager();

  /* Read the font program. */
  for (i=1; i<MAXTRIAL; i++){
    vm_init_count=0;
    vm_init_amount=MAX_STRING_LEN * i;
    if (!(initFont())) {
      /* we are really out of memory, not simulated! */
      *mode = SCAN_OUT_OF_MEMORY;
      return(FALSE);
    }
    /* Try to read font into memory */
    if ((result=readFont(env))==0){
      /* In order to get the amount of memory that was really used */      
      vm_used=vm_next_byte();
      return(TRUE);
    }
    else{
      /* VM did not suffice, free it and try again with larger
	 value: */
      free(vm_base);
    }
  }
  /* Font could not be loaded: */
  *mode = result;
  return(FALSE);

}


/***================================================================***/
/*   QueryFontLib(env, infoName,infoValue,rcodeP)                     */
/*                                                                    */
/*          env is a pointer to a string that contains the fontname.  */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/*     3) use the font to call getInfo for that value.                */
/***================================================================***/

void QueryFontLib(env,infoName,infoValue,rcodeP)
char *env;
char *infoName;
pointer infoValue;    /* parameter returned here    */
int  *rcodeP;
{

  void objFormatName(psobj *objP, int length, char *valueP);
  
  int rc,N,i;
  psdict *dictP;
  psobj  nameObj;
  psobj  *valueP;
 
  /* Has the FontP initialized?  If not, then   */
  /* Initialize  */
  if (FontP == NULL) {
    InitImager();
    if (!(initFont())) {
      *rcodeP = 1;
      return;
    }
  }
  /* if the env is null, then use font already loaded */
  /* if the not same font name, reset and load next font */
  if ( (env) && (strcmp(env,CurFontName) != 0 ) ) {
    /* restore the virtual memory and eliminate old font */
    rc = readFont(env);
    if (rc != 0 ) {
      strcpy(CurFontName, "");    /* no font loaded */
      *rcodeP = 1;
      return;
    }
  }
  dictP = FontP->fontInfoP;
  objFormatName(&nameObj,strlen(infoName),infoName);
  N = SearchDictName(dictP,&nameObj);
  /* if found */
  if ( N > 0 ) {
    *rcodeP = 0;
    switch (dictP[N].value.type) {
       case OBJ_ARRAY:
         valueP = dictP[N].value.data.arrayP;
         if (strcmp(infoName,"FontMatrix") == 0) {
           /* 6 elments, return them as floats      */
           for (i=0;i<6;i++) {
             if (valueP->type == OBJ_INTEGER )
               ((float *)infoValue)[i] = valueP->data.integer;
             else
               ((float *)infoValue)[i] = valueP->data.real;
            valueP++;
           }
         }
         if (strcmp(infoName,"FontBBox") == 0) {
           /* 4 elments for Bounding Box.  all integers   */
           for (i=0;i<4;i++) {
             ((int *)infoValue)[i] = valueP->data.integer;
             valueP++;
           }
         break;
       case OBJ_INTEGER:
       case OBJ_BOOLEAN:
         *((int *)infoValue) = dictP[N].value.data.integer;
         break;
       case OBJ_REAL:
         *((float *)infoValue) = dictP[N].value.data.real;
         break;
       case OBJ_NAME:
       case OBJ_STRING:
         *((char **)infoValue) =  dictP[N].value.data.valueP;
         break;
       default:
         *rcodeP = 1;
         break;
     }
   }
  }
  else *rcodeP = 1;
}


/***================================================================***/
/* RMz: instead of code, which is a character pointer to the name
        of the character, we use "ev" which is a pointer to a desired
	encoding vector (or NULL if font-internal encoding should be
	used) and "index" as an index into the desired encoding vector!
	The user thus has the opportunity of supplying whatever encoding
	he wants. Font_Ptr is the pointer to the local psfont-structure. 
	*/

xobject fontfcnB_string( int FontID, int modflag,
			 struct XYspace *S, char **ev,
			 unsigned char *string, int no_chars,
			 int *mode, psfont *Font_Ptr,
			 int *kern_pairs, long spacewidth,
			 int do_raster)
{
  path updateWidth();
 
  psobj *charnameP; /* points to psobj that is name of character*/
  int   N;
  psdict *CharStringsDictP; /* dictionary with char strings     */
  psobj   CodeName;   /* used to store the translation of the name*/
  psobj  *SubrsArrayP;
  psobj  *theStringP;
  long   acc_width=0;
  int    i;
  struct segment  *charpath, *tmppath1, *tmppath2;
  
  /* set the global font pointer to the address of already allocated
     structure */
  FontP=Font_Ptr;
  /* get pointers to CharStrings and Subroutines */
  CharStringsDictP =  FontP->CharStringsP;
  SubrsArrayP = &(FontP->Subrs);
  
  charpath=NULL;
  
  /* In the following for-loop, all characters are processed, one after
     the other. Between them, the amount of kerning is inserted: */
  for (i=0; i<no_chars;i++){
    if (ev==NULL){  /* font-internal encoding should be used */
      charnameP = &CodeName;
      charnameP->len = FontP->fontInfoP[ENCODING].value.data.arrayP[string[i]].len;
      charnameP->data.stringP = (unsigned char *) FontP->fontInfoP[ENCODING].value.data.arrayP[string[i]].data.arrayP;
    }
    else{           /* some user-supplied encoding is yo be used */
      charnameP = &CodeName;
      charnameP->len = strlen(ev[string[i]]);
      charnameP->data.stringP = (unsigned char*) ev[string[i]];
    }
    
    /* Spacing is to be under users control: => if space is the charname, don't
       raster it. Rather, generate a horizontal movement of spacewidth: */
    if (strcmp((char *)charnameP->data.stringP, "space")==0){
      tmppath1=(struct segment *)ILoc(S, spacewidth,0);
      acc_width += spacewidth;
      
    }
    /* We ignore the .notdef character completely */
    else if (strcmp((char *)charnameP->data.stringP, ".notdef")==0){
      tmppath1=(struct segment *)ILoc(S, 0, 0);
    }
    else{
      /* search the chars string for this charname as key */
      N = SearchDictName(CharStringsDictP,charnameP);
      if (N<=0) {
	if (no_chars==1){
	  /* Instead of returning an error, we substitute .notdef (RMz) */
	  charnameP = &CodeName;
	  charnameP->len = 7;
	  charnameP->data.stringP = (unsigned char *) &notdef;
	  N = SearchDictName(CharStringsDictP,charnameP);
	}
	else
	  /* We simply omit that character */
	  continue;
	/* Font must be completely damaged if it doesn't define a .notdef */
	if (N<=0){
	  *mode=FF_PARSE_ERROR;
	  return(NULL);
	}
      }

      /* Accumulate width of string */
      acc_width += (int) ((pFontBase->pFontArray[FontID].pAFMData->cmi[pFontBase->pFontArray[FontID].pEncMap[(int) string[i]]].wx));
      
      
      /* ok, the nth item is the psobj that is the string for this char */
      theStringP = &(CharStringsDictP[N].value);
      
      /* call the type 1 routine to rasterize the character     */
      tmppath1=(struct segment *) Type1Char(FontP,S,theStringP,SubrsArrayP,NULL,
					    FontP->BluesP , mode);
    }
    
    if (i<no_chars-1){
      tmppath2=(struct segment *)ILoc(S,kern_pairs[i],0); 
      tmppath1=(struct segment *)Join(tmppath1,tmppath2);
      acc_width += kern_pairs[i];
    }
    if (charpath!=NULL){
      charpath=(struct segment *)Join(charpath,tmppath1);
    }
    else{
      charpath=(struct segment *)tmppath1;
    }
  }

  /* Take care for underlining and such */
  if (modflag & T1_UNDERLINE){
    tmppath2=(struct segment *)Type1Line(FontP,S,
					 pFontBase->pFontArray[FontID].UndrLnPos,
					 pFontBase->pFontArray[FontID].UndrLnThick,
					 (float) acc_width);
    charpath=(struct segment *)Join(charpath,tmppath2);
  }
  if (modflag & T1_OVERLINE){
    tmppath2=(struct segment *)Type1Line(FontP,S,
					 pFontBase->pFontArray[FontID].OvrLnPos,
					 pFontBase->pFontArray[FontID].OvrLnThick,
					 (float) acc_width);
    charpath=(struct segment *)Join(charpath,tmppath2);
  }
  if (modflag & T1_OVERSTRIKE){
    tmppath2=(struct segment *)Type1Line(FontP,S,
					 pFontBase->pFontArray[FontID].OvrStrkPos,
					 pFontBase->pFontArray[FontID].OvrStrkThick,
					 (float) acc_width);
    charpath=(struct segment *)Join(charpath,tmppath2);
  }
  

  
  
  /*
  printf("charpath->type: %x\n",charpath->type);
  printf("path1->type: %x\n",path1->type);
  printf("path2->type: %x\n",path2->type);

  */

  /* if Type1Char reported an error, then return */

  if ( *mode == FF_PARSE_ERROR)  return(NULL);
  if ( *mode == FF_PATH_ERROR)  return(NULL);
  if (do_raster) { 
    /* fill with winding rule unless path was requested */
    if (*mode != FF_PATH) {
      charpath = (struct segment *) Interior((path) charpath,WINDINGRULE+CONTINUITY);
    }
  }
  
  return((path)charpath);
}


/* This special variant is for generating character bitmaps from
   charactername */
xobject fontfcnB_ByName( int FontID, int modflag,
			 struct XYspace *S,
			 unsigned char *charname,
			 int *mode, psfont *Font_Ptr,
			 int do_raster)
{
  path updateWidth();
 
  psobj *charnameP; /* points to psobj that is name of character*/
  int   N;
  psdict *CharStringsDictP; /* dictionary with char strings     */
  psobj   CodeName;   /* used to store the translation of the name*/
  psobj  *SubrsArrayP;
  psobj  *theStringP;
 
  struct segment *charpath;   /* the path for this character   */           
  
  /* set the global font pointer to the address of already allocated
     structure */
  FontP=Font_Ptr;

  charnameP = &CodeName;
  charnameP->len = strlen((char *)charname);
  charnameP->data.stringP = (unsigned char*) charname;
  
  CharStringsDictP =  FontP->CharStringsP;
  
  /* search the chars string for this charname as key */
  N = SearchDictName(CharStringsDictP,charnameP);
  if (N<=0) {
    *mode = FF_PARSE_ERROR;
    return(NULL);
  }
  /* ok, the nth item is the psobj that is the string for this char */
  theStringP = &(CharStringsDictP[N].value);
 
  /* get the dictionary pointers to the Subrs  */
 
  SubrsArrayP = &(FontP->Subrs);

  /* call the type 1 routine to rasterize the character     */
  charpath = (struct segment *)Type1Char(FontP,S,theStringP,
					 SubrsArrayP,NULL,
					 FontP->BluesP , mode);

  /* if Type1Char reported an error, then return */

  if ( *mode == FF_PARSE_ERROR)  return(NULL);
  if ( *mode == FF_PATH_ERROR)  return(NULL);
  if (do_raster) { 
    /* fill with winding rule unless path was requested */
    if (*mode != FF_PATH) {
      charpath = (struct segment *) Interior(charpath,WINDINGRULE+CONTINUITY);
    }
  }

  return((xobject) charpath);
}

