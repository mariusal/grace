/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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

#ifndef __XSTRINGS_H_
#define __XSTRINGS_H_

/* Element names */
#define EStrAnnotation          "annotation"
#define EStrArcData             "arc-data"
#define EStrAxis                "axis"
#define EStrAxisbar             "axisbar"
#define EStrAxislabel           "axislabel"
#define EStrBarline             "barline"
#define EStrBoxData             "box-data"
#define EStrColorDef            "color-def"
#define EStrColormap            "colormap"
#define EStrDataFormats         "data-formats"
#define EStrDataset             "dataset"
#define EStrDates               "dates"
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
#define EStrLineSpec            "line-spec"
#define EStrLocator             "locator"
#define EStrMajor               "major"
#define EStrMinor               "minor"
#define EStrObject              "object"
#define EStrPage                "page"
#define EStrPlacement           "placement"
#define EStrPresentationSpec    "presentation-spec"
#define EStrRiserline           "riserline"
#define EStrRow                 "row"
#define EStrSet                 "set"
#define EStrStringData          "string-data"
#define EStrSubtitle            "subtitle"
#define EStrSymbol              "symbol"
#define EStrText                "text"
#define EStrTicklabels          "ticklabels"
#define EStrTickmarks           "tickmarks"
#define EStrTick                "tick"
#define EStrTicks               "ticks"
#define EStrTimeStamp           "time-stamp"
#define EStrTitle               "title"
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
#define AStrAngle               "angle"
#define AStrAppend              "append"
#define AStrArrowClip           "arrow-clip"
#define AStrAutoTicking         "auto-ticking"
#define AStrBargap              "bargap"
#define AStrBaselineType        "baseline-type"
#define AStrChar                "char"
#define AStrCharSize            "char-size"
#define AStrClipLength          "clip-length"
#define AStrColorId             "color-id"
#define AStrComment             "comment"
#define AStrDataRef             "data-ref"
#define AStrDrawBaseline        "draw-baseline"
#define AStrDrawDroplines       "draw-droplines"
#define AStrFallback            "fallback"
#define AStrFillMode            "fill-mode"
#define AStrFillRule            "fill-rule"
#define AStrFillType            "fill-type"
#define AStrFontId              "font-id"
#define AStrFormat              "format"
#define AStrGridLines           "grid-lines"
#define AStrHeight              "height"
#define AStrHgap                "hgap"
#define AStrHotfile             "hotfile"
#define AStrId                  "id"
#define AStrInoutPlacement      "inout-placement"
#define AStrInvert              "invert"
#define AStrJustification       "justification"
#define AStrLabel               "label"
#define AStrLayout              "layout"
#define AStrLength              "length"
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
#define AStrSidePlacement       "side-placement"
#define AStrSize                "size"
#define AStrSkip                "skip"
#define AStrStacked             "stacked"
#define AStrStagger             "stagger"
#define AStrStart               "start"
#define AStrStartAngle          "start-angle"
#define AStrStop                "stop"
#define AStrStopAngle           "stop-angle"
#define AStrStyleId             "style-id"
#define AStrTransform           "transform"
#define AStrType                "type"
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
#define AStrZero                "zero"

/* Attribute value names */
#define VStrAuto                "auto"
#define VStrBoth                "both"
#define VStrEvenodd             "evenodd"
#define VStrIn                  "in"
#define VStrMajor               "major"
#define VStrMinor               "minor"
#define VStrNone                "none"
#define VStrNormal              "normal"
#define VStrOpposite            "opposite"
#define VStrOut                 "out"
#define VStrParallel            "parallel"
#define VStrPerpendicular       "perpendicular"
#define VStrTicks               "ticks"
#define VStrWinding             "winding"

#endif /* __XSTRINGS_H_ */
