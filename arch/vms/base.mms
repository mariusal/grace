#####################################################
# Makefile for Grace base library (VMS)             #
#####################################################

# Rolf Niepraschk, 12/97, niepraschk@ptb.de
# Richard Brodie, 11/03, R.Brodie@rl.ac.uk

TOP = [--]
INC = "../../include/"
ECHO = WRITE SYS$OUTPUT

INCLUDE $(TOP)Make.conf

CFLAGS = $(CFLAGS0)/INCLUDE=($(TOP), $(INC)) /DEFINE=("xfree=xfree_")

INCLUDE make.defs

all : msg $(LIB)($(OBJS))
	@ !

msg : 
        @ $(ECHO) ""
        @ $(ECHO) "Making $(LIB) ..."
        @ $(ECHO) ""
 
install : $(LIB)

tests : dummy

links : dummy

clean :
        IF F$SEARCH("*$(O)",).NES."" THEN $(RM) *$(O);*
        IF F$SEARCH("$(LIB)",).NES."" THEN $(RM) $(LIB);*

distclean : clean
	@ !

devclean : clean
	@ !

dummy :
