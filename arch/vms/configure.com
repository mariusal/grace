$! configure for GRACE -- VMS version
$! Rolf Niepraschk, 12/97, niepraschk@ptb.de
$! John Hasstedt, 12/98, John.Hasstedt@sunysb.edu
$!
$ IF (P1 .NES. "DEFINE_TOP_IN_UNIX_FORMAT") THEN GOTO CONFIGURE
$!
$! code to define a logical name as part of the build procedure
$!
$ LOG = F$ELEMENT (0, "]", F$ENVIRONMENT ("PROCEDURE")) - "[" - ".ARCH.VMS"
$ N = F$LOCATE(":",LOG)
$ LOG[N,1] := /
$LOOP_DEFINE_TOP_IN_UNIX_FORMAT:
$ N = F$LOCATE(".",LOG)
$ IF (N .NE. F$LENGTH(LOG))
$ THEN
$   LOG[N,1] := /
$   GOTO LOOP_DEFINE_TOP_IN_UNIX_FORMAT
$ ENDIF
$ LOG = "/" + LOG
$ DEFINE/NOLOG TOP_IN_UNIX_FORMAT "''LOG'"
$ EXIT
$!
$CONFIGURE:
$!
$ echo := WRITE SYS$OUTPUT
$!
$! get versions, hardware type, etc
$!
$ VMSVERSION = F$GETSYI ("NODE_SWVERS")
$ VMS_MAJOR = F$ELEMENT (0, ".", VMSVERSION) - "V"
$ HW = F$GETSYI("ARCH_NAME")
$ @SYS$UPDATE:DECW$GET_IMAGE_VERSION SYS$SHARE:DECW$XLIBSHR.EXE DECWVERSION
$ DECWVERSION = F$ELEMENT (1, " ", DECWVERSION)
$ DECW_MAJOR = F$ELEMENT (0, "-", DECWVERSION)
$ @SYS$UPDATE:DECW$GET_IMAGE_VERSION SYS$SYSTEM:DECC$COMPILER.EXE DECCVERSION
$ DECCVERSION = F$ELEMENT (1, " ", DECCVERSION)
$ DECC_MAJOR = F$ELEMENT (0, ".", DECCVERSION) - "V"
$!
$! set defaults for command line parameters
$!
$ IF (F$SEARCH("SYS$LIBRARY:DPML$SHR.EXE") .NES. "")
$ THEN
$   DPMLSHR = "Yes"
$ ELSE
$   DPMLSHR = "No"
$ ENDIF
$ DPML = DPMLSHR
$ OPTIMIZE = "Yes"
$ IF (HW .EQS. "Alpha")
$ THEN
$   FLOAT = "IEEE"
$ ELSE
$   FLOAT = "G_FLOAT"
$ ENDIF
$ HOME = ""
$ PRINT = ""
$ QUEUE = "decw$printer_format_ps"
$ HELP = "mosaic"
$ NETCDFINC = ""
$ NETCDFLIB = ""
$ FORCECOPY = 0
$ SAVE = 0
$!
$! read saved information and add command line parameters
$!
$ N = 1
$ SAVEFILE = F$ELEMENT (0, "]", F$ENVIRONMENT ("PROCEDURE")) + "]SAVED.DAT"
$ IF (F$SEARCH(SAVEFILE) .EQS. "") THEN GOTO NO_SAVEFILE
$ echo ""
$ echo "Using saved information"
$ OPEN/READ IN 'SAVEFILE'
$LOOP_SAVEFILE:
$ READ/END=DONE_SAVEFILE IN PAR'N'
$ N = N + 1
$ GOTO LOOP_SAVEFILE
$DONE_SAVEFILE:
$ CLOSE IN
$NO_SAVEFILE:
$ PAR'N' = P1
$ N = N + 1
$ PAR'N' = P2
$ N = N + 1
$ PAR'N' = P3
$ N = N + 1
$ PAR'N' = P4
$ N = N + 1
$ PAR'N' = P5
$ N = N + 1
$ PAR'N' = P6
$ N = N + 1
$ PAR'N' = P7
$ N = N + 1
$ PAR'N' = P8
$ N = N + 1
$ PAR'N' = ""
$ N = 0
$LOOP_PARAM:
$ N = N + 1
$ P = F$ELEMENT (0, "=", PAR'N')
$ IF (P .EQS. "") THEN GOTO DONE_PARAM
$ IF (P .EQS. "DPML")
$ THEN
$   DPML = DPMLSHR
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "NODPML")
$ THEN
$   DPML = "No"
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "OPTIMIZE")
$ THEN
$   OPTIMIZE = "Yes"
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "NOOPTIMIZE")
$ THEN
$   OPTIMIZE = "No"
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "D_FLOAT")
$ THEN
$   FLOAT = "D_FLOAT"
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "G_FLOAT")
$ THEN
$   FLOAT = "G_FLOAT"
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "IEEE")
$ THEN
$   IF (HW .EQS. "VAX")
$   THEN
$     echo ""
$     echo "Ignoring IEEE option on VAX"
$   ELSE
$     FLOAT = "IEEE"
$   ENDIF
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "HOME")
$ THEN
$   HOME = PAR'N' - "HOME="
$   IF (F$EXTRACT(0,1,HOME) .EQS. """") THEN -
        HOME = F$EXTRACT(1,F$LENGTH(HOME)-1,HOME)
$   IF (F$EXTRACT(F$LENGTH(HOME)-1,1,HOME) .EQS. """") THEN -
        HOME = F$EXTRACT(0,F$LENGTH(HOME)-1,HOME)
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "PRINT")
$ THEN
$   PRINT = PAR'N' - "PRINT="
$   IF (F$EXTRACT(0,1,PRINT) .EQS. """") THEN -
        PRINT = F$EXTRACT(1,F$LENGTH(PRINT)-1,PRINT)
$   IF (F$EXTRACT(F$LENGTH(PRINT)-1,1,PRINT) .EQS. """") THEN -
        PRINT = F$EXTRACT(0,F$LENGTH(PRINT)-1,PRINT)
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "QUEUE")
$ THEN
$   QUEUE = PAR'N' - "QUEUE="
$   IF (F$EXTRACT(0,1,QUEUE) .EQS. """") THEN -
        QUEUE = F$EXTRACT(1,F$LENGTH(QUEUE)-1,QUEUE)
$   IF (F$EXTRACT(F$LENGTH(QUEUE)-1,1,QUEUE) .EQS. """") THEN -
        QUEUE = F$EXTRACT(0,F$LENGTH(QUEUE)-1,QUEUE)
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "HELP")
$ THEN
$   HELP = PAR'N' - "HELP="
$   IF (F$EXTRACT(0,1,HELP) .EQS. """") THEN -
        HELP = F$EXTRACT(1,F$LENGTH(HELP)-1,HELP)
$   IF (F$EXTRACT(F$LENGTH(HELP)-1,1,HELP) .EQS. """") THEN -
        HELP = F$EXTRACT(0,F$LENGTH(HELP)-1,HELP)
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "NETCDF")
$ THEN
$   NETCDFINC = F$ELEMENT (1, "=", PAR'N')
$   NETCDFLIB = F$ELEMENT (2, "=", PAR'N')
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "COPY")
$ THEN
$   FORCECOPY = 1
$   GOTO LOOP_PARAM
$ ENDIF
$ IF (P .EQS. "SAVE")
$ THEN
$   SAVE = 1
$   GOTO LOOP_PARAM
$ ENDIF
$ echo "Unrecognized option: ", P
$ EXIT
$DONE_PARAM:
$ IF (NETCDFINC .NES. "" .AND. NETCDFLIB .EQS. "")
$ THEN
$   echo "You must specify both the include directory and the libraries"
$   echo "with the NETCDF option."
$   EXIT
$ ENDIF
$ IF (SAVE)
$ THEN
$   IF (F$SEARCH(SAVEFILE) .EQS. "")
$   THEN
$     OPEN/WRITE OUT 'SAVEFILE'
$   ELSE
$     OPEN/APPEND OUT 'SAVEFILE'
$   ENDIF
$   IF (P1 .NES. "" .AND. P1 .NES. "SAVE") THEN WRITE OUT P1
$   IF (P2 .NES. "" .AND. P2 .NES. "SAVE") THEN WRITE OUT P2
$   IF (P3 .NES. "" .AND. P3 .NES. "SAVE") THEN WRITE OUT P3
$   IF (P4 .NES. "" .AND. P4 .NES. "SAVE") THEN WRITE OUT P4
$   IF (P5 .NES. "" .AND. P5 .NES. "SAVE") THEN WRITE OUT P6
$   IF (P6 .NES. "" .AND. P6 .NES. "SAVE") THEN WRITE OUT P6
$   IF (P7 .NES. "" .AND. P7 .NES. "SAVE") THEN WRITE OUT P7
$   IF (P8 .NES. "" .AND. P8 .NES. "SAVE") THEN WRITE OUT P8
$   CLOSE OUT
$ ENDIF
$ IF (PRINT .EQS. "") THEN -
    PRINT = "print/name=""from Grace""/delete/queue=" + QUEUE
