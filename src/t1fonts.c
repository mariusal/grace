/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#include <config.h>
#include <cmath.h>

#include <stdlib.h>
#include <ctype.h>

#include "defines.h"
#include "draw.h"

#include "utils.h"
#include "files.h"
#include "device.h"
#include "t1fonts.h"

#include "protos.h"

static char LastEncodingFile[GR_MAXPATHLEN];
static char EncodingFile[GR_MAXPATHLEN];
static float lastExtent;
static float lastSlant;
static int bitmap_pad;

void (*devputpixmap) (VPoint vp, int width, int height, 
     char *databits, int pixmap_bpp, int bitmap_pad, int pixmap_type);
void (*devputtext) (VPoint start, VPoint end, double size, 
                                            CompositeString *cstring);

static int nfonts = 0;
static FontDB *FontDBtable = NULL;


int init_t1(void)
{
    int i;
    char buf[GR_MAXPATHLEN], *bufp;
    FILE *fd;
    static char **Encoding = NULL;
    
    /* Set search paths: */
    bufp = grace_path("fonts/type1");
    if (bufp == NULL) {
        return (GRACE_EXIT_FAILURE);
    }
    T1_SetFileSearchPath(T1_PFAB_PATH, bufp);
    T1_SetFileSearchPath(T1_AFM_PATH, bufp);
    bufp = grace_path("fonts/enc");
    if (bufp == NULL) {
        return (GRACE_EXIT_FAILURE);
    }
    T1_SetFileSearchPath(T1_ENC_PATH, bufp);
    
    /* Set font database: */
    bufp = grace_path("fonts/FontDataBase");
    if (bufp == NULL) {
        return (GRACE_EXIT_FAILURE);
    }
    T1_SetFontDataBase(bufp);

    /* Set log-level: */
    T1_SetLogLevel(T1LOG_DEBUG);
    
#if defined(DEBUG_T1LIB)
#  define T1LOGFILE LOGFILE
#else
#  define T1LOGFILE NO_LOGFILE
#endif
    
    /* Initialize t1-library */
    if (T1_InitLib(T1LOGFILE|IGNORE_CONFIGFILE) == NULL) {
        return (GRACE_EXIT_FAILURE);
    }
    
    nfonts = T1_Get_no_fonts();
    if (nfonts < 1) {
        return (GRACE_EXIT_FAILURE);
    }
    
    fd = grace_openr(bufp, SOURCE_DISK);
    if (fd == NULL) {
        return (GRACE_EXIT_FAILURE);
    }
    
    FontDBtable = (FontDB *) malloc(nfonts*sizeof(FontDB));
    
    /* skip the first line */
    fgets(buf, GR_MAXPATHLEN - 1, fd); 
    for (i = 0; i < nfonts; i++) {
        fgets(buf, GR_MAXPATHLEN - 1, fd); 
        if (sscanf(buf, "%s %s %*s", FontDBtable[i].alias, 
                                     FontDBtable[i].fallback) != 2) {
            fclose(fd);
            return (GRACE_EXIT_FAILURE);
        }
        FontDBtable[i].mapped_id = i;
    }
    fclose(fd);
    
    T1_SetDeviceResolutions(72.0, 72.0);
    

    Encoding = T1_LoadEncoding(T1_DEFAULT_ENCODING_FILE);
    if (Encoding != NULL) {
        strcpy(EncodingFile, T1_DEFAULT_ENCODING_FILE);
    } else {
        Encoding = T1_LoadEncoding(T1_FALLBACK_ENCODING_FILE);
        strcpy(EncodingFile, T1_FALLBACK_ENCODING_FILE);
    }
    if (Encoding != NULL) {
        T1_SetDefaultEncoding(Encoding);
        strcpy(LastEncodingFile, EncodingFile);
    } else {
        return (GRACE_EXIT_FAILURE);
    }
    
    lastExtent = 1.0;
    lastSlant = T1_DEFAULT_SLANT;

    T1_AASetBitsPerPixel(GRACE_BPP);
    
    bitmap_pad = T1_GetBitmapPad();
    
    return (GRACE_EXIT_SUCCESS);
}

