DEPEND = dependencies

BINDIR = bin
INCDIR = -I./../../clock/include/ -I./../../media_core/include/ -Iinclude -Isource -Iobj
SRCDIR = source
BIN_NAME = clock_test
OBJDIR = $(BIN_NAME).obj
BIN = $(BINDIR)/$(BIN_NAME)

CC = g++

LIBS = -lpthread -lm -lsupc++ -L../../media_core/lib -lmedia_core -lrt

CFLAGS = -g -ffloat-store $(INCDIR) -D__USE_LARGEFILE64 -D_FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)
# -g -Wall -DMEDIA_TRACE_FILE

SRC=./../../clock/source/abstract_clock.cpp \
	./../../clock/source/child_clock.cpp \
	./../../clock/source/master_clock.cpp \
	source/clock_test.cpp

OBJ = $(foreach file,$(SRC),$(OBJDIR)/$(strip $(basename $(notdir $(file)))).o)

$(OBJDIR)/%.o:../../clock/source/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<

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
 
