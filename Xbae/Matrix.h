/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1995-97 Andrew Lister
 *
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
 * MatrixWidget Author: Andrew Wason, Bellcore, aw@bae.bellcore.com
 *
 * $Id: Matrix.h,v 1.2 1999-06-04 21:50:09 fnevgeny Exp $
 */

#ifndef _Xbae_Matrix_h
#define _Xbae_Matrix_h

/*
 * Matrix Widget public include file
 */

#include <Xm/Xm.h>
#include <X11/Core.h>
#include <Xbae/patchlevel.h>

/*
 * A few definitions we like to use, but those with R4 won't have.
 * From Xfuncproto.h in R5.
 */

#ifndef XlibSpecificationRelease
# ifndef _XFUNCPROTOBEGIN
#   ifdef __cplusplus                      /* for C++ V2.0 */
#     define _XFUNCPROTOBEGIN extern "C" {   /* do not leave open across includes */
#     define _XFUNCPROTOEND }
#   else
#     define _XFUNCPROTOBEGIN
#     define _XFUNCPROTOEND
#   endif
# endif /* _XFUNCPROTOBEGIN */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Resources:
 * Name			Class			RepType		Default Value
 * ----			-----			-------		-------------
 * allowColumnResize	ColumnResize		Boolean		False
 * altRowCount		AltRowCount		int		1
 * boldLabels		BoldLabels		Boolean		False
 * buttonLabels		ButtonLabels		Boolean		False
 * buttonLabelBackground Color			Pixel		dynamic
 * cellBackgrounds	Colors			PixelTable	NULL
 * cellHighlightThickness HighlightThickness	HorizontalDimension 2
 * cellMarginHeight	MarginHeight		VerticalDimension   5
 * cellMarginWidth	MarginWidth		HorizontalDimension 5
 * cells		Cells			StringTable	NULL
 * cellShadowThickness	ShadowThickness		Dimension	2
 * cellShadowType	ShadowType		unsigned char	SHADOW_OUT
 * cellShadowTypes	CellShadowTypes		ShadowTypeTable NULL
 * cellUserData		CellUserData		UserDataTable	NULL
 * clipWindow		XmCClipWindow		Widget		NULL (get only)
 * colors		Colors			PixelTable	NULL
 * columnAlignments	Alignments		AlignmentArray	dynamic
 * columnButtonLabels	ButtonLabels		BooleanArray	NULL
 * columnLabelAlignments Alignments		AlignmentArray	dynamic
 * columnLabelColor	Color			Pixel		dynamic
 * columnLabels		Labels			StringArray	NULL
 * columnMaxLengths	ColumnMaxLengths	MaxLengthArray	NULL
 * columnShadowTypes	ShadowTypes		ShadowTypeArray NULL
 * columnUserData	UserDatas		UserDataArray	NULL
 * columnWidths		ColumnWidths		WidthArray	NULL
 * columns		Columns			int		0
 * defaultActionCallback Callback               Callback        NULL
 * doubleClickInterval  Interval                int             dynamic
 * drawCellCallback	Callback		Callback	NULL
 * enterCellCallback	Callback		Callback	NULL
 * evenRowBackground	Background		Pixel		dynamic
 * fill			Fill			Boolean		False
 * fixedColumns		FixedColumns		Dimension	0
 * fixedRows		FixedRows		Dimension	0
 * fontList		FontList		FontList	fixed
 * labelFont		FontList		FontList	fixed
 * gridLineColor	Color			Pixel		dynamic
 * gridType		GridType		GridType	XmGRID_LINE
 * highlightedCells	HighlightedCells	HighlightTable	dynamic
 * horizonalScrollBar	HorizonalScrollBar	Widget		NULL (get only)
 * horizontalScrollBarDisplayPolicy
 *			XmCMatrixScrollBarDisplayPolicy
 *						unsigned char	AS_NEEDED
 * labelActivateCallback Callback		Callback	NULL
 * leaveCellCallback	Callback		Callback	NULL
 * leftColumn           LeftColumn              int             0
 * modifyVerifyCallback	Callback		Callback	NULL
 * oddRowBackground	Background		Pixel		NULL
 * processDragCallback	Callback		Callback	NULL
 * resizeCallback	Callback		Callback	NULL
 * resizeColumnCallback	Callback		Callback	NULL
 * reverseSelect	reverseSelect		Boolean		False
 * rowButtonLabels		ButtonLabels		BooleanArray	NULL
 * rowLabelAlignment	Alignment		Alignment	XmALIGNMENT_END
 * rowLabelColor	Color			Pixel		dynamic
 * rowLabelWidth	RowLabelWidth		Short		dynamic
 * rowLabels		Labels			StringArray	NULL
 * rowShadowTypes	ShadowTypes		ShadowTypeArray NULL
 * rowUserData		UserDatas		UserDataArray	NULL
 * rows			Rows			int		0
 * selectCellCallback	Callback		Callback	NULL
 * selectedBackground	Color			Pixel		dynamic
 * selectedCells	SelectedCells		BooleanTable	dynamic
 * selectedForeground	Color			Pixel		dynamic
 * selectScrollVisible	SelectScrollVisible	Boolean		True
 * space		Space			Dimension	6
 * shadowType		ShadowType		unsigned char	SHADOW_OUT
 * textBackground	Backgound		Pixel   	dynamic
 * textField		TextField		Widget		NULL (get only)
 * textShadowThickness	TextShadowThickness	Dimension	0
 * textTranslations	Translations		TranslationTable dynamic
 * topRow		TopRow			int		0
 * trailingFixedColumns	TrailingFixedColumns	Dimension	0
 * trailingFixedRows	TrailingFixedRows	Dimension	0
 * traverseCellCallback	Callback		Callback	NULL
 * traverseFixedCells	TraverseFixedCells	Boolean		False
 * verticalScrollBar	VerticalScrollBar	Widget		NULL (get only)
 * verticalScrollBarDisplayPolicy
 *			XmCMatrixScrollBarDisplayPolicy
 *						unsigned char	AS_NEEDED
 * visibleColumns	VisibleColumns		Dimension	0
 * visibleRows		VisibleRows		Dimension	0
 * writeCellCallback	Callback		Callback	NULL
 */

