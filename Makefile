
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

jardin: jardin.o
	$(CC) -o bin/jardin.out jardin.c libjansson.a

test: bin/jardin.out 
	$(CC) -o bin/jardin jardin.c libjansson.a
	./bin/jardin tests/uq.schema.json > /dev/null