void update_t1(void)
{
    int i;
    
    float Slant = T1_DEFAULT_SLANT, Extent;
    
    static char **Encoding = NULL;
    
    if (strcmp(EncodingFile, LastEncodingFile)) {
      	/* Delete all size dependent data */
      	for (i = 0; i < T1_Get_no_fonts(); i++) {
  	    T1_DeleteAllSizes(i);
  	    T1_LoadFont(i);
      	}
      	Encoding = T1_LoadEncoding(EncodingFile);
      	for (i = 0; i < T1_Get_no_fonts(); i++) {
  	    T1_ReencodeFont(i, Encoding);
      	}
      	strcpy(LastEncodingFile, EncodingFile);
    }
    if (Slant != lastSlant) {
      	/* Delete all size dependent data */
      	for (i = 0; i < T1_Get_no_fonts(); i++) {
  	    T1_DeleteAllSizes(i);
  	    T1_LoadFont(i);
      	}
      	for (i = 0; i < T1_Get_no_fonts(); i++) {
  	    T1_SlantFont(i, Slant);
      	}
      	lastSlant = Slant;
    }

    Extent = 1.0;
    if (Extent != lastExtent) {
      	/* Delete all size dependent data */
      	for (i = 0; i < T1_Get_no_fonts(); i++) {
  	    T1_DeleteAllSizes(i);
  	    T1_LoadFont(i);
      	}
      	for (i = 0; i < T1_Get_no_fonts(); i++) {
  	    T1_ExtendFont(i, Extent);
      	}
      	lastExtent=Extent;
    }    
}

void map_fonts(int map)
{
    int i;
    
    if (map == FONT_MAP_ACEGR) {
        for (i = 0; i < nfonts; i++) {
            FontDBtable[i].mapped_id = BAD_FONT_ID;
        }
        map_font_by_name("Times-Roman", 0);
        map_font_by_name("Times-Bold", 1);
        map_font_by_name("Times-Italic", 2);
        map_font_by_name("Times-BoldItalic", 3);
        map_font_by_name("Helvetica", 4);
        map_font_by_name("Helvetica-Bold", 5);
        map_font_by_name("Helvetica-Oblique", 6);
        map_font_by_name("Helvetica-BoldOblique", 7);
        map_font_by_name("Symbol", 8);
        map_font_by_name("ZapfDingbats", 9);
    } else {
        for (i = 0; i < nfonts; i++) {
            FontDBtable[i].mapped_id = i;
        }
    }
}

int get_font_mapped_id(int font)
{
    if (font >= nfonts || font < 0) {
        return(BAD_FONT_ID);
    } else {
        return(FontDBtable[font].mapped_id);
    }
}

int get_mapped_font(int mapped_id)
{
    int i;
    
    for (i = 0; i < nfonts; i++) {
        if (FontDBtable[i].mapped_id == mapped_id) {
            return(i);
        }
    }
    
    return(BAD_FONT_ID);
}

int map_font(int font, int mapped_id)
{
    int i;
    
    if (font >= nfonts || font < 0) {
        return GRACE_EXIT_FAILURE;
    }
    
    /* make sure the mapping is unique */
    for (i = 0; i < nfonts; i++) {
        if (FontDBtable[i].mapped_id == mapped_id) {
            FontDBtable[i].mapped_id = BAD_FONT_ID;
        }
    }
    FontDBtable[font].mapped_id = mapped_id;

    return GRACE_EXIT_SUCCESS;
}

int map_font_by_name(char *fname, int mapped_id)
{
    return(map_font(get_font_by_name(fname), mapped_id));
}

int number_of_fonts(void)
{
    return (nfonts);
}

