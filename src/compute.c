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
 * perform math between sets
 *
 */

#include <config.h>

#include <stdio.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "protos.h"

void loadset(int gno, int selset, int toval, double startno, double stepno)
{
    int i, lenset;
    double *ltmp;
    double *xtmp, *ytmp;

    if ((lenset = getsetlength(gno, selset)) <= 0) {
	char stmp[60];

	sprintf(stmp, "Length of set %d <= 0", selset);
	errmsg(stmp);
	return;
    }
    xtmp = getx(gno, selset);
    ytmp = gety(gno, selset);
    switch (toval) {
    case 1:
	ltmp = xtmp;
	break;
    case 2:
	ltmp = ytmp;
	break;
    case 3:
	ltmp = ax;
	break;
    case 4:
	ltmp = bx;
	break;
    case 5:
	ltmp = cx;
	break;
    case 6:
	ltmp = dx;
	break;
    default:
	return;
    }
    for (i = 0; i < lenset; i++) {
	*ltmp++ = startno + i * stepno;
    }

    set_dirtystate();
#ifndef NONE_GUI
    update_set_status(gno, selset);
#endif
}

/*
 * evaluate the expression in sscanstr and place the result in selset
 */
int formula(int gno, int selset, char *sscanstr)
{
    char stmp[64];
    int i = 0, errpos, lenset, oldcg;
    double *xtmp, *ytmp;

    if ((lenset = getsetlength(gno, selset)) <= 0) {
	sprintf(stmp, "Length of set %d = 0", selset);
	errmsg(stmp);
	return 0;
    }
    xtmp = getx(gno, selset);
    ytmp = gety(gno, selset);
	
    oldcg = get_cg();         /* kludge to get around not being able to set result graph */ 
    select_graph(gno);
    scanner(sscanstr, xtmp, ytmp, lenset, ax, bx, cx, dx, MAXARR, i, selset, &errpos);
    select_graph(oldcg);

    if (!errpos) {

#ifndef NONE_GUI
    	update_set_status(gno, selset);
#endif
    	set_dirtystate();
    }
    return (errpos);
}
