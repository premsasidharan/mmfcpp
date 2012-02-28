DEPEND = dependencies

BINDIR = bin
INCDIR = -I../../media_core/include \
		 -Iinclude \
		 -Isource \
		 -Iobj

SRCDIR = source
BIN_NAME = buff_test
OBJDIR = $(BIN_NAME).obj
BIN = $(BINDIR)/$(BIN_NAME)

CC = g++

LIBS = -lpthread -lm -lsupc++ -L../../media_core/lib -lmedia_core

CFLAGS = -g -ffloat-store -Wall $(INCDIR) -D__USE_LARGEFILE64 -D_FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)

SRC=source/main.cpp

OBJ = $(foreach file,$(SRC),$(OBJDIR)/$(strip $(basename $(notdir $(file)))).o)

$(OBJDIR)/%.o:source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

all: media_core default

#dependencies:
#	@echo "" >dependencies

default: $(OBJ)
	@echo
	@mkdir -p $(BINDIR)
	@echo 'creating "$(BIN)"'
	@$(CC) -o $(BIN) $(OBJ) $(LIBS)
	@echo '... done'
	@echo

media_core:
	@make -C ../../media_core -f libmedia_core.mk

clean:
	@echo remove all objects
	@make -C ../../media_core -f libmedia_core.mk clean
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

