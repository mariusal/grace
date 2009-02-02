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

lib : library
	$(MAKE) -f Makefile.qt Make.common

clean: Makefile.qt
	$(MAKE) -f Makefile.qt clean

distclean: Makefile.qt
	$(MAKE) -f Makefile.qt distclean

