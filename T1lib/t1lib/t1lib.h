/*--------------------------------------------------------------------------
  ----- File:        t1lib.h
  ----- Author:      Rainer Menzner (rmz@neuroinformatik.ruhr-uni-bochum.de)
  ----- Date:        08/29/1998
  ----- Description: This file is part of the t1-library. It must be
                     included by the user of the t1lib. It contains
		     function declarations and some basic data types, the
		     user must deal with.
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
  


/* type definitions, needed by the user: */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


/* the data structure which contains the character bitmap description */
typedef struct
{
  char *bits;           /* A pointer to the characters local bitmap */
  struct                /* A struct containing diverse metric information */
  {
    int ascent;
    int descent;
    int leftSideBearing;
    int rightSideBearing;
    int advanceX;
    int advanceY;
  } metrics;
  void *pFontCacheInfo;
  unsigned long bpp;
} GLYPH;


/* A structure representing a matrix */
typedef struct
{
  double cxx;
  double cyx;
  double cxy;
  double cyy;
} T1_TMATRIX;


/* the bounding box data structure: */
typedef struct
{ 
   int llx;	/* lower left x-position  */
   int lly;	/* lower left y-position  */
   int urx;	/* upper right x-position */
   int ury;	/* upper right y-position */
} BBox;


/* A data that makes most important information available to user. */
typedef struct
{
  int      width;       /* The glyph's width */
  BBox     bbox;        /* The glyph's bounding box */
  int      numchars;    /* The number of characters in the glyph (string) */
  int      *charpos;    /* A pointer to an integer array were the horizontal
			   positions in (afm units) of the individual
			   characters in the string are stored */
} METRICSINFO;


#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))


/* def's for T1_InitLib() */
#define NO_LOGFILE          0x0
#define LOGFILE             0x1
#define IGNORE_FONTDATABASE 0x2  /* Default is to read database */
#define IGNORE_CONFIGFILE   0x4  /* Default is to read config file */


/* logfile levels: */
#define T1LOG_ERROR     1
#define T1LOG_WARNING   2
#define T1LOG_STATISTIC 3
#define T1LOG_DEBUG     4


/* #define's for manipulating searchpaths */
#define T1_PFAB_PATH        0x01
#define T1_AFM_PATH         0x02
#define T1_ENC_PATH         0x04
#define T1_APPEND_PATH      0x00
#define T1_PREPEND_PATH     0x01


/* #define's for character attributes (not all implemented) */
#define T1_UNDERLINE           0x0001
#define T1_OVERLINE            0x0002
#define T1_OVERSTRIKE          0x0004
#define T1_DOUBLEOVERSTRIKE    0x0008
#define T1_SUBSCRIPT           0x0100
#define T1_SUPERSCRIPT         0x0200
#define T1_OUTLINE             0x1000
#define T1_KERNING             0x2000


/* Setting the subsampling value */
#define T1_AA_LOW   2
#define T1_AA_HIGH  4


/* Error handling: */
extern int T1_errno;

/* These are from scanning a font file */
#define T1ERR_SCAN_FILE_OPEN_ERR     -4
#define T1ERR_SCAN_OUT_OF_MEMORY     -3
#define T1ERR_SCAN_ERROR             -2
#define T1ERR_SCAN_FILE_EOF          -1
/* These are from generating paths */
#define T1ERR_PATH_ERROR              1
#define T1ERR_PARSE_ERROR             2
/* These are from t1lib */
#define T1ERR_INVALID_FONTID          10
#define T1ERR_INVALID_PARAMETER       11
#define T1ERR_OP_NOT_PERMITTED        12
#define T1ERR_ALLOC_MEM               13
#define T1ERR_FILE_OPEN_ERR           14
#define T1ERR_UNSPECIFIED             15


/* Flags to control the rasterizer */
#define T1_IGNORE_FORCEBOLD           0x0001
#define T1_IGNORE_FAMILYALIGNMENT     0x0002
#define T1_IGNORE_HINTING             0x0004

#define T1_DEBUG_LINE                 0x0100
#define T1_DEBUG_REGION               0x0200
#define T1_DEBUG_PATH                 0x0400
#define T1_DEBUG_FONT                 0x0800
#define T1_DEBUG_HINT                 0x1000


/* function declarations: */

/* from t1base.c */
extern void *T1_InitLib( int log);
extern int scanFontDBase( char *filename);
extern int T1_CloseLib( void);
extern int T1_AddFont( char *fontfilename);
extern void print_msg( char *func_ident, char *msg_txt);
extern void T1_PrintLog( char *func_ident, char *msg_txt, int level);
extern void T1_SetLogLevel( int level);
extern int CheckForInit(void);
extern int CheckForFontID( int FontID);
extern int test_for_t1_file( char *buffer );
extern char *T1_GetFontFileName( int FontID);
extern int  T1_Get_no_fonts(void);
extern int T1_SetDeviceResolutions( float x_res, float y_res);
extern int T1_CopyFont( int FontID);
extern int T1_QueryX11Support( void);
extern void bin_dump_c(unsigned char value);
extern void bin_dump_s(unsigned short value);
extern void bin_dump_l(unsigned long value);
extern int T1_CheckEndian(void);
extern int T1_SetBitmapPad( int pad);
extern int T1_GetBitmapPad( void);
extern int T1_SetFontDataBase( char *filename);
extern char *T1_GetLibIdent( void);
extern void T1_SetRasterFlags( int flags);

