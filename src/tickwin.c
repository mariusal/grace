/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2001 Grace Development Team
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
 * ticks / tick labels / axis labels
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/ScrolledW.h>

#include "globals.h"
#include "protos.h"
#include "utils.h"
#include "graphs.h"
#include "draw.h"
#include "graphutils.h"
#include "motifinc.h"

static int curaxis;

static Widget axes_dialog = NULL;

static Widget axes_tab;

static OptionStructure *editaxis; /* which axis to edit */
static Widget axis_active;      /* active or not */
static Widget axis_zero;        /* "zero" or "plain" */
static OptionStructure *axis_scale; /* axis scale */
static Widget axis_invert;      /* invert axis */
static OptionStructure *axis_applyto;    /* override */
static Widget offx;             /* x offset of axis in viewport coords */
static Widget offy;             /* y offset of axis in viewport coords */
static Widget tonoff;           /* toggle display of axis ticks */
static Widget tlonoff;          /* toggle display of tick labels */
static TextStructure *axislabel;        /* axis label */
static OptionStructure *axislabellayout; /* axis label layout (perp or parallel) */
static OptionStructure *axislabelplace;  /* axis label placement, auto or specified */
static Widget axislabelspec_rc;

static Widget axislabelspec_para;    /* location of axis label if specified (viewport coords) */
static Widget axislabelspec_perp;    /* location of axis label if specified (viewport coords) */
static OptionStructure *axislabelfont;   /* axis label font */
static Widget axislabelcharsize;/* axis label charsize */
static OptionStructure *axislabelcolor;  /* axis label color */
static OptionStructure *axislabelop;     /* tick labels normal|opposite|both sides */
static Widget tmajor;           /* major tick spacing */
static SpinStructure *nminor;   /* # of minor ticks */
static OptionStructure *tickop;          /* ticks normal|opposite|both sides */
static OptionStructure *ticklop;         /* tick labels normal|opposite|both sides */
static OptionStructure *tlform; /* format for labels */
static OptionStructure *tlprec;    /* precision for labels */
static OptionStructure *tlfont;  /* tick label font */
static Widget tlcharsize;       /* tick label charsize */
static OptionStructure *tlcolor; /* tick label color */
static Widget tlappstr;         /* tick label append string */
static Widget tlprestr;         /* tick label prepend string */
static OptionStructure *tlskip;          /* tick marks to skip */
static OptionStructure *tlstarttype;     /* use graph min or starting value */
static Widget tlstart;          /* value to start tick labels */
static OptionStructure *tlstoptype;      /* use graph max or stop value */
static Widget tlstop;           /* value to stop tick labels */
static OptionStructure *tlgaptype; /* tick label placement, auto or specified */
static Widget tlgap_rc;
static Widget tlgap_para;       /* location of tick label if specified (viewport coords) */
static Widget tlgap_perp;       /* location of tick label if specified (viewport coords) */
static Widget tlangle;          /* angle */
static OptionStructure *tlstagger;       /* stagger */
static TextStructure *tlformula; /* transformation if tick labels */
static OptionStructure *autonum;         /* number of autotick divisions */
static Widget tround;           /* place at rounded positions */
static Widget tgrid;            /* major ticks grid */
static OptionStructure *tgridcol;
static SpinStructure *tgridlinew;
static OptionStructure *tgridlines;
static Widget tmgrid;           /* minor ticks grid */
static OptionStructure *tmgridcol;
static SpinStructure *tmgridlinew;
static OptionStructure *tmgridlines;
static Widget tlen;             /* tick length */
static Widget tmlen;
static OptionStructure *tinout;          /* ticks in out or both */
static Widget baronoff;         /* axis bar */
static OptionStructure *barcolor;
static SpinStructure *barlinew;
static OptionStructure *barlines;

static OptionStructure *specticks;      /* special ticks/labels */
static SpinStructure *nspec;
static Widget specloc[MAX_TICKS];
static Widget speclabel[MAX_TICKS];
static Widget axis_world_start;
static Widget axis_world_stop;

static void set_axis_proc(int value, void *data);
static void set_active_proc(int onoff, void *data);
static int axes_aac_cb(void *data);
static void axis_scale_cb(int value, void *data);
static void auto_spec_cb(int value, void *data);

static Widget instantupdate_item;

void create_axes_dialog_cb(void *data)
{
    create_axes_dialog(-1);
}


static void oc_axes_cb(int c, void *data)
{
    axes_aac_cb(data);
}
static void tb_axes_cb(int c, void *data)
{
    axes_aac_cb(data);
}
static void scale_axes_cb(int c, void *data)
{
    axes_aac_cb(data);
}
static void sp_axes_cb(double c, void *data)
{
    axes_aac_cb(data);
}
static void text_axes_cb(char *s, void *data)
{
    axes_aac_cb(data);
}


/*
 * Create the ticks popup
 */
