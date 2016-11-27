CXX = g++
CC = g++
FLAGS = -std=c++11

client_package_test: client_package

Utils.o: Utils.h Utils.cpp
	$(CXX) $(FLAGS) -c Utils.cpp

Truck.o: Truck.h Truck.cpp
	$(CXX) $(FLAGS) -c Truck.cpp

Client.o: Client.h Client.cpp
	$(CXX) $(FLAGS) -c Client.cpp

Package.o: Package.h Package.cpp
	$(CXX) $(FLAGS) -c Package.cpp

client_package: client_test.cpp Client.o Package.o Utils.o
	$(CXX) $(FLAGS) client_test.cpp Client.o Package.o Utils.o -o client_package_test

truck: truck_test.cpp Truck.o Client.o Package.o Utils.o
	$(CXX) $(FLAGS) truck_test.cpp Client.o Package.o Truck.o Utils.o -o truck_test

addresses: address.cpp
	$(CXX) $(FLAGS) address.cpp -o address

client_test2: client_test2.cpp Client.o Package.o Utils.o
	$(CXX) $(FLAGS) client_test2.cpp Client.o Package.o Utils.o -o client_test

greedy: greedy.cpp Client.o Package.o Truck.o Utils.o
	$(CXX) $(FLAGS) greedy.cpp Client.o Package.o Truck.o Utils.o -o greedy
clean:
	rm -f *.o *.exe core package client_package_test truck_test address client_test greedy
