#####################################################
# Top-level Makefile for Grace                      #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the current directory #
#####################################################

include Make.conf

subdirs : configure Make.conf
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE)) || exit 1; done

all : $(subdirs)

install : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) install) || exit 1; done
	$(MKINSTALLDIRS) $(GRACE_HOME)
	$(INSTALL_DATA) gracerc.sample $(GRACE_HOME)

tests : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) tests) || exit 1; done

check : tests

links : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) links) || exit 1; done

clean : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean) || exit 1; done

distclean : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) distclean) || exit 1; done
	$(RM) config.log config.status config.cache config.h Make.conf

devclean : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) devclean) || exit 1; done
	$(RM) config.log config.status config.cache config.h Make.conf configure

texts : CHANGES MIGRATION

CHANGES : doc/CHANGES.html
	@lynx -dump $? > CHANGES

MIGRATION : doc/MIGRATION.html
	@lynx -dump $? > MIGRATION

Make.conf : conf/Make.conf.in configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

configure : conf/configure.in
	autoconf $? > configure
	chmod +x configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

dummy :

