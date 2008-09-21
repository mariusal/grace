TEMPLATE = lib
CONFIG -= moc
CONFIG += staticlib
INCLUDEPATH += . ../../src/ ../../include

# Input
FORMS += mainwindow.ui

HEADERS += qtgrace.h \
	   mainwindow.h

SOURCES += qtgrace.cpp \
	   mainwindow.cpp

