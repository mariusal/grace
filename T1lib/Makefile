#####################################################
# Makefile for T1lib (bundled with GRACE)           #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

ALLCFLAGS = $(CFLAGS0) -DGLOBAL_CONFIG_DIR="\".\"" -DT1_AA_TYPE16=$(T1_AA_TYPE16) -DT1_AA_TYPE32=$(T1_AA_TYPE32) -I../..
SUBMAKE   = $(MAKE) 'CC=$(CC)' 'CFLAGS=$(ALLCFLAGS)'

# targets to build: 
MAIN_TARGET = libt1.a

TYPE1_OBJS = \
	type1/arith.o \
	type1/curves.o \
	type1/fontfcn.o \
	type1/hints.o \
	type1/lines.o \
	type1/objects.o \
	type1/paths.o \
	type1/regions.o \
	type1/scanfont.o \
	type1/spaces.o \
	type1/t1io.o \
	type1/t1snap.o \
	type1/t1stub.o \
	type1/token.o \
	type1/type1.o \
	type1/util.o 


T1LIB_OBJS = \
	t1lib/t1finfo.o \
	t1lib/t1base.o \
	t1lib/t1delete.o \
	t1lib/t1enc.o \
	t1lib/t1env.o \
	t1lib/t1load.o \
	t1lib/t1set.o \
	t1lib/t1trans.o \
	t1lib/t1aaset.o \
	t1lib/t1afmtool.o \
	t1lib/parseAFM.o 


TYPE1_SRCS = \
	type1/arith.c \
	type1/curves.c \
	type1/fontfcn.c \
	type1/hints.c \
	type1/lines.c \
	type1/objects.c \
	type1/paths.c \
	type1/regions.c \
	type1/scanfont.c \
	type1/spaces.c \
	type1/t1io.c \
	type1/t1snap.c \
	type1/t1stub.c \
	type1/token.c \
	type1/type1.c \
	type1/util.c 


T1LIB_SRCS = \
	t1lib/t1finfo.c \
	t1lib/t1base.c \
	t1lib/t1delete.c \
	t1lib/t1enc.c \
	t1lib/t1env.c \
	t1lib/t1load.c \
	t1lib/t1set.c \
	t1lib/t1trans.c \
	t1lib/t1aaset.c \
	t1lib/t1afmtool.c \
	t1lib/parseAFM.c 


TYPE1_HEADERS = \
	type1/Xstuff.h \
	type1/arith.h \
	type1/blues.h \
	type1/cluts.h \
	type1/curves.h \
	type1/digit.h \
	type1/ffilest.h \
	type1/font.h \
	type1/fontfcn.h \
	type1/fontfile.h \
	type1/fontmisc.h \
	type1/fonts.h \
	type1/fontstruct.h \
	type1/fontxlfd.h \
	type1/fsmasks.h \
	type1/hdigit.h \
	type1/hints.h \
	type1/lines.h \
	type1/objects.h \
	type1/paths.h \
	type1/paths_rmz.h \
	type1/pictures.h \
	type1/regions.h \
	type1/spaces.h \
	type1/spaces_rmz.h \
	type1/strokes.h \
	type1/t1hdigit.h \
	type1/t1imager.h \
	type1/t1intf.h \
	type1/t1stdio.h \
	type1/token.h \
	type1/tokst.h \
	type1/trig.h \
	type1/types.h \
	type1/util.h


T1LIB_HEADERS = \
	t1lib/parseAFM.h \
	t1lib/t1afmtool.h \
	t1lib/t1aaset.h \
	t1lib/t1base.h \
	t1lib/t1delete.h \
	t1lib/t1enc.h \
	t1lib/t1env.h \
	t1lib/t1extern.h \
	t1lib/t1finfo.h \
	t1lib/t1global.h \
	t1lib/t1lib.h \
	t1lib/t1load.h \
	t1lib/t1misc.h \
	t1lib/t1set.h \
	t1lib/t1trans.h \
	t1lib/t1types.h


LIBSUBDIRS = \
	type1 \
	t1lib


MAIN_TARGET = libt1.a


DUMMYSUBDIRS=XXXX


all: $(DUMMYSUBDIRS) $(MAIN_TARGET)


$(DUMMYSUBDIRS): dummy
	@for i in $(LIBSUBDIRS); do \
	  (cd $$i; $(SUBMAKE) ) || exit 1; \
	done



libt1.a: $(TYPE1_OBJS) $(T1LIB_OBJS) $(TYPE1_SRCS) $(T1LIB_SRCS) $(TYPE1_HEADERS) $(T1LIB_HEADERS)
	$(RM) $@
	$(AR) cr $@ $(TYPE1_OBJS) $(T1LIB_OBJS)
	$(RANLIB) $@


.PHONY: clean


clean: dummy
	for i in $(LIBSUBDIRS); do \
	  (cd $$i; $(MAKE) clean) || exit 1; \
	done
	$(RM) libt1*.a *~  

distclean: dummy
	for i in $(LIBSUBDIRS); do \
	  (cd $$i; $(MAKE) clean) || exit 1; \
	done
	$(RM) libt1*.a *~  

install: dummy

links: dummy

tests: dummy

dummy:

