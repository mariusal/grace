#####################################################
# Makefile for templates of Grace                   #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

DEFAULT_AGR = Default.agr

all : $(DEFAULT_AGR)

clean : dummy

distclean : 
	$(RM) *.orig

devclean : dummy

install : $(DEFAULT_AGR)
	$(MKINSTALLDIRS) $(GRACE_HOME)/templates
	@if test -f $(GRACE_HOME)/templates/$(DEFAULT_AGR); then \
		echo "	$(GRACE_HOME)/templates/$(DEFAULT_AGR) exists"; \
		echo "	Installing only $(GRACE_HOME)/templates/$(DEFAULT_AGR).sample"; \
		$(INSTALL_DATA) $(DEFAULT_AGR) $(GRACE_HOME)/templates/$(DEFAULT_AGR).sample; \
	else \
		$(INSTALL_DATA) $(DEFAULT_AGR) $(GRACE_HOME)/templates/$(DEFAULT_AGR); \
	fi

links : dummy

tests : dummy

dummy :
