TEMPLATE = lib
CONFIG -= debug_and_release debug release
CONFIG += release
CONFIG += staticlib
INCLUDEPATH += . ../ ../../include

# Input
FORMS += mainwindow.ui \
         fileselectiondialog.ui

HEADERS += qtgrace.h \
           mainwindow.h \
           canvaswidget.h \
           qtinc.h \
           fileselectiondialog.h \
           filterlineedit.h

SOURCES += qtgrace.cpp \
           mainwindow.cpp \
           canvaswidget.cpp \
           qtdrv.cpp \
           fileselectiondialog.cpp \
           filterlineedit.cpp

