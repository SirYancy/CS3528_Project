#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include "Client.h"
#include "Package.h"
#include "Truck.h"

using namespace std;

Warehouse::Warehouse(){}

//! setClients()
/*!
 * Replaces entire contact list. Use with caution!  Really only for testing.
 \param c unordered_map of clients
 */
void Warehouse::setClients(const unordered_map<string, Client*> c)
{
    clients = c;
}

//! addClient()
/*!
 * Adds a client to the client map. Checks if it already exists first.
 \param c a reference to a client
 */
void Warehouse::addClient(const Client &c)
{
    if(clients.find(c.getName()) == clients.end())
    {
        clients.emplace(c.getName(), c);
    }
}

//! getClients()
/*!
 * Returns the entire map of clients
 \return unordered_map of clients
 */
unordered_map<string, Client*> Warehouse::getClients() const
{
    return clients;
}

//! addPackages()
/*!
 * The preferred way to add any packages is to pass a vector containing pointers to the packages to this funciton
 \param vector<Package*> a vector containing new packages
 */
void Warehouse::addPackages(vector<Package*> packs)
{
    for(Package* p : packs)
    {
        undeliveredPackages.push_back(p);
    }
}

//! getUndelivered()
/*!
 * Returns a vector of all undelivered packages
 \return vector<Package*> undelivered packages
 */
vector<Package*> Warehouse::getUndelivered() const
{
    return undeliveredPackages;
}

//! getDelivered()
/*!
 * Returns a record of all packages delivered by the service. This vector would obviously get quite large over time, which why this should really be done with a database.
 \return vector<Package*> delivered packages
 */
vector<Package*> Warehouse::getDelivered() const
{
    return deliveredPackages;
}

//! getLoaded()
/*!
 * Returns a vector of all of the packages that are currently loaded in trucks. Any package here should not also be in undeliveredPackages.
 \return vector<Package*> loaded packages. Presumably out for delivery
 */
vector<Package*> Warehouse::getLoaded() const
{
    return loadedPackages;
}

//! getTrucks()
/*!
 * Returns a vector of all of the trucks currently in the fleet.
 \return vector<Truck*> a vector of pointers to trucks
 */
vector<Truck*> Warehouse::getTrucks() const
{
    return trucks;
}

//! makeTruck()
/*!
 * Creates a new truck with the passed weight limit and then passes a reference to the truck back to whatever called this.
 \param int weight limit of truck
 \return a reference to the truck
 */
Truck& makeTruck(const double w)
{
    trucks.push_back(new Truck(w));
}

//! loadTrucks()
/*!
 * Uses some sort of algorithm to decide which packages go into each truck. It must divide up the map and then prioritize packages to be loaded and then add them to the trucks in the fleet. This is currently unimplemented
 */
void loadTrucks()
{
    //TODO - Implement this business
}

//! dispatchTrucks()
/*!
 * In essence, this method should force a day of deliveries to go by. It should empty out all trucks, deliver all packages, increment age of all packages still in the warehouse after the trucks leave, and set everything up for the next day.
 */
void dispatchTrucks()
{
    //TODO - Implement this business
}
