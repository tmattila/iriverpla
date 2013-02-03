#-------------------------------------------------
#
# Project created by QtCreator 2012-12-26T11:25:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iriverpla
TEMPLATE = app


SOURCES += main.cpp\
        iriverpla.cpp \
    plafile.cpp

HEADERS  += iriverpla.h \
    plafile.h

FORMS    += iriverpla.ui

QMAKE_CXXFLAGS += -std=c++11
