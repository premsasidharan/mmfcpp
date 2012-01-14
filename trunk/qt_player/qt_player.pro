#-------------------------------------------------
#
# Project created by QtCreator 2011-01-01T16:46:16
#
#-------------------------------------------------

QT       += core gui

CONFIG += debug

TARGET = qt_test
TEMPLATE = app

DEFINES += __STDC_CONSTANT_MACROS

INCLUDEPATH += /usr/include/ \
			   include \
			   source \
			   obj \
         	   ../media_core/include \
               ../av_file_src/include \
               ../avcodec_audio_decoder/include \
               ../avcodec_video_decoder/include \
               ../qt_renderer/include \
               ../audio_renderer/include

LIBS += -L/usr/lib -lpthread -lm -lsupc++ -lrt -lasound -lX11 -lXv -lavformat -lavcodec -lswscale \
       -L../media_core/lib -lmedia_core

SOURCES += ../av_file_src/source/av_file_src.cpp \
		   ../avcodec_audio_decoder/source/avcodec_audio_decoder.cpp \
	       ../avcodec_video_decoder/source/avcodec_video_decoder.cpp \
	       ../audio_renderer/source/audio_renderer.cpp \
	       ../qt_renderer/source/xv_video_frame.cpp \
	       ../qt_renderer/source/yuv_window.cpp \
	       ../qt_renderer/source/xv_renderer.cpp \
		   source/player.cpp \
	       source/main.cpp

HEADERS  += \
    ../qt_renderer/include/xv_video_frame.h \
    ../qt_renderer/include/yuv_window.h \
	include/player.h

FORMS    +=
