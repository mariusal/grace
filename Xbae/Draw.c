/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1995-97 Andrew Lister
 *                        All rights reserved
 * Permission to use, copy, modify and distribute this material for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Bellcore not be used in advertising
 * or publicity pertaining to this material without the specific,
 * prior written permission of an authorized representative of
 * Bellcore.
 *
 * BELLCORE MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES, EX-
 * PRESS OR IMPLIED, WITH RESPECT TO THE SOFTWARE, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR ANY PARTICULAR PURPOSE, AND THE WARRANTY AGAINST IN-
 * FRINGEMENT OF PATENTS OR OTHER INTELLECTUAL PROPERTY RIGHTS.  THE
 * SOFTWARE IS PROVIDED "AS IS", AND IN NO EVENT SHALL BELLCORE OR
 * ANY OF ITS AFFILIATES BE LIABLE FOR ANY DAMAGES, INCLUDING ANY
 * LOST PROFITS OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES RELAT-
 * ING TO THE SOFTWARE.
 *
 * $Id: Draw.c,v 1.1 1999-01-11 23:37:43 fnevgeny Exp $
 */

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion > 1001
#include <Xm/DrawP.h>
#endif
#include <Xbae/MatrixP.h>
#include <Xbae/Utils.h>
#include <Xbae/Shadow.h>
#include <Xbae/Draw.h>

static void xbaeDrawCellString P(( XbaeMatrixWidget, int, int, String, Pixel,
				   Pixel ));
#if CELL_WIDGETS
static void xbaeDrawCellWidget P(( XbaeMatrixWidget, int, int, Widget, Pixel,
				   Pixel ));
#endif
static void xbaeDrawCellPixmap P(( XbaeMatrixWidget, int, int, Pixmap, Pixmap,
				   int, int, Pixel, Pixel, int ));

/*
 * Draw a fixed or non-fixed cell. The coordinates are calculated relative
 * to the correct window and pixmap is copied to that window.
 */
static void
xbaeDrawCellPixmap(mw, row, column, pixmap, mask, width, height, bg, fg, depth)
XbaeMatrixWidget mw;
int row;
int column;
Pixmap pixmap;
Pixmap mask;
int width;
int height;
Pixel bg;
Pixel fg;
int depth;
{
    int x, y;
    int src_x = 0, src_y, dest_x, dest_y;
    int copy_width, copy_height;
    int cell_height = ROW_HEIGHT(mw);
    int cell_width = COLUMN_WIDTH(mw, column);
    Widget w;
    unsigned char alignment = mw->matrix.column_alignments ?
	mw->matrix.column_alignments[ column ] : XmALIGNMENT_BEGINNING;
    Display *display = XtDisplay(mw);
    GC gc;
    Window win = xbaeGetCellWindow(mw, &w, row, column);

    if (!win)
	return;

    /*
     * Convert the row/column to the coordinates relative to the correct
     * window
     */
    xbaeRowColToXY(mw, row, column, &x, &y);
    dest_x = x + TEXT_WIDTH_OFFSET(mw);

    gc = mw->matrix.pixmap_gc;

    XSetForeground(display, gc, bg);

#if XmVersion >= 1002
    /*
     * If we are only changing the highlighting of a cell, we don't need
     * to do anything other than draw (or undraw) the highlight
     */
    if (mw->matrix.highlighted_cells &&
	mw->matrix.highlight_location != HighlightNone)
    {
	xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, cell_width,
			      cell_height, mw->matrix.highlight_location);
	return;
    }
