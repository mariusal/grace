#####################################################
# Makefile for Qt Grace                             #
#####################################################
# You should not change anything here.              #
#####################################################

all : ofiles

Makefile.qt : 
	qmake -o Makefile.qt qtgrace.pro

library : Makefile.qt
	$(MAKE) -f Makefile.qt

include Makefile.qt
ofiles : library
	echo "QTOBJS = "$(foreach object, $(OBJECTS), qtgui/$(object)) > Make.common
	
clean: Makefile.qt
	$(MAKE) -f Makefile.qt distclean

distclean: clean

