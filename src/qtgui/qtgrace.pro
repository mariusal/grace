TEMPLATE = lib
CONFIG -= debug_and_release debug release
CONFIG += release
CONFIG += staticlib
INCLUDEPATH += . ../ ../../include

# Input
FORMS += mainwindow.ui

HEADERS += mainwindow.h \
	   canvaswidget.h \
	   qtinc.h

SOURCES += qtgrace.cpp \
	   mainwindow.cpp \
	   canvaswidget.cpp \
	   qtdrv.cpp