void create_axes_dialog(int axisno)
{

    int cg = get_cg();
    
    set_wait_cursor();
    
    if (axisno >= 0 && axisno < MAXAXES) {
        curaxis = axisno;
    }
    
    if (axes_dialog == NULL) {
        int i;
        char buf[32];
        OptionItem opitems[MAXAXES];
        Widget rc_head, rc, rc2, rc3, fr, sw, axes_main, axes_label,
            axes_ticklabel, axes_tickmark, axes_special, menubar, menupane;

        axes_dialog = CreateDialogForm(app_shell, "Axes");

        menubar = CreateMenuBar(axes_dialog);
        AddDialogFormChild(axes_dialog, menubar);
        ManageChild(menubar);

        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuCloseButton(menupane, axes_dialog);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        instantupdate_item = CreateMenuToggle(menupane, "Instantaneous update",
                            'u', NULL, NULL);
        SetToggleButtonState(instantupdate_item, grace->gui->instant_update);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On axis properties", 'a',
            axes_dialog, "doc/UsersGuide.html#axis-properties");

        rc_head = CreateVContainer(axes_dialog);
        AddDialogFormChild(axes_dialog, rc_head);
        
        rc = CreateHContainer(rc_head);
        opitems[0].value = X_AXIS;
        opitems[0].label = "X axis";
        opitems[1].value = Y_AXIS;
        opitems[1].label = "Y axis";
        opitems[2].value = ZX_AXIS;
        opitems[2].label = "Alt X axis";
        opitems[3].value = ZY_AXIS;
        opitems[3].label = "Alt Y axis";
        editaxis = CreateOptionChoice(rc, "Edit:", 0, MAXAXES, opitems);
        AddOptionChoiceCB(editaxis, set_axis_proc, NULL);
        axis_active = CreateToggleButton(rc, "Active");
        AddToggleButtonCB(axis_active, set_active_proc, NULL);
        AddToggleButtonCB(axis_active, tb_axes_cb, axis_active);
        
        rc = CreateHContainer(rc_head);
        axis_world_start = CreateTextItem2(rc, 10, "Start:");
        AddTextItemCB(axis_world_start, text_axes_cb, axis_world_start);
	axis_world_stop = CreateTextItem2(rc, 10, "Stop:");
        AddTextItemCB(axis_world_stop, text_axes_cb, axis_world_stop);

        rc = CreateHContainer(rc_head);

        opitems[0].value = SCALE_NORMAL;
        opitems[0].label = "Linear";
        opitems[1].value = SCALE_LOG;
        opitems[1].label = "Logarithmic";
        opitems[2].value = SCALE_REC;
        opitems[2].label = "Reciprocal";
	opitems[3].value = SCALE_LOGIT;
	opitems[3].label = "Logit";
        axis_scale = CreateOptionChoice(rc, "Scale:", 0, 4, opitems);
        AddOptionChoiceCB(axis_scale, axis_scale_cb, NULL);
        AddOptionChoiceCB(axis_scale, oc_axes_cb, axis_scale);

	axis_invert = CreateToggleButton(rc, "Invert axis");
        AddToggleButtonCB(axis_invert, tb_axes_cb, axis_invert);
        

        /* ------------ Tabs --------------*/

        
        axes_tab = CreateTab(axes_dialog); 
        AddDialogFormChild(axes_dialog, axes_tab);       

        axes_main = CreateTabPage(axes_tab, "Main");

        fr = CreateFrame(axes_main, "Axis label");
        
        axislabel = CreateCSText(fr, "Label string:");
        AddTextInputCB(axislabel, text_axes_cb, axislabel);
        
        fr = CreateFrame(axes_main, "Tick properties");
        rc = CreateVContainer(fr);

        rc2 = CreateHContainer(rc);
        tmajor = CreateTextItem2(rc2, 8, "Major spacing:");
        AddTextItemCB(tmajor, text_axes_cb, tmajor);
        nminor = CreateSpinChoice(rc2, "Minor ticks:",
            2, SPIN_TYPE_INT, 0.0, (double) MAX_TICKS - 1, 1.0);
        AddSpinButtonCB(nminor, sp_axes_cb, nminor);

        rc2 = CreateHContainer(rc);
        tlform = CreateFormatChoice(rc2, "Format:");
        AddOptionChoiceCB(tlform , oc_axes_cb, tlform);
        tlprec = CreatePrecisionChoice(rc2, "Precision:");
        AddOptionChoiceCB(tlprec, oc_axes_cb, tlprec);

        fr = CreateFrame(axes_main, "Display options");
        rc = CreateHContainer(fr);
        
        rc2 = CreateVContainer(rc);
        tlonoff = CreateToggleButton(rc2, "Display tick labels");
        AddToggleButtonCB(tlonoff, tb_axes_cb, tlonoff);
        tonoff = CreateToggleButton(rc2, "Display tick marks");
        AddToggleButtonCB(tonoff, tb_axes_cb, tonoff);
        
        rc2 = CreateVContainer(rc);
        baronoff = CreateToggleButton(rc2, "Display axis bar");
        AddToggleButtonCB(baronoff, tb_axes_cb, baronoff);

        fr = CreateFrame(axes_main, "Axis placement");
        rc = CreateHContainer(fr);
	axis_zero = CreateToggleButton(rc, "Zero axis");
        AddToggleButtonCB(axis_zero, tb_axes_cb, axis_zero);
        offx = CreateTextItem2(rc, 5, "Offsets - Left/bottom:");
        AddTextItemCB(offx, text_axes_cb, offx);
        offy = CreateTextItem2(rc, 5, "Right/top:");
        AddTextItemCB(offy, text_axes_cb, offy);

        fr = CreateFrame(axes_main, "Tick label properties");
        rc = CreateHContainer(fr);

        tlfont = CreateFontChoice(rc, "Font:");
        AddOptionChoiceCB(tlfont, oc_axes_cb, tlfont);
        tlcolor = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(tlcolor, oc_axes_cb, tlcolor);


        axes_label = CreateTabPage(axes_tab, "Axis label & bar");

        fr = CreateFrame(axes_label, "Label properties");
        rc = CreateVContainer(fr);

        rc2 = CreateHContainer(rc);
        axislabelfont = CreateFontChoice(rc2, "Font:");
        AddOptionChoiceCB(axislabelfont, oc_axes_cb, axislabelfont);
        axislabelcolor = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(axislabelcolor, oc_axes_cb, axislabelcolor);
        
        rc2 = CreateHContainer(rc);
        axislabelcharsize = CreateCharSizeChoice(rc2, "Char size");
	SetScaleWidth(axislabelcharsize, 180);
        AddScaleCB(axislabelcharsize, scale_axes_cb, axislabelcharsize);
        
        axislabellayout = CreatePanelChoice(rc2, "Layout:",
                                            3,
                                            "Parallel to axis",
                                            "Perpendicular to axis",
                                            NULL,
                                            NULL);
        AddOptionChoiceCB(axislabellayout, oc_axes_cb, axislabellayout);

        rc2 = CreateHContainer(rc);
        axislabelop = CreatePanelChoice(rc2, "Side:",
                                             4,
                                             "Normal",
                                             "Opposite",
                                             "Both",
                                             0, 0);
        AddOptionChoiceCB(axislabelop, oc_axes_cb, axislabelop);
        opitems[0].value = TYPE_AUTO;
        opitems[0].label = "Auto";
        opitems[1].value = TYPE_SPEC;
        opitems[1].label = "Specified";
        axislabelplace = CreateOptionChoice(rc2, "Location:", 0, 2, opitems);
        axislabelspec_rc = CreateHContainer(rc);
        axislabelspec_para = CreateTextItem2(axislabelspec_rc, 5, "Parallel offset:");
        AddTextItemCB(axislabelspec_para, text_axes_cb, axislabelspec_para);
        axislabelspec_perp = CreateTextItem2(axislabelspec_rc, 5, "Perpendicular offset:");
        AddTextItemCB(axislabelspec_perp, text_axes_cb, axislabelspec_perp);
        AddOptionChoiceCB(axislabelplace, auto_spec_cb, axislabelspec_rc);
        AddOptionChoiceCB(axislabelplace, oc_axes_cb, axislabelplace);

        fr = CreateFrame(axes_label, "Bar properties");
        rc = CreateVContainer(fr);

        rc2 = CreateHContainer(rc);
        barcolor = CreateColorChoice(rc2, "Color:");
        AddOptionChoiceCB(barcolor, oc_axes_cb, barcolor);
        barlinew = CreateLineWidthChoice(rc2, "Width:");
        AddSpinButtonCB(barlinew, sp_axes_cb, barlinew);
        
        barlines = CreateLineStyleChoice(rc, "Line style:");
        AddOptionChoiceCB(barlines, oc_axes_cb, barlines);


        axes_ticklabel = CreateTabPage(axes_tab, "Tick labels");

        fr = CreateFrame(axes_ticklabel, "Labels");
        rc2 = CreateHContainer(fr);
        tlcharsize = CreateCharSizeChoice(rc2, "Char size");
	SetScaleWidth(tlcharsize, 200);
        AddScaleCB(tlcharsize, scale_axes_cb, tlcharsize);
        tlangle = CreateAngleChoice(rc2, "Angle");
	SetScaleWidth(tlangle, 180);
        AddScaleCB(tlangle, scale_axes_cb, tlangle);

        fr = CreateFrame(axes_ticklabel, "Placement");
        rc = CreateHContainer(fr);

        rc2 = CreateVContainer(rc);
        ticklop = CreatePanelChoice(rc2, "Side:",
                                    4,
                                    "Normal",
                                    "Opposite",
                                    "Both",
                                    0, 0);
        AddOptionChoiceCB(ticklop, oc_axes_cb, ticklop);
        tlstagger = CreatePanelChoice(rc2, "Stagger:",
                                      11,
                        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
                                      0);
        AddOptionChoiceCB(tlstagger, oc_axes_cb, tlstagger);
        
        
        rc2 = CreateVContainer(rc);
        rc3 = CreateHContainer(rc2);
        tlstarttype = CreatePanelChoice(rc3, "Start at:",
                                        3,
                                        "Axis min", "Specified:", 0,
                                        0);
        AddOptionChoiceCB(tlstarttype, oc_axes_cb, tlstarttype);
        tlstart = CreateTextItem2(rc3, 8, "");
        AddTextItemCB(tlstart, text_axes_cb, tlstart);

        rc3 = CreateHContainer(rc2);
        tlstoptype = CreatePanelChoice(rc3, "Stop at:",
                                       3,
                                       "Axis max", "Specified:", 0,
                                       0);
        AddOptionChoiceCB(tlstoptype, oc_axes_cb, tlstoptype);
        tlstop = CreateTextItem2(rc3, 8, "");
        AddTextItemCB(tlstop, text_axes_cb, tlstop);

        fr = CreateFrame(axes_ticklabel, "Extra");
        rc = CreateVContainer(fr);

        rc2 = CreateHContainer(rc);
        tlskip = CreatePanelChoice(rc2, "Skip every:",
                                   11,
                        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
                                   0);
        AddOptionChoiceCB(tlskip, oc_axes_cb, tlskip);

        tlformula = CreateTextInput(rc2, "Axis transform:");
        AddTextInputCB(tlformula, text_axes_cb, tlformula);

        rc2 = CreateHContainer(rc);
        tlprestr = CreateTextItem2(rc2, 13, "Prepend:");
        AddTextItemCB(tlprestr, text_axes_cb, tlprestr);
        tlappstr = CreateTextItem2(rc2, 13, "Append:");
        AddTextItemCB(tlappstr, text_axes_cb, tlappstr);

        opitems[0].value = TYPE_AUTO;
        opitems[0].label = "Auto";
        opitems[1].value = TYPE_SPEC;
        opitems[1].label = "Specified";
        tlgaptype = CreateOptionChoice(rc, "Location:", 0, 2, opitems);
        tlgap_rc = CreateHContainer(rc);
        AddOptionChoiceCB(tlgaptype, auto_spec_cb, tlgap_rc);
        AddOptionChoiceCB(tlgaptype, oc_axes_cb, tlgaptype);
        tlgap_para = CreateTextItem2(tlgap_rc, 5, "Parallel offset:");
        AddTextItemCB(tlgap_para, text_axes_cb, tlgap_para);
        tlgap_perp = CreateTextItem2(tlgap_rc, 5, "Perpendicular offset:");
        AddTextItemCB(tlgap_perp, text_axes_cb, tlgap_perp);


        axes_tickmark = CreateTabPage(axes_tab, "Tick marks");

        fr = CreateFrame(axes_tickmark, "Placement");
        rc2 = CreateVContainer(fr);
        rc = CreateHContainer(rc2);
        tinout = CreatePanelChoice(rc, "Pointing:",
                                   4,
                                   "In", "Out", "Both", 0,
                                   0);
        AddOptionChoiceCB(tinout, oc_axes_cb, tinout);
        tickop = CreatePanelChoice(rc, "Draw on:",
                                   4,
                                   "Normal side",
                                   "Opposite side",
                                   "Both sides",
                                   0, 0);
        AddOptionChoiceCB(tickop, oc_axes_cb, tickop);
        rc = CreateHContainer(rc2);
        tround = CreateToggleButton(rc, "Place at rounded positions");
        AddToggleButtonCB(tround, tb_axes_cb, tround);
	autonum = CreatePanelChoice(rc, "Autotick divisions",
                                    12,
		                    "2",
                                    "3",
                                    "4",
                                    "5",
                                    "6",
                                    "7",
                                    "8",
                                    "9",
                                    "10",
                                    "11",
                                    "12",
				    NULL,
				    NULL);
        AddOptionChoiceCB(autonum, oc_axes_cb, autonum);
        
        rc2 = CreateHContainer(axes_tickmark);

/* major tick marks */
        fr = CreateFrame(rc2, "Major ticks");
        rc = CreateVContainer(fr);
        tgrid = CreateToggleButton(rc, "Draw grid lines");
        AddToggleButtonCB(tgrid, tb_axes_cb, tgrid);
        tlen = CreateCharSizeChoice(rc, "Tick length");
        AddScaleCB(tlen, scale_axes_cb, tlen);
        tgridcol = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(tgridcol, oc_axes_cb, tgridcol);
        tgridlinew = CreateLineWidthChoice(rc, "Line width:");
        AddSpinButtonCB(tgridlinew, sp_axes_cb, tgridlinew);
        tgridlines = CreateLineStyleChoice(rc, "Line style:");
        AddOptionChoiceCB(tgridlines, oc_axes_cb, tgridlines);

        fr = CreateFrame(rc2, "Minor ticks");
        rc = CreateVContainer(fr);
        tmgrid = CreateToggleButton(rc, "Draw grid lines");
        AddToggleButtonCB(tmgrid, tb_axes_cb, tmgrid);
        tmlen = CreateCharSizeChoice(rc, "Tick length");
        AddScaleCB(tmlen, scale_axes_cb, tmlen);
        tmgridcol = CreateColorChoice(rc, "Color:");
        AddOptionChoiceCB(tmgridcol, oc_axes_cb, tmgridcol);
        tmgridlinew = CreateLineWidthChoice(rc, "Line width:");
        AddSpinButtonCB(tmgridlinew, sp_axes_cb, tmgridlinew);
        tmgridlines = CreateLineStyleChoice(rc, "Line style:");
        AddOptionChoiceCB(tmgridlines, oc_axes_cb, tmgridlines);


        axes_special = CreateTabPage(axes_tab, "Special");

        opitems[0].value = TICKS_SPEC_NONE;
        opitems[0].label = "None";
        opitems[1].value = TICKS_SPEC_MARKS;
        opitems[1].label = "Tick marks";
        opitems[2].value = TICKS_SPEC_BOTH;
        opitems[2].label = "Tick marks and labels";
        specticks = CreateOptionChoice(axes_special, "Special ticks:", 0, 3, opitems);
        AddOptionChoiceCB(specticks, oc_axes_cb, specticks);

        nspec = CreateSpinChoice(axes_special, "Number of user ticks to use:",
            3, SPIN_TYPE_INT, 0.0, (double) MAX_TICKS, 1.0);
        AddSpinButtonCB(nspec, sp_axes_cb, nspec);
        CreateLabel(axes_special, "Tick location - Label:");

        sw = XtVaCreateManagedWidget("sw",
                                     xmScrolledWindowWidgetClass, axes_special,
				     XmNheight, 240,
                                     XmNscrollingPolicy, XmAUTOMATIC,
                                     NULL);
        rc = CreateVContainer(sw);

        for (i = 0; i < MAX_TICKS; i++) {
            rc3 = CreateHContainer(rc);
            sprintf(buf, "%2d", i);
            specloc[i]   = CreateTextItem4(rc3, 12, buf);
            AddTextItemCB(specloc[i], text_axes_cb, specloc);
            speclabel[i] = CreateTextItem4(rc3, 30, "");
            AddTextItemCB(speclabel[i], text_axes_cb, specloc);
        }


        SelectTabPage(axes_tab, axes_main);

         
        rc = CreateVContainer(axes_dialog);
        axis_applyto = CreatePanelChoice(rc,
                                         "Apply to:",
                                         5,
                                         "Current axis",
                                         "All axes, current graph",
                                         "Current axis, all graphs",
                                         "All axes, all graphs",
                                         NULL,
                                         NULL);

        CreateAACDialog(axes_dialog, rc, axes_aac_cb, NULL);
    }
    update_ticks(cg);
    
    RaiseWindow(GetParent(axes_dialog));
    unset_wait_cursor();
}

