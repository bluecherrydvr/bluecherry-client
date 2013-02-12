#-------------------------------------------------
#
# Project created by QtCreator 2013-01-21T21:50:18
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle

TARGET = bluecherry-client-tests

TEMPLATE = app

INCLUDEPATH += ../src

SOURCES += main.cpp \
    ../src/utils/Range.cpp \
    ../src/utils/RangeMap.cpp \
    src/utils/RangeTestCase.cpp \
    src/utils/RangeMapTestCase.cpp

HEADERS += \
    src/utils/RangeTestCase.h \
    src/utils/RangeMapTestCase.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