#endif

    XFillRectangle(display, win, gc, x, y,
		   COLUMN_WIDTH(mw, column), ROW_HEIGHT(mw));

    XSetForeground(display, gc, fg);
    XSetBackground(display, gc, bg);

    /*
     * Adjust the x and y drawing destinations as appropriate.  First the
     * y value....
     */
    dest_y = y;
    if (height > cell_height)
    {
	/* Adjust the starting location in the src image */
	src_y = (height - cell_height) / 2;
	copy_height = cell_height;
    }
    else
    {
	/* Adjust the destination point */
	src_y = 0;
	dest_y += ((cell_height - height) / 2);
	copy_height = height;
    }

    /*
     * Adjust the x value, paying attention to the columnAlignment
     */
    if (width > cell_width)
	copy_width = cell_width;
    else
	copy_width = width;
    
    switch (alignment)
    {
    case XmALIGNMENT_BEGINNING:
	src_x = 0;
	break;
    case XmALIGNMENT_CENTER:
	if (width > cell_width)
	    src_x = (width - cell_width) / 2;
	else
	{
	    src_x = 0;
	    dest_x += ((cell_width - width) / 2);
	}
	break;
    case XmALIGNMENT_END:	
	if (width > cell_width)
	    src_x = width - cell_width;
	else
	{
	    src_x = 0;
	    dest_x = x + COLUMN_WIDTH(mw, column) - TEXT_WIDTH_OFFSET(mw) -
		width;
	}
	break;
    }

    /*
     * Draw the pixmap.  Clip it, if necessary
     */
    if (pixmap)
    {
	if (depth > 1)		/* A pixmap using xpm */
	{
	    if (mask)
	    {
		XSetClipMask(display, gc, mask);

		XSetClipOrigin(display, gc, dest_x - src_x, dest_y - src_y);
	    }
	    XCopyArea(display, pixmap, win, gc, src_x, src_y, copy_width,
		      copy_height, dest_x, dest_y);
	    if (mask)
		XSetClipMask(display, gc, None);
	}
	else			/* A plain old bitmap */
	    XCopyPlane(display, pixmap, win, gc, src_x, src_y, copy_width,
		       copy_height, dest_x, dest_y, 1L);
    }
    
    /*
     * If we need to fill the rest of the space, do so
     */
    if (mw->matrix.grid_type == XmGRID_COLUMN_SHADOW &&
	row == mw->matrix.rows - 1 && NEED_VERT_FILL(mw))
    {
	int ax, ay;
	int fill_width, fill_height;
	/*
	 * Need to check the actual window we are drawing on to ensure
	 * the correct visual
	 */
	xbaeCalcVertFill(mw, win, x, y, row, column, &ax, &ay,
			 &fill_width, &fill_height);
	XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
		       ax, ay, fill_width, fill_height);
    }
    else if (mw->matrix.grid_type == XmGRID_ROW_SHADOW &&
	     column == mw->matrix.columns - 1 && NEED_HORIZ_FILL(mw))
    {
	int ax, ay;
	int fill_width, fill_height;

	xbaeCalcHorizFill(mw, win, x, y, row, column, &ax, &ay,
			  &fill_width, &fill_height);
	XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
		       ax, ay, fill_width, fill_height);
    }

#if XmVersion >= 1002
    if (mw->matrix.highlighted_cells &&
	mw->matrix.highlighted_cells[row][column])
    {
	xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, cell_width,
			      cell_height, HIGHLIGHTING_SOMETHING);
    }
#endif
    xbaeDrawCellShadow(mw, win, row, column, x, y, cell_width,
		       cell_height, False, False, False);
}

/*
 * Draw a fixed or non-fixed cell. The coordinates are calculated relative
 * to the correct window and the cell is drawn in that window.
 */
