/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2000 Grace Development Team
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
 * Build info stuff
 */

#include <config.h>
#include "buildinfo.h"

long bi_version_id(void)
{
    return BI_VERSION_ID;
}

char *bi_version_string(void)
{
    return BI_VERSION;
}

char *bi_system(void)
{
    return BI_SYSTEM;
}

char *bi_date(void)
{
    return BI_DATE;
}

char *bi_gui(void)
{
    return BI_GUI;
}

#ifdef MOTIF_GUI
char *bi_gui_xbae(void)
{
    return BI_GUI_XBAE;
}
#endif

char *bi_t1lib(void)
{
    return BI_T1LIB;
}

char *bi_ccompiler(void)
{
    return BI_CCOMPILER;
}

char *bi_home(void)
{
    return GRACE_HOME;
}

char *bi_print_cmd(void)
{
    return GRACE_PRINT_CMD;
}

char *bi_editor(void)
{
    return GRACE_EDITOR;
}

char *bi_helpviewer(void)
{
    return GRACE_HELPVIEWER;
}