$ IF (F$LOCATE("%s",HELP) .EQ. F$LENGTH(HELP)) THEN HELP = HELP + " %s"
$!
$ echo ""
$ echo "Configuration of GRACE for VMS"
$ echo ""
$ echo "VMS version:  ", VMSVERSION
$ echo "Hardware:     ", HW
$ echo "GUI:          Motif ", DECWVERSION
$ echo "DPML:         ", DPML
$ echo "DECC version: ", DECCVERSION
$ echo "Optimize:     ", OPTIMIZE
$ echo "Float:        ", FLOAT
$ echo "Home dir:     ", HOME
$ echo "Print cmd:    ", PRINT
$ echo "Help viewer:  ", HELP
$ IF (NETCDFLIB .EQS. "")
$ THEN
$   echo "NetCDF:       Not used"
$ ELSE
$   echo "NetCDF:       Include dir: ", NETCDFINC
$   echo "              Libraries:   ", NETCDFLIB
$ ENDIF
$ echo ""
$!
$! define symbols for the other directories
$!
$ MAIN_DIR := [--]
$ CEPHES_DIR := [--.CEPHES]
$ T1LIB_DIR := [--.T1LIB]
$ T1LIB_T1LIB_DIR := [--.T1LIB.T1LIB]
$ T1LIB_TYPE1_DIR := [--.T1LIB.TYPE1]
$ SRC_DIR := [--.SRC]
$ GRCONVERT_DIR := [--.GRCONVERT]
$ EXAMPLES_DIR := [--.EXAMPLES]
$ CONF_DIR := [--.CONF]
$!
$! save the current directory and set default to the vms directory
$!
$ CURDIR = F$ENVIRONMENT ("DEFAULT")
$ VMSDIR = F$ELEMENT (0, "]", F$ENVIRONMENT ("PROCEDURE")) + "]"
$ SET DEFAULT 'VMSDIR'
$!
$! copy files to other directories
$!
$ IF (FORCECOPY .OR. F$SEARCH("''MAIN_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY DESCRIP.MMS 'MAIN_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''CEPHES_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY CEPHES.MMS 'CEPHES_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''T1LIB_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY T1LIB.MMS 'T1LIB_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''T1LIB_T1LIB_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY T1LIB_T1LIB.MMS 'T1LIB_T1LIB_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''T1LIB_TYPE1_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY T1LIB_TYPE1.MMS 'T1LIB_TYPE1_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''SRC_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY SRC.MMS 'SRC_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''GRCONVERT_DIR'DESCRIP.MMS") .EQS. "") THEN -
      COPY GRCONVERT.MMS 'GRCONVERT_DIR'DESCRIP.MMS
