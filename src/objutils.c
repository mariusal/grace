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

/*
 *
 * operations on objects (strings, lines, and boxes)
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

#include "graphs.h"
#include "utils.h"
#include "protos.h"

int number_of_boxes(void) {
    return maxboxes;
}

int number_of_ellipses(void) {
    return maxellipses;
}

int number_of_lines(void) {
    return maxlines;
}

int number_of_strings(void) {
    return maxstr;
}

/*
 * find the nearest object to (x,y)
 */
void find_item(int gno, double xin, double yin, int *type, int *numb)
{
    int i;
    double x, y;
    double tmp, xtmp1, ytmp1, xtmp2, ytmp2, m = MAXNUM;
    boxtype box;
    ellipsetype ellipse;
    linetype line;
    plotstr str;
    
    world2view(xin, yin, &x, &y);
    *type = -1;
    for (i = 0; i < maxboxes; i++) {
	get_graph_box(i, &box);
	if (isactive_box(i)) {
	    if (box.loctype == COORD_VIEW) {
		xtmp1 = box.x1;
		ytmp1 = box.y1;
		xtmp2 = box.x2;
		ytmp2 = box.y2;
	    } else {
		if (gno == box.gno || box.gno < 0 ) {
		    world2view(box.x1, box.y1, &xtmp1, &ytmp1);
		    world2view(box.x2, box.y2, &xtmp2, &ytmp2);
		} else {
		    continue;
		}
	    }
	    tmp = hypot((x - xtmp1), (y - ytmp1));
	    if (m > tmp) {
		*type = OBJECT_BOX;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp1), (y - ytmp2));
	    if (m > tmp) {
		*type = OBJECT_BOX;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp2), (y - ytmp1));
	    if (m > tmp) {
		*type = OBJECT_BOX;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp2), (y - ytmp2));
	    if (m > tmp) {
		*type = OBJECT_BOX;
		*numb = i;
		m = tmp;
	    }
	}
    }
    for (i = 0; i < maxboxes; i++) {
	get_graph_ellipse(i, &ellipse);
	if (isactive_ellipse(i)) {
	    if (ellipse.loctype == COORD_VIEW) {
		xtmp1 = ellipse.x1;
		ytmp1 = ellipse.y1;
		xtmp2 = ellipse.x2;
		ytmp2 = ellipse.y2;
	    } else {
		if (gno == ellipse.gno || ellipse.gno < 0 ) {
		    world2view(ellipse.x1, ellipse.y1, &xtmp1, &ytmp1);
		    world2view(ellipse.x2, ellipse.y2, &xtmp2, &ytmp2);
		} else {
		    continue;
		}
	    }
	    tmp = hypot((x - xtmp1), (y - ytmp1));
	    if (m > tmp) {
		*type = OBJECT_ELLIPSE;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp1), (y - ytmp2));
	    if (m > tmp) {
		*type = OBJECT_ELLIPSE;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp2), (y - ytmp1));
	    if (m > tmp) {
		*type = OBJECT_ELLIPSE;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp2), (y - ytmp2));
	    if (m > tmp) {
		*type = OBJECT_ELLIPSE;
		*numb = i;
		m = tmp;
	    }
	}
    }
    for (i = 0; i < maxlines; i++) {
	get_graph_line(i, &line);
	if (isactive_line(i)) {
	    if (line.loctype == COORD_VIEW) {
		xtmp1 = line.x1;
		ytmp1 = line.y1;
		xtmp2 = line.x2;
		ytmp2 = line.y2;
	    } else {
		if (gno == line.gno || line.gno<0) {
		    world2view(line.x1, line.y1, &xtmp1, &ytmp1);
		    world2view(line.x2, line.y2, &xtmp2, &ytmp2);
		} else {
		    continue;
		}
	    }
	    tmp = hypot((x - xtmp1), (y - ytmp1));
	    if (m > tmp) {
		*type = OBJECT_LINE;
		*numb = i;
		m = tmp;
	    }
	    tmp = hypot((x - xtmp2), (y - ytmp2));
	    if (m > tmp) {
		*type = OBJECT_LINE;
		*numb = i;
		m = tmp;
	    }
	}
    }
    for (i = 0; i < maxstr; i++) {
	get_graph_string(i, &str);
	if (isactive_string(i)) {
	    if (str.loctype == COORD_VIEW) {
		xtmp1 = str.x;
		ytmp1 = str.y;
	    } else {
		if (gno == str.gno || str.gno<0 ) {
		    world2view(str.x, str.y, &xtmp1, &ytmp1);
		} else {
		    continue;
		}
	    }
	    tmp = hypot((x - xtmp1), (y - ytmp1));
	    if (m > tmp) {
		*type = OBJECT_STRING;
		*numb = i;
		m = tmp;
	    }
	}
    }
    
    /* Ensure that the pointer is reasonably close to the object to
     * eliminate the "MUST" kill behaviour
     */
