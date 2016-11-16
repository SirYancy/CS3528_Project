CXX = g++
CC = g++
FLAGS = -std=c++11

client_test: client_test

Client.o: Client.h Client.cpp
	$(CXX) $(FLAGS) -c Client.cpp

Package.o: Package.h Package.cpp
	$(CXX) $(FLAGS) -c Package.cpp

client: Client.o Package.o
	$(CXX) $(FLAGS) client_test.cpp Client.o Package.o

clobber:
	rm -f *.0 *.exe core package
