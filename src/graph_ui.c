/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2005 Grace Development Team
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

/* Graph UI */

#include "core_utils.h"
#include "explorer.h"

static void axis_scale_cb(OptionStructure *opt, int value, void *data);

GraphUI *create_graph_ui(ExplorerUI *eui)
{
    GraphUI *ui;
    Widget tab, fr, rc, rc1;

    OptionItem opitems[4] = {
        {SCALE_NORMAL, "Linear"     },
        {SCALE_LOG,    "Logarithmic"},
        {SCALE_REC,    "Reciprocal" },
        {SCALE_LOGIT,  "Logit"      }
    };
    
    ui = xmalloc(sizeof(GraphUI));

    /* ------------ Tabs -------------- */

    tab = CreateTab(eui->scrolled_window);        
    AddHelpCB(tab, "doc/UsersGuide.html#graph-properties");


    /* ------------ Main tab -------------- */

    ui->main_tp = CreateTabPage(tab, "Main");

    fr = CreateFrame(ui->main_tp, "Presentation");
    rc1 = CreateVContainer(fr);

    rc = CreateHContainer(rc1);
    ui->graph_type = CreateOptionChoiceVA(rc, "Type:",
        "XY graph",          GRAPH_XY,
        "XY chart",          GRAPH_CHART,
        "Polar graph",       GRAPH_POLAR,
        "Smith chart (N/I)", GRAPH_SMITH,
        "Fixed",             GRAPH_FIXED,
        "Pie chart",         GRAPH_PIE,
        NULL);
    AddOptionChoiceCB(ui->graph_type, oc_explorer_cb, eui);
    ui->stacked = CreateToggleButton(rc, "Stacked chart");
    AddToggleButtonCB(ui->stacked, tb_explorer_cb, eui);

    rc = CreateHContainer(rc1);
    ui->flip_xy = CreateToggleButton(rc, "Flip XY (N/I)");
    AddToggleButtonCB(ui->flip_xy, tb_explorer_cb, eui);


    fr = CreateFrame(ui->main_tp, "X axis");
    rc1 = CreateVContainer(fr);
    
    rc = CreateHContainer(rc1);
    ui->start_x = CreateText2(rc, "Start:", 10);
    AddTextActivateCB(ui->start_x, text_explorer_cb, eui);
    ui->stop_x = CreateText2(rc, "Stop:", 10);
    AddTextActivateCB(ui->stop_x, text_explorer_cb, eui);

    rc = CreateHContainer(rc1);
    ui->scale_x = CreateOptionChoice(rc, "Scale:", 0, 4, opitems);
    AddOptionChoiceCB(ui->scale_x, axis_scale_cb, eui);
    AddOptionChoiceCB(ui->scale_x, oc_explorer_cb, eui);

    ui->invert_x = CreateToggleButton(rc, "Invert axis");
    AddToggleButtonCB(ui->invert_x, tb_explorer_cb, eui);
        
    fr = CreateFrame(ui->main_tp, "Y axis");
    rc1 = CreateVContainer(fr);
    
    rc = CreateHContainer(rc1);
    ui->start_y = CreateText2(rc, "Start:", 10);
    AddTextActivateCB(ui->start_y, text_explorer_cb, eui);
    ui->stop_y = CreateText2(rc, "Stop:", 10);
    AddTextActivateCB(ui->stop_y, text_explorer_cb, eui);

    rc = CreateHContainer(rc1);
    ui->scale_y = CreateOptionChoice(rc, "Scale:", 0, 4, opitems);
    AddOptionChoiceCB(ui->scale_y, axis_scale_cb, eui);
    AddOptionChoiceCB(ui->scale_y, oc_explorer_cb, eui);

    ui->invert_y = CreateToggleButton(rc, "Invert axis");
    AddToggleButtonCB(ui->invert_y, tb_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "2D+ graphs");
    ui->znorm = CreateText2(fr, "Z normalization", 10);
    AddTextActivateCB(ui->znorm, text_explorer_cb, eui);

    fr = CreateFrame(ui->main_tp, "XY charts");
    ui->bargap = CreateSpinChoice(fr, "Bar gap:", 5,
        SPIN_TYPE_FLOAT, -1.0, 1.0, 0.005);
    AddSpinChoiceCB(ui->bargap, sp_explorer_cb, eui);


    /* ------------ Locator tab -------------- */

    ui->locator_tp = CreateTabPage(tab, "Locator");
    ui->loc_type = CreateOptionChoiceVA(ui->locator_tp,
            "Locator display type:",
            "None",       GLOCATOR_TYPE_NONE,
            "[X, Y]",     GLOCATOR_TYPE_XY,
            "[Phi, Rho]", GLOCATOR_TYPE_POLAR,
            NULL);
    AddOptionChoiceCB(ui->loc_type, oc_explorer_cb, eui);

    fr = CreateFrame(ui->locator_tp, "X format");
    ui->loc_fx = CreateFormatChoice(fr);
    AddFormatChoiceCB(ui->loc_fx, format_explorer_cb, eui);
    
    fr = CreateFrame(ui->locator_tp, "Y format");
    ui->loc_fy = CreateFormatChoice(fr);
    AddFormatChoiceCB(ui->loc_fy, format_explorer_cb, eui);

    fr = CreateFrame(ui->locator_tp, "Fixed point");
    rc = CreateVContainer(fr);
    ui->fixedp = CreateToggleButton(rc, "Enable");
    AddToggleButtonCB(ui->fixedp, tb_explorer_cb, eui);
    rc1 = CreateHContainer(rc);
    ui->locx = CreateText2(rc1, "X:", 10);
    AddTextActivateCB(ui->locx, text_explorer_cb, eui);
    ui->locy = CreateText2(rc1, "Y:", 10);
    AddTextActivateCB(ui->locy, text_explorer_cb, eui);


    SelectTabPage(tab, ui->main_tp);

    ui->top = tab;
    
    return ui;
}

