/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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

/*
 *
 * transformations, curve fitting, etc.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "core_utils.h"
#include "utils.h"
#include "ssdata.h"
#include "numerics.h"
#include "motifinc.h"
#include "xprotos.h"
#include "globals.h"

typedef struct {
    TextStructure *formula_item;
    RestrictionStructure *restr_item;
} Eval_ui;

typedef struct {
    char *fstr;
    int restr_type;
    int restr_negate;
} Eval_pars;

static void *eval_build_cb(TransformStructure *tdialog)
{
    Eval_ui *ui;

    ui = xmalloc(sizeof(Eval_ui));
    if (ui) {
        Widget rc_trans;
        
        rc_trans = CreateVContainer(tdialog->frame);
        ui->formula_item = CreateScrolledTextInput(rc_trans, "Formula:", 3);
        ui->restr_item = CreateRestrictionChoice(rc_trans, "Source data filtering");
    }

    return (void *) ui;
}

static void *eval_get_cb(void *gui)
{
    Eval_ui *ui = (Eval_ui *) gui;
    Eval_pars *pars;
    
    pars = xmalloc(sizeof(Eval_pars));
    if (pars) {
        pars->restr_type = GetOptionChoice(ui->restr_item->r_sel);
        pars->restr_negate = GetToggleButtonState(ui->restr_item->negate);
        pars->fstr = GetTextString(ui->formula_item);
    }
    
    return (void *) pars;
}

static void eval_free_cb(void *tddata)
{
    Eval_pars *pars = (Eval_pars *) tddata;
    if (pars) {
        xfree(pars->fstr);
        xfree(pars);
    }
}

/*
 * evaluate a formula
 */
static int eval_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    char *rarray;
    Eval_pars *pars = (Eval_pars *) tddata;

    res = get_restriction_array(psrc, NULL, pars->restr_negate, &rarray);
    if (res != RETURN_SUCCESS) {
	errmsg("Error in evaluation of restriction");
        return RETURN_FAILURE;
    }

    res = do_compute(psrc, pdest, rarray, pars->fstr);
    xfree(rarray);
    if (res != RETURN_SUCCESS) {
	errmsg("Error in do_compute(), check formula");
        return RETURN_FAILURE;
    }
    
    return RETURN_SUCCESS;
}

void create_eval_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = eval_build_cb;
        cbs.get_cb   = eval_get_cb;
        cbs.free_cb  = eval_free_cb;
        cbs.run_cb   = eval_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Evaluate expression", LIST_TYPE_MULTIPLE, FALSE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

#define SAMPLING_MESH   0
#define SAMPLING_SET    1

/* interpolation */

typedef struct {
    OptionStructure *method;
    OptionStructure *sampling;
    Widget strict;
    Widget mrc;
    Widget mstart;
    Widget mstop;
    Widget mlength;
    GraphSetStructure *sel;
} Interp_ui;

typedef struct {
    int method;
    int strict;
    int sampling;
    int meshlen;
    double *mesh;
} Interp_pars;

static void sampling_cb(OptionStructure *opt, int value, void *data)
{
    Interp_ui *ui = (Interp_ui *) data;
    
    if (value == SAMPLING_MESH) {
        SetSensitive(ui->mrc, TRUE);
        SetSensitive(ui->sel->frame, FALSE);
    } else {
        SetSensitive(ui->mrc, FALSE);
        SetSensitive(ui->sel->frame, TRUE);
    }
}

static void *interp_build_cb(TransformStructure *tdialog)
{
    Interp_ui *ui;

    ui = xmalloc(sizeof(Interp_ui));
    if (ui) {
        Widget rc, rc2;
        OptionItem opitems[3];
        
        rc = CreateVContainer(tdialog->frame);

        rc2 = CreateHContainer(rc);
        opitems[0].value = INTERP_LINEAR;
        opitems[0].label = "Linear";
        opitems[1].value = INTERP_SPLINE;
        opitems[1].label = "Cubic spline";
        opitems[2].value = INTERP_ASPLINE;
        opitems[2].label = "Akima spline";
        ui->method = CreateOptionChoice(rc2, "Method:", 0, 3, opitems);

        ui->strict =
            CreateToggleButton(rc2, "Strict (within source set bounds)");

        CreateSeparator(rc);

        opitems[0].value = SAMPLING_MESH;
        opitems[0].label = "Linear mesh";
        opitems[1].value = SAMPLING_SET;
        opitems[1].label = "Abscissas of another set";
        ui->sampling = CreateOptionChoice(rc, "Sampling:", 0, 2, opitems);
        AddOptionChoiceCB(ui->sampling, sampling_cb, ui);

        ui->mrc = CreateHContainer(rc);
        ui->mstart  = CreateTextItem(ui->mrc, 10, "Start at:");
        ui->mstop   = CreateTextItem(ui->mrc, 10, "Stop at:");
        ui->mlength = CreateTextItem(ui->mrc,  6, "Length:");

        ui->sel = CreateGraphSetSelector(rc, "Sampling set", LIST_TYPE_SINGLE);
        SetSensitive(ui->sel->frame, FALSE);
    }

    return (void *) ui;
}

static void interp_free_cb(void *tddata)
{
    Interp_pars *pars = (Interp_pars *) tddata;
    if (pars) {
        xfree(pars->mesh);
        xfree(pars);
    }
}

