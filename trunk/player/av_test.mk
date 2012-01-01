DEPEND = dependencies

BINDIR = bin
INCDIR = -Iinclude -Isource -Iobj \
         -I../media_core/include \
         -I../av_file_src/include \
         -I../avcodec_audio_decoder/include \
         -I../avcodec_video_decoder/include \
         -I../xv_renderer/include \
         -I../audio_renderer/include
         
SRCDIR = source
BIN_NAME = av_test
OBJDIR = $(BIN_NAME).obj
BIN = $(BINDIR)/$(BIN_NAME)

CC = g++

LIBS = -L/usr/lib -lpthread -lm -lsupc++ -lrt -lasound -lX11 -lXv -lavformat -lavcodec -lswscale \
       -L../media_core/lib -lmedia_core

#CXXFLAGS=-D__STDC_CONSTANT_MACROS
CFLAGS = -ffloat-store -Wall $(INCDIR) -D__USE_LARGEFILE64 -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS $(GLOBALCOMPFLAGS)

SRC=../av_file_src/source/av_file_src.cpp \
	../avcodec_audio_decoder/source/avcodec_audio_decoder.cpp \
	../avcodec_video_decoder/source/avcodec_video_decoder.cpp \
	../audio_renderer/source/audio_renderer.cpp \
	../xv_renderer/source/x11_app.cpp \
	../xv_renderer/source/xv_video_frame.cpp \
	../xv_renderer/source/x11_window.cpp \
	../xv_renderer/source/xv_renderer.cpp \
	source/av_test.cpp

OBJ = $(foreach file,$(SRC),$(OBJDIR)/$(strip $(basename $(notdir $(file)))).o)

$(OBJDIR)/%.o:../av_file_src/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o:../avcodec_audio_decoder/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o:../avcodec_video_decoder/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o:../audio_renderer/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o:../xv_renderer/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o:source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

all: default

#dependencies:
#	@echo "" >dependencies

default: $(OBJ)
	@echo
	@mkdir -p $(BINDIR)
	@echo 'creating "$(BIN)"'
	@$(CC) -o $(BIN) $(OBJ) $(LIBS)
	@echo '... done'
	@echo

clean:
	@echo remove all objects
	rm -rf $(OBJDIR)
	rm $(BIN)

tags:
	@echo update tag table
	@ctags include/*.h src/*.cpp

depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(INCDIR) $(SRC)                  \
        | sed '\''s@\(.*\)\.o[ :]@$(OBJDIR)/\1.o:@g'\''               \
        >$(DEPEND)'
	@echo
