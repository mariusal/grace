#####################################################
# Makefile for auxiliary stuff of Grace             #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

AUXILIARIES = README convcal.c magic.grace
PROGRAMS = fdf2fit convcal$(EXE)

all : convcal$(EXE)

clean : dummy

distclean : clean
	$(RM) convcal$(EXE)

devclean : distclean

convcal$(EXE) : convcal.c
	$(CC) $(CFLAGS0) $(CPPFLAGS) -o $@ convcal.c $(NOGUI_LIBS)

install : $(AUXILIARIES) $(PROGRAMS)
	$(MKINSTALLDIRS) $(GRACE_HOME)/auxiliary
	for i in $(AUXILIARIES); do $(INSTALL_DATA) $$i $(GRACE_HOME)/auxiliary; done
	for i in $(PROGRAMS); do $(INSTALL_PROGRAM) $$i $(GRACE_HOME)/bin; done

tests : dummy

links : dummy

dummy :