#define XmNallowColumnResize		"allowColumnResize"
#define XmNaltRowCount			"altRowCount"
#define XmNboldLabels			"boldLabels"
#define XmNbuttonLabels			"buttonLabels"
#define XmNbuttonLabelBackground	"buttonLabelBackground"
#define XmNcellBackgrounds		"cellBackgrounds"
#define XmNcellHighlightThickness	"cellHighlightThickness"
#define XmNcellMarginHeight		"cellMarginHeight"
#define XmNcellMarginWidth		"cellMarginWidth"
#define XmNcellShadowType		"cellShadowType"
#define XmNcellShadowTypes		"cellShadowTypes"
#define XmNcellShadowThickness		"cellShadowThickness"
#define XmNcellUserData			"cellUserData"
#if CELL_WIDGETS
#define XmNcellWidgets			"cellWidgets"
#endif
#define XmNcells			"cells"
#define XmNcolors			"colors"
#define XmNcolumnAlignments		"columnAlignments"
#define XmNcolumnButtonLabels		"columnButtonLabels"
#define XmNcolumnLabelAlignments	"columnLabelAlignments"
#define XmNcolumnLabelBackground	"columnLabelBackground"
#define XmNcolumnLabelColor		"columnLabelColor"
#define XmNcolumnLabels			"columnLabels"
#define XmNcolumnMaxLengths		"columnMaxLengths"
#define XmNcolumnShadowTypes		"columnShadowTypes"
#define XmNcolumnUserData		"columnUserData"
#define XmNcolumnWidths			"columnWidths"
#define XmNeditVerifyCallback		"editVerifyCallback"
#define XmNdrawCellCallback		"drawCellCallback"
#define XmNenterCellCallback		"enterCellCallback"
#define XmNevenRowBackground		"evenRowBackground"
#define XmNfill				"fill"
#define XmNfixedColumns			"fixedColumns"
#define XmNfixedRows			"fixedRows"
#define XmNgridLineColor		"gridLineColor"
#define XmNgridType			"gridType"
#if XmVersion >= 1002
#define XmNhighlightedCells		"highlightedCells"
#endif
#define XmNhorizontalScrollBarDisplayPolicy "horizontalScrollBarDisplayPolicy"
#define XmNlabelActivateCallback	"labelActivateCallback"
#define XmNlabelFont			"labelFont"
#define XmNleaveCellCallback		"leaveCellCallback"
#define XmNleftColumn			"leftColumn"
#define XmNoddRowBackground		"oddRowBackground"
#if XmVersion > 1001
#define XmNprocessDragCallback		"processDragCallback"
#endif
#ifndef XmNresizeCallback
#define XmNresizeCallback               "resizeCallback"
#endif
#define XmNresizeColumnCallback		"resizeColumnCallback"
#define XmNreverseSelect		"reverseSelect"
#define XmNrowButtonLabels		"rowButtonLabels"
#define XmNrowLabelAlignment		"rowLabelAlignment"
#define XmNrowLabelWidth		"rowLabelWidth"
#define XmNrowLabelBackground		"rowLabelBackground"
#define XmNrowLabelColor		"rowLabelColor"
#define XmNrowLabels			"rowLabels"
#define XmNrowShadowTypes		"rowShadowTypes"
#define XmNrowUserData			"rowUserData"
#define XmNselectedCells		"selectedCells"
#define XmNselectedBackground		"selectedBackground"
#define XmNselectCellCallback		"selectCellCallback"
#define XmNselectedForeground		"selectedForeground"
#define XmNselectScrollVisible		"selectScrollVisible"
#define XmNtextBackground		"textBackground"
#ifndef XmNtextField
#define XmNtextField			"textField"
#endif
#define XmNtopRow			"topRow"
#define XmNtrailingFixedColumns		"trailingFixedColumns"
#define XmNtrailingFixedRows		"trailingFixedRows"
#define XmNleftColumn			"leftColumn"
#define XmNtextShadowThickness		"textShadowThickness"
#define XmNtraverseCellCallback		"traverseCellCallback"
#define XmNtraverseFixedCells		"traverseFixedCells"
#define XmNverticalScrollBarDisplayPolicy "verticalScrollBarDisplayPolicy"
#define XmNvisibleColumns		"visibleColumns"
#define XmNvisibleRows			"visibleRows"
#define XmNwriteCellCallback		"writeCellCallback"