static void *interp_get_cb(void *gui)
{
    Interp_ui *ui = (Interp_ui *) gui;
    Interp_pars *pars;
    int error = FALSE;
    
    pars = xmalloc(sizeof(Interp_pars));
    if (pars) {
        pars->method   = GetOptionChoice(ui->method);
        pars->sampling = GetOptionChoice(ui->sampling);
        pars->strict   = GetToggleButtonState(ui->strict);
        pars->mesh     = NULL;
        pars->meshlen  = 0;
        
        if (pars->sampling == SAMPLING_SET) {
            Quark *psampl;
            int res;
            
            res = GetSingleStorageChoice(ui->sel->set_sel, &psampl);
            if (res != RETURN_SUCCESS) {
                errmsg("Please select a single sampling set");
                error = TRUE;
            } else {
                pars->meshlen = set_get_length(psampl);
                pars->mesh = copy_data_column_simple(
                    set_get_col(psampl, DATA_X), pars->meshlen);
            }
        } else {
            double start, stop;
            if (xv_evalexpr(ui->mstart, &start)     != RETURN_SUCCESS ||
                xv_evalexpr(ui->mstop,  &stop)      != RETURN_SUCCESS ||
                xv_evalexpri(ui->mlength, &pars->meshlen) != RETURN_SUCCESS ) {
                 errmsg("Can't parse mesh settings");
                 error = TRUE;
            } else {
                pars->mesh = allocate_mesh(start, stop, pars->meshlen);
            }
        }
        if (pars->mesh == NULL) {
	    errmsg("Can't allocate mesh array");
            error = TRUE;
        }
    }
    
    if (error) {
        interp_free_cb(pars);
        return NULL;
    } else {
        return (void *) pars;
    }
}