/*
 * Callback function for definition of tick marks and axis labels.
 */
static int axes_aac_cb(void *data)
{
    int i, j;
    int applyto;
    int axis_start, axis_stop, gno, gmin, gmax;
    int scale, invert;
    double axeslim;
    world wo;
    int cg = get_cg();
    
    if (!GetToggleButtonState(instantupdate_item) && data != NULL) {
        return RETURN_SUCCESS;
    }
    
    applyto = GetOptionChoice(axis_applyto);

    /* somehow the value of axislabelop gets automagically correctly
       applied to all selected axes without checking for the value of
       applyto directly here (strange...) */
    
    switch (applyto) {
    case 0:                     /* current axis */
        axis_start = curaxis;
        axis_stop  = curaxis;
        gmin = cg;
        gmax = cg;
        break;
    case 1:                     /* all axes, current graph */
        axis_start = 0;
        axis_stop  = MAXAXES - 1;
        gmin = cg;
        gmax = cg;
        break;
    case 2:                     /* current axis, all graphs */
        axis_start = curaxis;
        axis_stop  = curaxis;
        gmin = 0;
        gmax = number_of_graphs();
        break;
    case 3:                     /* all axes, all graphs */
        axis_start = 0;
        axis_stop  = MAXAXES - 1;
        gmin = 0;
        gmax = number_of_graphs();
        break;
    default:
        axis_start = curaxis;
        axis_stop  = curaxis;
        gmin = cg;
        gmax = cg;
        break;        
    }
        
		
    for (gno = gmin; gno <= gmax; gno++) {
        for (j = axis_start; j <= axis_stop; j++) {
            tickmarks *ot;
        
            ot = copy_graph_tickmarks(get_graph_tickmarks(gno, j));

            if (!ot) {
                return RETURN_FAILURE;
            }

            if (data == axis_active || data == NULL) {
                ot->active = GetToggleButtonState(axis_active);
            } else {
                if (!(ot->active)) {
                    free_graph_tickmarks(ot);
                    continue;
                }
            }

            get_graph_world(gno, &wo);
            if (data == axis_world_start || data == NULL) {
                if (xv_evalexpr(axis_world_start, &axeslim) != RETURN_SUCCESS) {
                    errmsg("Axis start/stop values undefined");
                    free_graph_tickmarks(ot);
                    return RETURN_FAILURE;
                } else if (is_xaxis(j)) {
                    wo.xg1 = axeslim;
                } else if (is_yaxis(j)) {
                    wo.yg1 = axeslim; 
                }
            }
            if (data == axis_world_stop || data == NULL) {
                if (xv_evalexpr(axis_world_stop, &axeslim) != RETURN_SUCCESS) {
                    errmsg("Axis start/stop values undefined");
                    free_graph_tickmarks(ot);
                    return RETURN_FAILURE;
                } else if (is_xaxis(j)) {
                    wo.xg2 = axeslim;
                } else if (is_yaxis(j)) {
                    wo.yg2 = axeslim; 
                }
            }
            if (data == axis_world_start || data == axis_world_stop || data == NULL) {
                set_graph_world(gno, wo);
            }
            
            if (data == axis_scale || data == NULL) {
            scale = GetOptionChoice(axis_scale);
            if (is_xaxis(j)) {
                set_graph_xscale(gno, scale);
            } else {
                set_graph_yscale(gno, scale);
            }
            }

            if (data == &axis_invert || data == NULL)  {
            invert = GetToggleButtonState(axis_invert);
            if (is_xaxis(j)) {
                set_graph_xinvert(gno, invert);
            } else {
                set_graph_yinvert(gno, invert);
            }
            }
            
            if (data == axislabel || data == NULL) {
                set_plotstr_string(&ot->label, GetTextString(axislabel));
            }
            if (data == tmajor || data == NULL) {
                if (xv_evalexpr(tmajor, &ot->tmajor) != RETURN_SUCCESS) {
                    errmsg("Specify major tick spacing");
                    free_graph_tickmarks(ot);
                    return RETURN_FAILURE;
                }
            }
            if (data == nminor || data == NULL) {
                ot->nminor = (int) GetSpinChoice(nminor);
            }
            if (data == tlform || data == NULL) {
                ot->tl_format = GetOptionChoice(tlform);
            }
            if (data == tlprec || data == NULL) {
                ot->tl_prec = GetOptionChoice(tlprec);
            }
            if (data == tlonoff || data == NULL) {
                ot->tl_flag = GetToggleButtonState(tlonoff);
            }
            if (data == baronoff || data == NULL) {
                ot->t_drawbar = GetToggleButtonState(baronoff);
            }
            if (data == tonoff || data == NULL) {
                ot->t_flag = GetToggleButtonState(tonoff);
            }
            if (data == axis_zero || data == NULL) {
                ot->zero = GetToggleButtonState(axis_zero);
            }
            if (data == offx || data == NULL) {
                xv_evalexpr(offx, &ot->offsx);
            }
            if (data == &offy || data == NULL) {
                xv_evalexpr(offy, &ot->offsy);
            }
            if (data == tlfont || data == NULL) {
                ot->tl_font = GetOptionChoice(tlfont);
            }
            if (data == tlcolor || data == NULL) {
                ot->tl_color = GetOptionChoice(tlcolor);
            }
            if (data == axislabelfont || data == NULL) {
                ot->label.font = GetOptionChoice(axislabelfont);
            }
            if (data == axislabelcolor || data == NULL) {
                ot->label.color = GetOptionChoice(axislabelcolor);
            }
            if (data == axislabelcharsize || data == NULL) {
                ot->label.charsize = GetCharSizeChoice(axislabelcharsize);
            }
            if (data == axislabellayout || data == NULL) {
                ot->label_layout = 
                GetOptionChoice(axislabellayout)?LAYOUT_PERPENDICULAR:LAYOUT_PARALLEL;
            }
            if (data == axislabelop || data == NULL) {
                ot->label_op = GetOptionChoice(axislabelop);
            }
            if (data == axislabelplace || data == NULL) {
                ot->label_place = GetOptionChoice(axislabelplace);
            }
            if (data == axislabelspec_para || data == NULL) {
                xv_evalexpr(axislabelspec_para, &ot->label.offset.x);
            }
            if (data == axislabelspec_perp || data == NULL) {
                xv_evalexpr(axislabelspec_perp, &ot->label.offset.y);
            }
            if (data == barcolor || data == NULL) {
                ot->t_drawbarcolor = GetOptionChoice(barcolor);
            }
            if (data == barlinew || data == NULL) {
                ot->t_drawbarlinew = GetSpinChoice(barlinew);
            }
            if (data == barlines || data == NULL) {
                ot->t_drawbarlines = GetOptionChoice(barlines);
            }
            if (data == tlcharsize || data == NULL) {
                ot->tl_charsize = GetCharSizeChoice(tlcharsize);
            }
            if (data == tlangle || data == NULL) {
                ot->tl_angle = GetAngleChoice(tlangle);
            }
            if (data == ticklop || data == NULL) {
                ot->tl_op = GetOptionChoice(ticklop);
            }
            if (data == tlstagger || data == NULL) {
                ot->tl_staggered = (int) GetOptionChoice(tlstagger);
            }
            if (data == tlstarttype || data == NULL) {
                ot->tl_starttype =
                    (int) GetOptionChoice(tlstarttype) == 0 ? TYPE_AUTO : TYPE_SPEC;
            }
            if (data == tlstart || data == NULL) {
                if (ot->tl_starttype == TYPE_SPEC) {
                    if (xv_evalexpr(tlstart, &ot->tl_start) != RETURN_SUCCESS) {
                    errmsg("Specify tick label start");
                        free_graph_tickmarks(ot);
                        return RETURN_FAILURE;
                    }
                }
            }
            if (data == tlstoptype || data == NULL) {
                ot->tl_stoptype =
                    (int) GetOptionChoice(tlstoptype) == 0 ? TYPE_AUTO : TYPE_SPEC;
            }
            if (data == tlstop || data == NULL) {
                if (ot->tl_stoptype == TYPE_SPEC) {
                    if (xv_evalexpr(tlstop, &ot->tl_stop) != RETURN_SUCCESS) {
                        errmsg("Specify tick label stop");
                        free_graph_tickmarks(ot);
                        return RETURN_FAILURE;
                    }
                }
            }
            if (data == tlskip || data == NULL) {
                ot->tl_skip = GetOptionChoice(tlskip);
            }
            if (data == tlformula || data == NULL) {
                ot->tl_formula =
                    copy_string(ot->tl_formula, GetTextString(tlformula));
            }
            if (data == tlprestr || data == NULL) {
                strcpy(ot->tl_prestr, xv_getstr(tlprestr));
            }
            if (data == tlappstr || data == NULL) {
                strcpy(ot->tl_appstr, xv_getstr(tlappstr));
            }
            if (data == tlgaptype || data == NULL) {
                ot->tl_gaptype = GetOptionChoice(tlgaptype);
            }
            if (data == tlgap_para || data == NULL) {
                xv_evalexpr(tlgap_para, &ot->tl_gap.x);
            }
            if (data == tlgap_perp || data == NULL) {
                xv_evalexpr(tlgap_perp, &ot->tl_gap.y);
            }
            if (data == tinout || data == NULL) {
                switch ((int) GetOptionChoice(tinout)) {
                case 0:
                    ot->t_inout = TICKS_IN;
                    break;
                case 1:
                    ot->t_inout = TICKS_OUT;
                    break;
                case 2:
                    ot->t_inout = TICKS_BOTH;
                    break;
                }
            }
            if (data == tickop || data == NULL) {
                ot->t_op = GetOptionChoice(tickop);
            }
            if (data == tround || data == NULL) {
                ot->t_round = GetToggleButtonState(tround);
            }
            if (data == autonum || data == NULL) {
                ot->t_autonum = GetOptionChoice(autonum) + 2;
            }
            if (data == tgrid || data == NULL) {
                ot->props.gridflag = GetToggleButtonState(tgrid);
            }
            if (data == tlen || data == NULL) {
                ot->props.size = GetCharSizeChoice(tlen);
            }
            if (data == tgridcol || data == NULL) {
                ot->props.color = GetOptionChoice(tgridcol);
            }
            if (data == tgridlinew || data == NULL) {
                ot->props.linew = GetSpinChoice(tgridlinew);
            }
            if (data == tgridlines || data == NULL) {
                ot->props.lines = GetOptionChoice(tgridlines);
            }
            if (data == tmgrid || data == NULL) {
                ot->mprops.gridflag = GetToggleButtonState(tmgrid);
            }
            if (data == tmlen || data == NULL) {
                ot->mprops.size = GetCharSizeChoice(tmlen);
            }
            if (data == tmgridcol || data == NULL) {
                ot->mprops.color = GetOptionChoice(tmgridcol);
            }
            if (data == tmgridlinew || data == NULL) {
                ot->mprops.linew = GetSpinChoice(tmgridlinew);
            }
            if (data == tmgridlines || data == NULL) {
                ot->mprops.lines = GetOptionChoice(tmgridlines);
            }
            if (data == specticks ||data == nspec || 
                               data == specloc || data == NULL) {
                ot->t_spec = GetOptionChoice(specticks);
                /* only read special info if special ticks used */
                if (ot->t_spec != TICKS_SPEC_NONE) {
                    ot->nticks = (int) GetSpinChoice(nspec);
                    /* ensure that enough tick positions have been specified */
                    for (i = 0; i < ot->nticks; i++) {
                        if (xv_evalexpr(specloc[i], &ot->tloc[i].wtpos) ==
                                                            RETURN_SUCCESS) {
                            char *cp;
                            cp = xv_getstr(speclabel[i]);
                            if (cp[0] == '\0') {
                                ot->tloc[i].type = TICK_TYPE_MINOR;
                            } else {
                                ot->tloc[i].type = TICK_TYPE_MAJOR;
                            }
                            if (ot->t_spec == TICKS_SPEC_BOTH) {
                                ot->tloc[i].label =
                                    copy_string(ot->tloc[i].label, cp);
                            } else {
                                ot->tloc[i].label = 
                                    copy_string(ot->tloc[i].label, NULL);
                            }
                        }
                    } 
                }
            }
            set_graph_tickmarks(gno, j, ot);
            free_graph_tickmarks(ot);
        }
    }
    
    xdrawgraph();

    update_ticks(cg);
    
    return RETURN_SUCCESS;
}