static void
xbaeDrawCellString(mw, row, column, string, bg, fg)
XbaeMatrixWidget mw;
int row, column;
String string;
Pixel bg, fg;
{
    int x, y;
    GC gc;
    Widget w;
    Window win = xbaeGetCellWindow(mw, &w, row, column);
    Dimension column_width = COLUMN_WIDTH(mw, column);
    Dimension row_height = ROW_HEIGHT(mw);
    Dimension width = column_width;
    Dimension height = row_height;
    Boolean selected = mw->matrix.selected_cells ?
	mw->matrix.selected_cells[row][column] : False;
    String str = string;
    
    if (!win)
	return;

    /*
     * Convert the row/column to the coordinates relative to the correct
     * window
     */
    xbaeRowColToXY(mw, row, column, &x, &y);

#if 0
    /*
     * Probably not needed - time will tell!  If anybody gets a segv on
     * ignoring this code, be sure to let me know - AL 11/96
     *
     * Make sure y coordinate is valid 
     */
    if ((win == XtWindow(mw)) &&
	((y > (CLIP_VERT_VISIBLE_SPACE(mw) + ROW_LABEL_OFFSET(mw) - 1)) ||
	 (y < ROW_LABEL_OFFSET(mw))))
	return;
#endif
    gc = mw->matrix.draw_gc;
    XSetForeground(XtDisplay(mw), gc, bg);

    /*
     * If we are only changing the highlighting of a cell, we don't need
     * to do anything other than draw (or undraw) the highlight
     */
    if (mw->matrix.highlighted_cells &&
	mw->matrix.highlight_location != HighlightNone)
    {
	xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, width, height,
			      mw->matrix.highlight_location);
	return;
    }

    /*
     * Fill the cell's background if it can be done
     * without duplicating work below
     */
    if ((XtWindow(mw) != win) ||
	(!((XmGRID_COLUMN_SHADOW == mw->matrix.grid_type) &&
	   ((mw->matrix.rows - 1) == row) &&  NEED_VERT_FILL(mw)) &&
	 !((XmGRID_ROW_SHADOW == mw->matrix.grid_type) &&
	   ((mw->matrix.columns - 1) == column) && NEED_HORIZ_FILL(mw))))
	XFillRectangle(XtDisplay(mw), win, gc, x, y,
		       column_width, row_height);

    /*
     * If we need to fill the rest of the space, do so
     */
    if ((XmGRID_COLUMN_SHADOW == mw->matrix.grid_type) &&
	((mw->matrix.rows - 1) == row) && NEED_VERT_FILL(mw))
    {
	int ax, ay;
	int fill_width, fill_height;
	/*
	 * Need to check the actual window we are drawing on to ensure
	 * the correct visual
	 */
	xbaeCalcVertFill(mw, win, x, y, row, column, &ax, &ay,
			 &fill_width, &fill_height);
	XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
		       ax, ay, fill_width, fill_height);
    }
    else if (mw->matrix.grid_type == XmGRID_ROW_SHADOW &&
	     column == mw->matrix.columns - 1 && NEED_HORIZ_FILL(mw))
    {
	int ax, ay;
	int fill_width, fill_height;

	xbaeCalcHorizFill(mw, win, x, y, row, column, &ax, &ay,
			  &fill_width, &fill_height);
	XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
		       ax, ay, fill_width, fill_height);
    }

    /*
     * Draw the string in the cell.
     */

    xbaeDrawString(mw, win, gc, str, strlen(str),
		   x + TEXT_X_OFFSET(mw), y + TEXT_Y_OFFSET(mw),
		   mw->matrix.column_widths[column],
		   mw->matrix.column_alignments ?
		   mw->matrix.column_alignments[column] :
		   XmALIGNMENT_BEGINNING, selected,
		   False, False, False, fg);

#if XmVersion >= 1002
    if (mw->matrix.highlighted_cells &&
	mw->matrix.highlighted_cells[row][column])
    {
	xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, width, height,
			      HIGHLIGHTING_SOMETHING);
    }
#endif

    xbaeDrawCellShadow(mw, win, row, column, x, y, COLUMN_WIDTH(mw, column),
		       ROW_HEIGHT(mw), False, False, False);
}
#if CELL_WIDGETS
/*
 * Draw a user defined widget in the cell
 */
static void
xbaeDrawCellWidget(mw, row, column, widget, bg, fg)
XbaeMatrixWidget mw;
int row, column;
Widget widget;
Pixel bg, fg;
{
    int x, y;
    GC gc;
    Widget w;
    Window win = xbaeGetCellWindow(mw, &w, row, column);

    if (!win)
	return;

    /*
     * Convert the row/column to the coordinates relative to the correct
     * window
     */
    xbaeRowColToXY(mw, row, column, &x, &y);

    gc = mw->matrix.draw_gc;
    XSetForeground(XtDisplay(mw), gc, bg);
    XFillRectangle(XtDisplay(mw), win, gc, x, y,
		   COLUMN_WIDTH(mw, column), ROW_HEIGHT(mw));

    /*
     * Draw the widget in the cell.
     */
    XtMoveWidget(widget,
		 x + mw->matrix.cell_shadow_thickness +
		 mw->matrix.cell_highlight_thickness,
		 y + mw->matrix.cell_shadow_thickness +
		 mw->matrix.cell_highlight_thickness);

    xbaeDrawCellShadow(mw, win, row, column, x, y, COLUMN_WIDTH(mw, column),
		       ROW_HEIGHT(mw), False, clipped, False);
}
#endif

