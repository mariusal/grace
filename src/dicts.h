/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2004 Grace Development Team
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
 * Misc stuff using dictionaries
 */

#ifndef __DICTS_H_
#define __DICTS_H_

#include "defines.h"
#include "core_utils.h"

int grace_rt_init_dicts(RunTime *rt);
void grace_rt_free_dicts(RunTime *rt);

char *graph_types(RunTime *rt, GraphType it);
char *graph_type_descr(RunTime *rt, GraphType it);
GraphType graph_get_type_by_name(RunTime *rt, const char *name);

char *set_types(RunTime *rt, SetType it);
char *set_type_descr(RunTime *rt, SetType it);
SetType get_settype_by_name(RunTime *rt, const char *name);

char *inout_placement_name(RunTime *rt, int inout);
int get_inout_placement_by_name(RunTime *rt, const char *name);
char *side_placement_name(RunTime *rt, int inout);
int get_side_placement_by_name(RunTime *rt, const char *name);
char *spec_tick_name(RunTime *rt, int it);
int get_spec_tick_by_name(RunTime *rt, const char *name);
char *region_types(RunTime *rt, int it);
int get_regiontype_by_name(RunTime *rt, const char *name);

#endif /* __DICTS_H_ */
