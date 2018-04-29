
QT += core network
QT -= gui

CONFIG += c++11

TARGET = server
CONFIG += console
CONFIG -= app_bundle
INCLUDEPATH += .\..\..\include
INCLUDEPATH += .\..\isimage

TEMPLATE = app

HEADERS += server.h \
    client_hendler.h \
    server_storage.h \
    .\..\isimage\isimage.h

SOURCES += main.cpp \
           server.cpp \
    client_hendler.cpp \
    server_storage.cpp \
    .\..\isimage\isimage.cpp

DEFINES += QT_DEPRECATED_WARNINGS
