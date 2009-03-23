/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2004 Grace Development Team
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
 * Page/Device setup
 */

#include <config.h>

#include <stdio.h>

#include "globals.h"
#include "utils.h"

#include <Xm/Xm.h>
#include "motifinc.h"
#include "xprotos.h"

#define canvas grace_get_canvas(gapp->grace)

typedef struct {
    int             ndest;
    Widget          top;
    OptionStructure **opts;
} DestSetupUI;

typedef struct {
    Widget          top;

    Widget          psetup_rc;
    Widget          device_opts;
    Widget          printto;
    OptionStructure *destination;
    Widget          print_string;
    Widget          rc_filesel;
    Widget          rc_printsel;
    Widget          dest_opts;
    Widget          printfile;
    Widget          pdev_rc;
    OptionStructure *devices;
    Widget          output_frame;
    Widget          page_frame;

    OptionStructure *page_orient;
    OptionStructure *page_format;

    Widget          page_x;
    Widget          page_y;
    OptionStructure *page_size_unit;
    Widget          dev_res;

    Widget          autocrop;

    OptionStructure *fontrast;
    OptionStructure *color_trans;
    
    DestSetupUI     *destopts;

    int             current_page_units;
} PrintUI;

static PrintUI *pui = NULL;

static void do_pr_toggle(Widget tbut, int onoff, void *data);
static void do_format_toggle(OptionStructure *opt, int value, void *data);
static void do_orient_toggle(OptionStructure *opt, int value, void *data);

static int set_printer_proc(void *data);
void create_printfiles_popup(Widget but, void *data);
void create_devopts_popup(Widget but, void *data);
void create_destopts_popup(Widget but, void *data);

static void do_device_toggle(OptionStructure *opt, int value, void *data);
static void do_units_toggle(OptionStructure *opt, int value, void *data);
static void update_printer_setup(PrintUI *ui, int device_id);
static void update_device_setup(PrintUI *ui, int device_id);

static void do_print_cb(Widget but, void *data);

