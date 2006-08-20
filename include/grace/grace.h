/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2006 Grace Development Team
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

#ifndef __GRACE_H_
#define __GRACE_H_

#include "grace/core.h"
#include "grace/graal.h"

typedef struct _Grace Grace;

/* date formats */
typedef enum {
    FMT_iso,
    FMT_european,
    FMT_us,
    FMT_nohint
} Dates_format;

typedef struct {
    Quark *q;
    GrFILE *grf;
} GProject;

/* grace.c */
int grace_init(void);

Grace *grace_new(const char *home);
void grace_free(Grace *grace);

void grace_set_udata(Grace *grace, void *udata);
void *grace_get_udata(const Grace *grace);

Grace *grace_from_quark(const Quark *q);

Canvas *grace_get_canvas(const Grace *grace);
Graal *grace_get_graal(const Grace *grace);

int gproject_render(const GProject *gp);

Quark *gproject_get_top(const GProject *gp);

GProject *gproject_new(const Grace *grace, int mmodel);
void gproject_free(GProject *gp);
char *gproject_get_docname(const GProject *gp);

Grace *grace_from_gproject(const GProject *gp);

int grace_sync_canvas_devices(const GProject *gp);

/* xml_out.c */
int gproject_save(GProject *gp, GrFILE *grf);
/* xml_in.c */
GProject *gproject_load(Grace *grace, GrFILE *grf, int mmodel);

/* dicts.c */
char *graph_types(Grace *grace, GraphType it);
char *graph_type_descr(Grace *grace, GraphType it);
GraphType graph_get_type_by_name(Grace *grace, const char *name);

char *set_types(Grace *grace, SetType it);
char *set_type_descr(Grace *grace, SetType it);
SetType get_settype_by_name(Grace *grace, const char *name);

char *object_types(Grace *grace, OType it);
char *object_type_descr(Grace *grace, OType it);
OType get_objecttype_by_name(Grace *grace, const char *name);

char *inout_placement_name(Grace *grace, int inout);
int get_inout_placement_by_name(Grace *grace, const char *name);
char *spec_tick_name(Grace *grace, int it);
int get_spec_tick_by_name(Grace *grace, const char *name);
char *region_types(Grace *grace, int it);
int get_regiontype_by_name(Grace *grace, const char *name);

char *axis_position_name(Grace *grace, int it);
int get_axis_position_by_name(Grace *grace, const char *name);

char *arrow_type_name(Grace *grace, int it);
int get_arrow_type_by_name(Grace *grace, const char *name);
char *glocator_type_name(Grace *grace, int it);
int get_glocator_type_by_name(Grace *grace, const char *name);
char *sym_type_name(Grace *grace, int it);
int get_sym_type_by_name(Grace *grace, const char *name);
char *line_type_name(Grace *grace, int it);
int get_line_type_by_name(Grace *grace, const char *name);
char *setfill_type_name(Grace *grace, int it);
int get_setfill_type_by_name(Grace *grace, const char *name);
char *baseline_type_name(Grace *grace, int it);
int get_baseline_type_by_name(Grace *grace, const char *name);
char *framedecor_type_name(Grace *grace, int it);
int get_framedecor_type_by_name(Grace *grace, const char *name);
char *scale_type_name(Grace *grace, ScaleType it);
ScaleType get_scale_type_by_name(Grace *grace, const char *name);
char *arrow_placement_name(Grace *grace, int it);
int get_arrow_placement_by_name(Grace *grace, const char *name);
char *arcclosure_type_name(Grace *grace, int it);
int get_arcclosure_type_by_name(Grace *grace, const char *name);

char *format_type_name(Grace *grace, FormatType it);
FormatType get_format_type_by_name(Grace *grace, const char *name);
char *format_type_descr(Grace *grace, FormatType it);

char *frame_type_name(Grace *grace, FrameType it);
FrameType get_frame_type_by_name(Grace *grace, const char *name);
char *frame_type_descr(Grace *grace, FrameType it);

char *dataset_col_name(Grace *grace, DataColumn it);
DataColumn get_dataset_col_by_name(Grace *grace, const char *name);


/* paths.c */
char *grace_get_userhome(const Grace *grace);
char *grace_path(const Grace *grace, const char *path);


/* dates.c */
int parse_float(const char* s, double *value, const char **after);
int parse_date(const Quark *q, const char* s, Dates_format preferred, int absolute,
               double *jul, Dates_format *recognized);
int parse_date_or_number(const Quark *q, const char* s,
    int absolute, Dates_format hint, double *value);

#endif /* __GRACE_H_ */
