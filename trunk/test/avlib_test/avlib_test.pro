#-------------------------------------------------
#
# Project created by QtCreator 2011-01-01T16:46:16
#
#-------------------------------------------------

QT += core gui opengl

CONFIG += debug

TEMPLATE = app
TARGET = avlib_test
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj

DEFINES += __STDC_CONSTANT_MACROS

INCLUDEPATH += /usr/include/ \
			   include \
			   source \
			   obj \
         	   ../../media_core/include \
               ../../av_file_src/include \
               ../../avcodec_audio_decoder/include \
               ../../avcodec_video_decoder/include \
               ../../video_widget/include \
               ../../video_renderer/include \
               ../../audio_renderer/include

LIBS += -L/usr/lib -lpthread -lm -lsupc++ -lrt -lasound -lX11 -lXv -lavformat -lavcodec -lswscale \
       -L../../media_core/lib -lmedia_core

SOURCES += ../../av_file_src/source/av_file_src.cpp \
		   ../../avcodec_audio_decoder/source/avcodec_audio_decoder.cpp \
	       ../../avcodec_video_decoder/source/avcodec_video_decoder.cpp \
	       ../../audio_renderer/source/audio_renderer.cpp \
	       ../../video_widget/source/video_widget.cpp \
	       ../../video_renderer/source/video_renderer.cpp \
	       source/avlib_test.cpp

HEADERS  += ../../video_widget/include/video_widget.h

FORMS    +=

media_core.commands = make -C ../../media_core -f libmedia_core.mk
QMAKE_EXTRA_TARGETS += media_core
PRE_TARGETDEPS += media_core

QMAKE_CLEAN += -r ../../media_core/lib -r ../../media_core/obj