static int interp_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Interp_pars *pars = (Interp_pars *) tddata;

    res = do_interp(psrc, pdest,
        pars->mesh, pars->meshlen, pars->method, pars->strict);

    if (res != RETURN_SUCCESS) {
	errmsg("Error in do_interp()");
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

void create_interp_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = interp_build_cb;
        cbs.get_cb   = interp_get_cb;
        cbs.free_cb  = interp_free_cb;
        cbs.run_cb   = interp_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Interpolation", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* histograms */

typedef struct {
    Widget cumulative;
    Widget normalize;
    OptionStructure *sampling;
    Widget mrc;
    Widget mstart;
    Widget mstop;
    Widget mlength;
    GraphSetStructure *sel;
} Histo_ui;

typedef struct {
    int cumulative;
    int normalize;
    int sampling;
    int nbins;
    double *bins;
} Histo_pars;

static void binsampling_cb(OptionStructure *opt, int value, void *data)
{
    Histo_ui *ui = (Histo_ui *) data;
    
    if (value == SAMPLING_MESH) {
        SetSensitive(ui->mrc, TRUE);
        SetSensitive(ui->sel->frame, FALSE);
    } else {
        SetSensitive(ui->mrc, FALSE);
        SetSensitive(ui->sel->frame, TRUE);
    }
}

static void *histo_build_cb(TransformStructure *tdialog)
{
    Histo_ui *ui;

    ui = xmalloc(sizeof(Histo_ui));
    if (ui) {
        Widget rc, rc2;
        OptionItem opitems[3];
        
        rc = CreateVContainer(tdialog->frame);

        rc2 = CreateHContainer(rc);
        ui->cumulative = CreateToggleButton(rc2, "Cumulative histogram");
        ui->normalize = CreateToggleButton(rc2, "Normalize");
        
        CreateSeparator(rc);
        
        opitems[0].value = SAMPLING_MESH;
        opitems[0].label = "Linear mesh";
        opitems[1].value = SAMPLING_SET;
        opitems[1].label = "Abscissas of another set";
        ui->sampling = CreateOptionChoice(rc, "Bin sampling:", 0, 2, opitems);
        AddOptionChoiceCB(ui->sampling, binsampling_cb, ui);

        ui->mrc = CreateHContainer(rc);
	ui->mstart  = CreateTextItem(ui->mrc, 10, "Start at:");
	ui->mstop   = CreateTextItem(ui->mrc, 10, "Stop at:");
	ui->mlength = CreateTextItem(ui->mrc,  6, "# of bins");
        
        ui->sel = CreateGraphSetSelector(rc, "Sampling set", LIST_TYPE_SINGLE);
        SetSensitive(ui->sel->frame, FALSE);
    }

    return (void *) ui;
}

static void histo_free_cb(void *tddata)
{
    Histo_pars *pars = (Histo_pars *) tddata;
    if (pars) {
        xfree(pars->bins);
        xfree(pars);
    }
}

static void *histo_get_cb(void *gui)
{
    Histo_ui *ui = (Histo_ui *) gui;
    Histo_pars *pars;
    int error = FALSE;
    
    pars = xmalloc(sizeof(Histo_pars));
    if (pars) {
        pars->cumulative = GetToggleButtonState(ui->cumulative);
        pars->normalize  = GetToggleButtonState(ui->normalize);
        pars->sampling   = GetOptionChoice(ui->sampling);
        pars->bins       = NULL;
        pars->nbins      = 0;
        
        if (pars->sampling == SAMPLING_SET) {
            Quark *psampl;
            int res;
            
            res = GetSingleStorageChoice(ui->sel->set_sel, &psampl);
            if (res != RETURN_SUCCESS) {
                errmsg("Please select a single sampling set");
                error = TRUE;
            } else {
                pars->nbins = set_get_length(psampl) - 1;
                pars->bins = copy_data_column_simple(
                    set_get_col(psampl, DATA_X), pars->nbins + 1);
            }
        } else {
            double start, stop;
            if (xv_evalexpr(ui->mstart, &start)     != RETURN_SUCCESS ||
                xv_evalexpr(ui->mstop,  &stop)      != RETURN_SUCCESS ||
                xv_evalexpri(ui->mlength, &pars->nbins) != RETURN_SUCCESS ) {
                 errmsg("Can't parse mesh settings");
                 error = TRUE;
            } else {
                pars->bins = allocate_mesh(start, stop, pars->nbins + 1);
            }
        }
        if (pars->bins == NULL) {
	    errmsg("Can't allocate mesh array");
            error = TRUE;
        }
    }
    
    if (error) {
        histo_free_cb(pars);
        return NULL;
    } else {
        return (void *) pars;
    }
}

static int histo_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Histo_pars *pars = (Histo_pars *) tddata;

    res = do_histo(psrc, pdest,
        pars->bins, pars->nbins, pars->cumulative, pars->normalize);

    if (res != RETURN_SUCCESS) {
	errmsg("Error in do_histo()");
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

void create_histo_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = histo_build_cb;
        cbs.get_cb   = histo_get_cb;
        cbs.free_cb  = histo_free_cb;
        cbs.run_cb   = histo_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Histograms", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* FFTs */

typedef struct {
    Widget inverse;
    OptionStructure *xscale;
    OptionStructure *norm;
    Widget complexin;
    Widget dcdump;
    SpinStructure *oversampling;
    Widget round2n;
    OptionStructure *window;
    SpinStructure *winpar;
    Widget halflen;
    OptionStructure *output;
} Four_ui;

typedef struct {
    int invflag;
    int xscale;
    int norm;

    int complexin;
    int dcdump;
    double oversampling;

    int round2n;
    int window;
    double beta;
    
    int halflen;
    int output;
} Four_pars;

static void toggle_inverse_cb(Widget but, int onoff, void *data)
{
    Four_ui *ui = (Four_ui *) data;
    if (onoff) {
        SetToggleButtonState(ui->halflen, FALSE);
        SetSensitive(ui->halflen, FALSE);
        
        SetToggleButtonState(ui->dcdump, FALSE);
        SetSensitive(ui->dcdump, FALSE);

        SetOptionChoice(ui->window, FFT_WINDOW_NONE);
        SetSensitive(ui->window->menu, FALSE);
    } else {
        SetToggleButtonState(ui->halflen, TRUE);
        SetSensitive(ui->halflen, TRUE);
        
        SetSensitive(ui->dcdump, TRUE);

        SetSensitive(ui->window->menu, TRUE);
    }
}

static void toggle_complex_cb(Widget but, int onoff, void *data)
{
    Four_ui *ui = (Four_ui *) data;
    if (onoff) {
        SetToggleButtonState(ui->halflen, FALSE);
        SetSensitive(ui->halflen, FALSE);
    } else {
        SetToggleButtonState(ui->halflen, TRUE);
        SetSensitive(ui->halflen, TRUE);
    }
}

static void option_window_cb(OptionStructure *opt, int value, void *data)
{
    Four_ui *ui = (Four_ui *) data;
#ifdef HAVE_GSL
    SetSensitive(ui->winpar->rc, value == FFT_WINDOW_KAISER);
#endif
}

static void *fourier_build_cb(TransformStructure *tdialog)
{
    Four_ui *ui;

    ui = xmalloc(sizeof(Four_ui));
    if (ui) {
        Widget rc, fr, rc1, rc2;
        OptionItem window_opitems[] = {
            {FFT_WINDOW_NONE,       "None (Rectangular)"},
            {FFT_WINDOW_TRIANGULAR, "Triangular"        },
            {FFT_WINDOW_PARZEN,     "Parzen"            },
            {FFT_WINDOW_WELCH,      "Welch"             },
            {FFT_WINDOW_HANNING,    "Hanning"           },
            {FFT_WINDOW_HAMMING,    "Hamming"           },
            {FFT_WINDOW_FLATTOP,    "Flat top"          },
            {FFT_WINDOW_BLACKMAN,   "Blackman"          },
#ifdef HAVE_GSL
            {FFT_WINDOW_KAISER,     "Kaiser"            }
#endif
        };
        OptionItem output_opitems[] = {
            {FFT_OUTPUT_MAGNITUDE, "Magnitude"       },
            {FFT_OUTPUT_PHASE,     "Phase"           },
            {FFT_OUTPUT_RE,        "Real part"       },
            {FFT_OUTPUT_IM,        "Imaginary part"  },
            {FFT_OUTPUT_REIM,      "Complex"         },
            {FFT_OUTPUT_APHI,      "Complex (A, Phi)"}
        };
        OptionItem xscale_opitems[] = {
            {FFT_XSCALE_INDEX, "Index"         },
            {FFT_XSCALE_NU,    "Frequency"     },
            {FFT_XSCALE_OMEGA, "Ang. frequency"}
        };
        OptionItem norm_opitems[] = {
            {FFT_NORM_NONE,      "None"     },
            {FFT_NORM_SYMMETRIC, "Symmetric"},
            {FFT_NORM_FORWARD,   "Forward"  },
            {FFT_NORM_BACKWARD,  "Backward" }
        };
        
	rc = CreateVContainer(tdialog->frame);

        fr = CreateFrame(rc, "General");
	rc1 = CreateVContainer(fr);
	ui->inverse = CreateToggleButton(rc1, "Perform backward transform");
        AddToggleButtonCB(ui->inverse, toggle_inverse_cb, (void *) ui);
	rc2 = CreateHContainer(rc1);
	ui->xscale = CreateOptionChoice(rc2, "X scale:", 0, 3, xscale_opitems);
	ui->norm = CreateOptionChoice(rc2, "Normalize:", 0, 4, norm_opitems);
        
        fr = CreateFrame(rc, "Input");
	rc1 = CreateVContainer(fr);
	ui->complexin = CreateToggleButton(rc1, "Complex data");
        AddToggleButtonCB(ui->complexin, toggle_complex_cb, (void *) ui);
	ui->dcdump = CreateToggleButton(rc1, "Dump DC component");
	rc2 = CreateHContainer(rc1);
	ui->window = CreateOptionChoice(rc2,
            "Apply window:", 0, sizeof(window_opitems)/sizeof(OptionItem),
            window_opitems);
        AddOptionChoiceCB(ui->window, option_window_cb, (void *) ui);
        ui->winpar = CreateSpinChoice(rc2,
            "Parameter", 2, SPIN_TYPE_FLOAT, 0.0, 99.0, 1.0);
	rc2 = CreateHContainer(rc1);
        ui->oversampling = CreateSpinChoice(rc2,
            "Zero padding", 2, SPIN_TYPE_FLOAT, 1.0, 99.0, 1.0);
	ui->round2n = CreateToggleButton(rc2, "Round to 2^N");

        fr = CreateFrame(rc, "Output");
	rc1 = CreateHContainer(fr);
        ui->output = CreateOptionChoice(rc1,
            "Load:", 0, 6, output_opitems);
	ui->halflen = CreateToggleButton(rc1, "Half length");
        
        /* Default values */
        SetOptionChoice(ui->xscale, FFT_XSCALE_NU);
        SetOptionChoice(ui->norm, FFT_NORM_FORWARD);
        SetSpinChoice(ui->winpar, 1.0);
        SetSensitive(ui->winpar->rc, FALSE);
        SetToggleButtonState(ui->halflen, TRUE);
        SetSpinChoice(ui->oversampling, 1.0);
#ifndef HAVE_FFTW
        SetToggleButtonState(ui->round2n, TRUE);
#endif
    }

    return (void *) ui;
}

static void *fourier_get_cb(void *gui)
{
    Four_ui *ui = (Four_ui *) gui;
    Four_pars *pars;

    pars = xmalloc(sizeof(Four_pars));
    if (pars) {
        pars->invflag      = GetToggleButtonState(ui->inverse);
        pars->xscale       = GetOptionChoice(ui->xscale);
        pars->norm         = GetOptionChoice(ui->norm);

        pars->complexin    = GetToggleButtonState(ui->complexin);
        pars->dcdump       = GetToggleButtonState(ui->dcdump);
        pars->oversampling = GetSpinChoice(ui->oversampling);
        pars->round2n      = GetToggleButtonState(ui->round2n);
        pars->window       = GetOptionChoice(ui->window);
        pars->beta         = GetSpinChoice(ui->winpar);

        pars->halflen      = GetToggleButtonState(ui->halflen);
        pars->output       = GetOptionChoice(ui->output);
    }
    
    return (void *) pars;
}
    
static int fourier_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Four_pars *pars = (Four_pars *) tddata;
    
    res = do_fourier(psrc, pdest,
        pars->invflag, pars->xscale, pars->norm, pars->complexin, pars->dcdump, pars->oversampling, pars->round2n,
        pars->window, pars->beta, pars->halflen, pars->output);
        
    return res;
}


void create_fourier_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = fourier_build_cb;
        cbs.get_cb   = fourier_get_cb;
        cbs.free_cb  = xfree;
        cbs.run_cb   = fourier_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Fourier transform", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* finite differencing */

typedef struct {
    OptionStructure *type;
    OptionStructure *xplace;
    SpinStructure *period;
} Diff_ui;

typedef struct {
    int type;
    int xplace;
    int period;
} Diff_pars;

#define DIFF_TYPE_PLAIN         0
#define DIFF_TYPE_DERIVATIVE    1

static void *diff_build_cb(TransformStructure *tdialog)
{
    Diff_ui *ui;

    ui = xmalloc(sizeof(Diff_ui));
    if (ui) {
        Widget rc, rc2;
        OptionItem topitems[] = {
            {DIFF_TYPE_PLAIN,      "Plain differences"},
            {DIFF_TYPE_DERIVATIVE, "Derivative"       }
        };
        OptionItem xopitems[] = {
            {DIFF_XPLACE_LEFT,   "Left"  },
            {DIFF_XPLACE_CENTER, "Center"},
            {DIFF_XPLACE_RIGHT,  "Right" }
        };
	
        rc = CreateVContainer(tdialog->frame);
        ui->type   = CreateOptionChoice(rc, "Type:", 0, 2, topitems);
        rc2 = CreateHContainer(rc);
        ui->period = CreateSpinChoice(rc2, "Period", 6, SPIN_TYPE_INT,
            (double) 1, (double) 999999, (double) 1);
        ui->xplace = CreateOptionChoice(rc2, "X placement:", 0, 3, xopitems);
        
        SetSpinChoice(ui->period, (double) 1);
    }

    return (void *) ui;
}

static void *diff_get_cb(void *gui)
{
    Diff_ui *ui = (Diff_ui *) gui;
    Diff_pars *pars;
    
    pars = xmalloc(sizeof(Diff_pars));
    if (pars) {
        pars->type   = GetOptionChoice(ui->type);
        pars->xplace = GetOptionChoice(ui->xplace);
        pars->period = GetSpinChoice(ui->period);
    }
    
    return (void *) pars;
}
    
static int diff_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Diff_pars *pars = (Diff_pars *) tddata;

    res = do_differ(psrc, pdest,
        pars->type == DIFF_TYPE_DERIVATIVE, pars->xplace, pars->period);

    return res;
}

