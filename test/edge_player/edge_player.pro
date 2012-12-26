TEMPLATE = app
TARGET = edge_player
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj
LIBS += -L/usr/X11R6/lib/ -lGL -lGLEW
QT += opengl
DEPENDPATH += .
INCLUDEPATH += ./include ../../camera/include ./obj

HEADERS = include/gl_widget.h \
          include/offline_widget.h \
          include/gl_thread.h

SOURCES = source/gl_widget.cpp \
          source/offline_widget.cpp \
          source/gl_thread.cpp \
          source/main.cpp

CONFIG += qt debug
