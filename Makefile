include conanbuildinfo.mak

TARGETS := main libjardin
OBJ := $(addsuffix .o, $(TARGETS) )
EXE_FILENAME = jardin

#--------------------------------------------------------
CC      := gcc
BUILD   := build
CFLAGS  += $(CONAN_CFLAGS) $(addprefix -I, $(CONAN_INCLUDE_DIRS))
LDFLAGS += $(addprefix -L , $(CONAN_LIB_DIRS))
LDLIBS  += $(addprefix -l , $(CONAN_LIBS))

#--------------------------------------------------------

COMPILE_C_COMMAND  ?= \
    $(CC) -c $(CFLAGS) $< -o $@

CREATE_EXE_COMMAND ?= \
    $(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(BUILD)/$(EXE_FILENAME)


CREATE_EXE_DEBUG   ?= \
    $(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(BUILD)/$(EXE_FILENAME) -Wall -ggdb3

#--------------------------------------------------------
# Make rules
#--------------------------------------------------------

$(EXE_FILENAME) : $(OBJ)
	$(CREATE_EXE_COMMAND)

$(OBS) : %.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)

schema-test: main.c  libjardin.c#bin/jardin.out 
	$(CREATE_EXE_DEBUG)
	./bin/jardin $(d) tests/uq.schema.json #> /dev/null
	./bin/jardin -d tests/uq.schema.json

schema-debug: main.c  libjardin.c
	$(CREATE_EXE_DEBUG)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes ./bin/jardin $(d) tests/uq.schema.json
	$(BUILD)/$(EXE_FILENAME) -d tests/uq.schema.json

object-test: main.c  libjardin.c
	$(CREATE_EXE_DEBUG)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes ./bin/jardin $(d) -j tests/object.json
	$(BUILD)/$(EXE_FILENAME) -j tests/object.json
	$(BUILD)/$(EXE_FILENAME) -d -j tests/object.json

array-test: main.c  libjardin.c
	$(CREATE_EXE_DEBUG)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes -v ./bin/jardin $(d) -a tests/array.json
	$(BUILD)/$(EXE_FILENAME) -a tests/array.json
	$(BUILD)/$(EXE_FILENAME) -d -a tests/array.json

