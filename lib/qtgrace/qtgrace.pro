TEMPLATE = lib
CONFIG -= moc
CONFIG += staticlib
INCLUDEPATH += . ../../src/

# Input
FORMS += mainwindow.ui

HEADERS += qtgrace.h \
	   mainwindow.h

SOURCES += qtgrace.cpp \
	   mainwindow.cpp

