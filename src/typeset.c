/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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
 * Parsing escape sequences in composite strings
 */
 
#include "graceapp.h"

int init_font_db(GraceApp *gapp, Canvas *canvas)
{
    int i, nfonts;
    char buf[GR_MAXPATHLEN], abuf[GR_MAXPATHLEN], fbuf[GR_MAXPATHLEN], *bufp;
    FILE *fp;
    
    /* Set default encoding */
    bufp = gapp_path2(gapp, "fonts/enc/", T1_DEFAULT_ENCODING_FILE);
    if (canvas_set_encoding(canvas, bufp) != RETURN_SUCCESS) {
        bufp = gapp_path2(gapp, "fonts/enc/", T1_FALLBACK_ENCODING_FILE);
        if (canvas_set_encoding(canvas, bufp) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }
    
    /* Open & process the font database */
    fp = gapp_openr(gapp, "fonts/FontDataBase", SOURCE_DISK);
    if (fp == NULL) {
        return RETURN_FAILURE;
    }
    
    /* the first line - number of fonts */
    grace_fgets(buf, GR_MAXPATHLEN - 1, fp); 
    if (sscanf(buf, "%d", &nfonts) != 1 || nfonts <= 0) {
        gapp_close(fp);
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nfonts; i++) {
        grace_fgets(buf, GR_MAXPATHLEN - 1, fp); 
        if (sscanf(buf, "%s %*s %s", abuf, fbuf) != 2) {
            gapp_close(fp);
            return RETURN_FAILURE;
        }
        bufp = gapp_path2(gapp, "fonts/type1/", fbuf);
        if (canvas_add_font(canvas, bufp, abuf) != RETURN_SUCCESS) {
            gapp_close(fp);
            return RETURN_FAILURE;
        }
    }
    
    gapp_close(fp);
    
    return RETURN_SUCCESS;
}
