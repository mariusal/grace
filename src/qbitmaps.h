/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2004 Grace Development Team
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
 * Bitmaps for quark tree
 */

#ifndef __QBITMAPS_H_
#define __QBITMAPS_H_

static char *active_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 16 14 1",
"- c None",
"  c #000000",
". c #008000",
"X c #008080",
"o c #808000",
"O c #00C000",
"+ c #00FF00",
"@ c #00C0C0",
"# c #00FFFF",
"$ c #C0C000",
"% c #FFFF00",
"& c #C0FFC0",
"* c #C0FFFF",
"= c #FFFFC0",
/* pixels */
"-----  ---------",
"---- &&  -------",
"--- &&&++ ------",
"-- +++++.   ----",
"-- OO++.. **  --",
"-- OOO.. ***## -",
"-- OOO. #####X -",
"---   . @@##XX -",
"--- ==  @@@XXX -",
"-- ===%% @@XX --",
"- %%%%%o  @X ---",
"- $$%%oo -  ----",
"- $$$ooo -------",
"- $$$oo --------",
"--  $o ---------",
"----  ----------"
};

static char *hidden_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 16 13 1",
" 	c None",
".	c #000000",
"+	c #E5E5E5",
"@	c #969696",
"#	c #4B4B4B",
"$	c #717171",
"%	c #ECECEC",
"&	c #B2B2B2",
"*	c #595959",
"=	c #868686",
"-	c #F8F8F8",
";	c #E2E2E2",
">	c #AAAAAA",
"     ..         ",
"    .++..       ",
"   .+++@@.      ",
"  .@@@@@#...    ",
"  .$$@@##.%%..  ",
"  .$$$##.%%%&&. ",
"  .$$$#.&&&&&*. ",
"   ...#.==&&**. ",
"   .--..===***. ",
"  .---;;.==**.  ",
" .;;;;;$..=*.   ",
" .>>;;$$. ..    ",
" .>>>$$$.       ",
" .>>>$$.        ",
"  ..>$.         ",
"    ..          "
};

#endif /* __QBITMAPS_H_ */
