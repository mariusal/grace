/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
 * non linear curve fitting
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/ScrolledW.h>

#include "globals.h"
#include "defines.h"
#include "core_utils.h"
#include "utils.h"
#include "files.h"
#include "parser.h"
#include "motifinc.h"
#include "protos.h"

/* nonlprefs.load possible values */
#define LOAD_VALUES         0
#define LOAD_RESIDUALS      1
#define LOAD_FUNCTION       2

#define  WEIGHT_NONE    0
#define  WEIGHT_Y       1
#define  WEIGHT_Y2      2
#define  WEIGHT_DY      3
#define  WEIGHT_CUSTOM  4

/* prefs for non-linear fit */
typedef struct {
    int autoload;       /* do autoload */
    int load;           /* load to... */
    int npoints;        /* # of points to evaluate function at */
    double start;       /* start... */
    double stop;        /* stop ... */
} nonlprefs;

typedef struct {
    TextStructure *formula_item;
    Widget title_item;
    Widget parm_item[MAXPARM];
    Widget value_item[MAXPARM];
    Widget constr_item[MAXPARM];
    Widget lowb_item[MAXPARM];
    Widget uppb_item[MAXPARM];
    Widget tol_item;
    OptionStructure *nparm_item;
    SpinStructure *nsteps_item;
    OptionStructure *load_item;
    Widget autol_item;
    Widget npts_item;
    Widget start_item, stop_item;
    Widget fload_rc;
    RestrictionStructure *restr_item;
    OptionStructure *weigh_item;
    Widget wfunc_item;
} Nonl_ui;

typedef struct {
    NLFit nlfit;
    nonlprefs prefs;
    int nsteps;
    
    int autol;
    
    int npts;
    double start, stop;

    int restr_type;
    int restr_negate;

    int weight_method;
    char *wfunc;
} Nonl_pars;

static void do_nonl_toggle(OptionStructure *opt, int onoff, void *data);
static void nonl_wf_cb(OptionStructure *opt, int value, void *data);
static void do_constr_toggle(Widget tbut, int onoff, void *data);

static void update_nonl_frame(Nonl_ui *ui, NLFit *nlfit);

static void reset_frame_cb(Widget but, void *data);

static void do_nparm_toggle(OptionStructure *opt, int value, void *data);
static void create_openfit_popup(Widget but, void *data);
static void create_savefit_popup(Widget but, void *data);
static int do_openfit_proc(FSBStructure *fsb, char *filename, void *data);
static int do_savefit_proc(FSBStructure *fsb, char *filename, void *data);

static void do_nparm_toggle(OptionStructure *opt, int value, void *data)
{
    Nonl_ui *ui = (Nonl_ui *) data;
    int i;
    for (i = 0; i < MAXPARM; i++) {
        if (i < value) {
            ManageChild(ui->parm_item[i]);
        } else {
            UnmanageChild(ui->parm_item[i]);
        }
    }
}

static void reset_frame_cb(Widget but, void *data)
{
    Nonl_ui *ui = (Nonl_ui *) data;
    NLFit nlfit;
    
    memset(&nlfit, 0, sizeof(NLFit));

    reset_nonl(&nlfit);

    update_nonl_frame(ui, &nlfit);
}

