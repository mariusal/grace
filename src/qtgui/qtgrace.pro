TEMPLATE = lib
CONFIG -= debug_and_release debug release
CONFIG += debug
CONFIG += staticlib
include(../../Make.conf)
CPPFLAGS ~= s/-I//
INCLUDEPATH += . ../ ../../include $$CPPFLAGS

# Input
FORMS += mainwindow.ui \
         fileselectiondialog.ui

HEADERS += qtgrace.h \
           mainwindow.h \
           canvaswidget.h \
           qtinc.h \
           fileselectiondialog.h 

SOURCES += qtgrace.cpp \
           qtutil.cpp \
           mainwindow.cpp \
           canvaswidget.cpp \
           qtdrv.cpp \
           fileselectiondialog.cpp 

