#####################################################
# Makefile for Xbae matrix widget                   #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

CFLAGS=$(CFLAGS0) -I$(TOP) -I. $(GUI_FLAGS) \
	-DDRAW_RESIZE_SHADOW
#	-DALLOW_COLUMN_RESIZE -DDEFAULT_SCROLL_SPEED=500 -DNEED_WCHAR

LIB=libXbae.a

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