void create_diff_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = diff_build_cb;
        cbs.get_cb   = diff_get_cb;
        cbs.free_cb  = xfree;
        cbs.run_cb   = diff_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Differences", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* running averages */

#define RUN_TYPE_CUSTOM     0
#define RUN_TYPE_AVERAGE    1
#define RUN_TYPE_STDDEV     2
#define RUN_TYPE_MIN        3
#define RUN_TYPE_MAX        4

typedef struct {
    SpinStructure *length;
    TextStructure *formula;
    OptionStructure *xplace;
} Run_ui;

typedef struct {
    int length;
    char *formula;
    int xplace;
} Run_pars;

static void run_type_cb(OptionStructure *opt, int value, void *data)
{
    Run_ui *ui = (Run_ui *) data;
    char *formula;
    
    switch (value) {
    case RUN_TYPE_AVERAGE:
        formula = "AVG($t)";
        break;
    case RUN_TYPE_STDDEV:
        formula = "SD($t)";
        break;
    case RUN_TYPE_MIN:
        formula = "MIN($t)";
        break;
    case RUN_TYPE_MAX:
        formula = "MAX($t)";
        break;
    default:
        formula = NULL;
        break;
    }
    
    if (formula) {
        SetTextString(ui->formula, formula);
        SetTextEditable(ui->formula, FALSE);
    } else {
        SetTextEditable(ui->formula, TRUE);
    }
}

