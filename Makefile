#####################################################
# Top-level Makefile for Grace                      #
#####################################################
# You should not change anything here.              #
#####################################################

include Make.conf

subdirs : configure Make.conf
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE)) || exit 1; done

all : subdirs

install : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) install) || exit 1; done
	$(MKINSTALLDIRS) $(GRACE_HOME)
	@if test -f $(GRACE_HOME)/gracerc; then \
		echo "	$(GRACE_HOME)/gracerc exists"; \
		echo "	Installing only $(GRACE_HOME)/gracerc.sample"; \
		$(INSTALL_DATA) gracerc $(GRACE_HOME)/gracerc.sample; \
	else \
		$(INSTALL_DATA) gracerc $(GRACE_HOME); \
	fi
	$(INSTALL_DATA) gracerc.user $(GRACE_HOME)

tests : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) tests) || exit 1; done

check : tests

links : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) links) || exit 1; done

clean : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean) || exit 1; done

distclean : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) distclean) || exit 1; done
	$(RM) config.log config.status config.cache config.h Make.conf

devclean : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) devclean) || exit 1; done
	$(RM) config.log config.status config.cache config.h Make.conf configure

texts : CHANGES ChangeLog

CHANGES : doc/CHANGES.html
	@lynx -dump $? > CHANGES

ChangeLog : 
	./scripts/cvs2cl.pl

Make.conf : ac-tools/Make.conf.in configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

configure : ac-tools/configure.in ac-tools/aclocal.m4
	autoconf ac-tools/configure.in > configure
	chmod +x configure

dummy :

