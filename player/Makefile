all: default

export GLOBALCOMPFLAGS = -g -Wall 
#-DMEDIA_TRACE_CONSOLE

media_core:
	@make -C ../media_core -f libmedia_core.mk

yuv_test:
	@make -f yuv_test.mk

audio_test:
	@make -f audio_test.mk

av_test:
	@make -f av_test.mk
    
default: media_core yuv_test audio_test av_test

clean:
	@echo remove all objects
	@make -C ../media_core -f libmedia_core.mk clean
	@make -f yuv_test.mk clean
	@make -f audio_test.mk clean
	@make -f av_test.mk clean