static void *run_build_cb(TransformStructure *tdialog)
{
    Run_ui *ui;

    ui = xmalloc(sizeof(Run_ui));
    if (ui) {
        Widget rc;
        OptionStructure *type;
        OptionItem topitems[] = {
            {RUN_TYPE_CUSTOM,  "Custom"   },
            {RUN_TYPE_AVERAGE, "Average"  },
            {RUN_TYPE_STDDEV,  "Std. dev."},
            {RUN_TYPE_MIN,     "Minimum"  },
            {RUN_TYPE_MAX,     "Maximum"  }
        };
        OptionItem xopitems[] = {
            {RUN_XPLACE_LEFT,    "Left"   },
            {RUN_XPLACE_AVERAGE, "Average"},
            {RUN_XPLACE_RIGHT,   "Right"  }
        };
	
	rc = CreateVContainer(tdialog->frame);
        type = CreateOptionChoice(rc, "Type:", 0, 5, topitems);
        AddOptionChoiceCB(type, run_type_cb, (void *) ui);
	ui->formula = CreateTextInput(rc, "Formula:");
	ui->length = CreateSpinChoice(rc, "Length of sample", 6, SPIN_TYPE_INT,
            (double) 1, (double) 999999, (double) 1);
        ui->xplace = CreateOptionChoice(rc, "X placement:", 0, 3, xopitems);
        
        /* default settings */
        SetSpinChoice(ui->length, 1);
        SetOptionChoice(ui->xplace, RUN_XPLACE_AVERAGE);
    }

    return (void *) ui;
}

static void *run_get_cb(void *gui)
{
    Run_ui *ui = (Run_ui *) gui;
    Run_pars *pars;
    
    pars = xmalloc(sizeof(Run_pars));
    if (pars) {
        pars->length  = (int) GetSpinChoice(ui->length);
        pars->formula = GetTextString(ui->formula);
        pars->xplace  = GetOptionChoice(ui->xplace);
    }
    
    return (void *) pars;
}

static void run_free_cb(void *tddata)
{
    Run_pars *pars = (Run_pars *) tddata;
    if (pars) {
        xfree(pars->formula);
        xfree(pars);
    }
}

static int run_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Run_pars *pars = (Run_pars *) tddata;

    res = do_runavg(psrc, pdest, pars->length, pars->formula, pars->xplace);
    
    return res;
}

void create_run_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = run_build_cb;
        cbs.get_cb   = run_get_cb;
        cbs.free_cb  = run_free_cb;
        cbs.run_cb   = run_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Running properties", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* numerical integration */

typedef struct {
    Widget disponly;
} Int_ui;

typedef struct {
    int disponly;
} Int_pars;


static void *int_build_cb(TransformStructure *tdialog)
{
    Int_ui *ui;

    ui = xmalloc(sizeof(Int_ui));
    if (ui) {
	ui->disponly =
            CreateToggleButton(tdialog->frame, "Display integral value only");
    }

    return (void *) ui;
}

static void *int_get_cb(void *gui)
{
    Int_ui *ui = (Int_ui *) gui;
    Int_pars *pars;
    
    pars = xmalloc(sizeof(Int_pars));
    if (pars) {
        pars->disponly = GetToggleButtonState(ui->disponly);
    }
    
    return (void *) pars;
}

static int int_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    double sum;
    Int_pars *pars = (Int_pars *) tddata;

    res = do_int(psrc, pdest, pars->disponly, &sum);
    if (res == RETURN_SUCCESS) {
        char buf[64];
        sprintf(buf, "Integral of set %s: %g\n", quark_idstr_get(psrc), sum);
        stufftext(buf);
    }
    
    return res;
}

void create_int_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = int_build_cb;
        cbs.get_cb   = int_get_cb;
        cbs.free_cb  = xfree;
        cbs.run_cb   = int_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Integrate", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* linear convolution */

typedef struct {
    GraphSetStructure *convsel;
} Lconv_ui;

typedef struct {
    Quark *pconv;
} Lconv_pars;

static void *lconv_build_cb(TransformStructure *tdialog)
{
    Lconv_ui *ui;

    ui = xmalloc(sizeof(Lconv_ui));
    if (ui) {
	ui->convsel = CreateGraphSetSelector(tdialog->frame,
            "Convolve with:", LIST_TYPE_SINGLE);
    }

    return (void *) ui;
}

static void lconv_free_cb(void *tddata)
{
    Lconv_pars *pars = (Lconv_pars *) tddata;
    if (pars) {
        xfree(pars);
    }
}

