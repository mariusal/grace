#####################################################
# Makefile for grconvert -- VMS version (R.N.) 5/98 #
#####################################################

# not tested (RN)


TOP = [-]
HERE = SYS$DISK:[]
SRCDIR = [-.SRC]
VMSDIR = [-.ARCH.VMS]
#RM = DELETE/log

include $(TOP)Make.conf

PROG = grconvert$(EXE)

SRCS = grconvert.c defaults.c readbin.c writeasc.c util.c

OBJS = grconvert$(O) defaults$(O) readbin$(O) writeasc$(O) util$(O)

# CFLAGS = $(CFLAGS0) -I$(TOP) -I. -I$(TOP)/src -DGRCONVERT
CFLAGS = /INCLUDE=($(TOP),$(HERE),$(SRCDIR),$(VMSDIR))/DEF=GRCONVERT=1 \
  /FLOAT=D_FLOAT

LIBS = ,xdr.opt/opt

all : $(PROG)
	@ !

$(PROG) : $(OBJS)
	$(LINK) /EXE=$@ $+ $(LIBS)


clean :
	$(RM) $(OBJS) 

distclean : clean
	$(RM) $(PROG) 
	

