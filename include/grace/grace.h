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

#ifndef __GRACEA_H_
#define __GRACEA_H_

#include "grace/core.h"
#include "grace/graal.h"

/* Typesetting defines */
#define SSCRIPT_SCALE M_SQRT1_2
#define SUBSCRIPT_SHIFT 0.4
#define SUPSCRIPT_SHIFT 0.6
#define ENLARGE_SCALE sqrt(M_SQRT2)
#define OBLIQUE_FACTOR 0.25

typedef struct _Grace Grace;

struct _Grace {
    QuarkFactory *qfactory;
    
    /* safe mode flag */
    int safe_mode;

    /* location of the Grace home directory */
    char *grace_home;
    
    /* username */
    char *username;

    /* $HOME */
    char *userhome;
    
    Canvas *canvas;

    Graal  *graal;
    
    /* Misc dictionaries */
    Dictionary *graph_type_dict;
    Dictionary *set_type_dict;
    Dictionary *object_type_dict;
    Dictionary *inout_placement_dict;
    Dictionary *spec_ticks_dict;
    Dictionary *region_type_dict;
    Dictionary *axis_position_dict;
    Dictionary *arrow_type_dict;
    Dictionary *glocator_type_dict;
    Dictionary *sym_type_dict;
    Dictionary *line_type_dict;
    Dictionary *setfill_type_dict;
    Dictionary *baseline_type_dict;
    Dictionary *framedecor_type_dict;
    Dictionary *scale_type_dict;
    Dictionary *arrow_placement_dict;
    Dictionary *arcclosure_type_dict;
    Dictionary *format_type_dict;
    Dictionary *frame_type_dict;
    Dictionary *dataset_col_dict;
    
    void *udata;
};

/* date formats */
typedef enum {
    FMT_iso,
    FMT_european,
    FMT_us,
    FMT_nohint
} Dates_format;

/* tokens for the calendar dates parser */
typedef struct {
    int value;
    int digits;
} Int_token;

/* grace.c */
int grace_init(void);

Grace *grace_new(void);
void grace_free(Grace *grace);

void grace_set_udata(Grace *grace, void *udata);
void *grace_get_udata(const Grace *grace);

Grace *grace_from_quark(const Quark *q);

/* xml_out.c */
int grace_save(Quark *project, FILE *fp);
/* xml_in.c */
Quark *grace_load(Grace *grace, FILE *fp);

/* typeset.c */
int csparse_proc(const Canvas *canvas, const char *s, CompositeString *cstring);
int fmap_proc(const Canvas *canvas, int font);


/* dicts.c */
int grace_init_dicts(Grace *grace);
void grace_free_dicts(Grace *grace);

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

/* dates.c */
int parse_float(const char* s, double *value, const char **after);
int parse_date(const Quark *q, const char* s, Dates_format preferred, int absolute,
               double *jul, Dates_format *recognized);
int parse_date_or_number(const Quark *q, const char* s,
    int absolute, Dates_format hint, double *value);

#endif /* __GRACE_H_ */
