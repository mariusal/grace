##########################################
#      Makefile for GRACE (VMS)          #
##########################################

# Rolf Niepraschk, 5/98, niepraschk@ptb.de


TOP = [-]
HERE = SYS$DISK:[]
ECHO = WRITE SYS$OUTPUT

VMSDIR = [-.ARCH.VMS]
CEPHESDIR = [-.CEPHES]
T1LIBDIR = [-.T1LIB]
X11_MIT = $(VMSDIR)

CEPHES_LIB = $(CEPHESDIR)libcephes.olb
T1LIB_LIB = $(T1LIBDIR)libt1lib.olb

.INCLUDE $(TOP)Make.conf

NETCDF =
NETCDF_INC =
NETCDF_LIBS =

MYSTIC = ,"lines=lines_"

CFLAGS = $(CFLAGS0)/INC=($(VMSDIR),$(TOP)$(NETCDF_INC)) \
  /DEFINE=(GR_HOME="""$(GR_HOME)"""$(GUI)$(XBAE)$(LIBHELP)$(EDITRES) \
  ,GRACE_HELPVIEWER="""$(HELPVIEWER)"""$(NETCDF),PRINT_CMD="""$(PRINT_CMD)""" \
  $(MYSTIC))

# LIBS = ,$(VMSDIR)'MOTIF_OPT',$(CEPHES_LIB)/LIB
LIBS = ,$(CEPHES_LIB)/LIB,$(T1LIB_LIB)/LIB$(GUI_LIBS)$(NETCDF_LIBS)

VMSOBJS = alloca.obj getpwnam.obj vms_unix.obj

.FIRST

	@ define/nolog X11 DECW$INCLUDE, $(X11_MIT)
        @ define/nolog cephes 'f$string(f$parse("[-]","","","device")+ \
          f$parse("[-]","","","directory") - "]" + ".cephes]")
        @ define/nolog t1lib 'f$string(f$parse("[-]","","","device")+ \
          f$parse("[-]","","","directory") - "]" + ".t1lib]")

ALL : msg $(ACEGR)

	@ !

.INCLUDE Make.common

msg :
        @ $(ECHO) ""
        @ $(ECHO) "Making $(ACEGR) ..."
        @ $(ECHO) ""

$(ACEGR) : $(GROBJS) $(GUIOBJS) $(VMSOBJS)

	link /exe=$@ $(LDFLAGS) $+ $(LIBS)
        PURGE *$(O)

vms_unix.obj : $(VMSDIR)vms_unix.c $(VMSDIR)vms_unix.h

getpwnam.obj : $(VMSDIR)getpwnam.c

clean :
        IF F$SEARCH("*$(O)",).NES."" THEN $(RM) *$(O);*

distclean : clean

	IF F$SEARCH("$(ACEGR)",).NES."" THEN $(RM) $(ACEGR);*
