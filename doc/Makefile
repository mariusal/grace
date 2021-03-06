#####################################################
# Makefile for documentation of Grace               #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .sgml .dvi .html .pdf .ps .tex

# NB: the language option should be made `configure'able
LANGUAGE    = english

LINUXDOC    = linuxdoc --language=$(LANGUAGE) --papersize=$(PAPERSIZE)

SGML2HTML   = $(LINUXDOC) --backend=html  --split=0 --toc=2
SGML2LATEX  = $(LINUXDOC) --backend=latex --pass="\usepackage{pslatex}" \
	--output=tex
SGML2DVI    = $(LINUXDOC) --backend=latex --pass="\usepackage{pslatex}" \
	--output=dvi
		
SGML2PS     = $(LINUXDOC) --backend=latex --pass="\usepackage{pslatex}" \
	--output=ps
		
SGML2PDF    = $(LINUXDOC) --backend=latex --pass="\usepackage{pslatex}" \
	--output=pdf
		
DVI2PS      = dvips -t $(PAPERSIZE)
PS2PDF      = ps2pdf
LATEX       = latex

.sgml.html :
	$(SGML2HTML) $<

.sgml.dvi :
	$(SGML2DVI) $<
	
.sgml.ps :
	$(SGML2PS) $<
	
.sgml.pdf :
	$(SGML2PDF) $<
	
.sgml.tex :
	$(SGML2LATEX) $<

.tex.dvi :
	$(LATEX) $<
	$(LATEX) $<

.dvi.ps :
	$(DVI2PS) $< -o $@

.ps.pdf :
	$(PS2PDF) $< $@

HTMLS    = NEWS.html GPL.html philosophical-gnu-sm.jpg nohelp.html

HTMLDOCS = FAQ.html UsersGuide.html

DVIDOCS  = FAQ.dvi UsersGuide.dvi

PSDOCS   = FAQ.ps UsersGuide.ps

PDFDOCS  = FAQ.pdf UsersGuide.pdf

TEXDOCS  = FAQ.tex UsersGuide.tex

GRACEMAN = grace.1
MANLINKS = xmgrace.1 gracebat.1
MANPAGES = $(GRACEMAN) convcal.1

all : html pdf

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

install : $(HTMLS) $(HTMLDOCS) $(PDFDOCS) $(MANPAGES)
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

check : dummy

dummy :
