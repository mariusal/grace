#####################################################
# Makefile for Grace                                #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

GRBATCH=gracebat$(EXE)

GRACECLI=grace$(EXE)

CEPHES_LIB=$(TOP)/cephes/libcephes.a

CFLAGS=$(CFLAGS0) -I$(TOP) -I. $(T1_INC) $(XBAE_INC) $(CPPFLAGS) $(GUI_FLAGS)

LIBS=$(GUI_LIBS) $(CEPHES_LIB) $(NETCDF_LIBS) $(FFTW_LIB) \
	$(T1_LIB) $(PDF_LIB) $(TIFF_LIB) $(JPEG_LIB) $(PNG_LIB) $(Z_LIB) \
	$(NOGUI_LIBS) $(DL_LIB)

PREFS=-DGRACE_HOME=\"$(GRACE_HOME)\" -DGRACE_PRINT_CMD=\"$(PRINT_CMD)\" \
	-DGRACE_EDITOR=\"$(GRACE_EDITOR)\" $(HELPVIEWER) \
	-DCCOMPILER=\"'$(CC) $(CFLAGS) $(LIBS)'\" 

all : $(GRACE)


include Make.common

$(GROBJS) $(GUIOBJS) $(PARS_O) buildinfo$(O) : $(TOP)/config.h

include Make.dep

xmgrace$(EXE) : $(GROBJS) $(PARS_C) $(GUIOBJS) $(CEPHES_LIB)
	$(CC) $(CFLAGS) $(GROBJS) $(GUIOBJS) -o $@ $(LDFLAGS) $(LIBS)

grace$(EXE) : $(GROBJS) $(PARS_C) $(CEPHES_LIB)
	$(CC) $(CFLAGS) $(GROBJS) -o $@ $(LDFLAGS) $(LIBS)


buildinfo$(EXE) : buildinfo$(O)
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS) $(GUI_LIBS) $(T1_LIB) $(NOGUI_LIBS)

buildinfo$(O) : $(TOP)/Make.conf
	$(CC) $(CFLAGS) $(PREFS) -c buildinfo.c -o $@

buildinfo.h : buildinfo$(EXE) $(GRSRCS) $(GUISRCS)
	$(RM) t.h
	./buildinfo$(EXE) > t.h
	mv t.h $@

$(PARS_C) : $(PARS_Y)
	$(YACC) -t $?
	mv y.tab.c $@


install : $(GRACE)
	$(MKINSTALLDIRS) $(GRACE_HOME)/bin
	$(INSTALL_PROGRAM) -s $(GRACE) $(GRACE_HOME)/bin/$(GRACE)
	cd $(GRACE_HOME)/bin; $(RM) $(GRBATCH); $(LN_S) $(GRACE) $(GRBATCH)

tests : dummy

links : $(GRACE_HOME)/bin/$(GRACE)
	$(MKINSTALLDIRS) $(PREFIX)/bin
	cd $(PREFIX)/bin; $(RM) $(GRACE); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRACE)
	cd $(PREFIX)/bin; $(RM) $(GRBATCH); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRBATCH)
	cd $(PREFIX)/bin; $(RM) $(GRACECLI); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRACECLI)

clean : 
	$(RM) $(GROBJS) $(GUIOBJS) $(PARS_O) buildinfo.h buildinfo$(O) buildinfo$(EXE)

distclean : clean
	$(RM) $(GRACE) tags *.orig *.*gr .gdb_history t1lib.log

devclean : distclean
	$(RM) $(PARS_C)

depend :
	echo "# Generated automatically by \`make depend'" > Make.dep
	$(CC) $(CFLAGS) -MM $(GRSRCS) $(GUISRCS) $(PARS_C) buildinfo.c \
		| sed 's/\.o:/\$$(O)\ :/g' >> Make.dep

tags : $(GRSRCS) $(GUISRCS) $(PARS_Y)
	ctags $(GRSRCS) $(GUISRCS) $(PARS_Y)

dummy :

# DO NOT DELETE