void update_graph_ui(GraphUI *ui, Quark *q)
{
    if (q && quark_fid_get(q) == QFlavorGraph) {
        char buf[32];
        world w;
        GLocator *locator;

        graph_get_world(q, &w);
        locator = graph_get_locator(q);

        SetOptionChoice(ui->graph_type, graph_get_type(q));
        ToggleButtonSetState(ui->stacked, graph_is_stacked(q));
        ToggleButtonSetState(ui->flip_xy, graph_get_xyflip(q));

        sprintf(buf, "%.9g", w.xg1);
        TextSetString(ui->start_x, buf);
        sprintf(buf, "%.9g", w.xg2);
        TextSetString(ui->stop_x, buf);
        SetOptionChoice(ui->scale_x, graph_get_xscale(q));
        ToggleButtonSetState(ui->invert_x, graph_is_xinvert(q));

        sprintf(buf, "%.9g", w.yg1);
        TextSetString(ui->start_y, buf);
        sprintf(buf, "%.9g", w.yg2);
        TextSetString(ui->stop_y, buf);
        SetOptionChoice(ui->scale_y, graph_get_yscale(q));
        ToggleButtonSetState(ui->invert_y, graph_is_yinvert(q));

        sprintf(buf, "%g", graph_get_znorm(q));
        TextSetString(ui->znorm, buf);

        SpinChoiceSetValue(ui->bargap, graph_get_bargap(q));


        ToggleButtonSetState(ui->fixedp, locator->pointset);
        SetOptionChoice(ui->loc_type, locator->type);
        SetFormatChoice(ui->loc_fx, &locator->fx);
        SetFormatChoice(ui->loc_fy, &locator->fy);
        sprintf(buf, "%g", locator->origin.x);
        TextSetString(ui->locx, buf);
        sprintf(buf, "%g", locator->origin.y);
        TextSetString(ui->locy, buf);
    }
}

