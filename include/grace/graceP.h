/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2006 Grace Development Team
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

#ifndef __GRACEP_H_
#define __GRACEP_H_

#include "grace/grace.h"

/* XML Namespace/URL */
#define GRACE_NS_PREFIX "grace"
#define GRACE_NS_URI    "http://plasma-gate.weizmann.ac.il/Grace/"

/* Version ID of the current XGR format */
#define XGR_VERSION_ID  59901

/* Element names */
#define EStrAGrid               "agrid"
#define EStrAText               "atext"
#define EStrAnnotation          "annotation"
#define EStrArcData             "arc-data"
#define EStrArrow               "arrow"
#define EStrAxis                "axis"
#define EStrAxisbar             "axisbar"
#define EStrAxislabel           "axislabel"
#define EStrBarline             "barline"
#define EStrBoxData             "box-data"
#define EStrCell                "c"
#define EStrColorDef            "color-def"
#define EStrColormap            "colormap"
#define EStrDataFormats         "data-formats"
#define EStrDataset             "dataset"
#define EStrDates               "dates"
#define EStrDcolumn             "dcolumn"
#define EStrDefinitions         "definitions"
#define EStrDescription         "description"
#define EStrErrorbar            "errorbar"
#define EStrFaceSpec            "face-spec"
#define EStrFillSpec            "fill-spec"
#define EStrFixedpoint          "fixedpoint"
#define EStrFontDef             "font-def"
#define EStrFontmap             "fontmap"
#define EStrFormat              "format"
#define EStrFrame               "frame"
#define EStrGrace               "grace"
#define EStrGraph               "graph"
#define EStrLegend              "legend"
#define EStrLegendEntry         "legend-entry"
#define EStrLegframe            "legframe"
#define EStrLine                "line"
#define EStrLineData            "line-data"
#define EStrLineSpec            "line-spec"
#define EStrLocation            "location"
#define EStrLocator             "locator"
#define EStrMajorGridlines      "major-gridlines"
#define EStrMajorTickmarks      "major-tickmarks"
#define EStrMinorGridlines      "minor-gridlines"
#define EStrMinorTickmarks      "minor-tickmarks"
#define EStrObject              "object"
#define EStrPage                "page"
#define EStrPointer             "pointer"
#define EStrPresentationSpec    "presentation-spec"
#define EStrRegion              "region"
#define EStrRiserline           "riserline"
#define EStrRow                 "row"
#define EStrSSD                 "ssd"
#define EStrScales              "scales"
#define EStrScolumn             "scolumn"
#define EStrSet                 "set"
#define EStrSymbol              "symbol"
#define EStrText                "text"
#define EStrTextFrame           "text-frame"
#define EStrTextProperties      "text-properties"
#define EStrTicklabels          "ticklabels"
#define EStrTick                "tick"
#define EStrTimeStamp           "time-stamp"
#define EStrUserticks           "userticks"
#define EStrViewport            "viewport"
#define EStrWorld               "world"
#define EStrXformat             "xformat"
#define EStrXscale              "xscale"
#define EStrYformat             "yformat"
#define EStrYscale              "yscale"
#define EStrZscale              "zscale"

