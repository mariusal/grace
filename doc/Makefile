#####################################################
# Makefile for documentation of Grace               #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .sgml .dvi .html .pdf .ps .tex

# NB: the language and papersize options should be made `configure'able
LANGUAGE    = english
PAPERSIZE   = letter

LINUXDOC    = linuxdoc --language=$(LANGUAGE) --papersize=$(PAPERSIZE)

SGML2HTML   = $(LINUXDOC) --backend=html  --split=0 --toc=2
SGML2LATEX  = $(LINUXDOC) --backend=latex --output=tex
SGML2DVI    = $(LINUXDOC) --backend=latex --output=dvi
DVI2PS      = dvips -t $(PAPERSIZE)
LATEX       = latex
PDFLATEX    = pdflatex

.sgml.html :
	$(SGML2HTML) $<

.sgml.dvi :
	$(SGML2DVI) $<
	
.sgml.tex :
	$(SGML2LATEX) $<

.tex.dvi :
	$(LATEX) $<
	$(LATEX) $<

.dvi.ps :
	$(DVI2PS) $< -o $@

.tex.pdf :
	$(PDFLATEX) $<
	$(PDFLATEX) $<

SGMLS    = FAQ.sgml Tutorial.sgml UsersGuide.sgml 
HTMLS    = CHANGES.html GPL.html philosophical-gnu-sm.jpg nohelp.html

HTMLDOCS = FAQ.html Tutorial.html UsersGuide.html

DVIDOCS  = FAQ.dvi Tutorial.dvi UsersGuide.dvi

PSDOCS   = FAQ.ps Tutorial.ps UsersGuide.ps

PDFDOCS  = FAQ.pdf Tutorial.pdf UsersGuide.pdf

TEXDOCS  = FAQ.tex Tutorial.tex UsersGuide.tex

TUTDATA  = 2.1.dat 3.dat 7.1.2.agr 7.1.3.agr 8.2.dat \
           10.1.dat 10a.dat 10c.dat 10b.dat shiftdata.sh mygraph.png

GRACEMAN = grace.1
MANLINKS = xmgrace.1 gracebat.1
MANPAGES = $(GRACEMAN) grconvert.1 convcal.1

all : html dvi

html : $(HTMLDOCS)

dvi  : $(DVIDOCS)

pdf  : $(PDFDOCS)

ps   : $(PSDOCS)

tex  : $(TEXDOCS)

clean :
	$(RM) $(TEXDOCS) *.aux *.log *.toc

distclean : clean
	$(RM) *.orig

devclean : distclean
	$(RM) $(DVIDOCS) $(PSDOCS) $(PDFDOCS) $(HTMLDOCS)

install : $(HTMLS) $(HTMLDOCS) $(DVIDOCS) $(TUTDATA) $(MANPAGES)
	$(MKINSTALLDIRS) $(DESTDIR)$(GRACE_HOME)/doc
	for i in $?; do $(INSTALL_DATA) $$i $(DESTDIR)$(GRACE_HOME)/doc; done
	cd $(DESTDIR)$(GRACE_HOME)/doc; \
	for i in $(MANLINKS); do $(RM) $$i; $(LN_S) $(GRACEMAN) $$i; done

$(DESTDIR)$(GRACE_HOME)/doc/grace.1 : $(MANPAGES)
	$(MAKE) install

links : $(DESTDIR)$(GRACE_HOME)/doc/grace.1
	$(MKINSTALLDIRS) $(DESTDIR)$(PREFIX)/man
	$(MKINSTALLDIRS) $(DESTDIR)$(PREFIX)/man/man1
	cd $(DESTDIR)$(PREFIX)/man/man1; \
	for i in $(MANPAGES) $(MANLINKS); do $(RM) $$i; $(LN_S) $(GRACE_HOME)/doc/$$i $$i; done

tests : dummy

dummy :