static void *nonl_build_cb(TransformStructure *tdialog)
{
    Nonl_ui *ui;

    ui = xmalloc(sizeof(Nonl_ui));
    if (ui) {
        int i;
        char buf[256];
        OptionItem np_option_items[MAXPARM + 1], option_items[5];
        Widget frame, menubar, menupane;
        Widget nonl_tab, nonl_main, nonl_advanced;
        Widget sw, title_fr, fr3, rc1, rc2, rc3, lab;

        frame = tdialog->frame;

        menubar = tdialog->menubar;
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Open...", 'O', create_openfit_popup, (void *) ui);
        CreateMenuButton(menupane, "Save...", 'S', create_savefit_popup, (void *) ui);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Close", 'C', destroy_dialog_cb, GetParent(frame));

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

        CreateMenuButton(menupane, "Reset fit parameters", 'R', reset_frame_cb, (void *) ui);

        menupane = CreateMenu(menubar, "View", 'V', FALSE);
   
        ui->autol_item = CreateMenuToggle(menupane, "Autoload", 'A',
	    NULL, NULL);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);

        CreateMenuHelpButton(menupane, "On fit", 'f',
            frame, "doc/UsersGuide.html#non-linear-fit");

        ManageChild(menubar);
        
	title_fr = CreateFrame(frame, NULL);
	XtVaSetValues(title_fr, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
	ui->title_item = CreateLabel(title_fr, NULL);
        AddDialogFormChild(frame, title_fr);

        /* ------------ Tabs --------------*/

        nonl_tab = CreateTab(frame);        


        /* ------------ Main tab --------------*/
        
        nonl_main = CreateTabPage(nonl_tab, "Main");
    	
	ui->formula_item = CreateTextInput(nonl_main, "Formula:");
	rc1 = CreateHContainer(nonl_main);
	
	for (i = 0; i < MAXPARM + 1; i++) {
	    np_option_items[i].value = i;
            sprintf(buf, "%d", i);
	    np_option_items[i].label = copy_string(NULL, buf);
        }
	ui->nparm_item = CreateOptionChoice(rc1,
            "Parameters:", 1, MAXPARM + 1, np_option_items);
        AddOptionChoiceCB(ui->nparm_item, do_nparm_toggle, ui);
        
	ui->tol_item = CreateTextItem2(rc1, 8, "Tolerance:");
        
	ui->nsteps_item = CreateSpinChoice(rc1, "Iterations:", 3,
            SPIN_TYPE_INT, 0.0, 500.0, 5.0);
	SetSpinChoice(ui->nsteps_item, 5.0);
        
	sw = XtVaCreateManagedWidget("sw",
				     xmScrolledWindowWidgetClass, nonl_main,
				     XmNheight, 180,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     NULL);

	rc2 = CreateVContainer(sw);
	for (i = 0; i < MAXPARM; i++) {
	    ui->parm_item[i] = CreateHContainer(rc2);
            UnmanageChild(ui->parm_item[i]);
	    sprintf(buf, "A%1d: ", i);
	    ui->value_item[i] = CreateTextItem2(ui->parm_item[i], 10, buf);

	    ui->constr_item[i] = CreateToggleButton(ui->parm_item[i], "Bounds:");
	    AddToggleButtonCB(ui->constr_item[i], do_constr_toggle, (void *) i);

	    ui->lowb_item[i] = CreateTextItem2(ui->parm_item[i], 6, "");
	    
	    sprintf(buf, "< A%1d < ", i);
	    lab = CreateLabel(ui->parm_item[i], buf);

	    ui->uppb_item[i] = CreateTextItem2(ui->parm_item[i], 6, "");
	}

        /* ------------ Advanced tab --------------*/

        nonl_advanced = CreateTabPage(nonl_tab, "Advanced");

	ui->restr_item =
            CreateRestrictionChoice(nonl_advanced, "Source data filtering");

	fr3 = CreateFrame(nonl_advanced, "Weighting");
        rc3 = CreateHContainer(fr3);
        option_items[0].value = WEIGHT_NONE;
        option_items[0].label = "None";
        option_items[1].value = WEIGHT_Y;
        option_items[1].label = "1/Y";
        option_items[2].value = WEIGHT_Y2;
        option_items[2].label = "1/Y^2";
        option_items[3].value = WEIGHT_DY;
        option_items[3].label = "1/dY^2";
        option_items[4].value = WEIGHT_CUSTOM;
        option_items[4].label = "Custom";
	ui->weigh_item = CreateOptionChoice(rc3, "Weights", 1, 5, option_items);
	ui->wfunc_item = CreateTextItem2(rc3, 30, "Function:");
	AddOptionChoiceCB(ui->weigh_item, nonl_wf_cb, (void *) ui->wfunc_item);

	fr3 = CreateFrame(nonl_advanced, "Load options");
        rc3 = CreateVContainer(fr3);
        option_items[0].value = LOAD_VALUES;
        option_items[0].label = "Fitted values";
        option_items[1].value = LOAD_RESIDUALS;
        option_items[1].label = "Residuals";
        option_items[2].value = LOAD_FUNCTION;
        option_items[2].label = "Function";
	ui->load_item = CreateOptionChoice(rc3, "Load", 1, 3, option_items);
	ui->fload_rc = CreateHContainer(rc3);
	ui->start_item = CreateTextItem2(ui->fload_rc, 6, "Start load at:");
	ui->stop_item = CreateTextItem2(ui->fload_rc, 6, "Stop load at:");
	ui->npts_item = CreateTextItem2(ui->fload_rc, 4, "# of points:");
        AddOptionChoiceCB(ui->load_item, do_nonl_toggle, (void *) ui->fload_rc);

        /* defaults */
        SetToggleButtonState(ui->autol_item, TRUE);
        SetOptionChoice(ui->load_item, LOAD_VALUES);
        SetSensitive(ui->fload_rc, FALSE);
        SetSensitive(GetParent(ui->wfunc_item), FALSE);
        xv_setstr(ui->start_item, "0.0");
        xv_setstr(ui->stop_item,  "1.0");
        xv_setstr(ui->npts_item,  "10");
    }

    return (void *) ui;
}

