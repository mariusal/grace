/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
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
 * Parsing escape sequences in composite strings
 */
 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "grace/canvas.h"
#include "utils.h"
#include "files.h"

int init_font_db(Canvas *canvas)
{
    int i, nfonts;
    char buf[GR_MAXPATHLEN], abuf[GR_MAXPATHLEN], fbuf[GR_MAXPATHLEN], *bufp;
    FILE *fd;
    Grace *grace = (Grace *) canvas_get_udata(canvas);
    
    /* Set default encoding */
    bufp = grace_path2(grace, "fonts/enc/", T1_DEFAULT_ENCODING_FILE);
    if (canvas_set_encoding(canvas, bufp) != RETURN_SUCCESS) {
        bufp = grace_path2(grace, "fonts/enc/", T1_FALLBACK_ENCODING_FILE);
        if (canvas_set_encoding(canvas, bufp) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }
    
    /* Open & process the font database */
    fd = grace_openr(grace, "fonts/FontDataBase", SOURCE_DISK);
    if (fd == NULL) {
        return RETURN_FAILURE;
    }
    
    /* the first line - number of fonts */
    grace_fgets(buf, GR_MAXPATHLEN - 1, fd); 
    if (sscanf(buf, "%d", &nfonts) != 1 || nfonts <= 0) {
        fclose(fd);
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nfonts; i++) {
        grace_fgets(buf, GR_MAXPATHLEN - 1, fd); 
        if (sscanf(buf, "%s %*s %s", abuf, fbuf) != 2) {
            fclose(fd);
            return RETURN_FAILURE;
        }
        bufp = grace_path2(grace, "fonts/type1/", fbuf);
        if (canvas_add_font(canvas, bufp, abuf) != RETURN_SUCCESS) {
            fclose(fd);
            return RETURN_FAILURE;
        }
    }
    fclose(fd);
    
    return RETURN_SUCCESS;
}

/* TODO: optimize, e.g. via a hashed array */
int fmap_proc(const Canvas *canvas, int font_id)
{
    Grace *grace = (Grace *) canvas_get_udata(canvas);
    Project *pr = project_get_data(grace->project);
    int font = BAD_FONT_ID;
    unsigned int i;
    
    for (i = 0; i < pr->nfonts; i++) {
        Fontdef *f = &pr->fontmap[i];

        if (f->id == font_id) {
            font = canvas_get_font_by_name(canvas, f->fontname);
            if (font == BAD_FONT_ID) {
                font = canvas_get_font_by_name(canvas, f->fallback);
            }

            if (font == BAD_FONT_ID) {
                char buf[64];
                sprintf(buf, "Couldn't map font %d to any existing one", f->id);
                errmsg(buf);

                font = 0;
            }
            
            break;
        }
    }
    
    return font;
}

static const TextMatrix unit_tm = UNIT_TM;

static int get_escape_args(const char *s, char *buf)
{
    int i = 0;
    
    if (*s == '{') {
        s++;
        while (*s != '\0') {
            if (*s == '}') {
                *buf = '\0';
                return i;
            } else {
                *buf = *s;
                buf++; s++; i++;
            }
        }
    }
    
    return -1;
}

static char *expand_macros(const Canvas *canvas, const char *s)
{
    Grace *grace = (Grace *) canvas_get_udata(canvas);
    char *es, *macro, *subst;
    int i, j, k, slen, extra_len = 0;
    
    if (!s) {
        return NULL;
    }
    
    slen = strlen(s);
    macro = xmalloc(slen*SIZEOF_CHAR);
    if (!macro) {
        return NULL;
    }
    
    i = 0;
    while (i < slen) {
        if (s[i] == '\\' && s[i + 1] == '$' &&
            (k = get_escape_args(&(s[i + 2]), macro)) >= 0) {
            if (!strcmp(macro, "timestamp")) {
                subst = project_get_timestamp(grace->project);
            } else
            if (!strcmp(macro, "filename")) {
                subst = project_get_docname(grace->project);
            } else
            if (!strcmp(macro, "filebname")) {
                subst = get_docbname(grace->project);
            } else {
                subst = "";
            }
            /* 4 == strlen("\\${}") */
            extra_len += (strlen(subst) - (4 + k));
            i += (4 + k);
        } else {
            i++;
        }
    }
    
    es = xmalloc((slen + extra_len + 1)*SIZEOF_CHAR);
    if (!es) {
        xfree(macro);
        return NULL;
    }
    
    i = 0; j = 0;
    while (i < slen) {
        if (s[i] == '\\' && s[i + 1] == '$' &&
            (k = get_escape_args(&(s[i + 2]), macro)) >= 0) {
            if (!strcmp(macro, "timestamp")) {
                subst = project_get_timestamp(grace->project);
            } else
            if (!strcmp(macro, "filename")) {
                subst = project_get_docname(grace->project);
            } else
            if (!strcmp(macro, "filebname")) {
                subst = get_docbname(grace->project);
            } else {
                subst = "";
            }
            strcpy(&es[j], subst);
            i += (4 + k); j += strlen(subst);
        } else {
            es[j] = s[i];
            i++; j++;
        }
    }
    es[j] = '\0';
    
    xfree(macro);
    
    return es;
}

int csparse_proc(const Canvas *canvas, const char *s, CompositeString *cstring)
{
    Grace *grace = (Grace *) canvas_get_udata(canvas);
    CStringSegment *cseg;

    char *string, *ss, *buf, *acc_buf;
    int inside_escape = FALSE;
    int i, isub, j;
    int acc_len;
    int slen;
    char ccode;
    int upperset = FALSE;
    double scale;
    TextMatrix tm_buf;
    
    int font = BAD_FONT_ID, new_font = font;
    int color = BAD_COLOR, new_color = color;
    TextMatrix tm = unit_tm, tm_new = tm;
    double hshift = 0.0, new_hshift = hshift;
    double baseline = 0.0, baseline_old;
    double vshift = baseline, new_vshift = vshift;
    int underline = FALSE, overline = FALSE;
    int new_underline = underline, new_overline = overline;
    int kerning = FALSE, new_kerning = kerning;
    int direction = STRING_DIRECTION_LR, new_direction = direction;
    int advancing = TEXT_ADVANCING_LR, new_advancing = advancing;
    int ligatures = FALSE, new_ligatures = ligatures;

    int setmark = MARK_NONE;
    int gotomark = MARK_NONE, new_gotomark = gotomark;
    
    double val;

    string = expand_macros(canvas, s);

    if (string == NULL) {
        return RETURN_FAILURE;
    }
    
    slen = strlen(string);
    
    if (slen == 0) {
        return RETURN_FAILURE;
    }
    
    ss = xmalloc(slen + 1);
    buf = xmalloc(slen + 1);
    acc_buf = xmalloc(slen + 1);
    if (ss == NULL || buf == NULL || acc_buf == NULL) {
        xfree(acc_buf);
        xfree(buf);
        xfree(ss);
        xfree(string);
        return RETURN_FAILURE;
    }
     
    isub = 0;
    ss[isub] = 0;
    
    for (i = 0; i <= slen; i++) {
	ccode = string[i];
	acc_len = 0;
        if (ccode < 32 && ccode > 0) {
	    /* skip control codes */
            continue;
	}
        if (inside_escape) {
            inside_escape = FALSE;
            
            if (isdigit(ccode)) {
	        new_font = ccode - '0';
	        continue;
	    } else if (ccode == 'd') {
                i++;
                switch (string[i]) {
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
                default:
                    /* undo advancing */
                    i--;
		    break;
                }
                continue;
	    } else if (ccode == 'F') {
                i++;
                switch (string[i]) {
                case 'k':
		    new_kerning = TRUE;
		    break;
	        case 'K':
		    new_kerning = FALSE;
		    break;
	        case 'l':
		    new_ligatures = TRUE;
		    break;
	        case 'L':
		    new_ligatures = FALSE;
		    break;
                default:
                    /* undo advancing */
                    i--;
		    break;
                }
                continue;
            } else if (isoneof(ccode, "cCsSNBxuUoO+-qQn")) {
                switch (ccode) {
	        case 's':
                    new_vshift -= tm_size(&tm_new)*SUBSCRIPT_SHIFT;
                    tm_scale(&tm_new, SSCRIPT_SCALE);
		    break;
	        case 'S':
                    new_vshift += tm_size(&tm_new)*SUPSCRIPT_SHIFT;
                    tm_scale(&tm_new, SSCRIPT_SCALE);
		    break;
	        case 'N':
                    scale = 1.0/tm_size(&tm_new);
                    tm_scale(&tm_new, scale);
		    new_vshift = baseline;
		    break;
	        case 'B':
		    new_font = BAD_FONT_ID;
		    break;
	        case 'x':
		    new_font = get_font_by_name(grace->project, "Symbol");
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
                    tm_scale(&tm_new, 1.0/ENLARGE_SCALE);
		    break;
	        case '+':
                    tm_scale(&tm_new, ENLARGE_SCALE);
		    break;
	        case 'q':
                    tm_slant(&tm_new, OBLIQUE_FACTOR);
		    break;
	        case 'Q':
                    tm_slant(&tm_new, -OBLIQUE_FACTOR);
		    break;
	        case 'n':
                    new_gotomark = MARK_CR;
		    baseline -= 1.0;
                    new_vshift = baseline;
		    new_hshift = 0.0;
		    break;
                }
                continue;
            } else if (isoneof(ccode, "fhvVzZmM#rltTR") &&
                       (j = get_escape_args(&(string[i + 1]), buf)) >= 0) {
                i += (j + 2);
                switch (ccode) {
	        case 'f':
                    if (j == 0) {
                        new_font = BAD_FONT_ID;
                    } else if (isdigit(buf[0])) {
                        new_font = atoi(buf);
                    } else {
                        new_font = get_font_by_name(grace->project, buf);
                    }
                    break;
	        case 'v':
                    if (j == 0) {
                        new_vshift = baseline;
                    } else {
                        val = atof(buf);
                        new_vshift += tm_size(&tm_new)*val;
                    }
                    break;
	        case 'V':
                    baseline_old = baseline;
                    if (j == 0) {
                        baseline = 0.0;
                    } else {
                        val = atof(buf);
                        baseline += tm_size(&tm_new)*val;
                    }
                    new_vshift = baseline;
                    break;
	        case 'h':
                    val = atof(buf);
                    new_hshift = tm_size(&tm_new)*val;
                    break;
	        case 'z':
                    if (j == 0) {
                        scale = 1.0/tm_size(&tm_new);
                        tm_scale(&tm_new, scale);
                    } else {
                        scale = atof(buf);
                        tm_scale(&tm_new, scale);
                    }
                    break;
	        case 'Z':
                    scale = atof(buf)/tm_size(&tm_new);
                    tm_scale(&tm_new, scale);
                    break;
	        case 'r':
                    tm_rotate(&tm_new, atof(buf));
                    break;
	        case 'l':
                    tm_slant(&tm_new, atof(buf));
                    break;
	        case 't':
                    if (j == 0) {
                        tm_new = unit_tm;
                    } else {
                        if (sscanf(buf, "%lf %lf %lf %lf",
                                        &tm_buf.cxx, &tm_buf.cxy,
                                        &tm_buf.cyx, &tm_buf.cyy) == 4) {
                            tm_product(&tm_new, &tm_buf);
                        }
                    }
                    break;
	        case 'T':
                    if (sscanf(buf, "%lf %lf %lf %lf",
                                    &tm_buf.cxx, &tm_buf.cxy,
                                    &tm_buf.cyx, &tm_buf.cyy) == 4) {
                        tm_new = tm_buf;
                    }
                    break;
	        case 'm':
                    setmark = atoi(buf);
                    break;
	        case 'M':
                    new_gotomark = atoi(buf);
		    new_vshift = baseline;
		    new_hshift = 0.0;
                    break;
	        case 'R':
                    if (j == 0) {
                        new_color = BAD_COLOR;
                    } else if (isdigit(buf[0])) {
                            new_color = atof(buf);
                    } else {
                        new_color = get_color_by_name(canvas, buf);
                    }
                    break;
	        case '#':
                    if (j % 2 == 0) {
                        int k;
                        char hex[3];
                        hex[2] = '\0';
                        for (k = 0; k < j; k += 2) {
                            hex[0] = buf[k];
                            hex[1] = buf[k + 1];
                            acc_buf[acc_len] = strtol(hex, NULL, 16);
	                    acc_len++;
                        }
                    }
                    break;
                }

                if (ccode != '#') {
                    continue;
                }
	    } else {
                /* store the char */
                acc_buf[0] = (ccode + (upperset*0x80)) & 0xff;
                acc_len = 1;
            }
        } else {
            if (ccode == '\\') {
                inside_escape = TRUE;
                continue;
            } else {
                /* store the char */
                acc_buf[0] = (ccode + (upperset*0x80)) & 0xff;
                acc_len = 1;
            }
        }
	
        if ((new_font      != font      ) ||
	    (new_color     != color     ) ||
	    (tm_new.cxx    != tm.cxx    ) ||
	    (tm_new.cxy    != tm.cxy    ) ||
	    (tm_new.cyx    != tm.cyx    ) ||
	    (tm_new.cyy    != tm.cyy    ) ||
	    (new_hshift    != 0.0       ) ||
	    (new_vshift    != vshift    ) ||
	    (new_underline != underline ) ||
	    (new_overline  != overline  ) ||
	    (new_kerning   != kerning   ) ||
	    (new_direction != direction ) ||
	    (new_advancing != advancing ) ||
	    (new_ligatures != ligatures ) ||
	    (setmark       >= 0         ) ||
	    (new_gotomark  >= 0         ) ||
	    (ccode         == 0         )) {
	    
            if (isub != 0 || setmark >= 0) {	/* non-empty substring */
	
	        cseg = cstring_seg_new(cstring);
                cseg->font = font;
	        cseg->color = color;
	        cseg->tm = tm;
	        cseg->hshift = hshift;
	        cseg->vshift = vshift;
	        cseg->underline = underline;
	        cseg->overline = overline;
	        cseg->kerning = kerning;
	        cseg->direction = direction;
	        cseg->advancing = advancing;
	        cseg->ligatures = ligatures;
	        cseg->setmark = setmark;
                setmark = MARK_NONE;
	        cseg->gotomark = gotomark;

	        cseg->s = xmalloc(isub*SIZEOF_CHAR);
	        memcpy(cseg->s, ss, isub);
	        cseg->len = isub;
	        isub = 0;
            }
	    
	    font = new_font;
	    color = new_color;
	    tm = tm_new;
	    hshift = new_hshift;
            if (hshift != 0.0) {
                /* once a substring is manually advanced, all the following
                 * substrings will be advanced as well!
                 */
                new_hshift = 0.0;
            }
	    vshift = new_vshift;
	    underline = new_underline;
	    overline = new_overline;
	    kerning = new_kerning;
	    direction = new_direction;
	    advancing = new_advancing;
	    ligatures = new_ligatures;
            gotomark = new_gotomark;
            if (gotomark >= 0) {
                /* once a substring is manually advanced, all the following
                 * substrings will be advanced as well!
                 */
                new_gotomark = MARK_NONE;
            }
	} 
	memcpy(&ss[isub], acc_buf, acc_len*SIZEOF_CHAR);
	isub += acc_len;
    }
    
    xfree(acc_buf);
    xfree(buf);
    xfree(ss);
    xfree(string);

    return RETURN_SUCCESS;
}