int get_font_by_name(char *fname)
{
    int i;
    
    if (fname == NULL) {
        return(BAD_FONT_ID);
    }
    
    for (i = 0; i < nfonts; i++) {
        if (strcmp(get_fontalias(i), fname) == 0) {
            return(i);
        }
    }

    for (i = 0; i < nfonts; i++) {
        if (strcmp(get_fontfallback(i), fname) == 0) {
            return(i);
        }
    }

    return(BAD_FONT_ID);
}

char *get_fontfilename(int font)
{
    return (T1_GetFontFileName(font));
}

char *get_afmfilename(int font)
{
    char *s, *s1;
    static char buf[64];
    int len;

    s = T1_GetAfmFileName(font);
    if (s == NULL) {
        s = T1_GetFontFileName(font);
        len = strlen(s);
        s1 = s + (len - 1);
        while(s1 && *s1 != '.') {
            len--;
            s1--;
        }
        strncpy(buf, s, len);
        buf[len] = '\0';
        strcat(buf, "afm");
        return buf;
    } else {
        return s;
    }
}

char *get_fontname(int font)
{
    return (T1_GetFontName(font));
}

char *get_fontalias(int font)
{
    return (FontDBtable[font].alias);
}

char *get_fontfallback(int font)
{
    return (FontDBtable[font].fallback);
}

char *get_encodingscheme(int font)
{
    return (T1_GetEncodingScheme(font));
}

static T1_TMATRIX UNITY_MATRIX = {1.0, 0.0, 0.0, 1.0};

GLYPH *GetGlyphString(int FontID, double Size, double Angle, int modflag, 
                                                            char *theString)
{
    int len, i, j, k, l, m, none_found;
    
/*
 *     int Kerning = 0;
 */
    long Space = 0;
    int LigDetect = 0;
    
    GLYPH *glyph;
    T1_TMATRIX matrix, *matrixP;
    
    char *ligtheString = '\0';
    char *succs, *ligs;
    char buf_char;

    static int aacolors[T1_AALEVELS];
    unsigned int fg, bg;
    static unsigned long last_bg = 0, last_fg = 0;

    RGB fg_rgb, bg_rgb, delta_rgb, *prgb;
    CMap_entry cmap;
    
    Device_entry dev;

    if (strcmp(theString, "") == 0) {
        return NULL;
    }

    if (Size <= 0.0) {
        errmsg("t1lib: Size must be positive!");
        return NULL;
    }

    /* Now comes the ligatur handling */
    len = strlen(theString);
    ligtheString = (char *) malloc((len + 1)*sizeof(char));
    if (LigDetect){
      	for (j = 0, m = 0; j < len; j++, m++) { /* Loop through the characters */
  	    if ((k = T1_QueryLigs(FontID, theString[j], &succs, &ligs)) > 0) {
  	      	buf_char = theString[j];
  	      	while (k > 0){
  	    	    none_found = 1;
  	    	    for (l = 0; l < k; l++) { /* Loop through the ligatures */
  	    	      	if (succs[l] == theString[j + 1]) {
  	    	    	    buf_char = ligs[l];
  	    	    	    j++;
  	    	    	    none_found = 0;
  	    	    	    break;
  	    	      	}
  	    	    }
  	    	    if (none_found) {
  	    	        break;
                    }
  	    	    k = T1_QueryLigs(FontID, buf_char, &succs, &ligs);
  	      	}
  	      	ligtheString[m] = buf_char;
  	    } else { /* There are no ligatures */
  	        ligtheString[m] = theString[j];
  	    }
      	}
      	ligtheString[m] = 0;
    } else {
        strcpy(ligtheString, theString);
    }
    
    if (Angle == 0.0){
        matrixP = NULL;
    } else {
        matrix = UNITY_MATRIX;
        matrixP = T1_RotateMatrix(&matrix, (float) Angle);
    }

    dev = get_curdevice_props();
    if (dev.fontaa == TRUE) {
    	fg = getcolor();
    	bg = getbgcolor();

    	aacolors[0] = bg;
    	aacolors[T1_AALEVELS - 1] = fg;

    	if ((fg != last_fg) || (bg != last_bg)) {
    	    /* Get RGB values for fore- and background */
    	    prgb = get_rgb(fg);
    	    if (prgb == NULL) {
    		return NULL;
    	    }
    	    fg_rgb = *prgb;
 
    	    prgb = get_rgb(bg);
    	    if (prgb == NULL) {
    		return NULL;
    	    }
    	    bg_rgb = *prgb;
 
    	    delta_rgb.red   = (fg_rgb.red   - bg_rgb.red)   / (T1_AALEVELS - 1);
    	    delta_rgb.green = (fg_rgb.green - bg_rgb.green) / (T1_AALEVELS - 1);
    	    delta_rgb.blue  = (fg_rgb.blue  - bg_rgb.blue) / (T1_AALEVELS - 1);
 
    	    for (i = 1; i < T1_AALEVELS - 1; i++) {
    		cmap.rgb.red   = bg_rgb.red + i*delta_rgb.red;
    		cmap.rgb.green = bg_rgb.green + i*delta_rgb.green;
    		cmap.rgb.blue  = bg_rgb.blue + i*delta_rgb.blue;
    		cmap.cname = "";
    		cmap.ctype = COLOR_AUX;
    		aacolors[i] = add_color(cmap);
    	    }
 
    	    last_fg = fg;
    	    last_bg = bg;
    	}
 
    	/* Set the colors for Anti-Aliasing */
    	T1_AASetGrayValues(aacolors[0],
    			   aacolors[1],
    			   aacolors[2],
    			   aacolors[3],
    			   aacolors[4]);

    	glyph = T1_AASetString(FontID, ligtheString, 0,
    				   Space, modflag, (float) Size, matrixP);
    } else {
    	glyph = T1_SetString(FontID, ligtheString, 0,
    				   Space, modflag, (float) Size, matrixP);
    }
 
    free(ligtheString);
 
    return glyph;
}

