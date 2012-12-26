TEMPLATE = app
TARGET = edge_camera
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj
LIBS += -L/usr/X11R6/lib/ -lGL -lGLEW
QT += opengl
DEPENDPATH += .
INCLUDEPATH += ./include ../../camera/include ./obj

HEADERS = ../../camera/include/camera.h \
          include/gl_widget.h \
          include/offline_widget.h \
          include/gl_thread.h

SOURCES = ../../camera/source/camera.cpp \
          source/gl_widget.cpp \
          source/offline_widget.cpp \
          source/gl_thread.cpp \
          source/main.cpp

CONFIG += qt debug