/* Attribute names */
#define AStrActive              "active"
#define AStrAnchor              "anchor"
#define AStrAngle               "angle"
#define AStrAppend              "append"
#define AStrArrowClip           "arrow-clip"
#define AStrArrowsAt            "arrows-at"
#define AStrAutoPlacement       "auto-placement"
#define AStrAutoTicking         "auto-ticking"
#define AStrBar                 "bar"
#define AStrBargap              "bargap"
#define AStrBaselineType        "baseline-type"
#define AStrChar                "char"
#define AStrCharSize            "char-size"
#define AStrClipLength          "clip-length"
#define AStrClosureType         "closure-type"
#define AStrColorId             "color-id"
#define AStrColumn              "column"
#define AStrDataRef             "data-ref"
#define AStrDlFf                "dl-ff"
#define AStrDrawClosure         "draw-closure"
#define AStrLlFf                "ll-ff"
#define AStrDrawArrow           "draw-arrow"
#define AStrDrawBaseline        "draw-baseline"
#define AStrDrawDroplines       "draw-droplines"
#define AStrExtentAngle         "extent-angle"
#define AStrFallback            "fallback"
#define AStrFill                "fill"
#define AStrFillRule            "fill-rule"
#define AStrFillType            "fill-type"
#define AStrFontId              "font-id"
#define AStrFontSize            "font-size"
#define AStrFormat              "format"
#define AStrFormatString        "format-string"
#define AStrFrameDecor          "frame-decor"
#define AStrFrameOffset         "frame-offset"
#define AStrHJust               "hjust"
#define AStrHeight              "height"
#define AStrHgap                "hgap"
#define AStrHotfile             "hotfile"
#define AStrId                  "id"
#define AStrIndexed             "indexed"
#define AStrInoutPlacement      "inout-placement"
#define AStrInvert              "invert"
#define AStrLabel               "label"
#define AStrLabels              "labels"
#define AStrLayout              "layout"
#define AStrLength              "length"
#define AStrLineWidth           "line-width"
#define AStrMajorStep           "major-step"
#define AStrMax                 "max"
#define AStrMin                 "min"
#define AStrMinorDivisions      "minor-divisions"
#define AStrName                "name"
#define AStrNorm                "norm"
#define AStrOffset              "offset"
#define AStrPatternId           "pattern-id"
#define AStrPosition            "position"
#define AStrPrec                "prec"
#define AStrPrepend             "prepend"
#define AStrReference           "reference"
#define AStrRgb                 "rgb"
#define AStrRoundedPosition     "rounded-position"
#define AStrRow                 "row"
#define AStrRows                "rows"
#define AStrSingleSymbol        "single-symbol"
#define AStrSize                "size"
#define AStrSkip                "skip"
#define AStrSkipMinDist         "skipmindist"
#define AStrStacked             "stacked"
#define AStrStagger             "stagger"
#define AStrStart               "start"
#define AStrStartAngle          "start-angle"
#define AStrStop                "stop"
#define AStrStyleId             "style-id"
#define AStrTicks               "ticks"
#define AStrTransform           "transform"
#define AStrType                "type"
#define AStrVJust               "vjust"
#define AStrValue               "value"
#define AStrVersion             "version"
#define AStrVgap                "vgap"
#define AStrWidth               "width"
#define AStrWrap                "wrap"
#define AStrWrapYear            "wrap-year"
#define AStrX                   "x"
#define AStrXmax                "xmax"
#define AStrXmin                "xmin"
#define AStrY                   "y"
#define AStrYmax                "ymax"
#define AStrYmin                "ymin"

