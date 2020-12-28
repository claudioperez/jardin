include conanbuildinfo.mak

SHELL = /bin/sh

BINDIR   := bin

TARGETS := main libjardin
OBJ := $(addsuffix .o, $(TARGETS) )
EXE_FILENAME = jardin

#--------------------------------------------------------
CC      := gcc
CFLAGS  += $(CONAN_CFLAGS) $(addprefix -I , $(CONAN_INCLUDE_DIRS))
LDFLAGS += $(addprefix -L , $(CONAN_LIB_DIRS))
LDLIBS  += $(addprefix -l , $(CONAN_LIBS))
VALGND  := valgrind \
           --tool=memcheck \
           --leak-check=full \
           --show-leak-kinds=all \
           --show-reachable=yes 
           --num-callers=20 \
           --track-origins=yes \
           --track-fds=yes -v

#--------------------------------------------------------

COMPILE_C_COMMAND  ?= \
    $(CC) -c $(CFLAGS) $< -o $@

CREATE_EXE_COMMAND ?= \
    $(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(BINDIR)/$(EXE_FILENAME)


CREATE_EXE_DEBUG   ?= \
    $(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(BINDIR)/$(EXE_FILENAME) -Wall -ggdb3

#--------------------------------------------------------
# Make rules
#--------------------------------------------------------

$(EXE_FILENAME) : $(OBJ)
	$(CREATE_EXE_COMMAND)

$(OBS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@  

schema-test: main.o  libjardin.o  
	$(CREATE_EXE_DEBUG)
	./$(BINDIR)/jardin tests/uq.schema.json #> /dev/null
	./$(BINDIR)/jardin -d tests/uq.schema.json

schema-debug: main.o  libjardin.o
	$(CREATE_EXE_DEBUG)
	$(VALGND) ./bin/jardin tests/uq.schema.json
	$(BINDIR)/$(EXE_FILENAME) -d tests/uq.schema.json

object-test: main.c  libjardin.c tests/object.json
	$(CREATE_EXE_DEBUG)
	$(VALGND) ./$(BINDIR)/$(EXE_FILENAME) -j tests/object.json
	$(BINDIR)/$(EXE_FILENAME) -j tests/object.json
	$(BINDIR)/$(EXE_FILENAME) -d -j tests/object.json

array-test: main.c  libjardin.c
	$(CREATE_EXE_DEBUG)
	$(VALGND) ./bin/jardin -a tests/array.json
	$(BINDIR)/$(EXE_FILENAME) -a tests/array.json
	$(BINDIR)/$(EXE_FILENAME) -d -a tests/array.json

