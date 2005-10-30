#####################################################
# Makefile for Grace                                #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

# Links to the main exec
GRACEGUI=xmgrace$(PROG_SUFFIX)$(EXE)
GRACECLI=grace$(PROG_SUFFIX)$(EXE)
GRACEBAT=gracebat$(PROG_SUFFIX)$(EXE)

SCHEMA=grace.xsd

CEPHES_LIB=$(TOP)/cephes/libcephes.a

CFLAGS=$(CFLAGS0) -I$(TOP)/include -I$(TOP)/cephes -I. \
	$(T1_INC) $(EXPAT_INC) $(XBAE_INC) $(CUPS_CFLAGS) $(CPPFLAGS) $(GUI_FLAGS)

LIBS=$(GUI_LIBS) $(CEPHES_LIB) $(NETCDF_LIBS) $(FFTW_LIB) $(EXPAT_LIB) \
	$(GRACE_PLOT_LIB) $(GRACE_CORE_LIB) $(GRACE_CANVAS_LIB) \
	$(T1_LIB) $(XMI_LIB) \
	$(PDF_LIB) $(JPEG_LIB) $(PNG_LIB) $(Z_LIB) \
	$(GRACE_BASE_LIB) $(UNDO_LIB) $(CUPS_LIBS) $(NOGUI_LIBS) $(DL_LIB)

PREFS=-DMAJOR_REV=$(MAJOR_REV) -DMINOR_REV=$(MINOR_REV) \
	-DPATCHLEVEL=$(PATCHLEVEL) -DBETA_VER=\"$(BETA_VER)\" \
	-DGRACE_HOME=\"$(GRACE_HOME)\" -DGRACE_PRINT_CMD=\"$(PRINT_CMD)\" \
	-DGRACE_EDITOR=\"$(GRACE_EDITOR)\" $(HELPVIEWER) \
	-DCCOMPILER=\"'$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS)'\" 

all : $(GRACE) $(SCHEMA)


include Make.common

$(GROBJS) $(GUIOBJS) $(PARS_O) buildinfo$(O) : $(TOP)/include/config.h

include Make.dep

Make.dep : $(GRSRCS) $(GUISRCS) $(PARS_C) 
	echo "# Generated automatically by \`make depend'" > $@
	$(CC) $(CFLAGS) -MM $(GRSRCS) $(GUISRCS) $(PARS_C) \
		| sed 's/\.o:/\$$(O)\ :/g' >> $@

xmgrace$(EXE) : $(GROBJS) $(GUIOBJS) $(CEPHES_LIB) $(GRACE_PLOT_LIB) \
	$(GRACE_CORE_LIB) $(GRACE_CANVAS_LIB) $(GRACE_BASE_LIB)
	$(CC) $(CFLAGS) $(GROBJS) $(GUIOBJS) -o $@ $(LDFLAGS) $(LIBS)

grace$(EXE) : $(GROBJS) $(CEPHES_LIB) $(GRACE_PLOT_LIB) $(GRACE_CORE_LIB) \
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
	$(RM) t.c
	$(XCC) -b -i $? -o t.c && mv t.c $@

$(SCHEMA) : grace.xcc xstrings.h
	$(XCC) -s -i $? | cpp -P -nostdinc -imacros xstrings.h > $@

install : $(GRACE)
	$(MKINSTALLDIRS) $(DESTDIR)$(GRACE_HOME)/bin
	$(INSTALL_PROGRAM) -s $(GRACE) $(DESTDIR)$(GRACE_HOME)/bin/$(GRACEGUI)
	cd $(DESTDIR)$(GRACE_HOME)/bin; $(RM) $(GRACEBAT); $(LN_S) $(GRACEGUI) $(GRACEBAT)
	cd $(DESTDIR)$(GRACE_HOME)/bin; $(RM) $(GRACECLI); $(LN_S) $(GRACEGUI) $(GRACECLI)

tests : dummy

$(DESTDIR)$(GRACE_HOME)/bin/$(GRACE) : $(GRACE)
	$(MAKE) install

links : $(DESTDIR)$(GRACE_HOME)/bin/$(GRACE)
	$(MKINSTALLDIRS) $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin; $(RM) $(GRACEGUI); $(LN_S) $(GRACE_HOME)/bin/$(GRACEGUI) $(GRACEGUI)
	cd $(DESTDIR)$(PREFIX)/bin; $(RM) $(GRACEBAT); $(LN_S) $(GRACE_HOME)/bin/$(GRACEGUI) $(GRACEBAT)
	cd $(DESTDIR)$(PREFIX)/bin; $(RM) $(GRACECLI); $(LN_S) $(GRACE_HOME)/bin/$(GRACEGUI) $(GRACECLI)

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