/*
 * This CB services the axis "Scale" selector 
 */
static void axis_scale_cb(int value, void *data)
{
    int scale = value;
    double major_space, axestart, axestop;
    int auton;
    char buf[32];
    
    xv_evalexpr(tmajor, &major_space);
    xv_evalexpr(axis_world_start, &axestart) ;
    xv_evalexpr(axis_world_stop,  &axestop);
    auton = GetOptionChoice(autonum) + 2;
    
    switch (scale) {
    case SCALE_NORMAL:
        if (major_space <= 0.0) {
            sprintf(buf, "%g", (axestop - axestart)/auton);
            xv_setstr(tmajor, buf);
        }
        break;
    case SCALE_LOG:
        if (axestart <= 0.0 && axestop <= 0.0) {
            errmsg("Can't set logarithmic scale for negative coordinates");
            SetOptionChoice(axis_scale, SCALE_NORMAL);
            return;
        } else if (axestart <= 0.0) {
            axestart = axestop/1.0e3;
            sprintf(buf, "%g", axestart);
            xv_setstr(axis_world_start, buf);
        }
        xv_setstr(tmajor, "10");
        SetSpinChoice(nminor, 9);
        break;
     case SCALE_LOGIT:
        if (axestart <= 0.0 && axestop <= 0.0) {
            errmsg("Can't set logit scale for values outside 0 and 1");
            SetOptionChoice(axis_scale, SCALE_NORMAL);
            return;
        } 
	if (axestart <= 0.0) {
            axestart = 0.1;
            sprintf(buf, "%g", axestart);
            xv_setstr(axis_world_start, buf);
        }
	if (axestop >= 1.0) {
	    axestop = 0.95;
	    sprintf(buf, "%g", axestop);
            xv_setstr(axis_world_stop, buf);
	}
        if (major_space >= 1.0) {
            xv_setstr(tmajor, "0.6");
        }
        break;	
    }
}

