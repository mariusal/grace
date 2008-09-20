###############################################
# Universal Makefile stub for Grace libraries #
###############################################

all : library

Make.qt : 
	qmake -o Make.qt qtgrace.pro

library : Make.qt
	make -f Make.qt

clean: Make.qt
	make -f Make.qt distclean

distclean: clean