static void nonl_free_cb(void *tddata)
{
    Nonl_pars *pars = (Nonl_pars *) tddata;
    if (pars) {
        xfree(pars->nlfit.title);
        xfree(pars->nlfit.formula);

        xfree(pars->wfunc);
        xfree(pars);
    }
}

static void *nonl_get_cb(void *gui)
{
    Nonl_ui *ui = (Nonl_ui *) gui;
    Nonl_pars *pars;
    
    pars = xmalloc(sizeof(Nonl_pars));
    if (pars) {
        int i;
        NLFit *nlfit = &pars->nlfit;
        
        pars->nlfit.title   = NULL;
        nlfit->formula = GetTextString(ui->formula_item);
        nlfit->tolerance = atof(xv_getstr(ui->tol_item));
        nlfit->parnum = GetOptionChoice(ui->nparm_item);
        
        pars->nsteps = (int) GetSpinChoice(ui->nsteps_item);

        for (i = 0; i < nlfit->parnum; i++) {
            char buf[256];
	    nonlparm *nlp = &nlfit->parms[i];
            strcpy(buf, xv_getstr(ui->value_item[i]));
	    if (sscanf(buf, "%lf", &nlp->value) != 1) {
	        errmsg("Invalid input in parameter field");
	        nonl_free_cb(pars);
                return NULL;
	    }

	    nlp->constr = GetToggleButtonState(ui->constr_item[i]);
	    if (nlp->constr) {
	        strcpy(buf, xv_getstr(ui->lowb_item[i]));
	        if (sscanf(buf, "%lf", &nlp->min) != 1) {
	    	    errmsg("Invalid input in low-bound field");
	            nonl_free_cb(pars);
                    return NULL;
	        }
	        strcpy(buf, xv_getstr(ui->uppb_item[i]));
	        if (sscanf(buf, "%lf", &nlp->max) != 1) {
	    	    errmsg("Invalid input in upper-bound field");
	            nonl_free_cb(pars);
                    return NULL;
	        }
	        if ((nlp->value < nlp->min) || (nlp->value > nlp->max)) {
	    	    errmsg("Initial values must be within bounds");
	            nonl_free_cb(pars);
                    return NULL;
	        }
	    }
        }

        pars->prefs.autoload = GetToggleButtonState(ui->autol_item);
        pars->prefs.load     = GetOptionChoice(ui->load_item);

        if (pars->prefs.load == LOAD_FUNCTION) {
	    if (xv_evalexpr(ui->start_item, &pars->prefs.start) != RETURN_SUCCESS) {
	        errmsg("Invalid input in start field");
	        nonl_free_cb(pars);
                return NULL;
	    }
	    if (xv_evalexpr(ui->stop_item, &pars->prefs.stop) != RETURN_SUCCESS) {
	        errmsg("Invalid input in stop field");
	        nonl_free_cb(pars);
                return NULL;
	    }
	    if (xv_evalexpri(ui->npts_item, &pars->prefs.npoints) != RETURN_SUCCESS) {
	        errmsg("Invalid input in npoints field");
	        nonl_free_cb(pars);
                return NULL;
	    }
    	    if (pars->prefs.npoints <= 1) {
    	        errmsg("Number of points must be > 1");
	        nonl_free_cb(pars);
                return NULL;
    	    }
        }


        for (i = 0; i < nlfit->parnum; i++) {
            nonlparm *nlp = &nlfit->parms[i];
            double *var;

            var = define_parser_scalar(nlp->name);
            if (var) {
                *var = nlp->value;
            }
        }
	
        pars->weight_method = GetOptionChoice(ui->weigh_item);
        pars->restr_type    = GetOptionChoice(ui->restr_item->r_sel);
        pars->restr_negate  = GetToggleButtonState(ui->restr_item->negate);
    }
    
    return (void *) pars;
}

