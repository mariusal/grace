#####################################################
# Makefile for auxiliary stuff of Grace             #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

AUXILIARIES = README convcal.c magic.grace

all : dummy

clean : dummy

distclean : dummy

devclean : dummy

install : $(AUXILIARIES) fdf2fit
	$(MKINSTALLDIRS) $(GRACE_HOME)/auxiliary
	for i in $(AUXILIARIES); do $(INSTALL_DATA) $$i $(GRACE_HOME)/auxiliary; done
	$(INSTALL_PROGRAM) fdf2fit $(GRACE_HOME)/auxiliary

tests : dummy

links : dummy

dummy :
