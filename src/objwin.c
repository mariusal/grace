/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001 Grace Development Team
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
 * Dialog for editing drawing object properties
 *
 */

#include <config.h>

#include <stdio.h>

#include "cmath.h"
#include "utils.h"
#include "draw.h"
#include "storage.h"
#include "graphs.h"
#include "objutils.h"

#include "motifinc.h"

#include "protos.h"


SpinStructure *CreateViewCoordInput(Widget parent, char *s)
{
    return CreateSpinChoice(parent, s, 6, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.05);
}

typedef struct {
    Widget top;
    
    SpinStructure *length;
    OptionStructure *arrow_end;
    
    OptionStructure *a_type;
    SpinStructure *a_length;
    SpinStructure *a_dL_ff;
    SpinStructure *a_lL_ff;
} LineUI;

typedef struct {
    Widget top;
    
    SpinStructure *width;
    SpinStructure *height;
} BoxUI;

typedef struct {
    Widget top;
    
    SpinStructure *width;
    SpinStructure *height;
    
    Widget angle1;
    Widget angle2;
    
    OptionStructure *fillmode;
} ArcUI;

typedef struct {
    Widget top;
    
    TextStructure *text;
    OptionStructure *font;
    Widget size;
    OptionStructure *just;
} StringUI;

typedef struct {
    Widget top;
    
    ListStructure *gsel;
    StorageStructure *ss;
    
    TextStructure *x;
    TextStructure *y;
    OptionStructure *loctype;
    
    SpinStructure *offsetx;
    SpinStructure *offsety;
    Widget angle;
    
    SpinStructure *linew;
    OptionStructure *lines;
    Widget linepen;
    Widget fillpen;
    
    LineUI   *line_ui;
    BoxUI    *box_ui;
    ArcUI    *arc_ui;
    StringUI *string_ui;
    
    DObject *dobject;
} ObjectUI;

static void changegraphCB(int n, int *values, void *data)
{
    StorageStructure *ss = (StorageStructure *) data;
    if (n == 1) {
        graph *g;
        g = graph_get(values[0]);
        SetStorageChoiceStorage(ss, g->dobjects); 
    } else {
        SetStorageChoiceStorage(ss, NULL); 
    }
}


static void update_line_ui(LineUI *ui, DOLineData *odata)
{
    SetSpinChoice(ui->length, odata->length);
    SetOptionChoice(ui->arrow_end, odata->arrow_end);

    SetOptionChoice(ui->a_type, odata->arrow.type);
    SetSpinChoice(ui->a_length, odata->arrow.length);
    SetSpinChoice(ui->a_dL_ff, odata->arrow.dL_ff);
    SetSpinChoice(ui->a_lL_ff, odata->arrow.lL_ff);
}

static void set_line_odata(LineUI *ui, DOLineData *odata)
{
    odata->length    = GetSpinChoice(ui->length);
    odata->arrow_end = GetOptionChoice(ui->arrow_end);

    odata->arrow.type   = GetOptionChoice(ui->a_type);
    odata->arrow.length = GetSpinChoice(ui->a_length);
    odata->arrow.dL_ff  = GetSpinChoice(ui->a_dL_ff);
    odata->arrow.lL_ff  = GetSpinChoice(ui->a_lL_ff);
}

static LineUI *create_line_ui(Widget parent)
{
    LineUI *ui;
    Widget fr, rc, rc1;
    
    ui = xmalloc(sizeof(LineUI));
    
    ui->top = CreateVContainer(parent);
    fr = CreateFrame(ui->top, "Line properties");
    rc = CreateVContainer(fr);
    ui->length = CreateSpinChoice(rc, "Length:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    ui->arrow_end = CreatePanelChoice(rc, "Place arrows at:",
				      5,
				      "None",
				      "Start",
				      "End",
				      "Both ends",
				      0,
				      0);

    fr = CreateFrame(ui->top, "Arrows");
    rc = CreateVContainer(fr);
    rc1 = CreateHContainer(rc);
    ui->a_type = CreatePanelChoice(rc1, "Type:",
				   3,
				   "Line",
				   "Filled",
				   0,
				   0);
    ui->a_length = CreateSpinChoice(rc1, "Length:",
        4, SPIN_TYPE_FLOAT, -10.0, 10.0, 0.5);
    rc1 = CreateHContainer(rc);
    ui->a_dL_ff = CreateSpinChoice(rc1, "d/L FF:",
        4, SPIN_TYPE_FLOAT, 0.0, 10.0, 0.1);
    ui->a_lL_ff = CreateSpinChoice(rc1, "l/L FF:",
        4, SPIN_TYPE_FLOAT, -1.0, 1.0, 0.1);
    
    return ui;
}