static int nonl_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int i, res;
    Nonl_pars *pars = (Nonl_pars *) tddata;

    if (pars->nsteps) {
        int nlen, wlen;
        double *ytmp, *warray;
        char *rarray;
        
        /* apply weigh function */
    	nlen = set_get_length(psrc);
        switch (pars->weight_method) {
        case WEIGHT_Y:
        case WEIGHT_Y2:
            ytmp = set_get_col(psrc, DATA_Y);
            for (i = 0; i < nlen; i++) {
                if (ytmp[i] == 0.0) {
	            errmsg("Divide by zero while calculating weights");
                    return RETURN_FAILURE;
                }
            }
            warray = xmalloc(nlen*SIZEOF_DOUBLE);
            if (warray == NULL) {
	        errmsg("xmalloc failed in nonl_run_cb()");
                return RETURN_FAILURE;
            }
            for (i = 0; i < nlen; i++) {
                if (pars->weight_method == WEIGHT_Y) {
                    warray[i] = 1/ytmp[i];
                } else {
                    warray[i] = 1/(ytmp[i]*ytmp[i]);
                }
            }
            break;
        case WEIGHT_DY:
            ytmp = set_get_col(psrc, DATA_Y1);
            if (ytmp == NULL) {
	        errmsg("The set doesn't have dY data column");
                return RETURN_FAILURE;
            }
            for (i = 0; i < nlen; i++) {
                if (ytmp[i] == 0.0) {
	            errmsg("Divide by zero while calculating weights");
                    return RETURN_FAILURE;
                }
            }
            warray = xmalloc(nlen*SIZEOF_DOUBLE);
            if (warray == NULL) {
	        errmsg("xmalloc failed in nonl_run_cb()");
            }
            for (i = 0; i < nlen; i++) {
                warray[i] = 1/(ytmp[i]*ytmp[i]);
            }
            break;
        case WEIGHT_CUSTOM:
            if (set_parser_setno(psrc) != RETURN_SUCCESS) {
                errmsg("Bad set");
                return RETURN_FAILURE;
            }

            if (v_scanner(pars->wfunc, &wlen, &warray) != RETURN_SUCCESS) {
                errmsg("Error evaluating expression for weights");
                return RETURN_FAILURE;
            }
            if (wlen != nlen) {
                errmsg("The array of weights has different length");
                xfree(warray);
                return RETURN_FAILURE;
            }
            break;
        default:
            warray = NULL;
            break;
        }

        /* Apply restrictions */
        res = get_restriction_array(psrc,
            NULL, pars->restr_negate, &rarray);
	if (res != RETURN_SUCCESS) {
	    errmsg("Error in restriction evaluation");
	    xfree(warray);
            return RETURN_FAILURE;
	}

        /* The fit itself! */
    	res = do_nonlfit(psrc, &pars->nlfit, warray, rarray, pars->nsteps);
	
        /* Free temp arrays */
        xfree(warray);
	xfree(rarray);
    	
        if (res != RETURN_SUCCESS) {
	    errmsg("Fatal error in do_nonlfit()");  
	    return RETURN_FAILURE;  	
    	}
   	    	
        /* update_nonl_frame(ui, &pars->nlfit); */
    }

/*
 * Select & activate a set to load results to
 */    
    if (pars->prefs.autoload) {
        int npts = 0;
        double delx, *xfit, *y, *yfit;
    	
        switch (pars->prefs.load) {
    	case LOAD_VALUES:
    	case LOAD_RESIDUALS:
    	    npts = set_get_length(psrc);
    	    set_set_length(pdest, npts);
    	    copycol2(psrc, pdest, DATA_X);
    	    break;
    	case LOAD_FUNCTION:
    	    npts  = pars->prefs.npoints;
 
    	    set_set_length(pdest, npts);
 
    	    delx = (pars->prefs.stop - pars->prefs.start)/(npts - 1);
    	    xfit = getx(pdest);
	    for (i = 0; i < npts; i++) {
	        xfit[i] = pars->prefs.start + i * delx;
	    }
    	    break;
    	}
    	
    	set_set_comment(pdest, pars->nlfit.formula);
    	
    	do_compute(pdest, pdest, NULL, pars->nlfit.formula);
    	
    	if (pars->prefs.load == LOAD_RESIDUALS) { /* load residuals */
    	    y = gety(psrc);
    	    yfit = gety(pdest);
    	    for (i = 0; i < npts; i++) {
	        yfit[i] -= y[i];
	    }
    	}
    }
    
    return RETURN_SUCCESS;
}

