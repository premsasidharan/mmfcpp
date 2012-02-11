DEPEND = dependencies

BINDIR = bin
INCDIR = -I./../../wave_file/include/ -I./../../wave_file/include/ -Iinclude -Isource -Iobj
SRCDIR = source
BIN_NAME = wave_file_test
OBJDIR = $(BIN_NAME).obj
BIN = $(BINDIR)/$(BIN_NAME)

CC = g++

LIBS = -lpthread -lm -lsupc++ -lrt

CFLAGS = -g -ffloat-store $(INCDIR) -D__USE_LARGEFILE64 -D_FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)
# -g -Wall -DMEDIA_TRACE_FILE

SRC=./../../wave_file/source/wave_file.cpp \
	./../../wave_file/source/read_wave_file.cpp \
	./../../wave_file/source/write_wave_file.cpp \
	source/wave_file_test.cpp

OBJ = $(foreach file,$(SRC),$(OBJDIR)/$(strip $(basename $(notdir $(file)))).o)

$(OBJDIR)/%.o:../../wave_file/source/%.cpp
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

