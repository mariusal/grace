#####################################################
# Makefile for expat lib   (bundled with Grace)     #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

CFLAGS=$(CFLAGS0) -I$(TOP)/include -I. $(CPPFLAGS) -DHAVE_EXPAT_CONFIG_H

LIB=libexpat.a

all : $(LIB)

include Make.common


$(LIB) : $(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

install : $(LIB)

tests : dummy

links : dummy

clean :
	$(RM) $(LIB) $(OBJS)

distclean :
	$(RM) $(LIB) $(OBJS) *.orig

devclean :
	$(RM) $(LIB) $(OBJS) *.orig

dummy :
