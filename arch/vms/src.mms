##########################################
#      Makefile for GRACE (VMS)          #
##########################################

# Rolf Niepraschk, 5/98, niepraschk@ptb.de

TOP = [-]
ECHO = WRITE SYS$OUTPUT

VMSDIR = [-.ARCH.VMS]
CEPHESDIR = [-.CEPHES]
T1LIBDIR = [-.T1LIB]
T1LIBHDIR = [-.T1LIB.T1LIB]
XBAEDIR = [-.XBAE]

INCLUDE $(TOP)Make.conf

CEPHES_LIB = ,$(CEPHESDIR)libcephes.olb/LIBRARY
XBAE_LIB = ,$(XBAEDIR)libxbae.olb/LIBRARY

MYSTIC = ,"lines=lines_","xfree=xfree_"

CFLAGS = $(CFLAGS0)/INCLUDE=($(TOP),$(T1LIBHDIR)$(NETCDF_INC)) \
  /DEFINE=(GRACE_HOME="""$(GRACE_HOME)""",GRACE_HELPVIEWER="""$(HELPVIEWER)""" \
  ,GRACE_PRINT_CMD="""$(PRINT_CMD)"""$(MYSTIC))

LIBS = $(GUI_LIBS)$(CEPHES_LIB)$(NETCDF_LIBS)$(FFTW_LIB) \
       $(T1_LIB)$(PDF_LIB)$(GD_LIB)$(NOGUI_LIBS)$(DL_LIB)$(XBAE_LIB)

.FIRST
        @ define/nolog cephes 'f$string(f$parse("[-]","","","device")+ \
          f$parse("[-]","","","directory") - "]" + ".cephes]")
        @ define/nolog xbae 'f$string(f$parse("[-]","","","device")+ \
          f$parse("[-]","","","directory") - "]" + ".xbae]")

ALL : msg $(GRACE)
	@ !

INCLUDE Make.common

msg :
        @ $(ECHO) ""
        @ $(ECHO) "Making $(GRACE) ..."
        @ $(ECHO) ""

#INCLUDE Make.dep

$(GRACE) : $(GROBJS) $(GUIOBJS)
	LINK /EXECUTABLE=$@ $(LDFLAGS) $+ $(LIBS)
        PURGE *$(O)

clean :
        IF F$SEARCH("*$(O)").NES."" THEN $(RM) *$(O);*

distclean : clean
	IF F$SEARCH("$(GRACE)").NES."" THEN $(RM) $(GRACE);*
