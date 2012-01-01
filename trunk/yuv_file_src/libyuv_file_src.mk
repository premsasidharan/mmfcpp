DEPEND = dependencies

LIBDIR = lib
INCDIR = -I../media_core/include -Iinclude -Isource -Iobj
SRCDIR = source
OBJDIR = obj

CC = g++
AR = ar

LIBS = -L../media_core/lib -lmedia_core

CFLAGS = -ffloat-store -Wall $(INCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64 $(GLOBALCOMPFLAGS)

LIB_YUV_SRC = libyuv_src.a

#ifdef DBG
CFLAGS+= -g
#else
#CFLAGS += -O2
#endif

SRC = source/yuv_file_src.cpp
 
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o) 
LIB = $(LIBDIR)/$(LIB_YUV_SRC)

#dependencies:
#	@echo "" >dependencies

yuv_src: $(OBJ)
	@echo
	@mkdir -p $(LIBDIR)
	@echo 'creating library "$(LIB)"'
	@$(AR) rcs $(LIB) $(OBJ)
	@echo '... done'
	@echo

yuv_src_clean:
	@echo remove all objects
	rm -f $(OBJDIR)/*
	rm $(LIB)

yuv_src_tags:
	@echo update tag table
	@ctags include/*.h src/*.cpp

yuv_src_depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(INCDIR) $(SRC)                  \
        | sed '\''s@\(.*\)\.o[ :]@$(OBJDIR)/\1.o:@g'\''               \
        >$(DEPEND)'
	@echo

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	@echo 'compiling object file "$@" ...'
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $(CFLAGS) $(LIBS) $<

TARGETS += yuv_src