static void update_box_ui(BoxUI *ui, DOBoxData *odata)
{
    SetSpinChoice(ui->width,  odata->width);
    SetSpinChoice(ui->height, odata->height);
}

static void set_box_odata(BoxUI *ui, DOBoxData *odata)
{
    odata->width  = GetSpinChoice(ui->width);
    odata->height = GetSpinChoice(ui->height);
}

static BoxUI *create_box_ui(Widget parent)
{
    BoxUI *ui;
    Widget rc;
    
    ui = xmalloc(sizeof(BoxUI));
    
    ui->top = CreateFrame(parent, "Box properties");
    rc = CreateVContainer(ui->top);
    ui->width = CreateSpinChoice(rc, "Width: ",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    ui->height = CreateSpinChoice(rc, "Height:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    
    return ui;
}

static void update_arc_ui(ArcUI *ui, DOArcData *odata)
{
    SetSpinChoice(ui->width,  odata->width);
    SetSpinChoice(ui->height, odata->height);
    
    SetAngleChoice(ui->angle1, (int) rint(odata->angle1));
    SetAngleChoice(ui->angle2, (int) rint(odata->angle2));
    
    SetOptionChoice(ui->fillmode, odata->fillmode);
}

static void set_arc_odata(ArcUI *ui, DOArcData *odata)
{
    odata->width  = GetSpinChoice(ui->width);
    odata->height = GetSpinChoice(ui->height);
    
    odata->angle1 = GetAngleChoice(ui->angle1);
    odata->angle2 = GetAngleChoice(ui->angle2);
    
    odata->fillmode = GetOptionChoice(ui->fillmode);
}

static ArcUI *create_arc_ui(Widget parent)
{
    ArcUI *ui;
    Widget rc;
    OptionItem opitems[] = {
        {ARCFILL_CHORD,  "Cord"       },
        {ARCFILL_PIESLICE, "Pie slice"}
    };
    
    ui = xmalloc(sizeof(ArcUI));
    
    ui->top = CreateFrame(parent, "Arc properties");
    rc = CreateVContainer(ui->top);

    ui->width = CreateSpinChoice(rc, "Width: ",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    ui->height = CreateSpinChoice(rc, "Height:",
        8, SPIN_TYPE_FLOAT, 0, 10.0, 0.05);
    
    ui->angle1 = CreateAngleChoice(rc, "Start angle");
    ui->angle2 = CreateAngleChoice(rc, "End angle");
    
    ui->fillmode = CreateOptionChoice(rc, "Fill mode:", 1, 2, opitems);
    
    return ui;
}



static void update_string_ui(StringUI *ui, DOStringData *odata)
{
    SetTextString(ui->text,     odata->s);
    SetOptionChoice(ui->font,   odata->font);
    SetOptionChoice(ui->just,   odata->just);
    SetCharSizeChoice(ui->size, odata->size);
}

static void set_string_odata(StringUI *ui, DOStringData *odata)
{
    odata->s    = copy_string(odata->s, GetTextString(ui->text));
    odata->font = GetOptionChoice(ui->font);
    odata->just = GetOptionChoice(ui->just);
    odata->size = GetCharSizeChoice(ui->size);
}

static StringUI *create_string_ui(Widget parent)
{
    StringUI *ui;
    Widget rc;
    
    ui = xmalloc(sizeof(StringUI));
    
    ui->top = CreateFrame(parent, "String properties");
    rc = CreateVContainer(ui->top);
    
    ui->text = CreateCSText(rc, "Text:");
    ui->font = CreateFontChoice(rc, "Font:");
    ui->just = CreateJustChoice(rc, "Justification:");
    ui->size = CreateCharSizeChoice(rc, "Size");
    
    return ui;
}




static void selectobjectCB(int n, void **values, void *data)
{
    ObjectUI *ui = (ObjectUI *) data;
    
    if (n == 1) {
        DObject *o;
        char *format, buf[32];
        
        o = (DObject *) values[0];
        
        ui->dobject = o;
        
        SetOptionChoice(ui->loctype, o->loctype);
        if (o->loctype == COORD_VIEW) {
            format = "%.4f";
        } else {
            format = "%.8g";
        }
        sprintf(buf, format, o->ap.x);
        SetTextString(ui->x, buf);
        sprintf(buf, format, o->ap.y);
        SetTextString(ui->y, buf);
        
        SetSpinChoice(ui->offsetx, o->offset.x);
        SetSpinChoice(ui->offsety, o->offset.y);
        SetAngleChoice(ui->angle, (int) rint(o->angle));
        
        SetSpinChoice(ui->linew, o->line.width);
        SetOptionChoice(ui->lines, o->line.style);
        SetPenChoice(ui->linepen, &o->line.pen);
        SetPenChoice(ui->fillpen, &o->fillpen);
        
        switch (o->type) {
        case DO_LINE:
            update_line_ui(ui->line_ui, (DOLineData *) o->odata);
            
            ManageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        case DO_BOX:
            update_box_ui(ui->box_ui, (DOBoxData *) o->odata);
            
            UnmanageChild(ui->line_ui->top);
            ManageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        case DO_ARC:
            update_arc_ui(ui->arc_ui, (DOArcData *) o->odata);
            
            UnmanageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            ManageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        case DO_STRING:
            update_string_ui(ui->string_ui, (DOStringData *) o->odata);
            
            UnmanageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            ManageChild(ui->string_ui->top);
            break;
        default:
            UnmanageChild(ui->line_ui->top);
            UnmanageChild(ui->box_ui->top);
            UnmanageChild(ui->arc_ui->top);
            UnmanageChild(ui->string_ui->top);
            break;
        }
    } else 
    if (n == 0) {
        ui->dobject = NULL;
        
        UnmanageChild(ui->line_ui->top);
        UnmanageChild(ui->box_ui->top);
        UnmanageChild(ui->arc_ui->top);
        UnmanageChild(ui->string_ui->top);
    }
}


static int objects_aac(void *data)
{
    ObjectUI *ui = (ObjectUI *) data;
    int i, n;
    void **values;
    DObject *o;
    
    n = GetStorageChoices(ui->ss, &values);
    for (i = 0; i < n; i++) {
        o = (DObject *) values[i];
        
        o->loctype = GetOptionChoice(ui->loctype);
        xv_evalexpr(ui->x->text, &o->ap.x);
        xv_evalexpr(ui->y->text, &o->ap.y);
        
        o->offset.x = GetSpinChoice(ui->offsetx);
        o->offset.y = GetSpinChoice(ui->offsety);
        o->angle = GetAngleChoice(ui->angle);

        o->line.width = GetSpinChoice(ui->linew);
        o->line.style = GetOptionChoice(ui->lines);
        GetPenChoice(ui->linepen, &o->line.pen);
        GetPenChoice(ui->fillpen, &o->fillpen);
    }

    if (n == 1) {
        o = (DObject *) values[0];
        switch (o->type) {
        case DO_LINE:
            set_line_odata(ui->line_ui, (DOLineData *) o->odata);
            break;
        case DO_BOX:
            set_box_odata(ui->box_ui, (DOBoxData *) o->odata);
            break;
        case DO_ARC:
            set_arc_odata(ui->arc_ui, (DOArcData *) o->odata);
            break;
        case DO_STRING:
            set_string_odata(ui->string_ui, (DOStringData *) o->odata);
            break;
        default:
            break;
        }
    }

    xdrawgraph();
    set_dirtystate();
    
    return RETURN_SUCCESS;
}

static char *dobject_labeling(unsigned int step, void *data)
{
    char buf[128];
    DObject *o = (DObject *) data;
    
    sprintf(buf, "(%c) DObject #%d (%s)",
        o->active ? '+':'-', step, object_types(o->type));
    
    return copy_string(NULL, buf);
}

static void loctype_cb(int value, void *data)
{
    ObjectUI *ui = (ObjectUI *) data;
    double x, y;
    VPoint vp;
    WPoint wp;
    char *format, buf[32];
    
    /* FIXME: check that there is a selection at all */
    if (!ui->dobject) {
        return;
    }
    
    if (value == COORD_VIEW) {
        format = "%.4f";
    } else {
        format = "%.8g";
    }
    
    x = ui->dobject->ap.x;
    y = ui->dobject->ap.y;
    
    if (value != ui->dobject->loctype) {
        if (value == COORD_VIEW) {
            wp.x = x;
            wp.y = y;

            vp = Wpoint2Vpoint(wp);
            x = vp.x;
            y = vp.y;
        } else {
            vp.x = x;
            vp.y = y;

            view2world(vp.x, vp.y, &wp.x, &wp.y);
            x = wp.x;
            y = wp.y;
        }
    }
    
    sprintf(buf, format, x);
    SetTextString(ui->x, buf);

    sprintf(buf, format, y);
    SetTextString(ui->y, buf);
}

static ObjectUI *oui = NULL;

void define_objects_popup(void *data)
{
    graph *g;
    
    set_wait_cursor();
    
    if (!oui) {
        Widget menubar, menupane, panel, tabs, main_tab, odata_tab, fr, rc, rc1;
        OptionItem opitems[] = {
            {COORD_VIEW,  "View" },
            {COORD_WORLD, "World"}
        };

        oui = xmalloc(sizeof(ObjectUI));
        
        oui->top = CreateDialogForm(app_shell, "Drawing objects");
        menubar = CreateMenuBar(oui->top);
        ManageChild(menubar);
        AddDialogFormChild(oui->top, menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane,
            "Close", 'C', destroy_dialog_cb, GetParent(oui->top));

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On drawing objects", 's',
            oui->top, "doc/UsersGouide.html#drawing-objects");

        panel = CreateVContainer(oui->top);
        AddDialogFormChild(oui->top, panel);
        oui->gsel = CreateGraphChoice(panel, "Graphs:", LIST_TYPE_SINGLE);
        oui->ss = CreateStorageChoice(panel, "Objects:", LIST_TYPE_MULTIPLE, 6);
        SetStorageChoiceLabeling(oui->ss, dobject_labeling);

        AddStorageChoiceCB(oui->ss, selectobjectCB, (void *) oui);
        AddListChoiceCB(oui->gsel, changegraphCB, (void *) oui->ss);
        
        /* ------------ Tabs -------------- */
        tabs = CreateTab(oui->top);        

        /* ------------ Main tab -------------- */
        main_tab = CreateTabPage(tabs, "General");

        fr = CreateFrame(main_tab, "Anchor point");
        rc = CreateHContainer(fr);
        oui->loctype = CreateOptionChoice(rc, "Type:", 1, 2, opitems);
        AddOptionChoiceCB(oui->loctype, loctype_cb, oui);
        oui->x = CreateTextInput(rc, "X:");
        XtVaSetValues(oui->x->text, XmNcolumns, 10, NULL);
        oui->y = CreateTextInput(rc, "Y:");
        XtVaSetValues(oui->y->text, XmNcolumns, 10, NULL);
        
        fr = CreateFrame(main_tab, "Placement");
        rc = CreateVContainer(fr);
        rc1 = CreateHContainer(rc);
        oui->offsetx = CreateViewCoordInput(rc1, "dX:");
        oui->offsety = CreateViewCoordInput(rc1, "dY:");
        oui->angle = CreateAngleChoice(rc, "Angle");
        
        fr = CreateFrame(main_tab, "Drawing properties");
        rc = CreateVContainer(fr);
        rc1 = CreateHContainer(rc);
        oui->linew = CreateLineWidthChoice(rc1, "Line width:");
        oui->lines = CreateLineStyleChoice(rc1, "Line style:");
        rc1 = CreateHContainer(rc);
        oui->linepen = CreatePenChoice(rc1, "Outline pen:");
        oui->fillpen = CreatePenChoice(rc1, "Fill pen:");

        /* ------------ Object data tab -------------- */
        odata_tab = CreateTabPage(tabs, "Object data");

        oui->line_ui = create_line_ui(odata_tab);
        UnmanageChild(oui->line_ui->top);
        oui->box_ui = create_box_ui(odata_tab);
        UnmanageChild(oui->box_ui->top);
        oui->arc_ui = create_arc_ui(odata_tab);
        UnmanageChild(oui->arc_ui->top);
        oui->string_ui = create_string_ui(odata_tab);
        UnmanageChild(oui->string_ui->top);
        
        SelectTabPage(tabs, main_tab);

        CreateAACDialog(oui->top, tabs, objects_aac, oui);
    }

    g = graph_get(get_cg());
    SetStorageChoiceStorage(oui->ss, g->dobjects);
    
    RaiseWindow(GetParent(oui->top));
    unset_wait_cursor();
}

void object_edit_popup(graph *g, int id)
{
    if (g) {
        DObject *o = object_get(g, id);
        define_objects_popup(NULL);
        SetStorageChoiceStorage(oui->ss, g->dobjects);
        SelectStorageChoices(oui->ss, 1, (void **) &o);
    }
}
