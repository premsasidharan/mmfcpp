TEMPLATE = app
TARGET = tone_dumper
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj
QT += core gui

CONFIG += qt debug thread

DEPENDPATH += .
INCLUDEPATH += ./include ./obj

FORMS = tone_dlg.ui

LIBS += -lm

HEADERS = include/tone_dlg.h \
          include/tone_thread.h \
          include/complex.h

SOURCES = source/tone_dlg.cpp \
          source/tone_thread.cpp \
          source/complex.cpp \
          source/main.cpp