void create_nonl_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = nonl_build_cb;
        cbs.get_cb   = nonl_get_cb;
        cbs.free_cb  = nonl_free_cb;
        cbs.run_cb   = nonl_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Non-linear curve fitting", LIST_TYPE_SINGLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

static void update_nonl_frame(Nonl_ui *ui, NLFit *nlfit)
{
    int i;
    
    if (ui) {
        char buf[256];
        SetLabel(ui->title_item, nlfit->title);
/* 
 * If I define only XmALIGNMENT_CENTER (default!) then it's ignored - bug in Motif???
 */
    	XtVaSetValues(ui->title_item, XmNalignment, XmALIGNMENT_BEGINNING, NULL);
        XtVaSetValues(ui->title_item, XmNalignment, XmALIGNMENT_CENTER, NULL);
        
        SetTextString(ui->formula_item, nlfit->formula);
        sprintf(buf, "%g", nlfit->tolerance);
        xv_setstr(ui->tol_item, buf);
        SetOptionChoice(ui->nparm_item, nlfit->parnum);
        for (i = 0; i < MAXPARM; i++) {
            nonlparm *nlp = &nlfit->parms[i];
            sprintf(buf, "%g", nlp->value);
            xv_setstr(ui->value_item[i], buf);
            SetToggleButtonState(ui->constr_item[i], nlp->constr);
            sprintf(buf, "%g", nlp->min);
            xv_setstr(ui->lowb_item[i], buf);
            SetSensitive(ui->lowb_item[i], nlp->constr);
            sprintf(buf, "%g", nlp->max);
            xv_setstr(ui->uppb_item[i], buf);
            SetSensitive(ui->uppb_item[i], nlp->constr);
            if (i < nlfit->parnum) {
                if (!IsManaged (ui->parm_item[i])) {
                    ManageChild(ui->parm_item[i]);
                }
            } else {
                if (IsManaged (ui->parm_item[i])) {
                    UnmanageChild(ui->parm_item[i]);
                }
            }
        }
    }
}

static void nonl_wf_cb(OptionStructure *opt, int value, void *data)
{
    Widget rc = GetParent((Widget) data);
    
    if (value == WEIGHT_CUSTOM) {
    	SetSensitive(rc, True);
    } else {
    	SetSensitive(rc, False);
    }
}

static void do_nonl_toggle(OptionStructure *opt, int value, void *data)
{
    Widget rc = (Widget) data;
    
    if (value == LOAD_FUNCTION) {
    	SetSensitive(rc, True);
    } else {
    	SetSensitive(rc, False);
    }
}

static void do_constr_toggle(Widget tbut, int onoff, void *data)
{
#if 0
    int value = (int) data;
    if (onoff) {
    	SetSensitive(ui->lowb_item[value], True);
    	SetSensitive(ui->uppb_item[value], True);
    	nlfit.parms[value].constr = TRUE;
    } else {
    	SetSensitive(ui->lowb_item[value], False);
    	SetSensitive(ui->uppb_item[value], False);
    	nlfit.parms[value].constr = FALSE;
    }
#endif
}


static void create_openfit_popup(Widget but, void *data)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Open fit parameter file");
	AddFileSelectionBoxCB(fsb, do_openfit_proc, NULL);
        ManageChild(fsb->FSB);
    }
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

static int do_openfit_proc(FSBStructure *fsb, char *filename, void *data)
{
    errwin("Not implemented yet");
    
    return FALSE;
}


static void create_savefit_popup(Widget but, void *data)
{
    static FSBStructure *fsb = NULL;
    static Widget title_item = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        Widget fr;
        
        fsb = CreateFileSelectionBox(app_shell, "Save fit parameter file");
	fr = CreateFrame(fsb->rc, NULL);
	title_item = CreateTextItem2(fr, 25, "Title: ");
	AddFileSelectionBoxCB(fsb, do_savefit_proc, (void *) title_item);
        ManageChild(fsb->FSB);
    }
    
    /* xv_setstr(title_item, ui->nlfit.title); */
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

static int do_savefit_proc(FSBStructure *fsb, char *filename, void *data)
{
    FILE *pp;
    /* Widget title_item = (Widget) data; */
    
    pp = grace_openw(grace, filename);
    if (pp != NULL) {
#if 0
        nlfit.title = copy_string(nlfit.title, xv_getstr(title_item));
#endif
        errwin("Not implemented yet");
        /* FIXME */;
        grace_close(pp);
    }
    return TRUE;
}
