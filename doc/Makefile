#####################################################
# Makefile for documentation of XMGR                #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

SGML2HTML = sgml2html --split=0
SGML2DVI = sgml2latex

%.html : %.sgml
	$(SGML2HTML) $<

%.dvi : %.sgml
	$(SGML2DVI) $<

include $(TOP)/Make.conf

SGMLS = FAQ.sgml UsersGuide.sgml

HTMLDOCS = CHANGES.html GPL.html philosophical-gnu-sm.jpg \
	FAQ.html UsersGuide.html

DVIDOCS = FAQ.dvi UsersGuide.dvi

all : html dvi

html : $(HTMLDOCS)

dvi : $(DVIDOCS)

clean :
	$(RM) *.aux *.log

distclean : 
	$(RM) *.orig

devclean : dummy

install : $(HTMLDOCS) $(DVIDOCS)
	$(MKINSTALLDIRS) $(GRACE_HOME)/doc
	for i in $(HTMLDOCS) $(DVIDOCS); do $(INSTALL_DATA) $$i $(GRACE_HOME)/doc; done

links : dummy

tests : dummy

dummy :