#define XmCAlignments			"Alignments"
#define XmCAltRowCount			"AltRowCount"
#define XmCBoldLabels			"BoldLabels"
#define XmCButtonLabels			"ButtonLabels"
#define XmCCells			"Cells"
#define XmCCellShadowTypes		"CellShadowTypes"
#define XmCCellUserData			"CellUserData"
#if CELL_WIDGETS
#define XmCCellWidgets			"CellWidgets"
#endif
#define XmCColors			"Colors"
#define XmCColumnMaxLengths		"ColumnMaxLengths"
#define XmCColumnResize			"ColumnResize"
#define XmCColumnWidths			"ColumnWidths"
#define XmCFill				"Fill"
#define XmCFixedColumns			"FixedColumns"
#define XmCFixedRows			"FixedRows"
#define XmCGridType			"GridType"
#if XmVersion >= 1002
#define XmCHighlightedCells		"HighlightedCells"
#endif
#define XmCLabels			"Labels"
#define XmCLeftColumn			"LeftColumn"
#define XmCMatrixScrollBarDisplayPolicy	"MatrixScrollBarDisplayPolicy"
#define XmCReverseSelect		"ReverseSelect"
#define XmCRowLabelWidth		"RowLabelWidth"
#define XmCSelectedCells		"SelectedCells"
#define XmCSelectScrollVisible		"SelectScrollVisible"
#define XmCShadowTypes			"ShadowTypes"
#define XmCTextBackground		"TextBackground"
#ifndef XmCTextField
#define XmCTextField			"TextField"
#endif
#define XmCTextShadowThickness		"TextShadowThickness"
#define XmCTraverseFixedCells		"TraverseFixedCells"
#define XmCTopRow			"TopRow"
#define XmCTrailingFixedColumns		"TrailingFixedColumns"
#define XmCTrailingFixedRows		"TrailingFixedRows"
#define XmCUserDatas			"UserDatas"
#define XmCVisibleColumns		"VisibleColumns"
#define XmCVisibleRows			"VisibleRows"

#ifndef XmRStringArray
#define XmRStringArray			"StringArray"
#endif

#ifndef XmRBooleanArray
#define XmRBooleanArray			"BooleanArray"
#endif

