TEMPLATE = app
TARGET = edge_camera
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj
LIBS += -L/usr/X11R6/lib/ -lGL -lGLEW
QT += opengl
DEPENDPATH += .
INCLUDEPATH += ./include ../../camera/include ./obj

HEADERS = ../../camera/include/v4li_camera.h \
          include/gl_widget.h \
          include/gl_thread.h

SOURCES = ../../camera/source/v4li_camera.cpp \
          source/gl_widget.cpp \
          source/gl_thread.cpp \
          source/main.cpp

CONFIG += qt debug
