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

CFLAGS=$(CFLAGS0) -I$(TOP)/include -I$(TOP)/cephes -I. \
	$(T1_INC) $(EXPAT_INC) $(XBAE_INC) $(CPPFLAGS) $(GUI_FLAGS)

LIBS=$(GUI_LIBS) $(CEPHES_LIB) $(NETCDF_LIBS) $(FFTW_LIB) \
	$(EXPAT_LIB) \
	$(GRACE_CORE_LIB) $(GRACE_CANVAS_LIB) $(T1_LIB) $(XMI_LIB) \
	$(PDF_LIB) $(TIFF_LIB) $(JPEG_LIB) $(PNG_LIB) $(Z_LIB) \
	$(GRACE_BASE_LIB) $(NOGUI_LIBS) $(DL_LIB)

PREFS=-DMAJOR_REV=$(MAJOR_REV) -DMINOR_REV=$(MINOR_REV) \
	-DPATCHLEVEL=$(PATCHLEVEL) -DBETA_VER=\"$(BETA_VER)\" \
	-DGRACE_HOME=\"$(GRACE_HOME)\" -DGRACE_PRINT_CMD=\"$(PRINT_CMD)\" \
	-DGRACE_EDITOR=\"$(GRACE_EDITOR)\" $(HELPVIEWER) \
	-DCCOMPILER=\"'$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS)'\" 

all : $(GRACE)


include Make.common

$(GROBJS) $(GUIOBJS) $(PARS_O) buildinfo$(O) : $(TOP)/include/config.h

include Make.dep

Make.dep : $(GRSRCS) $(GUISRCS) $(PARS_C) buildinfo.c
	echo "# Generated automatically by \`make depend'" > $@
	$(CC) $(CFLAGS) -MM $(GRSRCS) $(GUISRCS) $(PARS_C) buildinfo.c \
		| sed 's/\.o:/\$$(O)\ :/g' >> $@

xmgrace$(EXE) : $(GROBJS) $(GUIOBJS) $(CEPHES_LIB) $(GRACE_CORE_LIB) \
	$(GRACE_CANVAS_LIB) $(GRACE_BASE_LIB)
	$(CC) $(CFLAGS) $(GROBJS) $(GUIOBJS) -o $@ $(LDFLAGS) $(LIBS)

grace$(EXE) : $(GROBJS) $(CEPHES_LIB) $(GRACE_CORE_LIB) \
	$(GRACE_CANVAS_LIB) $(GRACE_BASE_LIB)
	$(CC) $(CFLAGS) $(GROBJS) -o $@ $(LDFLAGS) $(LIBS)


buildinfo$(EXE) : buildinfo$(O)
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS) $(GUI_LIBS) $(T1_LIB) $(JPEG_LIB) $(PNG_LIB) $(Z_LIB) $(NOGUI_LIBS)

buildinfo$(O) : $(TOP)/Make.conf
	$(CC) $(CFLAGS) $(PREFS) -c buildinfo.c -o $@

buildinfo.h : buildinfo$(EXE) $(GRSRCS) $(GUISRCS)
	$(RM) t.h
	./buildinfo$(EXE) > t.h && mv t.h $@

bi$(O) : buildinfo.h

$(PARS_C) : $(PARS_Y)
	$(YACC) -t $? && mv y.tab.c $@

xml_in.c : grace.xcc
	$(XCC) -b -i $? -o $@

install : $(GRACE)
	$(MKINSTALLDIRS) $(DESTDIR)$(GRACE_HOME)/bin
	$(INSTALL_PROGRAM) -s $(GRACE) $(DESTDIR)$(GRACE_HOME)/bin/$(GRACE)
	cd $(DESTDIR)$(GRACE_HOME)/bin; $(RM) $(GRBATCH); $(LN_S) $(GRACE) $(GRBATCH)

tests : dummy

$(DESTDIR)$(GRACE_HOME)/bin/$(GRACE) : $(GRACE)
	$(MAKE) install

links : $(DESTDIR)$(GRACE_HOME)/bin/$(GRACE)
	$(MKINSTALLDIRS) $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin; $(RM) $(GRACE); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRACE)
	cd $(DESTDIR)$(PREFIX)/bin; $(RM) $(GRBATCH); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRBATCH)
	cd $(DESTDIR)$(PREFIX)/bin; $(RM) $(GRACECLI); $(LN_S) $(GRACE_HOME)/bin/$(GRACE) $(GRACECLI)

clean : 
	$(RM) $(GROBJS) $(GUIOBJS) $(PARS_O) buildinfo.h buildinfo$(O) buildinfo$(EXE)

distclean : clean
	$(RM) $(GRACE) tags *.orig .gdb_history t1lib.log

devclean : distclean
	$(RM) $(PARS_C)

depend : Make.dep

tags : $(GRSRCS) $(GUISRCS) $(PARS_Y)
	ctags $(GRSRCS) $(GUISRCS) $(PARS_Y) *.h $(TOP)/lib/*/*.[c,h] $(TOP)/include/grace/*.h

dummy :

# DO NOT DELETE
