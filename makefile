CXX = g++
CC = g++
FLAGS = -std=c++11

client_package_test: client_package

Utils.o: Utils.h Utils.cpp
	$(CXX) $(FLAGS) -c Utils.cpp

Client.o: Client.h Client.cpp
	$(CXX) $(FLAGS) -c Client.cpp

Package.o: Package.h Package.cpp
	$(CXX) $(FLAGS) -c Package.cpp

client_package: client_test.cpp Client.o Package.o Utils.o
	$(CXX) $(FLAGS) client_test.cpp Client.o Package.o Utils.o -o client_package_test

clean:
	rm -f *.o *.exe core package client_package_test
