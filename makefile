CXX = g++
CC = gcc
FLAGS = -std=c++11 -O3 -pthread -lpthread -ldl

all: package_delivery

sqlite3.o: sqlite3.h sqlite3.c
        $(CC) -c sqlite3.c

Utils.o: Utils.h Utils.cpp
	$(CXX) $(FLAGS) -c Utils.cpp

Client.o: Client.h Client.cpp
        $(CXX) $(FLAGS) -c Client.cpp

Package.o: Package.h Package.cpp
	$(CXX) $(FLAGS) -c Package.cpp

Genetic.o: Genetic.cpp Genetic.h
	$(CXX) $(FLAGS) -c Genetic.cpp

package_delivery: client_test.cpp Client.o Package.o Utils.o Genetic.o
	$(CXX) $(FLAGS) client_test.cpp Client.o Package.o Utils.o Genetic.o -o package_delivery

clean:
	rm -f *.o *.exe package_delivery
