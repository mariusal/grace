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

/* 
 * Page/Device setup
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>

#include "cmath.h"
#include "globals.h"
#include "device.h"
#include "utils.h"
#include "graphutils.h"
#include "plotone.h"

#include "motifinc.h"
#include "protos.h"


extern char print_file[];

static int current_page_units = 0;

static Widget psetup_frame;
static Widget psetup_rc;
static Widget device_opts_item;
static Widget *printto_item;
static Widget print_string_item;
static Widget rc_filesel;
static Widget printfile_item;
static Widget pdev_rc;
static OptionStructure *devices_item;
static Widget output_frame;
static Widget *page_orient_item;
static Widget *page_format_item;
static Widget page_x_item;
static Widget page_y_item;
static Widget *page_size_unit_item;
static Widget dev_res_item;
static Widget fontaa_item;
static Widget devfont_item;
static Widget dsync_item, psync_item;

static void do_pr_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_format_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_orient_toggle(Widget w, XtPointer client_data, XtPointer call_data);

static void set_printer_proc(void *data);
void create_printfiles_popup(void *data);
void create_devopts_popup(void *data);

static void do_device_toggle(int value, void *data);
static void do_units_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void update_printer_setup(int device_id);
static void update_device_setup(int device_id);

static void do_print_cb(void *data);

void create_printer_setup(void *data)
{
    int i, ndev;
    Widget device_panel, rc, rc1, fr, wbut;
    Widget menubar, menupane;
    OptionItem *option_items;
    
    set_wait_cursor();
    
    if (psetup_frame == NULL) {
	psetup_frame = XmCreateDialogShell(app_shell, "Device", NULL, 0);
        XtVaSetValues(psetup_frame, XmNallowShellResize, True, NULL);
        handle_close(psetup_frame);
        device_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        psetup_frame, 
                                        NULL, 0);

        menubar = CreateMenuBar(device_panel);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Print", 'P', do_print_cb, NULL);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Close", 'C',
            set_printer_proc, (void *) AAC_CLOSE);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
        dsync_item = CreateMenuToggle(menupane,
            "Sync page size of all devices", 'S', NULL, NULL);
        SetToggleButtonState(dsync_item, TRUE);
        psync_item = CreateMenuToggle(menupane,
            "Rescale plot on page size change", 'R', NULL, NULL);
        SetToggleButtonState(psync_item, FALSE);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);
        CreateMenuButton(menupane, "On device setup", 'd', HelpCB, NULL);

        XtManageChild(menubar);
        XtVaSetValues(menubar,
                      XmNtopAttachment, XmATTACH_FORM,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);

	psetup_rc = XmCreateRowColumn(device_panel, "psetup_rc", NULL, 0);
        XtVaSetValues(psetup_rc,
            XmNrecomputeSize, True,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, menubar,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);

        fr = CreateFrame(psetup_rc, "Device setup");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
	pdev_rc = XmCreateRowColumn(rc1, "pdev_rc", NULL, 0);
        XtVaSetValues(pdev_rc, XmNorientation, XmHORIZONTAL, NULL);

	ndev = number_of_devices();
        option_items = xmalloc(ndev*sizeof(OptionItem));
        for (i = 0; i < ndev; i++) {
            option_items[i].value = i;
            option_items[i].label = get_device_name(i);
        }
        devices_item = CreateOptionChoice(pdev_rc, "Device: ",
					    1, ndev, option_items);
	AddOptionChoiceCB(devices_item, do_device_toggle, NULL);
        xfree(option_items);
        
        device_opts_item = CreateButton(pdev_rc, "Device options...");
	AddButtonCB(device_opts_item, create_devopts_popup, NULL);
        XtManageChild(pdev_rc);
        
        XtManageChild(rc1);
        
        output_frame = CreateFrame(psetup_rc, "Output");
        rc1 = XmCreateRowColumn(output_frame, "rc", NULL, 0);
	printto_item = CreatePanelChoice(rc1, "Print to: ",
					 3,
					 "Printer",
					 "File", 0, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(printto_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_pr_toggle, (XtPointer) i);
	}

	print_string_item = CreateTextItem2(rc1, 25, "Print command:");

	rc_filesel = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc_filesel, XmNorientation, XmHORIZONTAL, NULL);

	printfile_item = CreateTextItem2(rc_filesel, 20, "File name:");

	wbut = CreateButton(rc_filesel, "Browse...");
	AddButtonCB(wbut, create_printfiles_popup, NULL);
	XtManageChild(rc_filesel);

	XtManageChild(rc1);
	
        fr = CreateFrame(psetup_rc, "Page");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
        
	rc = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        page_orient_item = CreatePanelChoice(rc, "Orientation: ",
					 3,
					 "Landscape",
					 "Portrait",
					 0, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(page_orient_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_orient_toggle, (XtPointer) i);
	}
	page_format_item = CreatePanelChoice(rc, "Size: ",
			      4,
			      "Custom",
			      "Letter",
			      "A4",
			      0, 0);
	for (i = 0; i < 3; i++) {
	    XtAddCallback(page_format_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_format_toggle, (XtPointer) i);
	}
        XtManageChild(rc);

	rc = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        page_x_item = CreateTextItem2(rc, 7, "Dimensions:");
        page_y_item = CreateTextItem2(rc, 7, "x ");
	page_size_unit_item = CreatePanelChoice(rc, " ",
			                        4,
			                        "pix",
			                        "in",
			                        "cm",
			                        0, 0);
	for (i = 0; i < 3; i++) {
	    XtAddCallback(page_size_unit_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_units_toggle, (XtPointer) i);
	}
        SetChoice(page_size_unit_item, current_page_units);
        XtManageChild(rc);

        dev_res_item = CreateTextItem2(rc1, 4, "Resolution (dpi):");

        XtManageChild(rc1);

        fr = CreateFrame(psetup_rc, "Fonts");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
	fontaa_item = CreateToggleButton(rc1, "Enable font antialiasing");
	devfont_item = CreateToggleButton(rc1, "Use device fonts");
        XtManageChild(rc1);
        
	CreateSeparator(psetup_rc);

	CreateAACButtons(psetup_rc, device_panel, set_printer_proc);
	XtManageChild(psetup_rc);
	XtManageChild(device_panel);
    }
    
    XtRaise(psetup_frame);
    update_printer_setup(hdevice);
    unset_wait_cursor();
}

static void update_printer_setup(int device_id)
{
    if (psetup_frame) {
        SetOptionChoice(devices_item, device_id);
        update_device_setup(device_id);
    }
}

static void update_device_setup(int device_id)
{
    char buf[GR_MAXPATHLEN], *bufptr;
    int page_units;
    double page_x, page_y;
    PageFormat pf;
    
    Page_geometry pg;
    Device_entry dev;
    
    if (psetup_frame) {
	dev = get_device_props(device_id);
        pg = dev.pg;
        	
        if (dev.setup == NULL) {
            XtSetSensitive(device_opts_item, False);
        } else {
            XtSetSensitive(device_opts_item, True);
        }

        strcpy(buf, mybasename(get_docname())); 
        bufptr = strrchr(buf, '.');
        if (bufptr) {
            *(bufptr+1)='\0';
        } else {
            strcat(buf, ".");
        }
        
        if (print_file == NULL || print_file[0] == '\0' || strstr(print_file, buf)) {
            strcat(buf, dev.fext);
            xv_setstr(printfile_item, buf);
        } else {
            xv_setstr(printfile_item, print_file);
        }
                
        xv_setstr(print_string_item, get_print_cmd());
        
        switch (dev.type) {
        case DEVICE_TERM:
            XtUnmanageChild(output_frame);
            break;
        case DEVICE_FILE:
            XtManageChild(output_frame);
            SetChoice(printto_item, TRUE);
            XtSetSensitive(printto_item[0], False);
            XtSetSensitive(XtParent(print_string_item), False);
            XtSetSensitive(rc_filesel, True);
            break;
        case DEVICE_PRINT:
            XtManageChild(output_frame);
            SetChoice(printto_item, get_ptofile());
            XtSetSensitive(printto_item[0], True);
            if (get_ptofile() == TRUE) {
                XtSetSensitive(rc_filesel, True);
                XtSetSensitive(XtParent(print_string_item), False);
            } else {
                XtSetSensitive(rc_filesel, False);
                XtSetSensitive(XtParent(print_string_item), True);
            }
            break;
        }
        
        if (pg.width < pg.height) {
            SetChoice(page_orient_item, PAGE_ORIENT_PORTRAIT);
        } else {
            SetChoice(page_orient_item, PAGE_ORIENT_LANDSCAPE);
        }
        
        pf = get_page_format(device_id);
        SetChoice(page_format_item, pf); 
        if (pf == PAGE_FORMAT_CUSTOM) {
            XtSetSensitive(page_x_item, True);
            XtSetSensitive(page_y_item, True);
            XtSetSensitive(page_orient_item[0], False);
        } else {
            XtSetSensitive(page_x_item, False);
            XtSetSensitive(page_y_item, False);
            XtSetSensitive(page_orient_item[0], True);
        }
        
        sprintf (buf, "%.0f", pg.dpi); 
        xv_setstr(dev_res_item, buf);
        
        page_units = GetChoice(page_size_unit_item);
        
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
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", page_y); 
        xv_setstr(page_y_item, buf);
        
        SetToggleButtonState(fontaa_item, dev.fontaa);
        
        SetToggleButtonState(devfont_item, dev.devfonts);
    }
}

static void set_printer_proc(void *data)
{
    int aac_mode;
    int seldevice;
    double page_x, page_y;
    double dpi;
    int page_units;
    Device_entry dev;
    Page_geometry pg;
    int do_redraw = FALSE;
    
    aac_mode = (int) data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(psetup_frame);
        return;
    }
    
    seldevice = GetOptionChoice(devices_item);

    dev = get_device_props(seldevice);

    if (dev.type != DEVICE_TERM) {
        hdevice = seldevice;
        set_ptofile(GetChoice(printto_item));
        if (get_ptofile()) {
            strcpy(print_file, xv_getstr(printfile_item));
        } else {
            set_print_cmd(xv_getstr(print_string_item));
        }
    }
    
    dev.devfonts = GetToggleButtonState(devfont_item);
    dev.fontaa = GetToggleButtonState(fontaa_item);
    
    if (xv_evalexpr(page_x_item, &page_x) != RETURN_SUCCESS || 
        xv_evalexpr(page_y_item, &page_y) != RETURN_SUCCESS  ) {
        errmsg("Invalid page dimension(s)");
        return;
    }

    if (xv_evalexpr(dev_res_item, &dpi) != RETURN_SUCCESS) {
        errmsg("Invalid dpi");
        return;
    }

    page_units = GetChoice(page_size_unit_item);

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
        return;
    }
    
    pg.dpi = dpi;
    
    dev.pg = pg;
    
    set_device_props(seldevice, dev);
    
    if (GetToggleButtonState(dsync_item) == TRUE) {
        set_page_dimensions((int) rint(72.0*pg.width/pg.dpi),
                            (int) rint(72.0*pg.height/pg.dpi),
                            GetToggleButtonState(psync_item) == TRUE);
        do_redraw = TRUE;
    }
    
    if (seldevice == tdevice) {
        do_redraw = TRUE;
    }
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(psetup_frame);
    }
    
    if (do_redraw) {
        drawgraph();
    }
}


/*
 * set the print options
 */