#define XmRAlignmentArray		"AlignmentArray"
#define XmRBooleanTable			"BooleanTable"
#define XmRCellTable			"CellTable"
#define XmRWidgetTable			"WidgetTable"
#define XmRGridType			"GridType"
#if XmVersion >= 1002
#define XmRHighlightTable		"HighlightTable"
#endif
#define XmRMatrixScrollBarDisplayPolicy "MatrixScrollBarDisplayPolicy"
#define XmRMaxLengthArray		"MaxLengthArray"
#define XmRPixelTable			"PixelTable"
#define XmRShadowTypeTable		"ShadowTypeTable"
#define XmRShadowTypeArray		"ShadowTypeArray"
#define XmRUserDataTable		"UserDataTable"
#define XmRUserDataArray		"UserDataArray"
#define XmRWidthArray			"WidthArray"


#ifndef XbaeIsXbaeMatrix
#define XbaeIsXbaeMatrix( w )	XtIsSubclass(w, xbaeMatrixWidgetClass)
#endif /* XbaeIsXbaeMatrix */

/* Class record constants */

externalref WidgetClass xbaeMatrixWidgetClass;

typedef struct _XbaeMatrixClassRec *XbaeMatrixWidgetClass;
typedef struct _XbaeMatrixRec *XbaeMatrixWidget;

/*
 * A few definitions we like to use, but those with R4 won't have.
 * From Xfuncproto.h.
 */

/*
 * Prototype wrapper
 */
#ifndef P
#if defined(__STDC__) || defined (__cplusplus)
#define P(x)		x
#else
#define P(x)		()
#define const
#define volatile
#endif
#endif

#ifndef XlibSpecificationRelease
# ifndef _XFUNCPROTOBEGIN
#   ifdef __cplusplus                      /* for C++ V2.0 */
#     define _XFUNCPROTOBEGIN extern "C" {
#     define _XFUNCPROTOEND }
#   else
#     define _XFUNCPROTOBEGIN
#     define _XFUNCPROTOEND
#   endif
# endif /* _XFUNCPROTOBEGIN */
#else
#include <X11/Xfuncproto.h>
#endif

/*
 * External interfaces to class methods
 */
_XFUNCPROTOBEGIN

extern void XbaeMatrixAddColumns P(( Widget, int, String *, String *, short *,
				     int *, unsigned char *, unsigned char *,
				     Pixel *, int ));
extern void XbaeMatrixAddRows P(( Widget,  int , String *, String *,
				  Pixel *, int ));
