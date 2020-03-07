# Makefile for chengine

#
# compilation details
#

EXE  = chengine
CC   = g++
DEPS = StateTree.hpp
OBJ  = Main.o StateTree.o

#
# system specifics
#

CFLAGS = -std=c++14 -Ofast -Wall
LIBS   = 
CLEAN  = rm -v $(EXE) *.o

#
# rules
#

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: $(OBJ)
	$(CC) -o $(EXE) $^ $(CFLAGS) $(LIBS)

clean:
	$(CLEAN)
