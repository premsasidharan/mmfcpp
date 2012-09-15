TEMPLATE = app
TARGET = yuv_player
DESTDIR = bin
MOC_DIR = obj
UI_DIR = obj
OBJECTS_DIR = obj
QT += opengl
DEPENDPATH += .
INCLUDEPATH += ../../media_core/include \
			   ../../clock/include \
			   ../../video_widget/include \
			   ../../yuv_file/include \
			   ../../yuv_file_src/include \
			   ../../video_renderer/include \
			   ./include \
			   ./obj \

LIBS += -L/usr/lib -lpthread -lm -lsupc++ -L../../media_core/lib -lmedia_core

HEADERS = include/yuv_player.h \
          include/yuv_dlg.h \
#          include/main_window.h \
          ../../video_widget/include/video_widget.h

FORMS = forms/player.ui \
        forms/yuv_dlg.ui \
        forms/main_window.ui

SOURCES = ../../video_widget/source/video_widget.cpp \
	      ../../yuv_file/source/yuv_file.cpp \
	      ../../yuv_file/source/read_yuv_file.cpp \
	      ../../yuv_file_src/source/yuv_file_src.cpp \
		  ../../video_renderer/source/video_renderer.cpp \
		  ../../clock/source/abstract_clock.cpp \
		  ../../clock/source/child_clock.cpp \
		  ../../clock/source/master_clock.cpp \
          source/yuv_player.cpp \
          source/yuv_dlg.cpp \
#          source/main_window.cpp \
          source/main.cpp

CONFIG += qt debug

media_core.commands = make -C ../../media_core -f libmedia_core.mk
QMAKE_EXTRA_TARGETS += media_core
PRE_TARGETDEPS += media_core

QMAKE_CLEAN += -r ../../media_core/lib -r ../../media_core/obj

