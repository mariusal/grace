#####################################################
# Makefile for GRACE                                #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

GRBATCH=gracebat$(EXE)

CEPHES_LIB=$(TOP)/cephes/libcephes.a

CFLAGS=$(CFLAGS0) -I$(TOP) -I. $(GUI_FLAGS) -DGRACE_HOME=\"$(GRACE_HOME)\" \
	$(HELPVIEWER) $(NETCDF) -DPRINT_CMD=\"$(PRINT_CMD)\"

LIBS=$(GUI_LIBS) $(CEPHES_LIB) $(NETCDF_LIBS) $(FFTW_LIB) \
	$(T1_LIB) $(PDF_LIB) $(GD_LIB) $(NOGUI_LIBS) $(DL_LIB)

all : $(GRACE)


include Make.common

$(GROBJS) $(GUIOBJS) $(PARS_O) : $(TOP)/config.h

include Make.dep

xmgrace$(EXE) : $(GROBJS) $(PARS_C) $(GUIOBJS) $(CEPHES_LIB)
	$(CC) $(CFLAGS) $(GROBJS) $(GUIOBJS) -o $@ $(LDFLAGS) $(LIBS)

grace$(EXE) : $(GROBJS) $(PARS_C) $(CEPHES_LIB)
	$(CC) $(CFLAGS) $(GROBJS) -o $@ $(LDFLAGS) $(LIBS)

buildinfo$(EXE) : buildinfo.o
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS) $(GUI_LIBS) $(T1_LIB) $(NOGUI_LIBS)

$(PARS_C) : $(PARS_Y)
	$(YACC) -t $?
	mv y.tab.c $@

buildinfo.h : buildinfo$(EXE)
	$? > $@

install : $(GRACE)
	$(MKINSTALLDIRS) $(GRACE_HOME)/bin
	$(INSTALL_PROGRAM) -s $(GRACE) $(GRACE_HOME)/bin/$(GRACE)
	cd $(GRACE_HOME)/bin; $(RM) $(GRBATCH); $(LN_S) $(GRACE) $(GRBATCH)

tests : dummy

links : $(GRACE)
	$(MKINSTALLDIRS) $(PREFIX)/bin
	cd $(PREFIX)/bin; $(RM) $(GRACE); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRACE)
	cd $(PREFIX)/bin; $(RM) $(GRBATCH); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRBATCH)

clean : 
	$(RM) $(GROBJS) $(GUIOBJS)

distclean : 
	$(RM) $(GRACE) $(GROBJS) $(GUIOBJS) \
	tags *.orig *.*gr .gdb_history t1lib.log
	
devclean : 
	$(RM) $(GRACE) $(GROBJS) $(GUIOBJS) \
	tags *.orig *.*gr .gdb_history t1lib.log $(PARS_C)

depend :
	$(CC) $(CFLAGS) -MM $(GRSRCS) $(GUISRCS) $(PARS_C) \
		| sed 's/\.o:/\$$(O)\ :/g' > Make.dep

tags : $(GRSRCS) $(GUISRCS) $(PARS_Y)
	ctags --format=1 $(GRSRCS) $(GUISRCS) $(PARS_Y)

dummy :