/*
 * Width in pixels of a character in a given font
 */
#define charWidth(fs,c) ((fs)->per_char ? \
                         (fs)->per_char[((c) < (fs)->min_char_or_byte2 ? \
					 (fs)->default_char : \
					 (c) - \
					 (fs)->min_char_or_byte2)].width : \
			 (fs)->min_bounds.width)


/*
 * Draw a string with specified attributes. We want to avoid having to
 * use a GC clip_mask, so we clip by characters. This complicates the code.
 */
void
#if NeedFunctionPrototypes
xbaeDrawString(XbaeMatrixWidget mw, Window win, GC gc, String string,
	       int length, int x, int y, int maxlen, unsigned char alignment,
	       Boolean highlight, Boolean bold, Boolean rowLabel,
	       Boolean colLabel, Pixel color)
#else
xbaeDrawString(mw, win, gc, string, length, x, y, maxlen, alignment,
	       highlight, bold, rowLabel, colLabel, color)
XbaeMatrixWidget mw;
Window win;
GC gc;
String string;
int length;
int x;
int y;
int maxlen;
unsigned char alignment;
Boolean highlight;
Boolean bold;
Boolean rowLabel;
Boolean colLabel;
Pixel color;
#endif
{
    int start, width, maxwidth;
    XFontStruct	*font;
    Boolean choppedStart = False;
    Boolean choppedEnd = False;

    if (rowLabel || colLabel)
	font = mw->matrix.label_font;
    else
	font = mw->matrix.font;
    /*
     * Initialize starting character in string
     */
    start = 0;

    if (!rowLabel)
	maxwidth = maxlen * FONT_WIDTH(mw);
    else 
	maxwidth = maxlen * LABEL_WIDTH(mw);

    width = XTextWidth(font, string, length);

    /*
     * If the width of the string is greater than the width of this cell,
     * we need to clip. We don't want to use the server to clip because
     * it is slow, so we truncate characters if we exceed a cells pixel width.
     */
    if (width > maxwidth)
    {
	switch (alignment)
	{

	case XmALIGNMENT_CENTER:
	{
	    int startx = x;
	    int endx = x + maxwidth - 1;
	    int newendx;

	    /*
	     * Figure out our x for the centered string.  Then loop and chop
	     * characters off the front until we are within the cell.
	     * Adjust x, the starting character and the length of the string
	     * for each char.
	     */
	    x += maxwidth / 2 - width / 2;
	    while (x < startx)
	    {
		int cw = charWidth(font, string[start]);

		x += cw;
		width -= cw;
		length--;
		start++;
                choppedStart = True;
	    }

	    /*
	     * Now figure out the end x of the string.  Then loop and chop
	     * characters off the end until we are within the cell.
	     */
	    newendx = x + width - 1;
	    while (newendx > endx && *(string + start))
	    {
		int cw = charWidth(font, string[start]);

		newendx -= cw;
		width -= cw;
		length--;
		choppedEnd = True;
	    }

	    break;
	}

	case XmALIGNMENT_END:
	{

	    /*
	     * Figure out our x for the right justified string.
	     * Then loop and chop characters off the front until we fit.
	     * Adjust x for each char lopped off. Also adjust the starting
	     * character and length of the string for each char.
	     */
	    x += maxwidth - width;
	    while (width > maxwidth)
	    {
		int cw = charWidth(font, string[start]);

		width -= cw;
		x += cw;
		length--;
		start++;
                choppedStart = True;
	    }
	    break;
	}

	case XmALIGNMENT_BEGINNING:
	default:
	    /*
	     * Leave x alone, but chop characters off the end until we fit
	     */
	    while (width > maxwidth)
	    {
		width -= charWidth(font, string[length - 1]);
		length--;
		choppedEnd = True;
	    }
	    break;
	}
    }

    /*
     * We fit inside our cell, so just compute the x of the start of our string
     */
    else
    {
	switch (alignment)
	{

	case XmALIGNMENT_CENTER:
	    x += maxwidth / 2 - width / 2;
	    break;

	case XmALIGNMENT_END:
	    x += maxwidth - width;
	    break;

	case XmALIGNMENT_BEGINNING:
	default:
	    /*
	     * Leave x alone
	     */
	    break;
	}
    }
    
    /*
     * Don't worry, XSetForeground is smart about avoiding unnecessary
     * protocol requests.
     */
    XSetForeground(XtDisplay(mw), gc, color);

    if (mw->matrix.show_arrows && choppedEnd)
    {
	XPoint points[ 3 ];
	points[ 0 ].x = points[ 1 ].x = x + width - FONT_WIDTH(mw);
	points[ 0 ].y = y + mw->matrix.font->max_bounds.descent;
	points[ 1 ].y = y + mw->matrix.font->max_bounds.descent -
	    TEXT_HEIGHT(mw);
	points[ 2 ].x = x + width;
	points[ 2 ].y = y + mw->matrix.font->max_bounds.descent -
	    TEXT_HEIGHT(mw) / 2;

	XFillPolygon(XtDisplay(mw), win, gc, points, 3,
		     Convex, CoordModeOrigin);

	/* Reduce the length to allow for our foreign character */
	length--;
    }
    if (mw->matrix.show_arrows && choppedStart)
    {
	XPoint points[ 3 ];
	points[ 0 ].x = points[ 1 ].x = x + FONT_WIDTH(mw);
	points[ 0 ].y = y + mw->matrix.font->max_bounds.descent -
	    TEXT_HEIGHT(mw);
	points[ 1 ].y = y + mw->matrix.font->max_bounds.descent;
	points[ 2 ].x = x;
	points[ 2 ].y = y + mw->matrix.font->max_bounds.descent -
	    TEXT_HEIGHT(mw) / 2;

	XFillPolygon(XtDisplay(mw), win, gc, points, 3,
		     Convex, CoordModeOrigin);

	/* Offset the start point so as to not draw on the triangle */
	x += FONT_WIDTH(mw);
	start++;
	length--;
    }

    /*
     * Now draw the string at x starting at char 'start' and of length 'length'
     */
#ifdef NEED_WCHAR
    if (TWO_BYTE_FONT(mw))
	XDrawString16(XtDisplay(mw), win, gc, x, y, &string[start], length);
    else
#endif
	XDrawString(XtDisplay(mw), win, gc, x, y, &string[start], length);

    /*
     * If bold is on, draw the string again offset by 1 pixel (overstrike)
     */
    if (bold)
#ifdef NEED_WCHAR
	if (TWO_BYTE_FONT(mw))
	    XDrawString16(XtDisplay(mw), win, gc, x - 1, y,
			  &string[start], length);
	else
#endif
	    XDrawString(XtDisplay(mw), win, gc, x - 1, y,
			&string[start], length);

}

