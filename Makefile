## This is a simple Makefile

# Define what compiler to use and the flags.
CC=cc
CXX=g++
CCFLAGS= -g -std=c++23 -Wall -Werror


all: proj

# Compile all .cpp files into .o files
# % matches all (like * in a command)
# $< is the source file (.cpp file)
%.o : %.cpp
	$(CXX) -c $(CCFLAGS) $<


proj: project.o
	$(CC) -o proj project.o -lm -lstdc++


clean:
	rm -f *.o project
