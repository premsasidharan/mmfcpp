DEPEND = dependencies

BINDIR = bin
INCDIR = -I../media_core/include -I../wave_file_src/include -I../audio_renderer/include -Iinclude -Isource -Iobj
SRCDIR = source
BIN_NAME = audio_test
OBJDIR = $(BIN_NAME).obj
BIN = $(BINDIR)/$(BIN_NAME)

CC = g++

LIBS = -lpthread -lm -lsupc++ -L../media_core/lib -lmedia_core -lrt -lasound

CFLAGS = -ffloat-store -Wall $(INCDIR) -D__USE_LARGEFILE64 -D_FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)

SRC=../wave_file_src/source/wave_file_src.cpp \
	../wave_file_src/source/wave_file.cpp \
	../audio_renderer/source/audio_renderer.cpp \
	source/wave_test.cpp

OBJ = $(foreach file,$(SRC),$(OBJDIR)/$(strip $(basename $(notdir $(file)))).o)

$(OBJDIR)/%.o:../wave_file_src/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o:../audio_renderer/source/%.cpp
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