void
xbaeComputeCellColors(mw, row, column, fg, bg)
XbaeMatrixWidget mw;
int row, column;
Pixel *fg, *bg;
{
    Boolean alt = mw->matrix.alt_row_count ?
	(row / mw->matrix.alt_row_count) % 2 : False;

    /*
     * Compute the background and foreground colours of the cell
     */
    if (mw->matrix.selected_cells && mw->matrix.selected_cells[row][column])
	if (mw->matrix.reverse_select)
	    if (mw->matrix.colors)
		*bg = mw->matrix.colors[ row ][ column ];
	    else
		*bg = mw->manager.foreground;
	else
	    *bg = mw->matrix.selected_background;
    else if (mw->matrix.cell_background &&
	     mw->matrix.cell_background[row][column] !=
	     mw->core.background_pixel)
	*bg = mw->matrix.cell_background[row][column];
    else
    {
	if (alt)
	    *bg = mw->matrix.odd_row_background;
	else
	    *bg = mw->matrix.even_row_background;
    }

    if (mw->matrix.selected_cells && mw->matrix.selected_cells[row][column])
	if (mw->matrix.reverse_select)
	    if (mw->matrix.cell_background)
		*fg = mw->matrix.cell_background[row][column];
	    else
		*fg = mw->core.background_pixel;
	else
	    *fg = mw->matrix.selected_foreground;
    else if (mw->matrix.colors)
	*fg = mw->matrix.colors[ row ][ column ];
    else
	*fg = mw->manager.foreground;    
}