void create_printer_setup(Widget but, void *data)
{
    int device;
    
    set_wait_cursor();
    
    if (data == NULL) {
        device = gapp->rt->hdevice;
    } else {
        device = *((int *) data);
    }
    
    if (pui == NULL) {
        int i, j, ndev;
        Widget rc, rc1, fr, wbut;
        Widget menubar, menupane;
        OptionItem *options;

	pui = xmalloc(sizeof(PrintUI));
        memset(pui, 0, sizeof(PrintUI));
        
        pui->destopts = xcalloc(gapp->rt->num_print_dests, sizeof(DestSetupUI));
        
        pui->top = CreateDialogForm(app_shell, "Device setup");
        SetDialogFormResizable(pui->top, TRUE);

        menubar = CreateMenuBar(pui->top);
        AddDialogFormChild(pui->top, menubar);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Print", 'P', do_print_cb, NULL);
        CreateMenuSeparator(menupane);
        CreateMenuCloseButton(menupane, pui->top);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuHelpButton(menupane, "On device setup", 'd',
            pui->top, "doc/UsersGuide.html#print-setup");

        ManageChild(menubar);

	pui->psetup_rc = CreateVContainer(pui->top);

        fr = CreateFrame(pui->psetup_rc, "Device setup");
        rc1 = CreateVContainer(fr);
	pui->pdev_rc = CreateHContainer(rc1);

	ndev = number_of_devices(canvas);
        options = xmalloc(ndev*sizeof(OptionItem));
        for (i = 0, j = 0; i < ndev; i++) {
            if (!device_is_aux(canvas, i)) {
                options[j].value = i;
                options[j].label = get_device_name(canvas, i);
                j++;
            }
        }
        pui->devices =
            CreateOptionChoice(pui->pdev_rc, "Device: ", 1, j, options);
	AddOptionChoiceCB(pui->devices, do_device_toggle, pui);
        xfree(options);
        
        pui->device_opts = CreateButton(pui->pdev_rc, "Device options...");
	AddButtonCB(pui->device_opts, create_devopts_popup, pui);
        
        pui->output_frame = CreateFrame(pui->psetup_rc, "Output");
        rc1 = CreateVContainer(pui->output_frame);
	pui->printto = CreateToggleButton(rc1, "Print to file");
        AddToggleButtonCB(pui->printto, do_pr_toggle, pui);

	pui->rc_printsel = CreateHContainer(rc1);
        if (gapp->rt->use_cups) {
            options = xmalloc(gapp->rt->num_print_dests*sizeof(OptionItem));

            for (i = 0; i < gapp->rt->num_print_dests; i++) {
                options[i].value = i;
                options[i].label = gapp->rt->print_dests[i].printer;
            }
	    pui->destination = CreateOptionChoice(pui->rc_printsel, "Destination:",
                0, gapp->rt->num_print_dests, options);

            xfree(options);
            
            pui->dest_opts = CreateButton(pui->rc_printsel, "Setup...");
	    AddButtonCB(pui->dest_opts, create_destopts_popup, pui);
        
        } else {
	    pui->print_string = CreateTextItem(pui->rc_printsel,
                25, "Print command:");
        }

	pui->rc_filesel = CreateHContainer(rc1);
	pui->printfile = CreateTextItem(pui->rc_filesel, 20, "File name:");
	wbut = CreateButton(pui->rc_filesel, "Browse...");
	AddButtonCB(wbut, create_printfiles_popup, pui);

	
        pui->page_frame = CreateFrame(pui->psetup_rc, "Page");
        rc1 = CreateVContainer(pui->page_frame);
        
	rc = CreateHContainer(rc1);

        pui->page_orient = CreatePaperOrientationChoice(rc, "Orientation:");
	AddOptionChoiceCB(pui->page_orient, do_orient_toggle, pui);


        pui->page_format = CreatePaperFormatChoice(rc, "Size:");
	AddOptionChoiceCB(pui->page_format, do_format_toggle, pui);

	rc = CreateHContainer(rc1);
        pui->page_x = CreateTextItem(rc, 7, "Dimensions:");
        pui->page_y = CreateTextItem(rc, 7, "x ");
        options = xmalloc(3*sizeof(OptionItem));
        options[0].value = 0;
        options[0].label = "pix";
        options[1].value = 1;
        options[1].label = "in";
        options[2].value = 2;
        options[2].label = "cm";
        pui->page_size_unit =
            CreateOptionChoice(rc, " ", 1, 3, options);
	AddOptionChoiceCB(pui->page_size_unit, do_units_toggle, pui);
        xfree(options);
        SetOptionChoice(pui->page_size_unit, pui->current_page_units);

        pui->dev_res = CreateTextItem(rc1, 4, "Resolution (dpi):");

	pui->autocrop = CreateToggleButton(rc1, "Auto crop");

        fr = CreateFrame(pui->psetup_rc, "Fonts & Colors");
        rc1 = CreateVContainer(fr);

        options = xmalloc(5*sizeof(OptionItem));
        options[0].value = FONT_RASTER_DEVICE;
        options[0].label = "Device";
        options[1].value = FONT_RASTER_MONO;
        options[1].label = "Mono";
        options[2].value = FONT_RASTER_AA_LOW;
        options[2].label = "AA-low";
        options[3].value = FONT_RASTER_AA_HIGH;
        options[3].label = "AA-high";
        options[4].value = FONT_RASTER_AA_SMART;
        options[4].label = "AA-smart";
	pui->fontrast = CreateOptionChoice(rc1,
            "Font rastering:", 1, 5, options);
        xfree(options);

        options = xmalloc(6*sizeof(OptionItem));
        options[0].value = COLOR_TRANS_NONE;
        options[0].label = "None";
        options[1].value = COLOR_TRANS_GREYSCALE;
        options[1].label = "Grayscale";
        options[2].value = COLOR_TRANS_BW;
        options[2].label = "B/W";
        options[3].value = COLOR_TRANS_NEGATIVE;
        options[3].label = "Negative";
        options[4].value = COLOR_TRANS_REVERSE;
        options[4].label = "Reverse";
        options[5].value = COLOR_TRANS_SRGB;
        options[5].label = "sRGB";
	pui->color_trans = CreateOptionChoice(rc1,
            "Color transform:", 1, 6, options);
        xfree(options);
        
	CreateAACDialog(pui->top, pui->psetup_rc, set_printer_proc, pui);
    }
    
    update_printer_setup(pui, device);
    
    RaiseWindow(GetParent(pui->top));
    unset_wait_cursor();
}