#ifndef MAXPICKDIST
#define	MAXPICKDIST 0.1
#endif
	if( m > MAXPICKDIST )
		*type = -1;
}

int isactive_line(int lineno)
{
    if (0 <= lineno && lineno < maxlines)
	return (lines[lineno].active == TRUE);
    return (0);
}

int isactive_box(int boxno)
{
    if (0 <= boxno && boxno < maxboxes)
	return (boxes[boxno].active == TRUE);
    return (0);
}

int isactive_string(int strno)
{
    if (0 <= strno && strno < maxstr)
	return (pstr[strno].s[0]);
    return (0);
}

int isactive_ellipse(int ellipno)
{
    if (0 <= ellipno && ellipno < maxellipses)
	return (ellip[ellipno].active == TRUE);
    return (0);
}

int next_line(void)
{
    int i;

    for (i = 0; i < maxlines; i++) {
	if (!isactive_line(i)) {
	    lines[i].active = TRUE;
	    set_dirtystate();
	    return (i);
	}
    }
    errmsg("Error - no lines available");
    return (-1);
}

int next_box(void)
{
    int i;

    for (i = 0; i < maxboxes; i++) {
	if (!isactive_box(i)) {
	    boxes[i].active = TRUE;
	    set_dirtystate();
	    return (i);
	}
    }
    errmsg("Error - no boxes available");
    return (-1);
}

int next_string(void)
{
    int i;

    for (i = 0; i < maxstr; i++) {
	if (!isactive_string(i)) {
	    set_dirtystate();
	    return (i);
	}
    }
    errmsg("Error - no strings available");
    return (-1);
}

int next_ellipse(void)
{
    int i;

    for (i = 0; i < maxellipses; i++) {
	if (!isactive_ellipse(i)) {
	    ellip[i].active = TRUE;
	    set_dirtystate();
	    return (i);
	}
    }
    errmsg("Error - no ellipes available");
    return (-1);
}

void copy_object(int type, int from, int to)
{
    char *tmpbuf;
    switch (type) {
	case OBJECT_BOX:
	boxes[to] = boxes[from];
	break;
 	case OBJECT_ELLIPSE:
	ellip[to] = ellip[from];
	break;
    case OBJECT_LINE:
	lines[to] = lines[from];
	break;
    case OBJECT_STRING:
	kill_string(to);
	free(pstr[to].s);
	tmpbuf = (char *) malloc((strlen(pstr[from].s) + 1) * sizeof(char));
	pstr[to] = pstr[from];
	pstr[to].s = tmpbuf;
	strcpy(pstr[to].s, pstr[from].s);
	break;
    }
    set_dirtystate();
}

plotstr copy_plotstr(plotstr p)
{
    static plotstr pto;
    pto = p;
    if (p.s != NULL) {
	pto.s = (char *) malloc((strlen(p.s) + 1) * sizeof(char));
	if (pto.s != NULL) {
	    strcpy(pto.s, p.s);
	}
    } else {
	pto.s = NULL;
    }
    return pto;
}

void kill_plotstr(plotstr p)
{
    if (p.s != NULL) {
	free(p.s);
    }
}

void kill_box(int boxno)
{
    boxes[boxno].active = FALSE;
    set_dirtystate();
}

void kill_ellipse(int ellipseno)
{
    ellip[ellipseno].active = FALSE;
    set_dirtystate();
}

void kill_line(int lineno)
{
    lines[lineno].active = FALSE;
    set_dirtystate();
}

void kill_string(int stringno)
{
    if (pstr[stringno].s != NULL) {
	free(pstr[stringno].s);
    }
    pstr[stringno].s = (char *) malloc(sizeof(char));
    pstr[stringno].s[0] = 0;
    pstr[stringno].active = FALSE;
    set_dirtystate();
}