void
xbaeDrawCell(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    Pixel bg, fg;
    String string;

    if (mw->matrix.disable_redisplay)
	return;

    xbaeComputeCellColors(mw, row, column, &fg, &bg);

#if CELL_WIDGETS
    if (mw->matrix.cell_widgets[row][column])
	xbaeDrawCellWidget(mw, row, column,
			   mw->matrix.cell_widgets[row][column], bg, fg);
    else
#endif

	if (!mw->matrix.draw_cell_callback)
	{	
	    string = mw->matrix.cells ? mw->matrix.cells[row][column] : "";
	    xbaeDrawCellString(mw, row, column, string, bg, fg);
	}
	else
	{
	    Pixmap pixmap;
	    Pixmap mask;
	    XbaeCellType type;
	    int width, height;
	    int depth;
	
	    type = xbaeGetDrawCellValue(mw, row, column, &string, &pixmap,
					&mask, &width, &height, &bg, &fg, &depth);
	    if (type == XbaeString)
		xbaeDrawCellString(mw, row, column, string, bg, fg);
	    else if (type == XbaePixmap)
		xbaeDrawCellPixmap(mw, row, column, pixmap, mask, width,
				   height, bg, fg, depth);
	}
}

XbaeCellType
xbaeGetDrawCellValue(mw, row, column, string, pixmap, mask, width,
		     height, bg, fg, depth)
XbaeMatrixWidget mw;
int row;
int column;
String *string;
Pixmap *pixmap;
Pixmap *mask;
int *width, *height;
Pixel *bg, *fg;
int *depth;
{
    XbaeMatrixDrawCellCallbackStruct cbd;

    cbd.reason = XbaeDrawCellReason;
    cbd.row = row;
    cbd.column = column;
    cbd.width = COLUMN_WIDTH(mw, column) - TEXT_WIDTH_OFFSET(mw) * 2;
    cbd.height = ROW_HEIGHT(mw) - TEXT_HEIGHT_OFFSET(mw) * 2;
    cbd.type = XbaeString;
    cbd.string = "";
    cbd.pixmap = (Pixmap)NULL;
    cbd.mask = (Pixmap)NULL;
    cbd.foreground = *fg;
    cbd.background = *bg;

    XtCallCallbackList((Widget)mw, mw->matrix.draw_cell_callback,
		       (XtPointer) &cbd);

    *pixmap = cbd.pixmap;
    *mask = cbd.mask;
    *string = cbd.string ? cbd.string : ""; /* Handle NULL strings */

    if (mw->matrix.reverse_select && mw->matrix.selected_cells &&
	mw->matrix.selected_cells[row][column])
    {
	/*
	 * if colours were set by the draw cell callback, handle reverse
	 * selection
	 */
	if (*bg != cbd.background)
	{
	    if (*fg != cbd.foreground)
		*bg = cbd.foreground;
	    *fg = cbd.background;
	}
	else if (*fg != cbd.foreground)
	    *bg = cbd.foreground;
    }
    else
    {
	*fg = cbd.foreground;
	*bg = cbd.background;
    }
    *width = cbd.width;
    *height = cbd.height;
    
    if (cbd.type == XbaePixmap)
    {
	if (*mask == XmUNSPECIFIED_PIXMAP || *mask == BadPixmap)
	    cbd.mask = 0;

	if (*pixmap == XmUNSPECIFIED_PIXMAP || *pixmap == BadPixmap)
	{
	    XtAppWarningMsg(
		XtWidgetToApplicationContext((Widget)mw),
		"drawCellCallback", "Pixmap", "XbaeMatrix",
		"XbaeMatrix: Bad pixmap passed from drawCellCallback",
		NULL, 0);
	    cbd.type = XbaeString;
	    *string = "";
	}
	else
	{
	    Window root_return;
	    int x_return, y_return;
	    unsigned int width_return, height_return;
	    unsigned int border_width_return;
	    unsigned int depth_return;

	    if (XGetGeometry(XtDisplay(mw), *pixmap, &root_return,
			     &x_return, &y_return, &width_return,
			     &height_return, &border_width_return,
			     &depth_return))
	    {
		*width = width_return;
		*height = height_return;
		*depth = depth_return;
	    }
	}
    }
    return (cbd.type);
}

