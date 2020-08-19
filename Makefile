
# RM=rm -f
# CPPFLAGS= -I /usr/local/jansson/include -I ./nanoflann/include -std=c++11 -I ./../..
# CXXFLAGS= -I /usr/local/jansson/include -I ./nanoflann/include
# LDFLAGS= -static-libgcc
# LDLIBS=/usr/local/jansson/lib/libjansson.a -lcurl


# HEADER = -I./cygdrive/c/cpros/kajj/source4
# LIBB = -L./cygdrive/c/cpros/kajj/source1   -L./cygdrive/c/cpros/kajj/source2
# LIBRA = -larith -ldekk

# target : game.o 
#     gcc $(HEADER)   $(LIBB)  $<  -o  $@  $(LIBRA)   

# game.o : game.c 
#     gcc -c  game.c
###################################################################################
CC      := gcc
# CCFLAGS := #-I ../include 
# LIBS  := libjansson.a

# TARGETS:= jardin
# MAINS  := $(addsuffix .o, $(TARGETS) )
# OBJ    := $(MAINS)
# # DEPS   := defs.h command.h
# # DEPS := libjansson.a
# DEPS :=

# .PHONY: all clean

# all: $(TARGETS)

# clean:
# 	rm -f $(TARGETS) $(OBJ)

# $(OBJ): %.o : %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CCFLAGS)

# $(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) %.o
# 	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)
###########################################################################################
# jardin: jardin.o
# 	$(CC) -o bin/jardin.out jardin.c libjansson.a

# test: jardin.c #bin/jardin.out 
# 	$(CC) -o bin/jardin jardin.c libjansson.a
# 	./bin/jardin $(d) tests/uq.schema.json #> /dev/null

# mem: jardin.c
# 	$(CC) -o bin/jardin jardin.c libjansson.a -Wall -ggdb3
# 	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes -v ./bin/jardin $(d) tests/uq.schema.json

# debug: bin/jardin.out 
# 	$(CC) -o bin/jardin jardin.c libjansson.a -Wall -ggdb3
# 	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes ./bin/jardin $(d) tests/uq.schema.json

jardin: main.c libjardin.c
	$(CC) -o bin/jardin main.c libjardin.c libjansson.a

schema-test: main.c  libjardin.c#bin/jardin.out 
	$(CC) -o bin/jardin main.c libjardin.c libjansson.a
	./bin/jardin $(d) tests/uq.schema.json #> /dev/null
	./bin/jardin -d tests/uq.schema.json

schema-debug: main.c  libjardin.c
	$(CC) -o bin/jardin main.c libjardin.c libjansson.a -Wall -ggdb3
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes ./bin/jardin $(d) tests/uq.schema.json
	./bin/jardin -d tests/uq.schema.json

object-test: main.c  libjardin.c
	$(CC) -o bin/jardin main.c libjardin.c libjansson.a -Wall -ggdb3
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes ./bin/jardin $(d) -j tests/object.json
	./bin/jardin  -j tests/object.json
	./bin/jardin -d -j tests/object.json

array-test: main.c  libjardin.c
	$(CC) -o bin/jardin main.c libjardin.c libjansson.a -Wall -ggdb3
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-origins=yes --track-fds=yes -v ./bin/jardin $(d) -a tests/array.json
	./bin/jardin  -a tests/array.json
	./bin/jardin -d -a tests/array.json

