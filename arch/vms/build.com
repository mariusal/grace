$ V = 'F$VERIFY(0)'
$ SAY = "WRITE SYS$OUTPUT"
$ !
$ OPEN/READ IN MAKE.CONF
$LOOP_CONF:
$ READ/END=DONE_CONF IN REC
$ IF (F$ELEMENT(1,"=",REC) .NES. "=")
$ THEN
$   SYM = F$ELEMENT (0, "=", REC)
$   VAL = REC - SYM - "="
$   'SYM' = "''VAL'"
$ ENDIF
$ GOTO LOOP_CONF
$DONE_CONF:
$ CLOSE IN
$ !
$ SAY "Building CEPHES"
$ SET DEFAULT [.CEPHES]
$ CFLAGS = CFLAGS0 + "/INCLUDE=[-]"
$ LIB = "libcephes.olb"
$ SRCS = "airy beta chbevl chdtr const dawsn ellie ellik " -
       + "ellpe ellpk expn fac fdtr fresnl gamma gdtr hyp2f1 " -
       + "hyperg i0 i1 igam igami incbet incbi iv j0 j1 " -
       + "jn jv k0 k1 kn log2 mtherr ndtri pdtr polevl " -
       + "polyn psi revers rgamma round shichi sici " -
       + "spence stdtr struve unity yn zeta zetac " -
       + "acosh asinh atanh ndtr isfinite cbrt"
$ GOSUB COMPILE
$ SET DEFAULT [-]
$ !
$ SAY "Building T1LIB"
$ SET DEFAULT [.T1LIB.TYPE1]
$ CFLAGS = CFLAGS0 -
         + "/INCLUDE=[--]/DEFINE=(GLOBAL_CONFIG_DIR=""""""[]"""""", " -
         + "T1_AA_TYPE16=""''T1_AA_TYPE16'"",T1_AA_TYPE32=""''T1_AA_TYPE32'"")"
$ LIB = "[-]libt1lib.olb"
$ SRCS = "arith curves fontfcn hints lines objects paths regions scanfont " -
       + "spaces t1io t1snap t1stub token type1 util"
$ GOSUB COMPILE
$ SET DEFAULT [-.T1LIB]
$ SRCS = "t1finfo t1base t1delete t1enc t1env t1load t1set t1trans t1aaset " -
       + "t1afmtool parseAFM"
$ GOSUB COMPILE
$ SET DEFAULT [--]
$ !
$ SAY "Building XBAE"
$ SET DEFAULT [.XBAE]
$ XBAE = F$PARSE("[-]",,,"DEVICE") + F$PARSE("[-]",,,"DIRECTORY") - "]" + ".XBAE]"
$ DEFINE/NOLOG XBAE 'XBAE'
$ CFLAGS = CFLAGS0 + "/INCLUDE=[-]" + GUI_FLAGS -
         + "/DEFINE=(DRAW_RESIZE_SHADOW)/WARNINGS=(DISABLE=LONGEXTERN)"
$ LIB = "libXbae.OLB"
$ SRCS = "Actions Cell Clip Converters Create Draw " -
       + "Matrix Methods Public ScrollMgr Shadow Utils"
$ GOSUB COMPILE
$ DEASSIGN XBAE
$ SET DEFAULT [-]
$ !
$ SAY "Building SRC"
$ SET DEFAULT [.SRC]
$ CEPHES = F$PARSE("[-]",,,"DEVICE") + F$PARSE("[-]",,,"DIRECTORY") -
       - "]" + ".CEPHES]"
$ DEFINE/NOLOG CEPHES 'CEPHES'
$ XBAE = F$PARSE("[-]","","","DEVICE") + F$PARSE("[-]","","","DIRECTORY") -
       - "]" + ".XBAE]"
$ DEFINE/NOLOG XBAE 'XBAE'
$ CFLAGS = CFLAGS0 + "/INCLUDE=([-],[-.T1LIB.T1LIB]''NETCDF_INC')" -
         + "/DEFINE=(GRACE_HOME=""""""''GRACE_HOME'""""""," -
         + "GRACE_HELPVIEWER=""""""''HELPVIEWER'""""""," -
         + "PRINT_CMD=""""""''PRINT_CMD'""""""," -
         + """lines=lines_"",""xfree=xfree_"")"
$ LIB = ""
$ SRCS = "main plotone files utils drawticks " -
       + "nonlfit lmdif as274c fit fourier " -
       + "graphs graphutils setutils regionutils " -
       + "objutils computils defaults params " -
       + "compute draw dlmodule missing " -
       + "iofilters t1fonts device mfdrv psdrv " -
       + "dummydrv pars"
$ IF (ALLOCA .NES. "") THEN SRCS = SRCS + " alloca"
$ IF (PDFDRV_O .NES. "") THEN SRCS = SRCS + " pdfdrv"
$ IF (RSTDRV_O .NES. "") THEN SRCS = SRCS + " rstdrv"
$ GOSUB COMPILE
$ SRC1 = SRCL
$ SRCS = "Tab motifutils " -
       + "compwin comwin eblockwin " -
       + "editpwin events featext fileswin plotwin " -
       + "graphappwin helpwin hotwin " -
       + "locatewin miscwin monwin " -
       + "nonlwin printwin ptswin regionwin " -
       + "setwin strwin setappwin " -
       + "tickwin worldwin fontwin xutil xmgrace"
$ IF (TERMDRV_O .NES. "") THEN SRCS = SRCS + " " + F$ELEMENT (0, "$", TERMDRV_O)
$ GOSUB COMPILE
$ SRC2 = SRCL
$ GUI_LIBS = F$ELEMENT (1, ")", GUI_LIBS)
$ V = F$VERIFY(1)
$ LINK /EXECUTABLE=xmgrace.exe 'LDFLAGS' 'SRC1','SRC2', -
    [-.ARCH.VMS]'GUI_LIBS',[-.CEPHES]LIBCEPHES.OLB/LIBRARY'NETCDF_LIBS', -
    [-.T1LIB]LIBT1LIB.OLB/LIBRARY,[-.XBAE]LIBXBAE.OLB/LIBRARY
$ V = 'F$VERIFY(0)'
$ DEASSIGN CEPHES
$ DEASSIGN XBAE
$ SET DEFAULT [-]
$ !
$ SAY "Done"
$ EXIT
$ !
$COMPILE:
$ IF (LIB .NES. "" .AND. F$SEARCH(LIB) .EQS. "") THEN LIBRARY/CREATE/LOG 'LIB'
$ SRCL = ""
$ N = 0
$LOOP_COMPILE:
$ FILE = F$ELEMENT (N, " ", SRCS)
$ N = N + 1
$ IF (FILE .EQS. " ") THEN RETURN
$ IF (FILE .EQS. "") THEN GOTO LOOP_COMPILE
$ V = F$VERIFY(1)
$ 'CC''CFLAGS' 'FILE'.C
$ V = 'F$VERIFY(0)'
$ IF (LIB .EQS. "")
$ THEN
$   IF (SRCL .NES. "") THEN SRCL = SRCL + ","
$   SRCL = SRCL + FILE + ".OBJ"
$ ELSE
$   LIBRARY/LOG 'LIB' 'FILE'.OBJ
$ ENDIF
$ GOTO LOOP_COMPILE