/* Attribute value names */
#define VStrArc                 "arc"
#define VStrAuto                "auto"
#define VStrBand                "band"
#define VStrBaseline            "baseline"
#define VStrBeginning           "beginning"
#define VStrBoth                "both"
#define VStrBottom              "bottom"
#define VStrBox                 "box"
#define VStrBreakBottom         "break-bottom"
#define VStrBreakLeft           "break-left"
#define VStrBreakRight          "break-right"
#define VStrBreakTop            "break-top"
#define VStrCenter              "center"
#define VStrChar                "char"
#define VStrChord               "chord"
#define VStrCircle              "circle"
#define VStrClosed              "closed"
#define VStrDateTime            "datetime"
#define VStrDecimal             "decimal"
#define VStrDiamond             "diamond"
#define VStrEnd                 "end"
#define VStrEngineering         "engineering"
#define VStrEvenodd             "evenodd"
#define VStrExponential         "exponential"
#define VStrFormula             "formula"
#define VStrGeneral             "general"
#define VStrGeographic          "geographic"
#define VStrGraphMax            "graph-max"
#define VStrGraphMin            "graph-min"
#define VStrHalfOpen            "half-open"
#define VStrFilled              "filled"
#define VStrIn                  "in"
#define VStrLeft                "left"
#define VStrLeftStairs          "left_stairs"
#define VStrLine                "line"
#define VStrLogarithmic         "logarithmic"
#define VStrLogit               "logit"
#define VStrMajor               "major"
#define VStrMiddle              "middle"
#define VStrMinor               "minor"
#define VStrNone                "none"
#define VStrNormal              "normal"
#define VStrOpposite            "opposite"
#define VStrOut                 "out"
#define VStrOval                "oval"
#define VStrParallel            "parallel"
#define VStrPerpendicular       "perpendicular"
#define VStrPieSlice            "pie-slice"
#define VStrPlus                "plus"
#define VStrPolar               "polar"
#define VStrPolygon             "polygon"
#define VStrPower               "power"
#define VStrReciprocal          "reciprocal"
#define VStrRectangle           "rectangle"
#define VStrRight               "right"
#define VStrRightStairs         "right_stairs"
#define VStrScientific          "scientific"
#define VStrSegment             "segment"
#define VStrSegment3            "segment3"
#define VStrSetMax              "set-max"
#define VStrSetMin              "set-min"
#define VStrSplat               "splat"
#define VStrSquare              "square"
#define VStrStraight            "straight"
#define VStrTicks               "ticks"
#define VStrTop                 "top"
#define VStrTriangleDown        "triangle-down"
#define VStrTriangleLeft        "triangle-left"
#define VStrTriangleRight       "triangle-right"
#define VStrTriangleUp          "triangle-up"
#define VStrView                "view"
#define VStrWinding             "winding"
#define VStrWorld               "world"
#define VStrX                   "x"
#define VStrXY                  "xy"
#define VStrZero                "zero"

#define T1_DEFAULT_ENCODING_FILE  "Default.enc"
#define T1_FALLBACK_ENCODING_FILE "IsoLatin1.enc"

/* Typesetting defines */
#define SSCRIPT_SCALE M_SQRT1_2
#define SUBSCRIPT_SHIFT 0.4
#define SUPSCRIPT_SHIFT 0.6
#define ENLARGE_SCALE sqrt(M_SQRT2)
#define OBLIQUE_FACTOR 0.25

struct _Grace {
    QuarkFactory *qfactory;
    
    /* location of the Grace home directory */
    char *grace_home;
    
    /* username */
    char *username;

    /* $HOME */
    char *userhome;
    
    Canvas *canvas;

    Graal  *graal;
    
    /* Misc dictionaries */
    Dictionary *graph_type_dict;
    Dictionary *set_type_dict;
    Dictionary *object_type_dict;
    Dictionary *inout_placement_dict;
    Dictionary *spec_ticks_dict;
    Dictionary *region_type_dict;
    Dictionary *axis_position_dict;
    Dictionary *arrow_type_dict;
    Dictionary *glocator_type_dict;
    Dictionary *sym_type_dict;
    Dictionary *line_type_dict;
    Dictionary *setfill_type_dict;
    Dictionary *baseline_type_dict;
    Dictionary *framedecor_type_dict;
    Dictionary *scale_type_dict;
    Dictionary *arrow_placement_dict;
    Dictionary *arcclosure_type_dict;
    Dictionary *format_type_dict;
    Dictionary *frame_type_dict;
    Dictionary *dataset_col_dict;
    
    void *udata;
};

/* tokens for the calendar dates parser */
typedef struct {
    int value;
    int digits;
} Int_token;

/* typeset.c */
int grace_init_font_db(const Grace *grace);
int grace_csparse_proc(const Canvas *canvas,
    const char *s, CompositeString *cstring);
int grace_fmap_proc(const Canvas *canvas, int font);

/* paths.c */
char *grace_path2(const Grace *grace, const char *prefix, const char *path);

/* dicts.c */
int grace_init_dicts(Grace *grace);
void grace_free_dicts(Grace *grace);


#endif /* __GRACEP_H_ */
