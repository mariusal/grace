#####################################################
# Makefile for documentation of Grace               #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .sgml .html .dvi

SGML2HTML = sgml2html --split=0
SGML2DVI = sgml2latex

%.sgml : dummy

%.html : %.sgml
	$(SGML2HTML) $<

%.dvi : %.sgml
	$(SGML2DVI) $<

SGMLS = FAQ.sgml Tutorial.sgml UsersGuide.sgml 

HTMLDOCS = CHANGES.html GPL.html philosophical-gnu-sm.jpg \
	FAQ.html Tutorial.html UsersGuide.html

DVIDOCS = FAQ.dvi Tutorial.dvi UsersGuide.dvi

TUTDATA = 2.1.dat 3.dat 7.1.2.agr 7.1.3.agr 8.2.dat \
	10.1.dat 10a.dat 10c.dat 10b.dat shiftdata.sh mygraph.gif

all : html dvi

html : $(HTMLDOCS)

dvi : $(DVIDOCS)

clean :
	$(RM) *.aux *.log

distclean : 
	$(RM) *.orig

devclean : dummy

install : $(HTMLDOCS) $(DVIDOCS) $(TUTDATA)
	$(MKINSTALLDIRS) $(GRACE_HOME)/doc
	for i in $?; do $(INSTALL_DATA) $$i $(GRACE_HOME)/doc; done

links : dummy

tests : dummy

dummy :
