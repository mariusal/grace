##########################################
#      Makefile for GRACE (VMS)          #
##########################################

# Rolf Niepraschk, 5/98, niepraschk@ptb.de

TOP = [-]
INC = "../include/"
CEPHES_INC = ,[-.CEPHES]
ECHO = WRITE SYS$OUTPUT

INCLUDE $(TOP)Make.conf

CEPHES_LIB = ,[-.CEPHES]libcephes.olb/LIBRARY

CFLAGS = $(CFLAGS0)/INCLUDE=($(TOP),$(INC)$(CEPHES_INC)$(T1_INC)$(LIB_INC)) \
  /DEFINE=("xfree=xfree_")

LIBS = $(GUI_LIBS)$(CEPHES_LIB)$(NETCDF_LIBS)$(FFTW_LIB) \
       ,$(GRACE_CANVAS_LIB)/LIB ,$(GRACE_BASE_LIB)/LIB $(NOGUI_LIBS)$(DL_LIB) \
       $(T1_LIB) $(XMI_LIB) $(PDF_LIB) $(TIFF_LIB) $(JPEG_LIB) $(PNG_LIB) $(Z_LIB) \
       $(XCC_LIB) $(EXPAT_LIB) 

PREFS = /DEFINE=(CCOMPILER="""$(CCOMPILER)""",\
	  GRACE_HOME="""$(GRACE_HOME)""",\
	  GRACE_EDITOR="""$(GRACE_EDITOR)""",\
	  GRACE_HELPVIEWER="""$(HELPVIEWER)""",\
	  GRACE_PRINT_CMD="""$(PRINT_CMD)""")

ALL : msg logicals buildinfo.h $(GRACE)
	@ !

INCLUDE Make.common

msg :
        @ $(ECHO) ""
        @ $(ECHO) "Making $(GRACE) ..."
        @ $(ECHO) ""

logicals :
        @ define/nolog cephes \
            'f$string(f$parse("[-.cephes]",,,,"syntax_only")-".;")'
        @ define/nolog xbae \
            'f$string(f$parse("$(XBAE_INC)",,,,"syntax_only")-".;")'
.IFDEF USE_DECC$CRTL
        @ define/nolog decc$crtlmap sys$library:decc$crtl.exe
.ENDIF

#INCLUDE Make.dep

$(GRACE) : xmgrace.olb($(GROBJS) $(GUIOBJS))
	LINK /EXECUTABLE=$@ $(LDFLAGS) xmgrace.olb/LIBRARY/INCLUDE=main $(LIBS)

buildinfo$(EXE) : buildinfo$(O)
	LINK /EXECUTABLE=$@ $? $(LDFLAGS) $(GUI_LIBS) $(T1_LIB) $(NOGUI_LIBS)

buildinfo$(O) : $(TOP)Make.conf
	$(CC) $(CFLAGS) $(PREFS)/OBJECT=$@ buildinfo.c

buildinfo.h : buildinfo$(EXE) 
	DEFINE/USER SYS$OUTPUT $@
	RUN $?

clean :
        IF F$SEARCH("*$(O)").NES."" THEN $(RM) *$(O);*

distclean : clean
	IF F$SEARCH("$(GRACE)").NES."" THEN $(RM) $(GRACE);*