static void *lconv_get_cb(void *gui)
{
    Lconv_ui *ui = (Lconv_ui *) gui;
    Lconv_pars *pars;
    
    pars = xmalloc(sizeof(Lconv_pars));
    if (pars) {
        if (GetSingleStorageChoice(ui->convsel->set_sel, &pars->pconv)
            != RETURN_SUCCESS) {
            errmsg("Please select a single set to be convoluted with");
            lconv_free_cb(pars);
            return NULL;
        }
    }
    
    return (void *) pars;
}

static int lconv_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Lconv_pars *pars = (Lconv_pars *) tddata;

    res = do_linearc(psrc, pdest, pars->pconv);
    
    return res;
}

void create_lconv_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = lconv_build_cb;
        cbs.get_cb   = lconv_get_cb;
        cbs.free_cb  = lconv_free_cb;
        cbs.run_cb   = lconv_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Linear convolution", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}


/* cross correlation */

typedef struct {
    Widget autocor;
    GraphSetStructure *corsel;
    SpinStructure *maxlag;
    Widget covar;
} Cross_ui;

typedef struct {
    int autocor;
    Quark *pcor;
    int maxlag;
    int covar;
} Cross_pars;

static void xcor_self_toggle(Widget but, int onoff, void *data)
{
    Cross_ui *ui = (Cross_ui *) data;
    
    SetSensitive(ui->corsel->frame, !onoff);
}

static void *cross_build_cb(TransformStructure *tdialog)
{
    Cross_ui *ui;

    ui = xmalloc(sizeof(Cross_ui));
    if (ui) {
	Widget rc;
        rc = CreateVContainer(tdialog->frame);
	ui->autocor = CreateToggleButton(rc, "Auto-correlation");
        AddToggleButtonCB(ui->autocor, xcor_self_toggle, (void *) ui);
        ui->corsel = CreateGraphSetSelector(rc,
            "Correlate with:", LIST_TYPE_SINGLE);
	ui->maxlag = CreateSpinChoice(rc, "Maximum lag:", 6, SPIN_TYPE_INT,
            (double) 1, (double) 999999, (double) 1);
        ui->covar = CreateToggleButton(rc, "Calculate covariance");
        
        /* default settings */
        SetSpinChoice(ui->maxlag, (double) 1);
    }

    return (void *) ui;
}

static void cross_free_cb(void *tddata)
{
    Cross_pars *pars = (Cross_pars *) tddata;
    if (pars) {
        xfree(pars);
    }
}

static void *cross_get_cb(void *gui)
{
    Cross_ui *ui = (Cross_ui *) gui;
    Cross_pars *pars;
    
    pars = xmalloc(sizeof(Cross_pars));
    if (pars) {
        pars->maxlag  = (int) GetSpinChoice(ui->maxlag);
        pars->autocor = GetToggleButtonState(ui->autocor);
        pars->covar   = GetToggleButtonState(ui->covar);

        if (!pars->autocor &&
            GetSingleStorageChoice(ui->corsel->set_sel, &pars->pcor)
            != RETURN_SUCCESS) {
            errmsg("Please select a single set to be correlated with");
            cross_free_cb(pars);
            return NULL;
        }
    }
    
    return (void *) pars;
}

static int cross_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Quark *pcor;
    Cross_pars *pars = (Cross_pars *) tddata;

    if (pars->autocor) {
        pcor = psrc;
    } else {
        pcor = pars->pcor;
    }
    res = do_xcor(psrc, pdest, pcor, pars->maxlag, pars->covar);
    
    return res;
}

void create_xcor_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = cross_build_cb;
        cbs.get_cb   = cross_get_cb;
        cbs.free_cb  = cross_free_cb;
        cbs.run_cb   = cross_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Correlation/covariance", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}


/* sample a set */

typedef struct {
    TextStructure *formula;
} Samp_ui;

typedef struct {
    char *formula;
} Samp_pars;

static void *samp_build_cb(TransformStructure *tdialog)
{
    Samp_ui *ui;

    ui = xmalloc(sizeof(Samp_ui));
    if (ui) {
	ui->formula = CreateTextInput(tdialog->frame, "Logical expression:");
    }

    return (void *) ui;
}

static void samp_free_cb(void *tddata)
{
    Samp_pars *pars = (Samp_pars *) tddata;
    if (pars) {
        xfree(pars->formula);
        xfree(pars);
    }
}

static void *samp_get_cb(void *gui)
{
    Samp_ui *ui = (Samp_ui *) gui;
    Samp_pars *pars;
    
    pars = xmalloc(sizeof(Samp_pars));
    if (pars) {
        pars->formula = GetTextString(ui->formula);
    }
    
    return (void *) pars;
}

static int samp_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Samp_pars *pars = (Samp_pars *) tddata;

    res = do_sample(psrc, pdest, pars->formula);
    
    return res;
}

void create_samp_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = samp_build_cb;
        cbs.get_cb   = samp_get_cb;
        cbs.free_cb  = samp_free_cb;
        cbs.run_cb   = samp_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Sample points", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}

/* Prune data */

/* for data pruning */
#define PRUNE_TYPE_POINTS       0
#define PRUNE_TYPE_INTERP       1

#define PRUNE_AREA_RECTANGLE    0
#define PRUNE_AREA_ELLIPSE      1

#define PRUNE_DELTA_ABSOLUTE    0
#define PRUNE_DELTA_RELATIVE    1

typedef struct {
    OptionStructure *type;
    OptionStructure *area;
    Widget dx;
    OptionStructure *dxtype;
    Widget dy;
    OptionStructure *dytype;
} Prune_ui;

typedef struct {
    int type;
    int area;
    double dx;
    int dxtype;
    double dy;
    int dytype;
} Prune_pars;

