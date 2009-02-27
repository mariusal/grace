TEMPLATE = lib
CONFIG -= debug_and_release debug release
CONFIG += release
CONFIG += staticlib
INCLUDEPATH += . ../ ../../include
QMAKE_CLEAN = Make.common

depfile.target = Make.common
depfile.commands = echo \""QTOBJS = "$(foreach object, $(OBJECTS), qtgui/$(object))\" > $$depfile.target
depfile.depends = $(OBJECTS)

QMAKE_EXTRA_TARGETS += depfile

# Input
FORMS += mainwindow.ui

HEADERS += mainwindow.h \
	   canvaswidget.h

SOURCES += qtgrace.cpp \
	   mainwindow.cpp \
	   canvaswidget.cpp \
	   qtdrv.cpp

