
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = viewer
TEMPLATE = app
INCLUDEPATH += .\..\..\include

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        viewer.cpp \
        viewer_client.cpp

HEADERS += \
        viewer.h \
        viewer_client.h

FORMS += \
        viewer.ui

CONFIG += mobility
MOBILITY = 