static void do_device_toggle(int value, void *data)
{ 
    update_device_setup(value);
}

static void do_pr_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
    
    if (value == TRUE) {
        XtSetSensitive(rc_filesel, True);
        XtSetSensitive(XtParent(print_string_item), False);
    } else {
        XtSetSensitive(rc_filesel, False);
        XtSetSensitive(XtParent(print_string_item), True);
    }
}

static void do_format_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
    int orientation;
    int x, y;
    double px, py;
    int page_units;
    double dpi;
    char buf[32];
    
    if (value == PAGE_FORMAT_CUSTOM) {
        XtSetSensitive(page_x_item, True);
        XtSetSensitive(page_y_item, True);
        XtSetSensitive(page_orient_item[0], False);
    } else {
        XtSetSensitive(page_x_item, False);
        XtSetSensitive(page_y_item, False);
        XtSetSensitive(page_orient_item[0], True);
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

    
    page_units = GetChoice(page_size_unit_item);
    
    switch (page_units) {
    case 0:      /* pixels */
        if (xv_evalexpr(dev_res_item, &dpi) != RETURN_SUCCESS) {
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
    
    orientation = GetChoice(page_orient_item);
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(page_y_item, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(page_y_item, buf);
    }
}

static void do_orient_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int orientation = (int) client_data;
    double px, py;
    char buf[32];

    if (xv_evalexpr(page_x_item, &px) != RETURN_SUCCESS || 
        xv_evalexpr(page_y_item, &py) != RETURN_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(page_y_item, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(page_y_item, buf);
    }
}

static int do_prfilesel_proc(char *filename, void *data)
{
    xv_setstr(printfile_item, filename);
    strcpy(print_file, filename);
    XtVaSetValues(printfile_item, XmNcursorPosition, strlen(filename), NULL);
    return TRUE;
}

void create_printfiles_popup(void *data)
{
    static FSBStructure *fsb = NULL;
    int device;
    Device_entry dev;
    char buf[16];

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Select print file", "*");
	AddFileSelectionBoxCB(fsb, do_prfilesel_proc, NULL);
        XtManageChild(fsb->FSB);
    }

    device = GetOptionChoice(devices_item);
    dev = get_device_props(device);

    sprintf(buf, "*.%s", dev.fext);
    SetFileSelectionBoxPattern(fsb, buf);
    
    XtRaise(fsb->dialog);

    unset_wait_cursor();
}

void create_devopts_popup(void *data)
{
    int device_id;
    Device_entry dev;
    
    device_id = GetOptionChoice(devices_item);
    dev = get_device_props(device_id);
    if (dev.setup == NULL) {
        /* Should never come to here */
        errmsg("No options can be set for this device");
    } else {
        (dev.setup)();
    }
}

static void do_units_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    char buf[32];
    double page_x, page_y;
    double dev_res;
    int page_units = (int) client_data;
    
    if (xv_evalexpr(page_x_item, &page_x) != RETURN_SUCCESS || 
        xv_evalexpr(page_y_item, &page_y) != RETURN_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if (xv_evalexpr(dev_res_item, &dev_res) != RETURN_SUCCESS) {
        errmsg("Invalid device resolution(s)");
        return;
    }
    
    if (dev_res <= 0.0) {
        errmsg("Device resolution(s) <= 0");
        return;
    }
    
    if (current_page_units == page_units) {
        ;
    } else if (current_page_units == 0 && page_units == 1) {
        page_x /= dev_res;
        page_y /= dev_res;
    } else if (current_page_units == 0 && page_units == 2) {
        page_x /= (dev_res/CM_PER_INCH);
        page_y /= (dev_res/CM_PER_INCH);
    } else if (current_page_units == 1 && page_units == 0) {
        page_x *= dev_res;
        page_y *= dev_res;
    } else if (current_page_units == 1 && page_units == 2) {
        page_x *= CM_PER_INCH;
        page_y *= CM_PER_INCH;
    } else if (current_page_units == 2 && page_units == 0) {
        page_x *= (dev_res/CM_PER_INCH);
        page_y *= (dev_res/CM_PER_INCH);
    } else if (current_page_units == 2 && page_units == 1) {
        page_x /= CM_PER_INCH;
        page_y /= CM_PER_INCH;
    } else {
        errmsg("Internal error");
        return;
    }
        
    current_page_units = page_units;
    
    sprintf (buf, "%.2f", page_x); 
    xv_setstr(page_x_item, buf);
    sprintf (buf, "%.2f", page_y); 
    xv_setstr(page_y_item, buf);
}

static void do_print_cb(void *data)
{
    set_wait_cursor();
    do_hardcopy();
    unset_wait_cursor();
}