/*
 * Fill 'Axes' dialog with values
 */

void update_ticks(int gno)
{
    tickmarks *t;
    world w;
    char buf[128];
    int i;

    if (axes_dialog && XtIsManaged(axes_dialog)) {
        t = get_graph_tickmarks(gno, curaxis);
        if (!t) {
            return;
        }

        SetToggleButtonState(axis_active, is_axis_active(gno, curaxis));
        if (is_axis_active(gno, curaxis) == FALSE) {
            SetSensitive(axes_tab, False);
        } else {
            SetSensitive(axes_tab, True);
        }

        SetOptionChoice(editaxis, curaxis);

        SetToggleButtonState(axis_zero, is_zero_axis(gno, curaxis));

        get_graph_world(gno, &w);
        if (is_xaxis(curaxis)) {
            sprintf(buf, "%.9g", w.xg1);
            xv_setstr(axis_world_start, buf);
            sprintf(buf, "%.9g", w.xg2);
            xv_setstr(axis_world_stop, buf);
            SetOptionChoice(axis_scale, get_graph_xscale(gno));
            SetToggleButtonState(axis_invert, is_graph_xinvert(gno));
        } else {
            sprintf(buf, "%.9g", w.yg1);
            xv_setstr(axis_world_start, buf);
            sprintf(buf, "%.9g", w.yg2);
            xv_setstr(axis_world_stop, buf);
            SetOptionChoice(axis_scale, get_graph_yscale(gno));
            SetToggleButtonState(axis_invert, is_graph_yinvert(gno));
        }

        sprintf(buf, "%.2f", t->offsx);
        xv_setstr(offx, buf);
        sprintf(buf, "%.2f", t->offsy);
        xv_setstr(offy, buf);

        SetOptionChoice(axislabellayout, t->label_layout == LAYOUT_PERPENDICULAR ? 1 : 0);
        SetOptionChoice(axislabelplace, t->label_place);
        sprintf(buf, "%.2f", t->label.offset.x);
        xv_setstr(axislabelspec_para, buf);
        sprintf(buf, "%.2f", t->label.offset.y);
        xv_setstr(axislabelspec_perp, buf);
        SetSensitive(axislabelspec_rc, t->label_place == TYPE_SPEC);
        SetOptionChoice(axislabelfont, t->label.font);
        SetOptionChoice(axislabelcolor, t->label.color);
        SetCharSizeChoice(axislabelcharsize, t->label.charsize);
        SetOptionChoice(axislabelop, t->label_op);

        SetToggleButtonState(tlonoff, t->tl_flag);
        SetToggleButtonState(tonoff, t->t_flag);
        SetToggleButtonState(baronoff, t->t_drawbar);
        SetTextString(axislabel, t->label.s);

        if (is_log_axis(gno, curaxis)) {
            if (t->tmajor <= 1.0) {
                t->tmajor = 10.0;
            }
            sprintf(buf, "%g", t->tmajor);	    
        } else if (is_logit_axis(gno, curaxis)) {
	    if (t->tmajor <= 0.0) {
                t->tmajor = 0.1;
            }
	    else if (t->tmajor >= 0.5) {
                t->tmajor = 0.4;
	    }
            sprintf(buf, "%g", t->tmajor);
        } else if (t->tmajor > 0) {
            sprintf(buf, "%g", t->tmajor);
        } else {
            strcpy(buf, "UNDEFINED");
        }
        xv_setstr(tmajor, buf);
 
        SetSpinChoice(nminor, t->nminor);

        SetOptionChoice(tlfont, t->tl_font);
        SetOptionChoice(tlcolor, t->tl_color);
        SetOptionChoice(tlskip, t->tl_skip);
        SetOptionChoice(tlstagger, t->tl_staggered);
        xv_setstr(tlappstr, t->tl_appstr);
        xv_setstr(tlprestr, t->tl_prestr);
        SetOptionChoice(tlstarttype, t->tl_starttype == TYPE_SPEC);
        if (t->tl_starttype == TYPE_SPEC) {
            sprintf(buf, "%f", t->tl_start);
            xv_setstr(tlstart, buf);
            sprintf(buf, "%f", t->tl_stop);
            xv_setstr(tlstop, buf);
        }
        SetOptionChoice(tlstoptype, t->tl_stoptype == TYPE_SPEC);
        if (t->tl_stoptype == TYPE_SPEC) {
            sprintf(buf, "%f", t->tl_stop);
            xv_setstr(tlstop, buf);
        }
        SetOptionChoice(tlform, t->tl_format);
        SetOptionChoice(ticklop, t->tl_op);
        SetTextString(tlformula, t->tl_formula);
        SetOptionChoice(tlprec, t->tl_prec);

        SetOptionChoice(tlgaptype, t->tl_gaptype);
        sprintf(buf, "%.2f", t->tl_gap.x);
        xv_setstr(tlgap_para, buf);
        sprintf(buf, "%.2f", t->tl_gap.y);
        xv_setstr(tlgap_perp, buf);
        SetSensitive(tlgap_rc, t->tl_gaptype == TYPE_SPEC);

        SetCharSizeChoice(tlcharsize, t->tl_charsize);
        SetAngleChoice(tlangle, t->tl_angle);

        switch (t->t_inout) {
        case TICKS_IN:
            SetOptionChoice(tinout, 0);
            break;
        case TICKS_OUT:
            SetOptionChoice(tinout, 1);
            break;
        case TICKS_BOTH:
            SetOptionChoice(tinout, 2);
            break;
        }
        
        SetOptionChoice(tickop, t->t_op);
        
        SetOptionChoice(tgridcol, t->props.color);
        SetSpinChoice(tgridlinew, t->props.linew);
        SetOptionChoice(tgridlines, t->props.lines);
        SetOptionChoice(tmgridcol, t->mprops.color);
        SetSpinChoice(tmgridlinew, t->mprops.linew);
        SetOptionChoice(tmgridlines, t->mprops.lines);
        SetCharSizeChoice(tlen, t->props.size);
        SetCharSizeChoice(tmlen, t->mprops.size);

        SetOptionChoice(autonum, t->t_autonum - 2);

        SetToggleButtonState(tround, t->t_round);
        SetToggleButtonState(tgrid, t->props.gridflag);
        SetToggleButtonState(tmgrid, t->mprops.gridflag);

        SetOptionChoice(barcolor, t->t_drawbarcolor);
        SetSpinChoice(barlinew, t->t_drawbarlinew);
        SetOptionChoice(barlines, t->t_drawbarlines);

        SetOptionChoice(specticks, t->t_spec);
        SetSpinChoice(nspec, t->nticks);
        for (i = 0; i < t->nticks; i++) {
            sprintf(buf, "%g", t->tloc[i].wtpos);
            xv_setstr(specloc[i], buf);
            if (t->tloc[i].type == TICK_TYPE_MAJOR) {
                xv_setstr(speclabel[i], t->tloc[i].label);
            } else {
                xv_setstr(speclabel[i], "");
            }
        }
    }
}


static void set_active_proc(int onoff, void *data)
{
    SetSensitive(axes_tab, onoff);
}

static void set_axis_proc(int value, void *data)
{
    int cg = get_cg();
    curaxis = value;
    update_ticks(cg);
}

static void auto_spec_cb(int value, void *data)
{
    Widget rc = (Widget) data;
    SetSensitive(rc, value);
}