void set_plotstr_string(plotstr * pstr, char *buf)
{
    int n;
    
    if (pstr->s != buf) {
        if (pstr->s != NULL) {
          free(pstr->s);
        }
        pstr->s = NULL;
    }
    
    if (buf != NULL) {
	n = strlen(buf);
	pstr->s = (char *) malloc(sizeof(char) * (n + 1));
	strcpy(pstr->s, buf);
	pstr->s[n] = 0;
    } else {
	pstr->s = (char *) malloc(sizeof(char));
	pstr->s[0] = 0;
    }
}

int define_string(char *s, double wx, double wy)
{
    int i;

    i = next_string();
    if (i >= 0) {
	if (s != NULL) {
	    free(pstr[i].s);
	}
	if (s != NULL) {
	    pstr[i].s = (char *) malloc(sizeof(char) * (strlen(s) + 1));
	    strcpy(pstr[i].s, s);
	} else {
	    pstr[i].s = (char *) malloc(sizeof(char));
	    pstr[i].s[0] = 0;
	}
	pstr[i].font = string_font;
	pstr[i].color = string_color;
	pstr[i].rot = string_rot;
	pstr[i].charsize = string_size;
	pstr[i].loctype = string_loctype;
	pstr[i].just = string_just;
	pstr[i].active = TRUE;
	if (string_loctype == COORD_VIEW) {
	    world2view(wx, wy, &pstr[i].x, &pstr[i].y);
	    pstr[i].gno = -1;
	} else {
	    pstr[i].x = wx;
	    pstr[i].y = wy;
	    pstr[i].gno = get_cg();
	}
	set_dirtystate();
	return i;
    }
    return -1;
}

void do_clear_lines(void)
{
    int i;

    for (i = 0; i < maxlines; i++) {
	kill_line(i);
    }
}

void do_clear_boxes(void)
{
    int i;

    for (i = 0; i < maxboxes; i++) {
	kill_box(i);
    }
}

void do_clear_ellipses(void)
{
    int i;

    for (i = 0; i < maxboxes; i++) {
	kill_ellipse(i);
    }
}

void do_clear_text(void)
{
    int i;

    for (i = 0; i < maxstr; i++) {
	kill_string(i);
    }
}

void realloc_lines(int n)
{
    int i;
    if (n > maxlines) {
	lines = (linetype *) realloc(lines, n * sizeof(linetype));
	for (i = maxlines; i < n; i++) {
	    set_default_line(&lines[i]);
	}
	maxlines = n;
    }
}

void realloc_boxes(int n)
{
    int i;
    if (n > maxboxes) {
	boxes = (boxtype *) realloc(boxes, n * sizeof(boxtype));
	for (i = maxboxes; i < n; i++) {
	    set_default_box(&boxes[i]);
	}
	maxboxes = n;
    }
}

void realloc_ellipses(int n)
{
    int i;
    if (n > maxellipses) {
	ellip = (ellipsetype *) realloc(ellip, n * sizeof(ellipsetype));
	for (i = maxellipses; i < n; i++) {
	    set_default_ellipse(&ellip[i]);
	}
	maxellipses = n;
    }
}

void realloc_strings(int n)
{
    int i;
    if (n > maxstr) {
	pstr = (plotstr *) realloc(pstr, n * sizeof(plotstr));
	for (i = maxstr; i < n; i++) {
	    set_default_string(&pstr[i]);
	}
	maxstr = n;
    }
}


void get_graph_box(int i, boxtype * b)
{
    memcpy(b, &boxes[i], sizeof(boxtype));
}

void get_graph_ellipse(int i, ellipsetype * b)
{
    memcpy(b, &ellip[i], sizeof(ellipsetype));
}

void get_graph_line(int i, linetype * l)
{
    memcpy(l, &lines[i], sizeof(linetype));
}

void get_graph_string(int i, plotstr * s)
{
    memcpy(s, &pstr[i], sizeof(plotstr));
}

void set_graph_box(int i, boxtype * b)
{
    memcpy(&boxes[i], b, sizeof(boxtype));
}

void set_graph_line(int i, linetype * l)
{
    memcpy(&lines[i], l, sizeof(linetype));
}

void set_graph_ellipse(int i, ellipsetype * e)
{
    memcpy(&ellip[i], e, sizeof(ellipsetype));
}

void set_graph_string(int i, plotstr * s)
{
    memcpy(&pstr[i], s, sizeof(plotstr));
}