extern void XbaeMatrixCancelEdit P(( Widget, Boolean ));
extern Boolean XbaeMatrixCommitEdit P(( Widget, Boolean ));
extern void XbaeMatrixDeleteColumns P(( Widget, int, int ));
extern void XbaeMatrixDeleteRows P(( Widget, int, int ));
extern void XbaeMatrixDeselectAll P(( Widget ));
extern void XbaeMatrixDeselectCell P(( Widget, int, int ));
extern void XbaeMatrixDeselectColumn P(( Widget, int ));
extern void XbaeMatrixDeselectRow P(( Widget, int ));
extern void XbaeMatrixEditCell P(( Widget, int, int ));
extern void XbaeMatrixFirstSelectedCell P(( Widget, int *, int * ));
extern int XbaeMatrixFirstSelectedColumn P(( Widget ));
extern int XbaeMatrixFirstSelectedRow P(( Widget ));
extern String XbaeMatrixGetCell P(( Widget, int, int ));
extern XtPointer XbaeMatrixGetCellUserData P(( Widget, int, int ));
extern XtPointer XbaeMatrixGetColumnUserData P(( Widget, int ));
extern void XbaeMatrixGetCurrentCell P(( Widget, int *, int * ));
extern int XbaeMatrixGetEventRowColumn P(( Widget, XEvent *, int *, int * ));
extern Boolean XbaeMatrixEventToXY P(( Widget, XEvent *, int *, int * ));
extern Boolean XbaeMatrixRowColToXY P(( Widget, int, int, int *, int * ));
extern int XbaeMatrixGetNumSelected P(( Widget ));
extern XtPointer XbaeMatrixGetRowUserData P(( Widget, int ));
extern Boolean XbaeMatrixIsCellSelected P(( Widget, int, int ));
extern Boolean XbaeMatrixIsColumnSelected P(( Widget, int ));
extern Boolean XbaeMatrixIsRowSelected P(( Widget, int ));
extern void XbaeMatrixRefresh P(( Widget ));
extern void XbaeMatrixRefreshCell P(( Widget, int, int ));
extern void XbaeMatrixSelectAll P(( Widget ));
extern void XbaeMatrixSelectCell P(( Widget, int, int ));
extern void XbaeMatrixSelectColumn P(( Widget, int ));
extern void XbaeMatrixSelectRow P(( Widget, int ));
#if XmVersion >= 1002
extern void XbaeMatrixHighlightCell P(( Widget, int, int ));
extern void XbaeMatrixHighlightRow P(( Widget, int ));
extern void XbaeMatrixHighlightColumn P(( Widget, int ));
extern void XbaeMatrixUnhighlightCell P(( Widget, int, int ));
extern void XbaeMatrixUnhighlightRow P(( Widget, int ));
extern void XbaeMatrixUnhighlightColumn P(( Widget, int ));
extern void XbaeMatrixUnhighlightAll P(( Widget ));
#endif
extern void XbaeMatrixSetCell P(( Widget, int, int, const String ));
extern void XbaeMatrixSetCellBackground P(( Widget, int, int, Pixel ));
extern void XbaeMatrixSetCellColor P(( Widget, int, int, Pixel ));
extern void XbaeMatrixSetCellUserData P(( Widget, int, int, XtPointer ));
#if CELL_WIDGETS
extern void XbaeMatrixSetCellWidget P(( Widget, int, int, Widget ));
#endif
extern void XbaeMatrixSetColumnBackgrounds P(( Widget, int, Pixel *, int ));
extern void XbaeMatrixSetColumnColors P(( Widget, int, Pixel *, int ));
extern void XbaeMatrixSetColumnUserData P(( Widget, int, XtPointer ));
extern void XbaeMatrixSetRowBackgrounds P(( Widget, int, Pixel *, int ));
extern void XbaeMatrixSetRowColors P(( Widget, int , Pixel *, int ));
extern void XbaeMatrixSetRowUserData P(( Widget, int, XtPointer ));
extern int XbaeMatrixVisibleColumns P(( Widget ));
extern int XbaeMatrixVisibleRows P(( Widget ));
extern int XbaeMatrixNumColumns P(( Widget ));
extern int XbaeMatrixNumRows P(( Widget ));
extern void XbaeMatrixDisableRedisplay P(( Widget ));
extern void XbaeMatrixEnableRedisplay P(( Widget, Boolean ));
extern void XbaeMatrixMakeCellVisible P(( Widget, int, int ));
extern Boolean XbaeMatrixIsRowVisible P(( Widget, int ));
extern Boolean XbaeMatrixIsColumnVisible P(( Widget, int ));
extern Boolean XbaeMatrixIsCellVisible P(( Widget, int, int ));
extern void XbaeMatrixVisibleCells P(( Widget, int *, int *, int *, int * ));
extern String XbaeMatrixGetColumnLabel P(( Widget, int ));
extern String XbaeMatrixGetRowLabel P(( Widget, int ));
extern void XbaeMatrixSetColumnLabel P(( Widget, int, String ));
extern void XbaeMatrixSetRowLabel P(( Widget, int, String ));

_XFUNCPROTOEND

typedef unsigned char	Alignment;
typedef Alignment *	AlignmentArray;
typedef String *	StringTable;
typedef short 		Width;
typedef Width *		WidthArray;
typedef int 		MaxLength;
typedef MaxLength *	MaxLengthArray;

/*
 * cell shadow types
 */
 
enum
{
    XmGRID_NONE,
    XmGRID_LINE,
    XmGRID_SHADOW_IN,
    XmGRID_SHADOW_OUT,
    XmGRID_ROW_SHADOW,
    XmGRID_COLUMN_SHADOW
};


/*
 * Enumeration for Matrix ScrollBar Display Policy
 */
enum
{
    XmDISPLAY_NONE,
    XmDISPLAY_AS_NEEDED,
    XmDISPLAY_STATIC
};


/*
 * Enumeration for type of a cell
 */
typedef enum {
    FixedCell, NonFixedCell, RowLabelCell, ColumnLabelCell
} CellType;

#if XmVersion >= 1002
/*
 * Enumeration for highlight reason/location
 */
enum {
    HighlightNone	= 0x0000,
    HighlightCell	= 0x0001,
    HighlightRow	= 0x0002,
    HighlightColumn	= 0x0004,
    HighlightOther	= 0x0008,
    UnhighlightCell	= 0x0010,
    UnhighlightRow	= 0x0020,
    UnhighlightColumn	= 0x0040,
    UnhighlightAll	= UnhighlightCell | UnhighlightRow | UnhighlightColumn
};
#endif

