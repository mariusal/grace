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
	@if test -f $(DESTDIR)$(GRACE_HOME)/templates/$(DEFAULT_XGR); then \
		echo "	$(DESTDIR)$(GRACE_HOME)/templates/$(DEFAULT_XGR) exists"; \
		echo "	Installing only $(DESTDIR)$(GRACE_HOME)/templates/$(DEFAULT_XGR).sample"; \
		$(INSTALL_DATA) $(DEFAULT_XGR) $(DESTDIR)$(GRACE_HOME)/templates/$(DEFAULT_XGR).sample; \
	else \
		$(INSTALL_DATA) $(DEFAULT_XGR) $(DESTDIR)$(GRACE_HOME)/templates/$(DEFAULT_XGR); \
	fi

links : dummy

tests : dummy

dummy :