$ IF (FORCECOPY .OR. F$SEARCH("''GRCONVERT_DIR'XDR.OPT") .EQS. "") THEN -
      COPY XDR.OPT 'GRCONVERT_DIR'XDR.OPT
$ IF (FORCECOPY .OR. F$SEARCH("''EXAMPLES_DIR'DOTEST.COM") .EQS. "") THEN -
      COPY DOTEST.COM 'EXAMPLES_DIR'DOTEST.COM
$!
$! copy the default font encoding file
$!
$ IF (F$SEARCH("[--.FONTS.ENC]DEFAULT.ENC") .EQS. "") THEN -
      COPY [--.FONTS.ENC]ISOLATIN1.ENC [--.FONTS.ENC]DEFAULT.ENC
$!
$! define symbols for make.conf
$! These symbols are in make.conf_in; they are set to the value they
$! should be in make.conf.
$!
$ O = ".obj"
$ EXE = ".exe"
$ BAT = ".com"
$ SHELL = ""
$ PREFIX = ""
$ SUBDIRS = "cephes t1lib src"
$ GRACE = "xmgrace$(EXE)"
$ GRACE_HOME = HOME
$ MISSING_O = "missing$(O)"
$ IF (HW .EQS. "Alpha" .OR. DECC_MAJOR .GE. 6)
$ THEN
$   ALLOCA = ""
$ ELSE
$   ALLOCA = "alloca.obj"
$ ENDIF
$ T1_LIB = ""
$ T1_AA_TYPE16 = "short"
$ T1_AA_TYPE32 = "int"
$ T1_AA_TYPE64 = ""
$ XDR_LIB = ""
$ DL_LIB = ""
$ FFTW_LIB = ""
$ PDF_LIB = ""
$ PDFDRV_O = ""
$ GD_LIB = ""
$ RSTDRV_O = ""
$ TERMDRV_O = "x11drv$(O)"
$ YACC = ""
$ CC = "cc"
$ FC = "fortran"
$ AR = "library"
$ RANLIB = ""
$ RM = "delete/log"
$ LN_S = ""
$ INSTALL = ""
$ INSTALL_PROGRAM = ""
$ INSTALL_DATA = ""
$ MKINSTALLDIRS = ""
$ IF (DPMLSHR .AND. .NOT. DPML)
$ THEN
$   CFLAGS0 = "/PREFIX=(ALL,EXCEPT=(CBRT,LOG2,RINT,ASINH,ACOSH,ATANH," -
            + "ERF,ERFC,J0,J1,JN,Y0,Y1,YN))"