/*
 * Callback reasons.  Try to stay out of range of the Motif XmCR_* reasons.
 */
typedef enum _XbaeReasonType
{
    XbaeModifyVerifyReason = 102,
    XbaeEnterCellReason,
    XbaeLeaveCellReason,
    XbaeTraverseCellReason,
    XbaeSelectCellReason,
    XbaeDrawCellReason,
    XbaeWriteCellReason,
    XbaeResizeReason,
    XbaeResizeColumnReason,
    XbaeDefaultActionReason,
    XbaeProcessDragReason,
    XbaeLabelActivateReason
}
XbaeReasonType;

/*
 * DrawCell types.
 */
typedef enum
{
    XbaeString=1,
    XbaePixmap
}
XbaeCellType;

/*
 * Struct passed to modifyVerifyCallback
 */
typedef struct _XbaeMatrixModifyVerifyCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    XmTextVerifyCallbackStruct *verify;
    const char *prev_text;
}
XbaeMatrixModifyVerifyCallbackStruct;

/*
 * Struct passed to enterCellCallback
 */
typedef struct _XbaeMatrixEnterCellCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    Boolean select_text;
    Boolean map;
    Boolean doit;
}
XbaeMatrixEnterCellCallbackStruct;

/*
 * Struct passed to leaveCellCallback
 */
typedef struct _XbaeMatrixLeaveCellCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    String value;
    Boolean doit;
}
XbaeMatrixLeaveCellCallbackStruct;

/*
 * Struct passed to traverseCellCallback
 */
typedef struct _XbaeMatrixTraverseCellCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    int next_row, next_column;
    int fixed_rows, fixed_columns;
    int trailing_fixed_rows, trailing_fixed_columns;
    int num_rows, num_columns;
    String param;
    XrmQuark qparam;
}
XbaeMatrixTraverseCellCallbackStruct;

/*
 * Struct passed to selectCellCallback
 */
typedef struct _XbaeMatrixSelectCellCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    Boolean **selected_cells;
    String **cells;
    Cardinal num_params;
    String *params;
    XEvent *event;
}
XbaeMatrixSelectCellCallbackStruct;

/*
 * Struct passed to drawCellCallback
 */
typedef struct _XbaeMatrixDrawCellCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    int width, height;
    XbaeCellType type;
    String string;
    Pixmap pixmap;
    Pixmap mask;
    Pixel foreground, background;
}
XbaeMatrixDrawCellCallbackStruct;

/*
 * Struct passed to writeCellCallback
 */
typedef struct _XbaeMatrixWriteCellCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    XbaeCellType type;
    String string;
    Pixmap pixmap;
    Pixmap mask;
}
XbaeMatrixWriteCellCallbackStruct;


/*
 * Struct passed to resizeCallback
 */
typedef struct _XbaeMatrixResizeCallbackStruct
{
    XbaeReasonType reason;
    Dimension width;
    Dimension height;
}
XbaeMatrixResizeCallbackStruct;

/*
 * Struct passed to resizeColumnCallback
 *
 */
typedef struct _XbaeMatrixResizeColumnCallbackStruct
{
    XbaeReasonType reason;
    int which;
    int columns;    
    short *column_widths;
    XEvent *event;
}
XbaeMatrixResizeColumnCallbackStruct;

#if XmVersion > 1001
/*
 * Struct passed to processDragCallback
 */
typedef struct _XbaeMatrixProcessDragCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    String string;
    XbaeCellType type;
    Pixmap pixmap;
    Pixmap mask;
    Cardinal num_params;
    String *params;
    XEvent *event;
}
XbaeMatrixProcessDragCallbackStruct;
#endif

/*
 * Struct passed to defaultActionCallback
 */
typedef struct _XbaeMatrixDefaultActionCallbackStruct
{
    XbaeReasonType reason;
    int row, column;
    XEvent *event;
}
XbaeMatrixDefaultActionCallbackStruct;

/*
 * Struct passed to labelActivateCallback
 */
typedef struct _XbaeMatrixLabelActivateCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    Boolean row_label;
    int row, column;
    String label;
}
XbaeMatrixLabelActivateCallbackStruct;


/* provide clean-up for those with R4 */
#ifndef XlibSpecificationRelease
# undef _Xconst
# undef _XFUNCPROTOBEGIN
# undef _XFUNCPROTOEND
#endif

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _Xbae_Matrix_h */
