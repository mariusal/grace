#####################################################
# Makefile for T1 library (VMS)                     #
#####################################################

# Rolf Niepraschk, 12/97, niepraschk@ptb.de

TOP = [-]
ECHO = WRITE SYS$OUTPUT

.INCLUDE $(TOP)Make.conf

CFLAGS = $(CFLAGS0)/INCLUDE=[--]/DEFINE=(GLOBAL_CONFIG_DIR=""" """)

LIB = libt1lib.olb

OBJS =	[.t1lib]parseafm$(O) [.t1lib]t1aaset$(O) [.t1lib]t1afmtool$(O) \
	[.t1lib]t1base$(O) [.t1lib]t1delete$(O) [.t1lib]t1enc$(O) \
	[.t1lib]t1env$(O) [.t1lib]t1finfo$(O) [.t1lib]t1load$(O) \
	[.t1lib]t1set$(O) [.t1lib]t1trans$(O) [.type1]arith$(O) \
	[.type1]curves$(O) [.type1]fontfcn$(O) [.type1]hints$(O) \
	[.type1]lines$(O) [.type1]objects$(O) [.type1]paths$(O) \
	[.type1]regions$(O) [.type1]scanfont$(O) [.type1]spaces$(O) \
	[.type1]t1io$(O) [.type1]t1snap$(O) [.type1]t1stub$(O) \
	[.type1]token$(O) [.type1]type1$(O) [.type1]util$(O)

all : msg $(LIB)($(OBJS))
	@ !

msg :
        @ $(ECHO) ""
        @ $(ECHO) "Making $(LIB) ..."
        @ $(ECHO) ""

[.t1lib]parseafm$(O)  : [.t1lib]parseafm.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) parseafm.c
	@ $(CD) [-]
[.t1lib]t1aaset$(O)   : [.t1lib]t1aaset.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1aaset.c
	@ $(CD) [-]
[.t1lib]t1afmtool$(O) : [.t1lib]t1afmtool.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1afmtool.c
	@ $(CD) [-]
[.t1lib]t1base$(O)    : [.t1lib]t1base.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1base.c
	@ $(CD) [-]
[.t1lib]t1delete$(O)  : [.t1lib]t1delete.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1delete.c
	@ $(CD) [-]
[.t1lib]t1enc$(O)     : [.t1lib]t1enc.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1enc.c
	@ $(CD) [-]
[.t1lib]t1env$(O)     : [.t1lib]t1env.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1env.c
	@ $(CD) [-]
[.t1lib]t1finfo$(O)   : [.t1lib]t1finfo.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1finfo.c
	@ $(CD) [-]
[.t1lib]t1load$(O)    : [.t1lib]t1load.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1load.c
	@ $(CD) [-]
[.t1lib]t1set$(O)     : [.t1lib]t1set.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1set.c
	@ $(CD) [-]
[.t1lib]t1trans$(O)   : [.t1lib]t1trans.c
	@ $(CD) [.T1LIB]
	@ $(CC)$(CFLAGS) t1trans.c
	@ $(CD) [-]
[.type1]arith$(O)     : [.type1]arith.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) arith.c
	@ $(CD) [-]
[.type1]curves$(O)    : [.type1]curves.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) curves.c
	@ $(CD) [-]
[.type1]fontfcn$(O)   : [.type1]fontfcn.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) fontfcn.c
	@ $(CD) [-]
[.type1]hints$(O)     : [.type1]hints.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) hints.c
	@ $(CD) [-]
[.type1]lines$(O)     : [.type1]lines.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) lines.c
	@ $(CD) [-]
[.type1]objects$(O)   : [.type1]objects.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) objects.c
	@ $(CD) [-]
[.type1]paths$(O)     : [.type1]paths.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) paths.c
	@ $(CD) [-]
[.type1]regions$(O)   : [.type1]regions.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) regions.c
	@ $(CD) [-]
[.type1]scanfont$(O)  : [.type1]scanfont.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) scanfont.c
	@ $(CD) [-]
[.type1]spaces$(O)    : [.type1]spaces.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) spaces.c
	@ $(CD) [-]
[.type1]t1io$(O)      : [.type1]t1io.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) t1io.c
	@ $(CD) [-]
[.type1]t1snap$(O)    : [.type1]t1snap.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) t1snap.c
	@ $(CD) [-]
[.type1]t1stub$(O)    : [.type1]t1stub.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) t1stub.c
	@ $(CD) [-]
[.type1]token$(O)     : [.type1]token.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) token.c
	@ $(CD) [-]
[.type1]type1$(O)     : [.type1]type1.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) type1.c
	@ $(CD) [-]
[.type1]util$(O)      : [.type1]util.c
	@ $(CD) [.TYPE1]
	@ $(CC)$(CFLAGS) util.c
	@ $(CD) [-]

install : $(LIB)

tests : dummy

links : dummy

clean :
        IF F$SEARCH("*$(O)",).NES."" THEN $(RM) *$(O);*
        IF F$SEARCH("$(LIB)",).NES."" THEN $(RM) $(LIB);*

distclean : clean
	@ !

devclean : clean
	@ !

dummy :