/* from t1delete.c */
extern int T1_DeleteSize( int FontID, float size);
extern int T1_DeleteAllSizes( int FontID);
extern int T1_FreeGlyph( GLYPH *glyph);
extern int T1_DeleteFont( int FontID);

/* from t1enc.c */
extern char **ScanEncodingFile( char *FileName);
extern char **T1_LoadEncoding( char *FileName);
extern int T1_DeleteEncoding( char **encoding);
extern int T1_ReencodeFont( int FontID, char **Encoding);
extern int T1_SetDefaultEncoding( char **encoding);
extern char *T1_GetEncodingScheme( int FontID);

/* from t1env.c */
extern int ScanConfigFile( char **pfabenv_ptr, char **afmenv_ptr,
			   char **encenv_ptr, char **fontdatabase_ptr);
extern char *Env_GetCompletePath( char *FileName, char *env_ptr );
extern int T1_SetFileSearchPath( int type, char *pathname);
extern int T1_AddToFileSearchPath( int pathtype, int mode, char *pathname);
extern char *T1_GetFileSearchPath( int type);

/* from t1finfo.c */
extern int T1_GetKerning( int FontID, char char1,
			  char char2);
extern int T1_GetCharWidth( int FontID, char char1);
extern BBox T1_GetCharBBox( int FontID, char char1);
extern float T1_GetUnderlinePosition( int FontID);
extern float T1_GetUnderlineThickness( int FontID);
extern float T1_GetItalicAngle( int FontID);
extern int T1_GetIsFixedPitch( int FontID);
extern char *T1_GetFontName( int FontID);
extern char *T1_GetFullName( int FontID);
extern char *T1_GetFamilyName( int FontID);
extern char *T1_GetWeight( int FontID);
extern char *T1_GetVersion( int FontID);
extern char *T1_GetNotice( int FontID);
extern char *T1_GetCharName( int FontID, char char1);
extern int T1_QueryLigs( int FontID,
			 char char1,
			 char **successors,
			 char **ligatures);
extern int T1_GetEncodingIndex( int FontID, char *char1);
extern int T1_GetStringWidth( int FontID, char *string,
			      int len,  long spaceoff, int kerning);
extern BBox T1_GetStringBBox( int FontID, char *string,
			      int len,  long spaceoff, int kerning);
extern METRICSINFO T1_GetMetricsInfo( int FontID, char *string,
				      int len,  long spaceoff, int kerning);
extern BBox T1_GetFontBBox( int FontID);
extern char **T1_GetAllCharNames( int FontID);

/* from t1load.c */
extern int T1_LoadFont( int FontID);
extern int openFontMetricsFile( int FontID);
extern void *CreateNewFontSize( int FontID, float size, int aa);
extern void *GetLastFontSize( int FontID);
extern void *QueryFontSize( int FontID, float size, int aa);

/* from t1set.c */
extern GLYPH *T1_SetChar( int FontID, char charcode, 
			  float size, T1_TMATRIX *transform);
extern GLYPH *T1_SetString( int FontID, char *string, int len,
			    long spaceoff, int modflag,
			    float size, T1_TMATRIX *transform);
extern GLYPH *T1_CopyGlyph(GLYPH *glyph);
extern void T1_DumpGlyph( GLYPH *glyph);
extern GLYPH *T1_ConcatGlyphs( GLYPH *glyph1, GLYPH *glyph2,
			       int x_off, int y_off);
extern void T1_DumpPixmap( GLYPH *glyph);

/* from t1trans.c */
extern int T1_ExtendFont( int FontID, double extend);
extern int T1_SlantFont( int FontID, double slant);
extern int T1_SetLinePosition( int FontID, int linetype, float value);
extern int T1_SetLineThickness( int FontID, int linetype, float value);
extern float T1_GetLinePosition( int FontID, int linetype);
extern float T1_GetLineThickness( int FontID, int linetype);
extern T1_TMATRIX *T1_RotateMatrix( T1_TMATRIX *matrix, float angle);
extern T1_TMATRIX *T1_MirrorHMatrix( T1_TMATRIX *matrix);
extern T1_TMATRIX *T1_MirrorVMatrix( T1_TMATRIX *matrix);
extern T1_TMATRIX *T1_ShearHMatrix( T1_TMATRIX *matrix, float shear);
extern T1_TMATRIX *T1_ShearVMatrix( T1_TMATRIX *matrix, float shear);
extern T1_TMATRIX *T1_ExtendHMatrix( T1_TMATRIX *matrix, float extent);
extern T1_TMATRIX *T1_ExtendVMatrix( T1_TMATRIX *matrix, float extent);
extern T1_TMATRIX *T1_TransformMatrix( T1_TMATRIX *matrix,
				       float cxx, float cyx,
				       float cxy, float cyy);


/* from t1aaset.c */
extern GLYPH *T1_AASetChar( int FontID, char charcode, 
			    float size, T1_TMATRIX *transform);
extern GLYPH *T1_AASetString( int FontID, char *string, int len,
			      long spaceoff, int modflag,
			      float size, T1_TMATRIX *transform);
extern int T1_AASetGrayValues(unsigned long white,
			      unsigned long gray75,
			      unsigned long gray50,
			      unsigned long gray25,
			      unsigned long black);
extern int T1_AAHSetGrayValues( unsigned long *grayvals);
extern int T1_AASetBitsPerPixel( int bpp);
extern int T1_AASetLevel( int level);
extern int T1_AAGetLevel( void);

/* from t1afmtool.c */
extern int      T1_WriteAFMFallbackFile( int FontID);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