static void *prune_build_cb(TransformStructure *tdialog)
{
    Prune_ui *ui;

    ui = xmalloc(sizeof(Prune_ui));
    if (ui) {
	Widget rc, rc2;
        OptionItem topitems[] = {
            {PRUNE_TYPE_POINTS, "Points"       },
            {PRUNE_TYPE_INTERP, "Interpolation"}
        };
        OptionItem aopitems[] = {
            {PRUNE_AREA_RECTANGLE, "Rectangle"},
            {PRUNE_AREA_ELLIPSE,   "Ellipse"  }
        };
        OptionItem dopitems[] = {
            {PRUNE_DELTA_ABSOLUTE, "Absolute"},
            {PRUNE_DELTA_RELATIVE, "Relative"}
        };
        
	rc = CreateVContainer(tdialog->frame);

        ui->type = CreateOptionChoice(rc, "Prune type:", 0, 2, topitems);
        ui->area = CreateOptionChoice(rc, "Prune area:", 0, 2, aopitems);

	rc2 = CreateHContainer(rc);
	ui->dx = CreateTextItem(rc2, 16, "DX:");
	ui->dxtype = CreateOptionChoice(rc2, "Type:", 0, 2, dopitems);
	
	rc2 = CreateHContainer(rc);
	ui->dy = CreateTextItem(rc2, 16, "DY:");
	ui->dytype = CreateOptionChoice(rc2, "Type:", 0, 2, dopitems);
    }

    return (void *) ui;
}

static void *prune_get_cb(void *gui)
{
    Prune_ui *ui = (Prune_ui *) gui;
    Prune_pars *pars;
    
    pars = xmalloc(sizeof(Prune_pars));
    if (pars) {
        pars->type   = GetOptionChoice(ui->type);
        pars->area   = GetOptionChoice(ui->area);
        pars->dxtype = GetOptionChoice(ui->dxtype);
        pars->dytype = GetOptionChoice(ui->dytype);

        if (xv_evalexpr(ui->dx, &pars->dx) != RETURN_SUCCESS) {
            errmsg("Can't parse value for X");
            xfree(pars);
            return NULL;
        }
        if (xv_evalexpr(ui->dy, &pars->dy) != RETURN_SUCCESS) {
            errmsg("Can't parse value for Y");
            xfree(pars);
            return NULL;
        }
    }
    
    return (void *) pars;
}

static int prune_run_cb(Quark *psrc, Quark *pdest, void *tddata)
{
    int res;
    Prune_pars *pars = (Prune_pars *) tddata;

    res = do_prune(psrc, pdest,
        pars->type == PRUNE_TYPE_INTERP, pars->area == PRUNE_AREA_ELLIPSE,
        pars->dx, pars->dxtype, pars->dy, pars->dytype);
    
    return res;
}

void create_prune_frame(Widget but, void *data)
{
    static TransformStructure *tdialog = NULL;

    if (!tdialog) {
        TD_CBProcs cbs;
        cbs.build_cb = prune_build_cb;
        cbs.get_cb   = prune_get_cb;
        cbs.free_cb  = xfree;
        cbs.run_cb   = prune_run_cb;
        
        tdialog = CreateTransformDialogForm(app_shell,
            "Prune data", LIST_TYPE_MULTIPLE, TRUE, &cbs);
    }
    
    RaiseTransformationDialog(tdialog);
}



typedef struct _Featext_ui {
    Widget top;
    GraphSetStructure *src;
    SSDColStructure *dst;
    TextStructure *formula;
} Featext_ui;

static int do_fext_proc(void *data)
{
    char *formula;
    int nsrc, ndst;
    Quark **srcsets, *dst_ssd;
    DArray *da;
    int retval;
    int *dst_cols;
    unsigned int ncols, nrows, dst_col = COL_NONE;

    Featext_ui *ui = (Featext_ui *) data;

    nsrc = GetStorageChoices(ui->src->set_sel, &srcsets);
    if (nsrc < 1) {
        errmsg("No source sets selected");
        return RETURN_FAILURE;
    }

    ndst = GetSSDColChoices(ui->dst, &dst_ssd, &dst_cols);
    if (ndst < 0) {
        errmsg("No destination SSD selected");
        return RETURN_FAILURE;
    } else
    if (ndst > 1) {
        xfree(dst_cols);
        errmsg("Please select a single or none destination column");
        return RETURN_FAILURE;
    }

    ncols = ssd_get_ncols(dst_ssd);
    nrows = ssd_get_nrows(dst_ssd);

    if (ndst == 0) {
        ssd_add_col(dst_ssd, FFORMAT_NUMBER);
        dst_col = ncols;
        
        UpdateStorageChoice(ui->dst->ssd_sel);
        SelectListChoice(ui->dst->col_sel, dst_col);
    } else if (ndst == 1) {
        dst_col = dst_cols[0];
        xfree(dst_cols);
    }

    if (nrows != nsrc) {
        if (yesno("Destination data will be resized to new length. Are you sure?",
            NULL, NULL, NULL)) {
            ssd_set_nrows(dst_ssd, nsrc);
        } else {
            xfree(srcsets);
            return RETURN_FAILURE;
        }
    }
    
    formula = GetTextString(ui->formula);
    
    da = featext(srcsets, nsrc, formula);

    if (da && ssd_set_darray(dst_ssd, dst_col, da) == RETURN_SUCCESS) {
        ssd_set_col_label(dst_ssd, dst_col, formula);
        
        snapshot_and_update(gapp->gp, TRUE);

        retval = RETURN_SUCCESS;
    } else {
        retval = RETURN_FAILURE;
    }
    
    xfree(formula);
    xfree(srcsets);
    
    darray_free(da);
    
    return retval;
}