static void update_printer_setup(PrintUI *ui, int device_id)
{
    if (ui) {
        SetOptionChoice(ui->devices, device_id);
        update_device_setup(ui, device_id);
    }
}

static void update_device_setup(PrintUI *ui, int device_id)
{
    if (ui) {
        char buf[GR_MAXPATHLEN], *bufptr;
        int page_units;
        double page_x, page_y;
        PageFormat pf;
        
        dev_gui_setup *setup_data;

        Page_geometry pg;
        Device_entry *dev;

	dev = get_device_props(canvas, device_id);
        pg = dev->pg;
        	
        setup_data = (dev_gui_setup *) device_get_udata(canvas, device_id);
        
        if (setup_data == NULL) {
            SetSensitive(ui->device_opts, False);
        } else {
            SetSensitive(ui->device_opts, True);
        }

        if (string_is_empty(gapp->rt->print_file)) {
            char *docname = gproject_get_docname(gapp->gp);
            if (!docname) {
                docname = NONAME;
            }
            strcpy(gapp->rt->print_file, mybasename(docname)); 
        }
        
        /* Replace existing filename extension */
        bufptr = strrchr(gapp->rt->print_file, '.');
        if (bufptr) {
            *(bufptr + 1) = '\0';
        } else {
            strcat(gapp->rt->print_file, ".");
        }
        if (dev->fext) {
            strcat(gapp->rt->print_file, dev->fext);
        }
        
        if (gapp->rt->use_cups) {
            SetOptionChoice(ui->destination, get_print_dest(gapp));
        } else {
            xv_setstr(ui->print_string, get_print_cmd(gapp));
        }
        xv_setstr(ui->printfile, gapp->rt->print_file);
        
        switch (dev->type) {
        case DEVICE_TERM:
            UnmanageChild(ui->output_frame);
            UnmanageChild(ui->page_frame);
            break;
        case DEVICE_FILE:
            ManageChild(ui->output_frame);
            ManageChild(ui->page_frame);
            SetToggleButtonState(ui->printto, TRUE);
            SetSensitive(ui->printto, False);
            SetSensitive(ui->rc_printsel, False);
            SetSensitive(ui->rc_filesel, True);
            break;
        case DEVICE_PRINT:
            ManageChild(ui->output_frame);
            ManageChild(ui->page_frame);
            SetToggleButtonState(ui->printto, get_ptofile(gapp));
            SetSensitive(ui->printto, True);
            if (get_ptofile(gapp) == TRUE) {
                SetSensitive(ui->rc_filesel, True);
                SetSensitive(ui->rc_printsel, False);
            } else {
                SetSensitive(ui->rc_filesel, False);
                SetSensitive(ui->rc_printsel, True);
            }
            break;
        }
        
        SetOptionChoice(ui->page_orient, pg.width < pg.height ?
            PAGE_ORIENT_PORTRAIT : PAGE_ORIENT_LANDSCAPE);
        
        pf = get_page_format(canvas, device_id);
        SetOptionChoice(ui->page_format, pf); 
        if (pf == PAGE_FORMAT_CUSTOM) {
            SetSensitive(ui->page_x, True);
            SetSensitive(ui->page_y, True);
            SetSensitive(ui->page_orient->menu, False);
        } else {
            SetSensitive(ui->page_x, False);
            SetSensitive(ui->page_y, False);
            SetSensitive(ui->page_orient->menu, True);
        }
        
        sprintf (buf, "%.0f", pg.dpi); 
        xv_setstr(ui->dev_res, buf);

        if (dev->type == DEVICE_TERM || dev->type == DEVICE_PRINT) {
            SetToggleButtonState(ui->autocrop, FALSE);
            SetSensitive(ui->autocrop, FALSE);
        } else {
            SetToggleButtonState(ui->autocrop, dev->autocrop);
            SetSensitive(ui->autocrop, TRUE);
        }
        
        page_units = GetOptionChoice(ui->page_size_unit);
        
        switch (page_units) {
        case 0:     /* pixels */
            page_x = (float) pg.width;
            page_y = (float) pg.height;
            break;
        case 1:      /* inches */
            page_x = (float) pg.width / pg.dpi;
            page_y = (float) pg.height / pg.dpi;
            break;
        case 2:      /* cm */ 
            page_x = (float) CM_PER_INCH * pg.width / pg.dpi;
            page_y = (float) CM_PER_INCH * pg.height / pg.dpi;
            break;
        default:
            errmsg("Internal error");
            return;
        }
        
        sprintf (buf, "%.2f", page_x); 
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", page_y); 
        xv_setstr(ui->page_y, buf);
        
        SetOptionChoice(ui->fontrast, dev->fontrast);
        SetOptionChoice(ui->color_trans, dev->color_trans);
    }
}

