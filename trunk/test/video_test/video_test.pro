TEMPLATE = app
TARGET = video_test
DESTDIR = bin
MOC_DIR = obj
OBJECTS_DIR = obj
QT += opengl
DEPENDPATH += .
INCLUDEPATH += ../../media_core/include \
			   ../../clock/include \
			   ../../video_widget/include \
			   ../../yuv_file/include \
			   ../../yuv_file_src/include \
			   ../../video_renderer/include \
			   ./include ./obj
LIBS += -L/usr/lib -lpthread -lm -lsupc++ -lrt -L../../media_core/lib -lmedia_core

HEADERS = ../../video_widget/include/video_widget.h

SOURCES = ../../video_widget/source/video_widget.cpp \
		  ../../video_widget/source/video_widget_shader.cpp \
	      ../../yuv_file/source/yuv_file.cpp \
	      ../../yuv_file/source/read_yuv_file.cpp \
	      ../../yuv_file_src/source/yuv_file_src.cpp \
		  ../../video_renderer/source/video_renderer.cpp \
		  ../../clock/source/abstract_clock.cpp \
		  ../../clock/source/child_clock.cpp \
		  ../../clock/source/master_clock.cpp \
          source/main.cpp

CONFIG += qt debug

media_core.commands = make -C ../../media_core -f libmedia_core.mk
QMAKE_EXTRA_TARGETS += media_core
PRE_TARGETDEPS += media_core

QMAKE_CLEAN += -r ../../media_core/lib -r ../../media_core/obj

