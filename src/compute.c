/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 Grace Development Team
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

#include "defines.h"
#include "utils.h"
#include "parser.h"

/*
 * evaluate the expression in sscanstr and place the result in selset
 */
int formula(int gno, int selset, char *sscanstr)
{
    char stmp[64];
    int errpos;

    if (set_parser_setno(gno, selset) != GRACE_EXIT_SUCCESS) {
	sprintf(stmp, "Length of set %d = 0", selset);
	errmsg(stmp);
	return 0;
    }
    errpos = scanner(sscanstr);

    if (!errpos) {
    	set_dirtystate();
    }
    return (errpos);
}
