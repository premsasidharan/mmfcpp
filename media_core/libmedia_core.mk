DEPEND = dependencies

LIBDIR = lib
INCDIR = -Iinclude \
		 -Isource \
		 -Iobj
SRCDIR = source
OBJDIR = obj

CC = g++
AR = ar

LIBS = -lpthread -lm -lsupc++

CFLAGS = -ffloat-store -Wall $(INCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)

LIB_MEDIA_CORE = libmedia_core.a

SRC = source/abstract_media_object.cpp \
	  source/media_logger.cpp \
	  source/media_tracer.cpp \
	  source/buffer.cpp
 
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o) 
LIB = $(LIBDIR)/$(LIB_MEDIA_CORE)

#default: depend $(LIB_MEDIA_CORE) tags
#default: $(LIB_MEDIA_CORE)

all: default

#dependencies:
#	@echo "" >dependencies

default: $(OBJ)
	@echo
	@mkdir -p $(LIBDIR)
	@echo 'creating library "$(LIB)"'
	@$(AR) rcs $(LIB) $(OBJ)
	@echo '... done'
	@echo

clean:
	@echo remove all objects
	rm -f $(OBJDIR)/*
	rm $(LIB)

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

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $<


