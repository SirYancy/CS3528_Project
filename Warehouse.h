#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Client.h"
#include "Package.h"
#include "Truck.h"
using namespace std;

class Client;
class Package;
class Truck;

//! Warehouse Class
/*!
 * This Warehouse class defines a warehouse that will keep track of the all clients, packages, and trucks. It will also generate the routes for all the trucks and load the trucks with packages.
 */
class Warehouse
{
    public:
        //! Basic constructor builds an empty warehouse
        Warehouse();

        //! Add client
        void addClient(Client*);
       
        //! Returns the map of all clients in the system
        unordered_map<string, Client*> getClients() const;
    
        //! Deliver Packages to warehouse
        void addPackages(const vector<Package*>&);
        
        //! Returns a vector of all currently undelivered packages
        vector<Package*> getUndelivered() const;

        //! Returns a vector of all packages previously delivered
        vector<Package*> getDelivered() const;

        //! Returns a vector of all packages that have been loaded onto a truck
        vector<Package*> getLoaded() const;

        //! Returns a vector of all trucks in the fleet
        vector<Truck*> getTrucks() const;
    
        //! Returns the client that represents this warehouse's location
        Client getMyAddress() const;

        //! Sets thie client's address location
        void setMyAddress(const Client&);

        //! Creates a new truck, adds it to the fleet, and then returns a pointer to that truck.
        Truck& makeTruck(const double);

        //! Load Trucks
        vector<string*> loadTrucks();
        
        //! Dispatch Trucks
        void dispatchTrucks();

    private:
        //! Map of clients
        unordered_map<string, Client*> clients;
        //! Vector of undelivered packages
        vector<Package*> undeliveredPackages;
        //! Vector of packages currently loaded on trucks
        vector<Package*> loadedPackages;
        //! Vector of packages that are delivered
        vector<Package*> deliveredPackages;
        //! Vector of trucks
        vector<Truck*> trucks;
        //! The client information for this warehouse
        Client* myAddress;
        
        //! Private method to sort packages
        void sortPackages()
        {
            std::sort(undeliveredPackages.begin(), undeliveredPackages.end(), comparePriority);
        }

        //! Private comparator
        static bool comparePriority(Package* left, Package* right)
        {
            return (*left > *right);
        }
};

#endif //WARHOUSE_H