/*
 * Draw the column label for the specified column.  Handles labels in
 * fixed and non-fixed columns.
 */
void
#if NeedFunctionPrototypes
xbaeDrawColumnLabel(XbaeMatrixWidget mw, int column, Boolean pressed)
#else
xbaeDrawColumnLabel(mw, column, pressed)
XbaeMatrixWidget mw;
int column;
Boolean pressed;
#endif
{
    String label;
    int labelX, labelY;
    int buttonX;
    int i;
    GC gc;
    Window win = XtWindow(mw);
    Boolean clipped = (column >= (int)mw->matrix.fixed_columns &&
		       column < TRAILING_HORIZ_ORIGIN(mw));

    Boolean button = mw->matrix.button_labels ||
	(mw->matrix.column_button_labels &&
	 mw->matrix.column_button_labels[column]);

    if (mw->matrix.column_labels[column][0] == '\0' && !button)
	return;

    /*
     * If the column label is in a fixed column, we don't need to account
     * for the horiz_origin
     */
    if (column < (int)mw->matrix.fixed_columns)
    {
	labelX = COLUMN_LABEL_OFFSET(mw) + COLUMN_POSITION(mw, column) +
	    TEXT_X_OFFSET(mw);
	buttonX = COLUMN_LABEL_OFFSET(mw) + COLUMN_POSITION(mw, column);
    }
    else if (column >= TRAILING_HORIZ_ORIGIN(mw))
    {
	labelX = TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw) +
	    COLUMN_POSITION(mw, column) -
	    COLUMN_POSITION(mw, TRAILING_HORIZ_ORIGIN(mw)) +
	    TEXT_X_OFFSET(mw);
	buttonX = TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw) +
	    COLUMN_POSITION(mw, column) -
	    COLUMN_POSITION(mw, TRAILING_HORIZ_ORIGIN(mw));
    }
    else
    {
	labelX = COLUMN_LABEL_OFFSET(mw) +
	    (COLUMN_POSITION(mw, column) - HORIZ_ORIGIN(mw)) +
	    TEXT_X_OFFSET(mw);
	buttonX = COLUMN_LABEL_OFFSET(mw) + (COLUMN_POSITION(mw, column) -
					     HORIZ_ORIGIN(mw));
    }	

    /*
     * Set our y to the baseline of the first line in this column
     */
    labelY = mw->matrix.label_font->max_bounds.ascent +
	mw->matrix.cell_shadow_thickness +
	mw->matrix.cell_highlight_thickness +
	mw->matrix.cell_margin_height +
	mw->matrix.text_shadow_thickness +
	(mw->matrix.column_label_maxlines -
	 mw->matrix.column_label_lines[column].lines) * LABEL_HEIGHT(mw) +
	HORIZ_SB_OFFSET(mw);

    if (clipped)
	gc = mw->matrix.label_clip_gc;
    else
	gc = mw->matrix.label_gc;
    
    if (button)
    {
	XSetForeground(XtDisplay(mw), gc, mw->matrix.button_label_background);
	XFillRectangle(XtDisplay(mw), win, gc, buttonX, HORIZ_SB_OFFSET(mw),
		       COLUMN_WIDTH(mw, column), COLUMN_LABEL_HEIGHT(mw));
    }

    XSetForeground(XtDisplay(mw), gc, mw->matrix.column_label_color);
    XSetBackground(XtDisplay(mw), gc, mw->matrix.button_label_background);

    label = mw->matrix.column_labels[column];

    if (label[ 0 ] != '\0')
	for (i = 0; i < mw->matrix.column_label_lines[column].lines; i++)
	{
	    xbaeDrawString(mw, XtWindow(mw), gc, label,
			   mw->matrix.column_label_lines[column].lengths[i],
			   labelX, labelY, mw->matrix.column_widths[column],
			   mw->matrix.column_label_alignments ?
			   mw->matrix.column_label_alignments[column] :
			   XmALIGNMENT_BEGINNING, False,
			   mw->matrix.bold_labels, False, True,
			   mw->matrix.column_label_color);
	
	    labelY += LABEL_HEIGHT(mw);
	    label += mw->matrix.column_label_lines[column].lengths[i] + 1;
	}
    if (button)
	xbaeDrawCellShadow(mw, XtWindow(mw), -1, column,
			   buttonX, HORIZ_SB_OFFSET(mw),
			   COLUMN_WIDTH(mw, column),
			   COLUMN_LABEL_HEIGHT(mw), True, clipped, pressed);
}