static int set_printer_proc(void *data)
{
    PrintUI *ui = (PrintUI *) data;
    int seldevice;
    double page_x, page_y;
    double dpi;
    int page_units;
    Device_entry *dev;
    Page_geometry pg;
    int do_redraw = FALSE;
    
    seldevice = GetOptionChoice(ui->devices);

    dev = get_device_props(canvas, seldevice);

    if (dev->type != DEVICE_TERM) {
        gapp->rt->hdevice = seldevice;
        set_ptofile(gapp, GetToggleButtonState(ui->printto));
        if (get_ptofile(gapp)) {
            strcpy(gapp->rt->print_file, xv_getstr(ui->printfile));
        } else {
            if (gapp->rt->use_cups) {
                set_print_dest(gapp, GetOptionChoice(ui->destination));
            } else {
                set_print_cmd(gapp, xv_getstr(ui->print_string));
            }
        }

        if (xv_evalexpr(ui->page_x, &page_x) != RETURN_SUCCESS || 
            xv_evalexpr(ui->page_y, &page_y) != RETURN_SUCCESS ||
            page_x <= 0.0 || page_y <= 0.0) {
            errmsg("Invalid page dimension(s)");
            return RETURN_FAILURE;
        }

        if (xv_evalexpr(ui->dev_res, &dpi) != RETURN_SUCCESS ||
            dpi <= 0.0) {
            errmsg("Invalid dpi");
            return RETURN_FAILURE;
        }

        dev->autocrop = GetToggleButtonState(ui->autocrop);

        page_units = GetOptionChoice(ui->page_size_unit);

        switch (page_units) {
        case 0: 
            pg.width =  (long) page_x;
            pg.height = (long) page_y;
            break;
        case 1: 
            pg.width =  (long) (page_x * dpi);
            pg.height = (long) (page_y * dpi);
            break;
        case 2: 
            pg.width =  (long) (page_x * dpi / CM_PER_INCH);
            pg.height = (long) (page_y * dpi / CM_PER_INCH);
            break;
        default:
            errmsg("Internal error");
            return RETURN_FAILURE;
        }

        pg.dpi = dpi;
    
        dev->pg = pg;
    }
    
    dev->fontrast = GetOptionChoice(ui->fontrast);
    dev->color_trans = GetOptionChoice(ui->color_trans);
    
    if (seldevice == gapp->rt->tdevice) {
        do_redraw = TRUE;
    }
    
    if (do_redraw) {
        xdrawgraph(gapp->gp);
    }
    
    return RETURN_SUCCESS;
}


/*
 * set the print options
 */
static void do_device_toggle(OptionStructure *opt, int value, void *data)
{ 
    update_device_setup((PrintUI *) data, value);
}

static void do_pr_toggle(Widget tbut, int onoff, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    
    if (onoff == TRUE) {
        SetSensitive(ui->rc_filesel, True);
        SetSensitive(ui->rc_printsel, False);
    } else {
        SetSensitive(ui->rc_filesel, False);
        SetSensitive(ui->rc_printsel, True);
    }
}

