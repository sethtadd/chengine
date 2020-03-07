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

# Windows
ifeq ($(OS),Windows_NT)
	CFLAGS = -std=c++14 -Ofast -Wall
	LIBS   = 
	CLEAN  = del $(EXE).exe *.o
endif
# Linux
ifeq ($(OS),Linux)
	CFLAGS = -std=c++14 -Ofast -Wall
	LIBS   = 
	CLEAN  = rm -v $(EXE) *.o
endif
# MacOS
ifeq ($(OS),Darwin)
	CFLAGS = -std=c++14 -Ofast -Wall
	LIBS   = 
	CLEAN  = rm -v $(EXE) *.o
endif

#
# rules
#

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: $(OBJ)
	$(CC) -o $(EXE) $^ $(CFLAGS) $(LIBS)

clean:
	$(CLEAN)
