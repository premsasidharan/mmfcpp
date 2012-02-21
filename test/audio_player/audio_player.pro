#-------------------------------------------------
#
# Project created by QtCreator 2011-01-01T16:46:16
#
#-------------------------------------------------

QT       += core gui

CONFIG += debug

TARGET = audio_player
TEMPLATE = app
DESTDIR = bin
MOC_DIR = obj
UI_DIR = obj
OBJECTS_DIR = obj

DEFINES += __STDC_CONSTANT_MACROS

INCLUDEPATH += /usr/include/ \
			   include \
			   source \
			   obj \
         	   ../../wave_file/include \
         	   ../../media_core/include \
               ../../wave_file_src/include \
               ../../audio_renderer/include

FORMS = forms/player_window.ui

LIBS += -L/usr/lib -lpthread -lm -lsupc++ -lrt -lasound \
       -L../../media_core/lib -lmedia_core

SOURCES += ../../wave_file_src/source/wave_file_src.cpp \
		   ../../wave_file/source/wave_file.cpp \
	       ../../wave_file/source/read_wave_file.cpp \
	       ../../audio_renderer/source/audio_renderer.cpp \
		   source/audio_player.cpp \
	       source/player_window.cpp \
	       source/main.cpp

HEADERS  += \
	include/audio_player.h \
    include/player_window.h

