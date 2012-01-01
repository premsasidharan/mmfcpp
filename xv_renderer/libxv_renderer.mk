DEPEND = dependencies

LIBDIR = lib
INCDIR = -I../media_core/include -Iinclude -Isource -Iobj
SRCDIR = source
OBJDIR = obj

CC = g++
AR = ar

LIBS = -L../media_core/lib -lmedia_core

CFLAGS = -ffloat-store -Wall $(INCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)

LIB_XV_RENDERER = libxv_renderer.a

#ifdef DBG
CFLAGS+= -g
#else
#CFLAGS += -O2
#endif

SRC = source/xv_renderer.cpp
 
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o) 
LIB = $(LIBDIR)/$(LIB_XV_RENDERER)

#dependencies:
#	@echo "" >dependencies

xv_renderer: $(OBJ)
	@echo
	@mkdir -p $(LIBDIR)
	@echo 'creating library "$(LIB)"'
	@$(AR) rcs $(LIB) $(OBJ)
	@echo '... done'
	@echo

xv_renderer_clean:
	@echo remove all objects
	rm -f $(OBJDIR)/*
	rm $(LIB)

xv_renderer_tags:
	@echo update tag table
	@ctags include/*.h src/*.cpp

xv_renderer_depend:
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

TARGETS += xv_renderer

