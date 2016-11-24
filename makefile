CXX = g++
CC = g++
FLAGS = -std=c++11 -g -pg

all: package_delivery

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