$ ELSE
$   CFLAGS0 = "/PREFIX=ALL"
$ ENDIF
$ CFLAGS0 = CFLAGS0 + "/FLOAT=" + FLOAT
$ IF (.NOT. OPTIMIZE) THEN CFLAGS0 = CFLAGS0 + "/NOOPTIMIZE"
$ GUI_FLAGS = ""
$ LDFLAGS = ""
$ NOGUI_LIBS = ""
$ IF (DECW_MAJOR .EQS. "V1.1")
$ THEN
$   GUI_LIBS = ",$(VMSDIR)motif1_1.opt/option"
$ ELSE
$   GUI_LIBS = ",$(VMSDIR)motif1_2.opt/option"
$ ENDIF
$ PRINT_CMD = F$ELEMENT (0, """", PRINT)
$ N = 1
$LOOP_PRINT_CMD:
$ P = F$ELEMENT (N, """", PRINT)
$ IF (P .NES. """")
$ THEN
$   PRINT_CMD = PRINT_CMD + "\042" + P
$   N = N + 1
$   GOTO LOOP_PRINT_CMD
$ ENDIF
$ HELPVIEWER = HELP
$ NETCDF_LIBS = ""
$ IF (NETCDFLIB .EQS. "") THEN GOTO DONE_NETCDF_LIBS
$ N = 0
$LOOP_NETCDF_LIBS:
$ LIB = F$ELEMENT (N, ",", NETCDFLIB)
$ IF (LIB .EQS. ",") THEN GOTO DONE_NETCDF_LIBS
$ NETCDF_LIBS = NETCDF_LIBS + "," + LIB + "/LIBRARY"
$ N = N + 1
$ GOTO LOOP_NETCDF_LIBS
$DONE_NETCDF_LIBS:
$!
$! create make.conf
$!
$ echo "Creating make.conf"
$ OPEN/READ IN 'CONF_DIR'MAKE.CONF_IN
$ OPEN/WRITE OUT 'MAIN_DIR'MAKE.CONF
$LOOP_MAKE_CONF:
$ READ/END=DONE_MAKE_CONF IN REC
$ IF (F$LOCATE("=",REC) .NE. F$LENGTH(REC))
$ THEN
$   SYM = F$ELEMENT(0,"=",REC)
$   IF (F$TYPE('SYM') .EQS. "")
$   THEN
$     WRITE SYS$OUTPUT "No DCL symbol for ", REC
$   ELSE
$     REC = SYM + "=" + 'SYM'
$   ENDIF
$ ELSE
$   IF (REC .EQS. ".SUFFIXES:") THEN REC = "#.SUFFIXES:"  ! allow make rules
$ ENDIF
$ WRITE OUT REC
$ GOTO LOOP_MAKE_CONF
$DONE_MAKE_CONF:
$ CLOSE IN
$ WRITE OUT ""
$ WRITE OUT "# NetCDF header directory"
$ IF (NETCDFINC .EQS. "")
$ THEN
$   WRITE OUT "NETCDF_INC="
$ ELSE
$   WRITE OUT "NETCDF_INC=,", NETCDFINC
$ ENDIF
$ CLOSE OUT
$!
$! define symbols for config.h
$! These symbols are in config.h_in; if the DCL symbol is equal to 0,
$! the symbol should be undefined in config.h; otherwise, it should be
$! defined to the value of the DCL symbol.  I define all values in DCL
$! (instead of just those that need to be defined in config.h) so I can
$! check when symbols are added to config.h_in.
$!
$ SIZEOF_CHAR = "sizeof(char)"
$ SIZEOF_SHORT = "sizeof(short)"
$ SIZEOF_INT = "sizeof(int)"
$ SIZEOF_LONG = "sizeof(long)"
$ IF (HW .EQS. "Alpha")
$ THEN
$   SIZEOF_LONG_LONG = "sizeof(long long)"
$ ELSE
$   SIZEOF_LONG_LONG = "0"
$ ENDIF
$ SIZEOF_FLOAT = "sizeof(float)"
$ SIZEOF_DOUBLE = "sizeof(double)"
$ IF (HW .EQS. "Alpha")
$ THEN
$   SIZEOF_LONG_DOUBLE = "sizeof(long double)"
$ ELSE
$   SIZEOF_LONG_DOUBLE = "0"
$ ENDIF
$ SIZEOF_VOID_P = "sizeof(void *)"
$ _ALL_SOURCE = 0
$ _POSIX_SOURCE = 0
$ STDC_HEADERS = 1
$ __CHAR_UNSIGNED__ = 0
$ const = 0
$ pid_t = 0
$ size_t = 0
$ HAVE_UNISTD_H = 1
$ CRAY_STACKSEG_END = 0
$ HAVE_ALLOCA = (HW .EQS. "Alpha") .OR. (DECC_MAJOR .GE. 6)
$ C_ALLOCA = (.NOT. HAVE_ALLOCA) .AND. 1
$ HAVE_ALLOCA_H = 0
$ RETSIGTYPE = "void"
$ HAVE_SYS_WAIT_H = 1
$ HAVE_FCNTL_H = 1
$ HAVE_SYS_PARAM_H = 0
$ TM_IN_SYS_TIME = 1
$ HAVE_VFORK_H = 0
$ vfork = 0
$ HAVE_GETCWD = 1
$ HAVE_GETHOSTNAME = 1
$ HAVE_MEMCPY = 1
$ HAVE_MEMMOVE = 1
$ HAVE_UNLINK = 0
$ HAVE_POPEN = VMS_MAJOR .GE. 7
$ HAVE_FNMATCH = 0
$ HAVE_ON_EXIT = 0
$ HAVE_STRSTR = 1
$ HAVE_STRERROR = 1
$ HAVE_VSNPRINTF = 0
$ HAVE_DLOPEN = 0
$ HAVE_RTLD_NOW = 0
$ HAVE_SHL_LOAD = 0
$ WORDS_BIGENDIAN = 0
$ HAVE_DEC_FPU = FLOAT .NES. "IEEE"
$ HAVE_LIEEE_FPU = (.NOT. HAVE_DEC_FPU) .AND. 1
$ HAVE_BIEEE_FPU = 0
$ REALLOC_IS_BUGGY = 0
$ HAVE_DRAND48 = VMS_MAJOR .GE. 7
$ HAVE_DRAND48_IN_STDLIB_H = HAVE_DRAND48
$ HAVE_LIBM = 1
$ HAVE_MATH_H = 1
$ HAVE_FLOAT_H = 1
$ HAVE_IEEEFP_H = 0
$ HAVE_HYPOT = 1
$ HAVE_CBRT = F$INTEGER(DPML)
$ HAVE_LOG2 = F$INTEGER(DPML)
$ HAVE_RINT = F$INTEGER(DPML)
$ HAVE_LGAMMA = 0
$ HAVE_LGAMMA_IN_MATH_H = 0
$ HAVE_ASINH = F$INTEGER(DPML)
$ HAVE_ACOSH = F$INTEGER(DPML)
$ HAVE_ATANH = F$INTEGER(DPML)
$ HAVE_ERF = F$INTEGER(DPML)
$ HAVE_ERFC = F$INTEGER(DPML)
$ HAVE_FINITE = (HW .EQS. "Alpha")
$ HAVE_ISFINITE = 0
$ HAVE_J0 = F$INTEGER(DPML)
$ HAVE_J1 = F$INTEGER(DPML)
$ HAVE_JN = F$INTEGER(DPML)
$ HAVE_Y0 = F$INTEGER(DPML)
$ HAVE_Y1 = F$INTEGER(DPML)
$ HAVE_YN = F$INTEGER(DPML)
$ HAVE_NETCDF = NETCDFLIB .NES. ""
$ HAVE_MFHDF = 0
$ HAVE_FFTW = 0
$ HAVE_LIBPDF = 0
$ HAVE_LIBGD = 0
$ HAVE_F77 = 1
$ NEED_F77_UNDERSCORE = 0
$ X_DISPLAY_MISSING = 0
$ HAVE_MOTIF = 1
$ HAVE_LESSTIF = 0
$ HAVE_XPM = 0
$ HAVE_XPM_H = 0
$ HAVE_X11_XPM_H = 0
$ HAVE_LIBXBAE = 0
$ WITH_LIBHELP = 0
$ WITH_EDITRES = 0
$ PRINT_CMD_UNLINKS = 1
$ WITH_DEBUG = 0
$ HAVE_LIBT1 = 0
$!
$! create config.h
$! Any lines beginning with #define SIZEOF or #undef are rewritten; all
$! other lines are copied to the output.
$!
$ echo "Creating config.h"
$ OPEN/READ IN 'CONF_DIR'CONFIG.H_IN
$ OPEN/WRITE OUT 'MAIN_DIR'CONFIG.H
$LOOP_CONFIG_H:
$ READ/END=DONE_CONFIG_H IN REC
$ IF (F$ELEMENT(0," ",REC) .EQS. "#define")   ! check for #define SIZEOF*
$ THEN
$   SYM = F$ELEMENT(1," ",REC)
$   IF (F$EXTRACT(0,6,SYM) .EQS. "SIZEOF")
$   THEN
$     IF (F$TYPE('SYM') .EQS. "")
$     THEN
$       WRITE SYS$OUTPUT "No DCL symbol for ", REC
$     ELSE
$       REC = "#define " + SYM + " " + 'SYM'
$     ENDIF
$   ENDIF
$ ELSE
$   IF (F$ELEMENT(0," ",REC) .EQS. "#undef")  ! check for #undef *
$   THEN
$     SYM = F$ELEMENT(1," ",REC)
$     IF (F$TYPE('SYM') .EQS. "")
$     THEN
$       WRITE SYS$OUTPUT "No DCL symbol for ", REC
$     ELSE
$       VAL = 'SYM'
$       IF (F$TYPE(VAL) .EQS. "STRING" .OR. VAL .NE. 0)
$       THEN
$         REC = "#define " + SYM + " " + F$STRING(VAL)
$       ENDIF
$     ENDIF
$   ENDIF
$ ENDIF
$ WRITE OUT REC
$ GOTO LOOP_CONFIG_H
$DONE_CONFIG_H:
$ CLOSE IN
$ CLOSE OUT
$!
$! restore directory and exit
$!
$ SET DEFAULT 'CURDIR'
$ EXIT
