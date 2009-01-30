TEMPLATE = lib
CONFIG -= moc
CONFIG += staticlib
INCLUDEPATH += . ../ ../../include

# Input
FORMS += mainwindow.ui

HEADERS += qtgrace.h \
	   mainwindow.h \
	   canvaswidget.h

SOURCES += qtgrace.cpp \
	   mainwindow.cpp \
	   canvaswidget.cpp \
	   qtdrv.cpp

