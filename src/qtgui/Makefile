#####################################################
# Makefile for Qt Grace                             #
#####################################################
# You should not change anything here.              #
#####################################################

all : lib

Makefile.qt : qtgrace.pro
	qmake -o Makefile.qt qtgrace.pro

library : Makefile.qt
	$(MAKE) -f Makefile.qt

lib : library Make.common

clean: Makefile.qt
	$(MAKE) -f Makefile.qt distclean

distclean: clean

include Makefile.qt
Make.common : $(TARGET)
	echo "QTOBJS = "$(foreach object, $(OBJECTS), qtgui/$(object)) > Make.common