void create_featext_frame(Widget but, void *data)
{
    static Featext_ui *ui = NULL;
 
    set_wait_cursor();
    
    if (!ui) {
        Widget grid;

        ui = xmalloc(sizeof(Featext_ui));
        if (!ui) {
            unset_wait_cursor();
            return;
        }
        
        ui->top = CreateDialogForm(app_shell, "Feature extraction");
        grid = CreateGrid(ui->top, 2, 1);
        AddDialogFormChild(ui->top, grid);
	ui->src = CreateGraphSetSelector(grid,
            "Source group", LIST_TYPE_MULTIPLE);
	ui->dst = CreateSSDColSelector(grid,
            "Destination", LIST_TYPE_SINGLE);

        PlaceGridChild(grid, ui->src->frame, 0, 0);
        PlaceGridChild(grid, ui->dst->frame, 1, 0);

	ui->formula = CreateTextInput(ui->top, "Formula:");

	CreateAACDialog(ui->top, ui->formula->form, do_fext_proc, ui);
    }
    
    RaiseWindow(GetParent(ui->top));
    
    unset_wait_cursor();
}


typedef struct _Cumulative_ui {
    Widget top;
    SSDColStructure *src;
    SSDColStructure *dst;
    OptionStructure *type;
} Cumulative_ui;


static int fill_darray_from_column(const Quark *ssd, unsigned int ncol,
    DArray *darray)
{
    ss_column *col = ssd_get_col(ssd, ncol);
    if (!col || col->format != FFORMAT_NUMBER) {
        return RETURN_FAILURE;
    } else {
        darray->allocated = FALSE;
        darray->size = ssd_get_nrows(ssd);
        darray->x = col->data;
        return RETURN_SUCCESS;
    }
}

static int do_cumulative_proc(void *data)
{
    Cumulative_ui *ui = (Cumulative_ui *) data;
    int nsrc, ndst;
    Quark *src_ssd, *dst_ssd;
    int *src_cols, *dst_cols;
    unsigned int i, nrows, ncols, dst_col = COL_NONE;
    DArray *src_arrays, dst_array;
    int type;

    nsrc = GetSSDColChoices(ui->src, &src_ssd, &src_cols);
    if (nsrc < 1) {
        errmsg("No source columns selected");
        return RETURN_FAILURE;
    }

    ndst = GetSSDColChoices(ui->dst, &dst_ssd, &dst_cols);
    if (ndst < 0) {
        errmsg("No destination SSD selected");
        xfree(src_cols);
        return RETURN_FAILURE;
    } else
    if (ndst > 1) {
        xfree(src_cols);
        xfree(dst_cols);
        errmsg("Please select a single or none destination column");
        return RETURN_FAILURE;
    }
    
    type = GetOptionChoice(ui->type);
    
    nrows = ssd_get_nrows(src_ssd);
    ncols = ssd_get_ncols(dst_ssd);
    if (ncols && nrows != ssd_get_nrows(dst_ssd)) {
        /* there is a data in dst_ssd which will be reallocated */
        if (yesno("Destination data will be resized to new length. Are you sure?",
                NULL, NULL, NULL)) {
            ssd_set_nrows(dst_ssd, nrows);
        } else {
            xfree(src_cols);
            xfree(dst_cols);
            return RETURN_FAILURE;
        }
    }
    
    if (ndst == 0) {
        ssd_add_col(dst_ssd, FFORMAT_NUMBER);
        dst_col = ncols;
        
        UpdateStorageChoice(ui->dst->ssd_sel);
        SelectListChoice(ui->dst->col_sel, dst_col);
    } else if (ndst == 1) {
        dst_col = dst_cols[0];
        xfree(dst_cols);
    }
    
    src_arrays = xmalloc(sizeof(DArray)*nsrc);
    for (i = 0; i < nsrc; i++) {
        fill_darray_from_column(src_ssd, src_cols[i], &src_arrays[i]);
    }
    xfree(src_cols);

    fill_darray_from_column(dst_ssd, dst_col, &dst_array);

    num_cumulative(src_arrays, nsrc, &dst_array, type);
    
    quark_dirtystate_set(dst_ssd, TRUE);
    
    xfree(src_arrays);

    snapshot_and_update(gapp->gp, TRUE);
  
    return RETURN_SUCCESS;
}

void create_cumulative_frame(Widget but, void *data)
{
    static Cumulative_ui *ui = NULL;
 
    set_wait_cursor();
    
    if (!ui) {
        Widget grid, rc;

        ui = xmalloc(sizeof(Cumulative_ui));
        if (!ui) {
            unset_wait_cursor();
            return;
        }
        
        ui->top = CreateDialogForm(app_shell, "Cumulative properties");
        grid = CreateGrid(ui->top, 2, 1);
        AddDialogFormChild(ui->top, grid);
	ui->src = CreateSSDColSelector(grid,
            "Source group", LIST_TYPE_MULTIPLE);
	ui->dst = CreateSSDColSelector(grid,
            "Destination", LIST_TYPE_SINGLE);
        PlaceGridChild(grid, ui->src->frame, 0, 0);
        PlaceGridChild(grid, ui->dst->frame, 1, 0);
        rc = CreateHContainer(ui->top);
        ui->type = CreateOptionChoiceVA(rc, "Property type:",
            "Average", RUN_AVG,
            "Std.dev", RUN_STD,
            "Minimum", RUN_MIN,
            "Maximum", RUN_MAX,
            NULL);

	CreateAACDialog(ui->top, rc, do_cumulative_proc, (void *) ui);
    }
    
    RaiseWindow(GetParent(ui->top));
    
    unset_wait_cursor();
}
