#####################################################
# Makefile for documentation of Grace               #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .sgml .html .dvi .ps

SGML2HTML = sgml2html --split=0
SGML2DVI  = sgml2latex
DVI2PS    = dvips

.sgml.html :
	$(SGML2HTML) $<

.sgml.dvi :
	$(SGML2DVI) $<

.dvi.ps :
	$(DVI2PS) $<

SGMLS = FAQ.sgml Tutorial.sgml UsersGuide.sgml 

HTMLDOCS = CHANGES.html GPL.html philosophical-gnu-sm.jpg nohelp.html \
	FAQ.html Tutorial.html UsersGuide.html

DVIDOCS = FAQ.dvi Tutorial.dvi UsersGuide.dvi

PSDOCS = FAQ.ps Tutorial.ps UsersGuide.ps

TUTDATA = 2.1.dat 3.dat 7.1.2.agr 7.1.3.agr 8.2.dat \
	10.1.dat 10a.dat 10c.dat 10b.dat shiftdata.sh mygraph.gif

GRACEMAN = grace.1
MANLINKS = xmgrace.1 gracebat.1
MANPAGES = $(GRACEMAN) grconvert.1

all : html dvi

html : $(HTMLDOCS)

dvi : $(DVIDOCS)

ps : $(PSDOCS)

clean :
	$(RM) *.aux *.log

distclean : 
	$(RM) *.orig

devclean : 
	$(RM) *.orig $(DVIDOCS) $(PSDOCS)

install : $(HTMLDOCS) $(DVIDOCS) $(TUTDATA) $(MANPAGES)
	$(MKINSTALLDIRS) $(GRACE_HOME)/doc
	for i in $?; do $(INSTALL_DATA) $$i $(GRACE_HOME)/doc; done
	cd $(GRACE_HOME)/doc; \
	for i in $(MANLINKS); do $(RM) $$i; $(LN_S) $(GRACEMAN) $$i; done

links : $(MANPAGES)
	$(MKINSTALLDIRS) $(PREFIX)/man
	$(MKINSTALLDIRS) $(PREFIX)/man/man1
	cd $(PREFIX)/man/man1; \
	for i in $? $(MANLINKS); do $(RM) $$i; $(LN_S) $(GRACE_HOME)/doc/$$i $$i; done

tests : dummy

dummy :