void FreeCompositeString(CompositeString *cs)
{
    int i = 0;
    
    while (cs[i].s != NULL) {
	free (cs[i].s);
	i++;
    }
    free (cs);
}

CompositeString *String2Composite(char *string)
{
    static CompositeString *csbuf = NULL;

    char ss[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int slen;
    int nss;
    int i, isub, j;
    
    int upperset = FALSE;
    int underline = FALSE, overline = FALSE;
    int new_underline = underline, new_overline = overline;
    double hshift = 0.0;
    double new_hshift = hshift; 
    double vshift = 0.0;
    double new_vshift = vshift; 
    double scale = 1.0;
    double new_scale = scale;
    int font = getfont();
    int new_font = font;
    
    int setmark = MARK_NONE;
    int gotomark = MARK_NONE;
    int new_gotomark = gotomark;
    
    int direction = STRING_DIRECTION_LR;
    int advancing = TEXT_ADVANCING_LR;
    int new_direction = direction, new_advancing = advancing;

    
    slen = strlen(string);
    
    if ((slen == 0) || slen > (MAX_STRING_LENGTH - 1)) {
        return NULL;
    }
     
    if (csbuf != NULL) {
        FreeCompositeString(csbuf);
	csbuf = NULL;
    }
    
    nss = 0;
    
    isub = 0;
    ss[isub] = 0;
    
    for (i = 0; i <= slen; i++) {
/*
 * 	if (string[i] < 32) {
 * 	    continue;
 * 	}
 */
	if (string[i] == '\\' && isdigit(string[i + 1])) {
	    new_font = get_mapped_font(string[i + 1] - '0');
	    i++;
	    continue;
	} else if (string[i] == '\\' && string[i + 1] == '\\') {
	    if (!upperset) {	/* special case */
	    	i++;
	    }
	} else if (string[i] == '\\' && string[i + 1] == 'd') {
            switch (string[i + 2]) {
            case 'l':
		new_direction = STRING_DIRECTION_LR;
		break;
	    case 'r':
		new_direction = STRING_DIRECTION_RL;
		break;
	    case 'L':
		new_advancing = TEXT_ADVANCING_LR;
		break;
	    case 'R':
		new_advancing = TEXT_ADVANCING_RL;
		break;
            }
            i += 2;
            continue;
        } else if (string[i] == '\\' && 
                            isoneof(string[i + 1], "cCsSNBxuUoO+-fhvzZmM")) {
	    i++;
	    switch (string[i]) {
	    case 'f':
	    case 'h':
	    case 'v':
	    case 'z':
	    case 'Z':
	    case 'm':
	    case 'M':
		if (string[i + 1] == '{') {
                    j = 0;
                    while (string[i + 2 + j] != '}' &&
                           string[i + 2 + j] != '\0') {
                        buf[j] = string[i + 2 + j];
                        j++;
                    }
                    if (string[i + 2 + j] == '}') {
                        buf[j] = '\0';
                        switch (string[i]) {
	                case 'f':
                            if (j == 0) {
                                new_font = getfont();
                            } else if (isdigit(buf[0])) {
                                new_font = get_mapped_font(atoi(buf));
                            } else {
                                new_font = get_font_by_name(buf);
                            }
                            break;
	                case 'v':
                            if (j == 0) {
                                new_vshift = 0.0;
                            } else {
                                new_vshift += new_scale*atof(buf);
                            }
                            break;
	                case 'h':
                            new_hshift = new_scale*atof(buf);
                            break;
	                case 'z':
                            if (j == 0) {
                                new_scale = 1.0;
                            } else {
                                new_scale *= atof(buf);
                            }
                            break;
	                case 'Z':
                            new_scale = atof(buf);
                            break;
	                case 'm':
                            setmark = atoi(buf);
                            break;
	                case 'M':
                            new_gotomark = atoi(buf);
		            new_vshift = 0.0;
		            new_hshift = 0.0;
                            break;
                        }
                        i += (j + 2);
                    }
                }
		break;
	    case 's':
		new_scale *= SSCRIPT_SCALE;
		new_vshift -= 0.4*scale;
		break;
	    case 'S':
		new_scale *= SSCRIPT_SCALE;
		new_vshift += 0.6*scale;
		break;
	    case 'N':
		new_scale = 1.0;
		new_vshift = 0.0;
		break;
	    case 'B':
		new_font = getfont();
		break;
	    case 'x':
		new_font = get_font_by_name("Symbol");
		break;
	    case 'c':
	        upperset = TRUE;
		break;
	    case 'C':
	        upperset = FALSE;
		break;
	    case 'u':
		new_underline = TRUE;
		break;
	    case 'U':
		new_underline = FALSE;
		break;
	    case 'o':
		new_overline = TRUE;
		break;
	    case 'O':
		new_overline = FALSE;
		break;
	    case '-':
		new_scale /= ENLARGE_SCALE;
		break;
	    case '+':
		new_scale *= ENLARGE_SCALE;
		break;
	    }
	    continue;
	}
	if ((new_font  != font          ) ||
	    (new_scale != scale         ) ||
	    (new_hshift != 0.0          ) ||
	    (new_vshift != vshift       ) ||
	    (new_underline != underline ) ||
	    (new_overline != overline   ) ||
	    (new_direction != direction ) ||
	    (new_advancing != advancing ) ||
	    (setmark >= 0               ) ||
	    (new_gotomark >= 0          ) ||
	    (string[i] == 0             )) {
	    
	    
            if (isub != 0) {	/* non-empty substring */
                ss[isub] = '\0';
	        isub = 0;
	
	        csbuf = xrealloc(csbuf, (nss + 1)*sizeof(CompositeString));
	        csbuf[nss].font = font;
	        csbuf[nss].scale = scale;
	        csbuf[nss].hshift = hshift;
	        csbuf[nss].vshift = vshift;
	        csbuf[nss].underline = underline;
	        csbuf[nss].overline = overline;
	        csbuf[nss].advancing = advancing;
	        csbuf[nss].setmark = setmark;
                setmark = MARK_NONE;
	        csbuf[nss].gotomark = gotomark;
	        if (direction == STRING_DIRECTION_RL) {
                    reverse_string(ss);
                }
	        csbuf[nss].s = copy_string(NULL, ss);
	
                nss++;
            }
	    
	    font = new_font;
	    scale = new_scale;
	    hshift = new_hshift;
            if (hshift != 0.0) {
                /* once a substring is manually advanced, all the following
                 * subtrings will be advanced as well!
                 */
                new_hshift = 0.0;
            }
	    vshift = new_vshift;
	    underline = new_underline;
	    direction = new_direction;
	    advancing = new_advancing;
	    overline = new_overline;
            gotomark = new_gotomark;
            if (gotomark >= 0) {
                /* once a substring is manually advanced, all the following
                 * substrings will be advanced as well!
                 */
                new_gotomark = MARK_NONE;
            }
	} 
	ss[isub] = (string[i] + (upperset*0x80)) & 0xff;
	isub++;
    }
    csbuf = xrealloc(csbuf, (nss + 1)*sizeof(CompositeString));
    csbuf[nss].s = NULL;
    
    return (csbuf);
}

/*
 * Convenience wrapper for T1_ConcatGlyphs()
 */
GLYPH *CatGlyphs(GLYPH *dest_glyph, GLYPH *src_glyph,
    int x_off, int y_off, int advancing)
{
    GLYPH *buf_glyph;
    int mode;
    
    if (src_glyph == NULL) {
        /* even if T1lib fails for some reason, don't miss the offsets! */
        if (dest_glyph != NULL) {
            dest_glyph->metrics.advanceX += x_off;
            dest_glyph->metrics.advanceY += y_off;
        }
        return (dest_glyph);
    }
    
    if (dest_glyph != NULL) {
        if (advancing == TEXT_ADVANCING_RL) {
            mode = T1_RIGHT_TO_LEFT;
        } else {
            mode = T1_DEFAULT;
        }
        buf_glyph = T1_ConcatGlyphs(dest_glyph, src_glyph, x_off, y_off, mode);
        if (buf_glyph != NULL) {
            T1_FreeGlyph(dest_glyph);
            dest_glyph = T1_CopyGlyph(buf_glyph);
        }
    } else {
        dest_glyph = T1_CopyGlyph(src_glyph);
    }
    
    return (dest_glyph);
}

void WriteString(VPoint vp, int rot, int just, char *theString)
{    
    VPoint vptmp;
 
    int hjust, vjust, just_type;
    float hfudge, vfudge;
    
    double page_ipv, page_dpv;
 
    /* Variables for raster parameters */
    double Size, Angle = 0.0;
    int FontID;
    int modflag;
    int text_advancing;

    int iglyph;
    GLYPH *glyph;
    GLYPH *CSglyph = NULL;
 
    CompositeString *cstring;
 
    double scale_factor;
    
    int pheight, pwidth;
    
    double si, co;
    float h_off, v_off, v_off_buf, v_off_first = 0.0, v_off_last = 0.0;
    int x_off = 0, y_off = 0;
    
    int first;
    
    int baseline_start_x, baseline_start_y, baseline_end_x, baseline_end_y;
    int bbox_left_x, bbox_right_x, bbox_lower_y, bbox_upper_y;
    int pinpoint_x, pinpoint_y, justpoint_x, justpoint_y;

    int xshift, yshift;
    
    int setmark, gotomark;
    CSMark cs_marks[MAX_MARKS];
    
    VPoint vp_baseline_start, vp_baseline_end;
    
    Device_entry dev;
 
    if (theString == NULL || strlen(theString) == 0) {
	return;
    }
    
    dev = get_curdevice_props();
    
    /* No patterned texts */
    setpattern(1);
    
    cstring = String2Composite(theString);
    
    /* inches per 1 unit of viewport */
    page_ipv = MIN2(page_width_in, page_height_in);

    /* dots per 1 unit of viewport */
    page_dpv = page_ipv*page_dpi;

    scale_factor = page_dpv * MAGIC_FONT_SCALE * getcharsize();
    if (scale_factor <= 0.0) {
        return;
    }

    Angle = (double) rot;
    si = sin(M_PI/180.0*Angle);
    co = cos(M_PI/180.0*Angle);
    
    /* zero marks */
    for (gotomark = 0; gotomark < MAX_MARKS; gotomark++) {
        cs_marks[gotomark].x = 0;
        cs_marks[gotomark].y = 0;
    }
    
    first = FALSE;
    iglyph = 0;
    while (cstring[iglyph].s != NULL) {
        Size = scale_factor * cstring[iglyph].scale;
  	FontID = cstring[iglyph].font;
        text_advancing = cstring[iglyph].advancing;
        modflag = T1_UNDERLINE * cstring[iglyph].underline |
                  T1_OVERLINE  * cstring[iglyph].overline;
	glyph = GetGlyphString(FontID, Size, Angle, modflag, cstring[iglyph].s);

        gotomark = cstring[iglyph].gotomark;
        if (CSglyph != NULL && gotomark >= 0 && gotomark < MAX_MARKS) {
            cstring[iglyph].hshift += 
                (co*(cs_marks[gotomark].x - CSglyph->metrics.advanceX) +
                 si*(cs_marks[gotomark].y - CSglyph->metrics.advanceY)
                )/scale_factor;
            cstring[iglyph].vshift += 
                (-si*(cs_marks[gotomark].x - CSglyph->metrics.advanceX) +
                  co*(cs_marks[gotomark].y - CSglyph->metrics.advanceY) +
                  v_off_last
                )/scale_factor;
/*
 *             v_off_last = 0.0;
 */
            
            /* not a must; just to avoid confusion in backend devices */
            cstring[iglyph].gotomark = MARK_NONE;
        }

        v_off = scale_factor * cstring[iglyph].vshift;
        if (first == FALSE) {
            v_off_first = v_off;
            first = TRUE;
        }
        h_off = scale_factor * cstring[iglyph].hshift;
        v_off_buf = v_off;
        v_off -= v_off_last;
        v_off_last = v_off_buf;
        x_off = (int) rint(h_off*co - v_off*si);
        y_off = (int) rint(v_off*co + h_off*si);
        CSglyph = CatGlyphs(CSglyph, glyph, x_off, y_off, text_advancing);
        setmark = cstring[iglyph].setmark;
        if (CSglyph != NULL && setmark >= 0 && setmark < MAX_MARKS) {
            cs_marks[setmark].x = CSglyph->metrics.advanceX;
            cs_marks[setmark].y = CSglyph->metrics.advanceY;
        }
	iglyph++;
    }
    if (CSglyph == NULL) {
        return;
    }
    if (CSglyph->bits == NULL) {
        /* a string containing only spaces or unrasterizable chars */
        /*
         *  Ideally, we should call here T1_FreeGlyph(). However,
         *  the current t1lib does NOT check whether the "bits" field
         *  is NULL - so we use a hack
         */
        free(CSglyph);
        return;
    }
    
    pinpoint_x = CSglyph->metrics.leftSideBearing;
    pinpoint_y = CSglyph->metrics.ascent;
    
    baseline_start_x = 0 + (int) rint(v_off_first*si);
    baseline_start_y = 0 - (int) rint(v_off_first*co);
    baseline_end_x = CSglyph->metrics.advanceX + (int) rint(v_off_last*si);
    baseline_end_y = CSglyph->metrics.advanceY - (int) rint(v_off_last*co);

    bbox_left_x =  MIN3(baseline_start_x, CSglyph->metrics.leftSideBearing,  baseline_end_x);
    bbox_right_x = MAX3(baseline_start_x, CSglyph->metrics.rightSideBearing, baseline_end_x);
    bbox_lower_y = MIN3(baseline_start_y, CSglyph->metrics.descent, baseline_end_y);
    bbox_upper_y = MAX3(baseline_start_y, CSglyph->metrics.ascent,  baseline_end_y);
    
    hjust = just & 03;
    switch (hjust) {
    case JUST_LEFT:
        hfudge = 0.0;
        break;
    case JUST_RIGHT:
        hfudge = 1.0;
        break;
    case JUST_CENTER:
        hfudge = 0.5;
        break;
    default:
        errmsg("Wrong justification type of string");
        return;
    }

    vjust = just & 014;
    switch (vjust) {
    case JUST_BOTTOM:
        vfudge = 0.0;
        break;
    case JUST_TOP:
        vfudge = 1.0;
        break;
    case JUST_MIDDLE:
        vfudge = 0.5;
        break;
    default:
        errmsg("Wrong justification type of string");
        return;
    }
 
    just_type = just & 020;
    switch (just_type) {
    case JUST_OBJECT:
        justpoint_x = (int) rint(baseline_start_x + 
                                hfudge*(baseline_end_x - baseline_start_x));
        justpoint_y = (int) rint(baseline_start_y + 
                                hfudge*(baseline_end_y - baseline_start_y));
        break;
    case JUST_BBOX:
        justpoint_x = (int) rint(bbox_left_x + 
                                hfudge*(bbox_right_x - bbox_left_x));
        justpoint_y = (int) rint(bbox_lower_y + 
                                vfudge*(bbox_upper_y - bbox_lower_y));
        break;
    default:
        errmsg("Wrong justification type of string");
        return;
    }
 
    pheight = CSglyph->metrics.ascent - CSglyph->metrics.descent;
    pwidth = CSglyph->metrics.rightSideBearing - CSglyph->metrics.leftSideBearing;

    xshift = pinpoint_x - justpoint_x;
    yshift = pinpoint_y - justpoint_y;
    
    vptmp.x = vp.x + (double) xshift/page_dpv;
    vptmp.y = vp.y + (double) yshift/page_dpv;

    if (get_draw_mode() == TRUE) {
        if (dev.devfonts == FONTSRC_BITMAP) {
            (*devputpixmap) (vptmp, pwidth, pheight, CSglyph->bits, 
                                CSglyph->bpp, bitmap_pad, PIXMAP_TRANSPARENT);
        } else {
            if (devputtext == NULL) {
                errmsg("Device has no fonts built-in");
            } else {
                vp_baseline_start.x = vptmp.x + 
                    (double) (baseline_start_x - bbox_left_x)/page_dpv;
                vp_baseline_start.y = vptmp.y +
                    (double) (baseline_start_y - bbox_upper_y)/page_dpv;
                vp_baseline_end.x   = vptmp.x + 
                    (double) (baseline_end_x - bbox_left_x)/page_dpv;
                vp_baseline_end.y   = vptmp.y +
                    (double) (baseline_end_y - bbox_upper_y)/page_dpv;
                (*devputtext) (vp_baseline_start, vp_baseline_end, 
                                                scale_factor, cstring);
            }
        }
    }

    T1_FreeGlyph(CSglyph);
    
    update_bboxes(vptmp);
    vptmp.x += (double) pwidth/page_dpv;
    vptmp.y -= (double) pheight/page_dpv;
    update_bboxes(vptmp);
}
