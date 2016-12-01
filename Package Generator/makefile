CXX = g++
CC = gcc
FLAGS = -std=c++11 -O3

all: package_generator

Utils.o: ../Utils.h ../Utils.cpp
	$(CXX) $(FLAGS) -c ../Utils.cpp

package_generator: package_generator.cpp ../Utils.o
	$(CXX) $(FLAGS) package_generator.cpp ../Utils.o -o package_generator

clean:
	rm -f *.o *.exe package_generator
