#####################################################
# Makefile for examples of Grace                    #
#####################################################
# You should not change anything here.              #
#####################################################

TOP=..

include $(TOP)/Make.conf

DOTEST = dotest$(BAT)

EXAMPLES = times.agr \
	au.agr manygraphs.agr terr.agr tinset.agr \
	axes.agr tlog.agr log2.agr logit.agr reciprocal.agr \
	bar.agr explain.agr \
	fills.agr props.agr \
	chartebar.agr charts.agr \
	hilo.agr test2.agr arrows.agr txyr.agr \
	co2.agr stackedb.agr tfonts.agr typeset.agr xyz.agr \
	logistic.agr symslines.agr polar.agr \
	motif.agr spectrum.agr pie.agr txttrans.agr boxplot.agr vmap.agr \
	xysize.agr xycolor.agr

all : dummy

clean : dummy

distclean : dummy

devclean : dummy

install : $(EXAMPLES)
	$(MKINSTALLDIRS) $(DESTDIR)$(GRACE_HOME)/examples
	for i in $(EXAMPLES); do $(INSTALL_DATA) $$i $(DESTDIR)$(GRACE_HOME)/examples; done

links : dummy

tests : $(EXAMPLES) $(DOTEST)
	./$(DOTEST)

dummy :
