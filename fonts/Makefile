#####################################################
# Makefile for the fonts directory of GRACE         #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

FONTDATABASE = FontDataBase

FONTS = type1/n021003l.pfb \
	type1/n021023l.pfb \
	type1/n021004l.pfb \
	type1/n021024l.pfb \
	type1/n019003l.pfb \
	type1/n019023l.pfb \
	type1/n019004l.pfb \
	type1/n019024l.pfb \
	type1/n022003l.pfb \
	type1/n022023l.pfb \
	type1/n022004l.pfb \
	type1/n022024l.pfb \
	type1/d050000l.pfb \
	type1/s050000l.pfb

AFMS =  type1/d050000l.afm \
	type1/n019003l.afm \
	type1/n019004l.afm \
	type1/n019023l.afm \
	type1/n019024l.afm \
	type1/n021003l.afm \
	type1/n021004l.afm \
	type1/n021023l.afm \
	type1/n022003l.afm \
	type1/n022023l.afm \
	type1/n022004l.afm \
	type1/n022024l.afm \
	type1/n021024l.afm \
	type1/s050000l.afm

ENCODINGS = enc/IsoLatin1.enc enc/IsoLatin2.enc enc/Default.enc

all : dummy

tests : dummy

clean : dummy

distclean : dummy

devclean : dummy

install : $(FONTS) $(AFMS) $(ENCODINGS)
	$(MKINSTALLDIRS) $(GRACE_HOME)/fonts
	$(INSTALL_DATA) $(FONTDATABASE) $(GRACE_HOME)/fonts
	$(MKINSTALLDIRS) $(GRACE_HOME)/fonts/type1
	for i in $(FONTS); do $(INSTALL_DATA) $$i $(GRACE_HOME)/fonts/type1; done
	for i in $(AFMS); do $(INSTALL_DATA) $$i $(GRACE_HOME)/fonts/type1; done
	$(MKINSTALLDIRS) $(GRACE_HOME)/fonts/enc
	for i in $(ENCODINGS); do $(INSTALL_DATA) $$i $(GRACE_HOME)/fonts/enc; done

links : dummy

dummy :