static void do_format_toggle(OptionStructure *opt, int value, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    int orientation;
    int x, y;
    double px, py;
    int page_units;
    double dpi;
    char buf[32];
    
    if (value == PAGE_FORMAT_CUSTOM) {
        SetSensitive(ui->page_x, True);
        SetSensitive(ui->page_y, True);
        SetSensitive(ui->page_orient->menu, False);
    } else {
        SetSensitive(ui->page_x, False);
        SetSensitive(ui->page_y, False);
        SetSensitive(ui->page_orient->menu, True);
    }
    
    
    switch (value) {
    case PAGE_FORMAT_USLETTER:
        x = 612;
        y = 792;
        break;
    case PAGE_FORMAT_A4:
        x = 595;
        y = 842;
        break;
    case PAGE_FORMAT_CUSTOM:
    default:
        return;
    }

    
    page_units = GetOptionChoice(ui->page_size_unit);
    
    switch (page_units) {
    case 0:      /* pixels */
        if (xv_evalexpr(ui->dev_res, &dpi) != RETURN_SUCCESS) {
            errmsg("Invalid dpi");
            return;
        }
        px = (float) x*dpi/72.0;
        py = (float) y*dpi/72.0;
        break;
    case 1:      /* inches */
        px = (float) x/72.0;
        py = (float) y/72.0;
        break;
    case 2:      /* cm */ 
        px = (float) x/72.0*CM_PER_INCH;
        py = (float) y/72.0*CM_PER_INCH;
        break;
    default:
        errmsg("Internal error");
        return;
    }
    
    orientation = GetOptionChoice(ui->page_orient);
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_y, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_y, buf);
    }
}