int graph_set_data(GraphUI *ui, Quark *q, void *caller)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        double axislim, znorm;
        world w;
        GLocator *locator;

        graph_get_world(q, &w);
        locator = graph_get_locator(q);

        if (!caller || caller == ui->graph_type) {
            graph_set_type(q, GetOptionChoice(ui->graph_type));
        }
        if (!caller || caller == ui->stacked) {
            graph_set_stacked(q, ToggleButtonGetState(ui->stacked));
        }
        if (!caller || caller == ui->flip_xy) {
            graph_set_xyflip(q, ToggleButtonGetState(ui->flip_xy));
        }

        if (!caller || caller == ui->start_x) {
            if (xv_evalexpr(ui->start_x, &axislim) != RETURN_SUCCESS) {
                errmsg("Axis start/stop values undefined");
                return RETURN_FAILURE;
            }
            w.xg1 = axislim;
        }
        if (!caller || caller == ui->stop_x) {
            if (xv_evalexpr(ui->stop_x, &axislim) != RETURN_SUCCESS) {
                errmsg("Axis start/stop values undefined");
                return RETURN_FAILURE;
            }
            w.xg2 = axislim;
        }

        if (!caller || caller == ui->start_y) {
            if (xv_evalexpr(ui->start_y, &axislim) != RETURN_SUCCESS) {
                errmsg("Axis start/stop values undefined");
                return RETURN_FAILURE;
            }
            w.yg1 = axislim;
        }
        if (!caller || caller == ui->stop_y) {
            if (xv_evalexpr(ui->stop_y, &axislim) != RETURN_SUCCESS) {
                errmsg("Axis start/stop values undefined");
                return RETURN_FAILURE;
            }
            w.yg2 = axislim;
        }

        if (!caller ||
            caller == ui->start_x || caller == ui->stop_x ||
            caller == ui->start_y || caller == ui->stop_y) {
            graph_set_world(q, &w);
        }

        if (!caller || caller == ui->scale_x) {
            graph_set_xscale(q, GetOptionChoice(ui->scale_x));
        }
        if (!caller || caller == ui->invert_x)  {
            graph_set_xinvert(q, ToggleButtonGetState(ui->invert_x));
        }

        if (!caller || caller == ui->scale_y) {
            graph_set_yscale(q, GetOptionChoice(ui->scale_y));
        }
        if (!caller || caller == ui->invert_y)  {
            graph_set_yinvert(q, ToggleButtonGetState(ui->invert_y));
        }

        if (!caller || caller == ui->bargap) {
            graph_set_bargap(q, SpinChoiceGetValue(ui->bargap));
        }
        if (!caller || caller == ui->znorm) {
            xv_evalexpr(ui->znorm, &znorm);
            graph_set_znorm(q, znorm);
        }


        if (!caller || caller == ui->loc_type) {
            locator->type = GetOptionChoice(ui->loc_type);
            quark_dirtystate_set(q, TRUE);
        }
        if (!caller || caller == ui->loc_fx) {
            Format *format = GetFormatChoice(ui->loc_fx);
            AMem *amem = quark_get_amem(q);
            amem_free(amem, locator->fx.fstring);
            locator->fx = *format;
            locator->fx.fstring = amem_strdup(amem, format->fstring);
            format_free(format);
            quark_dirtystate_set(q, TRUE);
        }
        if (!caller || caller == ui->loc_fy) {
            Format *format = GetFormatChoice(ui->loc_fy);
            AMem *amem = quark_get_amem(q);
            amem_free(amem, locator->fy.fstring);
            locator->fy = *format;
            locator->fy.fstring = amem_strdup(amem, format->fstring);
            format_free(format);
            quark_dirtystate_set(q, TRUE);
        }
        if (!caller || caller == ui->fixedp) {
            locator->pointset = ToggleButtonGetState(ui->fixedp);
            quark_dirtystate_set(q, TRUE);
        }
        if (!caller || caller == ui->locx) {
            xv_evalexpr(ui->locx, &locator->origin.x);
            quark_dirtystate_set(q, TRUE);
        }
        if (!caller || caller == ui->locy) {
            xv_evalexpr(ui->locy, &locator->origin.y);
            quark_dirtystate_set(q, TRUE);
        }

        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


/*
 * This CB services the axis "Scale" selector 
 */
static void axis_scale_cb(OptionStructure *opt, int value, void *data)
{
    int scale = value;
    double axestart, axestop;
    char buf[32];
    TextStructure *axis_world_start;
    TextStructure *axis_world_stop;

    ExplorerUI *eui = (ExplorerUI *) data;
    GraphUI *ui = eui->graph_ui;

    if (opt == ui->scale_x) {
        axis_world_start = ui->start_x;
        axis_world_stop  = ui->stop_x;
    } else {
        axis_world_start = ui->start_y;
        axis_world_stop  = ui->stop_y;
    }

    xv_evalexpr(axis_world_start, &axestart) ;
    xv_evalexpr(axis_world_stop,  &axestop);

    switch (scale) {
    case SCALE_LOG:
        if (axestart <= 0.0 && axestop <= 0.0) {
            errmsg("Can't set logarithmic scale for negative coordinates");
            SetOptionChoice(opt, SCALE_NORMAL);
            return;
        } else if (axestart <= 0.0) {
            axestart = axestop/1.0e3;
            sprintf(buf, "%g", axestart);
            TextSetString(axis_world_start, buf);
        }
        break;
    case SCALE_LOGIT:
        if (axestart <= 0.0 && axestop <= 0.0) {
            errmsg("Can't set logit scale for values outside 0 and 1");
            SetOptionChoice(opt, SCALE_NORMAL);
            return;
        } 
        if (axestart <= 0.0) {
            axestart = 0.1;
            sprintf(buf, "%g", axestart);
            TextSetString(axis_world_start, buf);
        }
        if (axestop >= 1.0) {
            axestop = 0.95;
            sprintf(buf, "%g", axestop);
            TextSetString(axis_world_stop, buf);
        }
        break;
    }
}
