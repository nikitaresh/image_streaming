
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = viewer
TEMPLATE = app
INCLUDEPATH += .\..\..\include \
               .\..\..\thirdparty\snappy \
               .\..\..\thirdparty\snappy\build

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        viewer.cpp \
        viewer_client.cpp \
        .\..\..\thirdparty\snappy\snappy-c.cc \
        .\..\..\thirdparty\snappy\snappy-sinksource.cc \
        .\..\..\thirdparty\snappy\snappy-stubs-internal.cc \
        .\..\..\thirdparty\snappy\snappy.cc

HEADERS += \
        viewer.h \
        viewer_client.h

FORMS += \
        viewer.ui

CONFIG += mobility
MOBILITY = 