static void do_orient_toggle(OptionStructure *opt, int value, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    int orientation = value;
    double px, py;
    char buf[32];

    if (xv_evalexpr(ui->page_x, &px) != RETURN_SUCCESS || 
        xv_evalexpr(ui->page_y, &py) != RETURN_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_y, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(ui->page_x, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(ui->page_y, buf);
    }
}

static int do_prfilesel_proc(FSBStructure *fsb, char *filename, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    
    xv_setstr(ui->printfile, filename);
    strcpy(gapp->rt->print_file, filename);
    XtVaSetValues(ui->printfile, XmNcursorPosition, strlen(filename), NULL);
    return TRUE;
}

void create_printfiles_popup(Widget but, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    static FSBStructure *fsb = NULL;
    int device;
    Device_entry *dev;
    char buf[16];

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Select print file");
	AddFileSelectionBoxCB(fsb, do_prfilesel_proc, ui);
        ManageChild(fsb->FSB);
    }

    device = GetOptionChoice(ui->devices);
    dev = get_device_props(canvas, device);

    sprintf(buf, "*.%s", dev->fext);
    SetFileSelectionBoxPattern(fsb, buf);
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

void create_devopts_popup(Widget but, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    int device_id;
    Device_entry *dev;
    dev_gui_setup *setup_data;
    
    device_id = GetOptionChoice(ui->devices);
    dev = get_device_props(canvas, device_id);

    setup_data = (dev_gui_setup *) device_get_udata(canvas, device_id);
    
    if (setup_data == NULL || setup_data->setup == NULL) {
        /* Should never come to here */
        errmsg("No options can be set for this device");
    } else {
        (setup_data->setup)(canvas, setup_data->ui);
    }
}

static int set_destopts_proc(void *data)
{
    DestSetupUI *dsui = (DestSetupUI *) data;
    PrintDest *pd = &gapp->rt->print_dests[dsui->ndest];
    int i, j, nopts = 0;
    
    for (i = 0; i < pd->nogroups; i++) {
        PrintOptGroup *og = &pd->ogroups[i];

        for (j = 0; j < og->nopts; j++) {
            PrintOption *po = &og->opts[j];
            
            po->selected = GetOptionChoice(dsui->opts[nopts]);
            nopts++;
        }
    }
    
    return RETURN_SUCCESS;
}

void create_destopts_popup(Widget but, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    int ndest = GetOptionChoice(ui->destination);
    DestSetupUI *dsui = &ui->destopts[ndest];
    
    if (dsui->top == NULL) {
        PrintDest *pd = &gapp->rt->print_dests[ndest];
        Widget top, tab;
        int i, j, k, nopts = 0;
        char buf[128];
        
        dsui->ndest = ndest;
        dsui->opts = xmalloc(pd->nopts*SIZEOF_VOID_P);

        sprintf(buf, "CUPS setup: %s", pd->printer);
        top = CreateDialogForm(app_shell, buf);

	tab = CreateTab(top);
        
        for (i = 0; i < pd->nogroups; i++) {
            PrintOptGroup *og = &pd->ogroups[i];
            Widget page = CreateTabPage(tab, og->text);
            XtVaSetValues(page, XmNpacking, XmPACK_COLUMN,
                                XmNnumColumns, og->nopts,
                                XmNorientation, XmHORIZONTAL,
                                XmNentryAlignment, XmALIGNMENT_END,
                                XmNadjustLast, False,
                                XmNisAligned, True,
                                NULL);
            
            for (j = 0; j < og->nopts; j++) {
                PrintOption *po = &og->opts[j];
                int nchoises = po->choices->size;
                OptionItem *options;

                sprintf(buf, "%s:", po->text);
                CreateLabel(page, buf);

                options = xmalloc(nchoises*sizeof(OptionItem));
                for (k = 0; k < nchoises; k++) {
                    options[k].value = po->choices->entries[k].key;
                    options[k].label = po->choices->entries[k].descr;
                }
                dsui->opts[nopts] = CreateOptionChoice(page, "", 0, nchoises, options);
                xfree(options);
                
                SetOptionChoice(dsui->opts[nopts], po->selected);
                nopts++;
            }
            
            if (strings_are_equal(og->text, "General")) {
                SelectTabPage(tab, page);
            }
        }

	CreateAACDialog(top, tab, set_destopts_proc, dsui);
        
        dsui->top = top;
    }
    
    RaiseWindow(GetParent(dsui->top));
}

static void do_units_toggle(OptionStructure *opt, int value, void *data)
{
    PrintUI *ui = (PrintUI *) data;
    char buf[32];
    double page_x, page_y;
    double dev_res;
    int page_units = value;
    
    if (xv_evalexpr(ui->page_x, &page_x) != RETURN_SUCCESS || 
        xv_evalexpr(ui->page_y, &page_y) != RETURN_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if (xv_evalexpr(ui->dev_res, &dev_res) != RETURN_SUCCESS) {
        errmsg("Invalid device resolution(s)");
        return;
    }
    
    if (dev_res <= 0.0) {
        errmsg("Device resolution(s) <= 0");
        return;
    }
    
    if (ui->current_page_units == page_units) {
        ;
    } else if (ui->current_page_units == 0 && page_units == 1) {
        page_x /= dev_res;
        page_y /= dev_res;
    } else if (ui->current_page_units == 0 && page_units == 2) {
        page_x /= (dev_res/CM_PER_INCH);
        page_y /= (dev_res/CM_PER_INCH);
    } else if (ui->current_page_units == 1 && page_units == 0) {
        page_x *= dev_res;
        page_y *= dev_res;
    } else if (ui->current_page_units == 1 && page_units == 2) {
        page_x *= CM_PER_INCH;
        page_y *= CM_PER_INCH;
    } else if (ui->current_page_units == 2 && page_units == 0) {
        page_x *= (dev_res/CM_PER_INCH);
        page_y *= (dev_res/CM_PER_INCH);
    } else if (ui->current_page_units == 2 && page_units == 1) {
        page_x /= CM_PER_INCH;
        page_y /= CM_PER_INCH;
    } else {
        errmsg("Internal error");
        return;
    }
        
    ui->current_page_units = page_units;
    
    sprintf (buf, "%.2f", page_x); 
    xv_setstr(ui->page_x, buf);
    sprintf (buf, "%.2f", page_y); 
    xv_setstr(ui->page_y, buf);
}

static void do_print_cb(Widget but, void *data)
{
    set_wait_cursor();
    do_hardcopy(gapp->gp);
    unset_wait_cursor();
}
