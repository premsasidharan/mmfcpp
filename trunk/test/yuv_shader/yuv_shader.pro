TEMPLATE = app
TARGET = yuv_shader
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj
QT += opengl
DEPENDPATH += .
INCLUDEPATH += ./include ./obj

HEADERS = include/gl_widget.h \
		  include/yuv_window.h

SOURCES = source/gl_widget.cpp \
		  source/yuv_window.cpp \
          source/main.cpp

CONFIG += qt debug
