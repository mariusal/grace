#####################################################
# Top-level Makefile for GRACE   (VMS)              #
#####################################################

# Rolf Niepraschk, 11/97, niepraschk@ptb.de

include Make.conf

CD = SET DEFAULT
TOP = [-]
ECHO = WRITE SYS$OUTPUT

ALL : CEPHES T1LIB GRACE 
	@ !


.LAST
	 @ $(ECHO) ""
	 @ $(ECHO) "Done."

CEPHES :
	 @ $(CD) [.CEPHES]
	 @ $(MMS) $(MMSQUALIFIERS) $(MMSTARGETS)
	 @ $(CD) $(TOP)

T1LIB :
	 @ $(CD) [.T1LIB]
	 @ $(MMS) $(MMSQUALIFIERS) $(MMSTARGETS)
	 @ $(CD) $(TOP)

GRACE :
	 @ $(CD) [.SRC]
	 @ $(MMS)/IGNORE=WARNING $(MMSQUALIFIERS) $(MMSTARGETS)
	 @ $(CD) $(TOP)

clean : CEPHES T1LIB GRACE
	@ !

distclean : CEPHES T1LIB GRACE
	@ !
