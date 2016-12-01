CXX = g++
CC = g++
FLAGS = -std=c++11 -g

all: main

Utils.o: Utils.h Utils.cpp
	$(CXX) $(FLAGS) -c Utils.cpp

Truck.o: Truck.h Truck.cpp
	$(CXX) $(FLAGS) -c Truck.cpp

Client.o: Client.h Client.cpp
	$(CXX) $(FLAGS) -c Client.cpp

Package.o: Package.h Package.cpp
	$(CXX) $(FLAGS) -c Package.cpp

Warehouse.o: Warehouse.h Warehouse.cpp
	$(CXX) $(FLAGS) -c Warehouse.cpp

Greedy.o: Greedy.h Greedy.cpp
	$(CXX) $(FLAGS) -c Greedy.cpp

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

pack_test: packageTest.cpp Client.o Package.o Utils.o
	$(CXX) $(FLAGS) packageTest.cpp Client.o Package.o Utils.o -o pack_test

warehouse: warehouse_test.cpp Client.o Package.o Truck.o Warehouse.o Utils.o
	$(CXX) $(FLAGS) warehouse_test.cpp Client.o Package.o Truck.o Warehouse.o Utils.o -o warehouse

main: main.cpp Client.o Package.o Truck.o Warehouse.o Greedy.o Utils.o
	$(CXX) $(FLAGS) main.cpp Client.o Package.o Truck.o Warehouse.o Greedy.o Utils.o -o main.o

clean:
	rm -f *.o *.exe core package client_package_test truck_test address client_test greedy pack_test warehouse
