#####################################################
# Makefile for templates of Grace                   #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

DEFAULT_XGR = Default.xgr

all : $(DEFAULT_XGR)

clean : dummy

distclean : 
	$(RM) *.orig

devclean : distclean

install : $(DEFAULT_XGR)
	$(MKINSTALLDIRS) $(DESTDIR)$(GRACE_HOME)/templates
	$(INSTALL_DATA) $(DEFAULT_XGR) $(DESTDIR)$(GRACE_HOME)/templates/$(DEFAULT_XGR)

links : dummy

tests : dummy

dummy :
