#####################################################
# Makefile for auxiliary stuff of Grace             #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

AUXILIARIES = README convcal.c magic.grace
PROGRAMS = fdf2fit convcal$(EXE)

all : convcal

clean : dummy

distclean : dummy

devclean : dummy

convcal$(EXE) : convcal.c
	$(CC) $(CFLAGS) -o $@ convcal.c -lm

install : $(AUXILIARIES) $(PROGRAMS)
	$(MKINSTALLDIRS) $(GRACE_HOME)/auxiliary
	for i in $(AUXILIARIES); do $(INSTALL_DATA) $$i $(GRACE_HOME)/auxiliary; done
	for i in $(PROGRAMS); do $(INSTALL_PROGRAM) $$i $(GRACE_HOME)/bin; done

tests : dummy

links : dummy

dummy :