/*
 * Draw the row label for the specified row. Handles labels in fixed and
 * non-fixed rows.
 */
void
#if NeedFunctionPrototypes
xbaeDrawRowLabel(XbaeMatrixWidget mw, int row, Boolean pressed)
#else
xbaeDrawRowLabel(mw, row, pressed)
XbaeMatrixWidget mw;
int row;
Boolean pressed;
#endif
{
    int y;
    GC gc;
    Window win = XtWindow(mw);
    Boolean clipped = (row >= (int)mw->matrix.fixed_rows &&
		       row < TRAILING_VERT_ORIGIN(mw));
    
    Boolean button = mw->matrix.button_labels ||
	(mw->matrix.row_button_labels && mw->matrix.row_button_labels[row]);

    if (mw->matrix.row_labels[row][0] == '\0' && !button)
	return;

    /*
     * If the row label is in a fixed row we don't need to account
     * for the vert_origin
     */
    if (row < (int)mw->matrix.fixed_rows)
	y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) * row + TEXT_Y_OFFSET(mw);
    else if (row >= TRAILING_VERT_ORIGIN(mw))
	y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw) +
	    ROW_HEIGHT(mw) * (row - TRAILING_VERT_ORIGIN(mw)) +
	    TEXT_Y_OFFSET(mw);
    else
	y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) * (row - VERT_ORIGIN(mw)) +
	    LABEL_Y_OFFSET(mw);

    if (clipped)
	gc = mw->matrix.label_clip_gc;
    else
	gc = mw->matrix.label_gc;
    
    if (button)
    {
	XSetForeground(XtDisplay(mw), gc, mw->matrix.button_label_background);
	XFillRectangle(XtDisplay(mw), win, gc, VERT_SB_OFFSET(mw),
		       y - TEXT_Y_OFFSET(mw), ROW_LABEL_WIDTH(mw),
		       ROW_HEIGHT(mw));
    }

    XSetForeground(XtDisplay(mw), gc, mw->matrix.row_label_color);
    XSetBackground(XtDisplay(mw), gc, mw->matrix.button_label_background);

    if (mw->matrix.row_labels[row][0] != '\0')
	xbaeDrawString(mw, win, gc,
		       mw->matrix.row_labels[row],
		       strlen(mw->matrix.row_labels[row]),
		       TEXT_X_OFFSET(mw) + VERT_SB_OFFSET(mw), y,
		       mw->matrix.row_label_width,
		       mw->matrix.row_label_alignment, False,
		       mw->matrix.bold_labels, True, False,
		       mw->matrix.row_label_color);
    
    if (button)
	xbaeDrawCellShadow(mw, win, row, -1, VERT_SB_OFFSET(mw),
			   y - TEXT_Y_OFFSET(mw), ROW_LABEL_WIDTH(mw),
			   ROW_HEIGHT(mw), True, clipped, pressed);
}
