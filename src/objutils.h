/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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

#ifndef __OBJUTILS_H_
#define __OBJUTILS_H_

typedef struct {
    double x;
    double y;
} APoint;

/* Object types */
typedef enum {
    DO_NONE,
    DO_LINE,
    DO_BOX,
    DO_ARC,
    DO_STRING
} OType;

typedef struct _DObject {
    int active;

    int gno;

    int loctype;
    APoint ap;
    
    VPoint offset;
    double angle;
    
    Pen pen;
    int lines;
    double linew;
    Pen fillpen;

    OType type;     /* object type */
    void *odata;
    
    view bb;        /* BBox (calculated at run-time) */
} DObject;

typedef struct _DOLineData {
    double length;
    int arrow_end;
    Arrow arrow;
} DOLineData;

typedef struct _DOBoxData {
    double width;
    double height;
} DOBoxData;

typedef struct _DOArcData {
    double width;
    double height;
    
    double angle1;
    double angle2;
    
    int fillmode;
} DOArcData;

typedef struct _DOStringData {
    char *s;
    int font;
    double size;
    int just;
} DOStringData;

char *object_types(OType type);

void *object_data_new(OType type);

DObject *object_new(void);
void object_free(DObject *o);

DObject *object_get(int id);
DObject *object_copy(DObject *o);

int get_object_ids(int **ids);

void do_clear_objects(void);

int isactive_object(DObject *o);

int kill_object(int id);
int next_object(OType type);
int duplicate_object(int id);

int get_object_bb(DObject *o, view *bb);
void move_object(int id, VVector shift);
int object_place_at_vp(int id, VPoint vp);

#endif /* __OBJUTILS_H_ */
