$ V = 'F$VERIFY(0)'
$ SAY = "WRITE SYS$OUTPUT"
$ !
$ OPEN/READ IN MAKE.CONF
$LOOP_CONF:
$ READ/END=DONE_CONF IN REC
$ IF (F$EXTRACT(0,1,REC) .NES. "#" .AND. F$ELEMENT(1,"=",REC) .NES. "=")
$ THEN
$   SYM = F$ELEMENT (0, "=", REC)
$   VAL = REC - SYM - "="
$   'SYM' = "''VAL'"
$ ENDIF
$ GOTO LOOP_CONF
$DONE_CONF:
$ CLOSE IN
$ !
$ IF (P1 .EQS. "BUILDINFO")
$ THEN
$   SET DEFAULT [.SRC]
$   CREATE BUILDINFO.OBJ
$   CREATE BUILDINFO.EXE
$   GOSUB BUILDINFO
$   SET DEFAULT [-]
$   V = F$VERIFY(V)
$   EXIT
$ ENDIF
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
       + "acosh asinh atanh ndtr cbrt isnan"
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
$ CFLAGS = CFLAGS0 -
       + "/INCLUDE=[--]/DEFINE=(GLOBAL_CONFIG_DIR=""""""[]"""""", " -
       + "T1_AA_TYPE16=""''T1_AA_TYPE16'"",T1_AA_TYPE32=""''T1_AA_TYPE32'"")" -
       + "/WARNING=(DISABLE=DUPEXTERN)"
$ LIB = "[-]libt1lib.olb"
$ SRCS = "t1finfo t1base t1delete t1enc t1env t1load t1set t1trans t1aaset " -
       + "t1afmtool t1outline parseAFM"
$ GOSUB COMPILE
$ SET DEFAULT [--]
$ !
$ SAY "Building XBAE"
$ SET DEFAULT [.XBAE.XBAE]
$ XBAE = F$PARSE("[]",,,,"SYNTAX_ONLY") - ".;"
$ DEFINE/NOLOG XBAE 'XBAE'
$ CFLAGS = CFLAGS0 + "/INCLUDE=[-]" + GUI_FLAGS -
         + "/DEFINE=(DRAW_RESIZE_SHADOW)/WARNINGS=(DISABLE=LONGEXTERN)"
$ LIB = "libXbae.OLB"
$ SRCS = "Actions Clip Converters Create Draw Input " -
       + "Matrix Methods Public ScrollMgr Shadow Utils"
$ GOSUB COMPILE
$ DEASSIGN XBAE
$ SET DEFAULT [--]
$ !
$ SAY "Building SRC"
$ SET DEFAULT [.SRC]
$ IF (F$TYPE(USE_DECC$CRTL) .EQS. "") THEN USE_DECC$CRTL = 0
$ IF (USE_DECC$CRTL) THEN DEFINE/NOLOG DECC$CRTLMAP SYS$LIBRARY:DECC$CRTL.EXE
$ VMS_MAJOR = F$ELEMENT (0, ".", F$GETSYI ("NODE_SWVERS")) - "V"
$ IF (USE_DECC$CRTL .OR. VMS_MAJOR .GE. 7)
$ THEN
$   CFLAGS = CFLAGS0 + "/INCLUDE=([-],[-.T1LIB.T1LIB]''NETCDF_INC')" -
           + "/DEFINE=(CCOMPILER=""""""''CCOMPILER'""""""," -
           + "GRACE_HOME=""""""''GRACE_HOME'""""""," -
           + "GRACE_EDITOR=""""""''GRACE_EDITOR'""""""," -
           + "GRACE_HELPVIEWER=""""""''HELPVIEWER'""""""," -
           + "GRACE_PRINT_CMD=""""""''PRINT_CMD'"""""")"
$   LIB = ""
$   SRCS = "buildinfo"
$   GOSUB COMPILE
$   V = F$VERIFY(1)
$   LINK/EXECUTABLE=BUILDINFO.EXE BUILDINFO.OBJ,[-.T1LIB]libt1lib.olb/LIBRARY -
         'NOGUI_LIBS'
$   DEFINE/USER SYS$OUTPUT BUILDINFO.H
$   RUN BUILDINFO.EXE
$   V = 'F$VERIFY(0)'
$ ELSE
$   GOSUB BUILDINFO
$ ENDIF
$ CEPHES = F$PARSE("[-.CEPHES]",,,,"SYNTAX_ONLY") - ".;"
$ DEFINE/NOLOG CEPHES 'CEPHES'
$ XBAE = F$PARSE(XBAE_INC,,,,"SYNTAX_ONLY") - ".;"
$ DEFINE/NOLOG XBAE 'XBAE'
$ CFLAGS = CFLAGS0 + "/INCLUDE=([-],[-.T1LIB.T1LIB]''LIB_INC')" -
         + "/DEFINE=(""xfree=xfree_"")"
$ LIB = "xmgrace.olb"
$ SRCS = "main plotone files ssdata utils drawticks " -
       + "nonlfit lmdif as274c fit fourier " -
       + "graphs graphutils setutils regionutils " -
       + "objutils computils defaults params " -
       + "compute draw dlmodule pars missing " -
       + "iofilters dates t1fonts device " -
       + "dummydrv mfdrv psdrv gd rstdrv"
$ IF (ALLOCA .NES. "") THEN SRCS = SRCS + " alloca"
$ IF (PDFDRV_O .NES. "") THEN SRCS = SRCS + " pdfdrv"
$ GOSUB COMPILE
$ SRCS = "Tab motifutils " -
       + "compwin comwin eblockwin " -
       + "editpwin events featext fileswin plotwin " -
       + "graphappwin helpwin hotwin " -
       + "locatewin miscwin monwin " -
       + "nonlwin printwin ptswin regionwin " -
       + "setwin strwin setappwin " -
       + "tickwin worldwin fontwin xutil x11drv xmgrace"
$ GOSUB COMPILE
$ CEPHES_LIB = ",[-.CEPHES]LIBCEPHES.OLB/LIB"
$ V = F$VERIFY(1)
$ LINK /EXECUTABLE=xmgrace.exe 'LDFLAGS' xmgrace.olb/lib/inc=main -
    'GUI_LIBS''CEPHES_LIB''T1_LIB''NETCDF_LIBS''FFTW_LIB' -
    'PDF_LIB''TIFF_LIB''JPEG_LIB''PNG_LIB''Z_LIB' -
    'NOGUI_LIBS''DL_LIB'
$ V = 'F$VERIFY(0)'
$ DEASSIGN CEPHES
$ DEASSIGN XBAE
$ SET DEFAULT [-]
$ !
$ SAY "Done"
$ V = F$VERIFY(V)
$ EXIT
$ !
$COMPILE:
$ IF (LIB .NES. "" .AND. F$SEARCH(LIB) .EQS. "") THEN LIBRARY/CREATE/LOG 'LIB'
$ N = 0
$LOOP_COMPILE:
$ FILE = F$ELEMENT (N, " ", SRCS)
$ N = N + 1
$ IF (FILE .EQS. " ") THEN RETURN
$ IF (FILE .EQS. "") THEN GOTO LOOP_COMPILE
$ V = F$VERIFY(1)
$ 'CC''CFLAGS' 'FILE'.C
$ V = 'F$VERIFY(0)'
$ IF (LIB .NES. "") THEN LIBRARY/LOG 'LIB' 'FILE'.OBJ
$ GOTO LOOP_COMPILE
$ !
$BUILDINFO:
$ OPEN/READ IN BUILDINFO.C
$BUILDINFO_C_LOOP:
$ READ/END=BUILDINFO_C_DONE IN REC
$ IF (F$ELEMENT(0," ",REC) .NES. "#define") THEN GOTO BUILDINFO_C_LOOP
$ VAL = F$ELEMENT(2," ",REC)
$ IF (VAL .NES. " ") THEN 'F$ELEMENT(1," ",REC)' = 'VAL'
$ GOTO BUILDINFO_C_LOOP
$BUILDINFO_C_DONE:
$ CLOSE IN
$ @SYS$UPDATE:DECW$GET_IMAGE_VERSION SYS$SHARE:DECW$XLIBSHR.EXE DECWVERSION
$ OPEN/READ IN [-.T1LIB.T1LIB]SYSCONF.H
$SYSCONF_H_LOOP:
$ READ/END=SYSCONF_H_DONE IN REC
$ IF (F$ELEMENT(0," ",REC) .NES. "#define") THEN GOTO SYSCONF_H_LOOP
$ VAL = F$ELEMENT(2," ",REC)
$ IF (VAL .NES. " ") THEN 'F$ELEMENT(1," ",REC)' = 'VAL'
$ GOTO SYSCONF_H_LOOP
$SYSCONF_H_DONE:
$ CLOSE IN
$ OPEN/WRITE OUT BUILDINFO.H
$ VERSION = MAJOR_REV*10000 + MINOR_REV*100 + PATCHLEVEL
$ WRITE OUT "#define BI_VERSION_ID ", VERSION
$ VERSION = "Grace-''MAJOR_REV'.''MINOR_REV'.''PATCHLEVEL' ''BETA_VER'"
$ WRITE OUT "#define BI_VERSION """, VERSION, """"
$ WRITE OUT "#define BI_GUI ""@(#)OSF/Motif Version ", DECWVERSION, """"
$ WRITE OUT "#define BI_T1LIB """, T1LIB_IDENT, """"
$ WRITE OUT "#define BI_CCOMPILER """, CCOMPILER, """"
$ WRITE OUT "#define BI_SYSTEM ""OpenVMS 0 ", F$GETSYI("HW_NAME"), """"
$ WRITE OUT "#define BI_DATE """, F$TIME(), """"
$ WRITE OUT ""
$ WRITE OUT "#define GRACE_HOME """, GRACE_HOME, """"
$ WRITE OUT "#define GRACE_EDITOR """, GRACE_EDITOR, """"
$QUOTELOOP:
$ N = F$LOCATE ("\\", PRINT_CMD)
$ IF (N .NE. F$LENGTH(PRINT_CMD))
$ THEN
$   PRINT_CMD = F$EXTRACT (0, N, PRINT_CMD) + F$EXTRACT (N+1, 255, PRINT_CMD)
$   GOTO QUOTELOOP
$ ENDIF
$ WRITE OUT "#define GRACE_PRINT_CMD """, PRINT_CMD, """"
$ WRITE OUT "#define GRACE_HELPVIEWER """, HELPVIEWER, """"
$ CLOSE OUT
$ RETURN
