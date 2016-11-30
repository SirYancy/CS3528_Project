#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include "Client.h"
#include "Package.h"
#include "Truck.h"
#include "Warehouse.h"

using namespace std;

void sortPackages(vector<Package*>&);
bool compare(Package*, Package*);
static void getStops(vector<Client*>&, vector<Package*>&);
static void makeDirections(vector<int>&, vector<Client*>&, vector<string*>&);
static vector<vector<unsigned int> > makeMatrix(vector<Package*>&);
static vector<int> greedyRoute(vector<vector<unsigned int> >&);
static pair<int,int> getDistanceAndTime(vector<int>&, vector<vector<unsigned int> >&);



Warehouse::Warehouse()
{
    myAddress = new Client("WAREHOUSE, 123 Maint St. N, Bemidji, MN, 56601");
}

//! addClient()
/*!
 * Adds a client to the client map. Checks if it already exists first.
 \param c a reference to a client
 */
void Warehouse::addClient(Client* c)
{
    if(clients.find(c->getName()) == clients.end())
    {
        clients.insert(make_pair(c->getName(), c));
    }
}

//! getClients()
/*!
 * Returns the entire map of clients
 \return unordered_map<string, Client*> > map of clients
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
void Warehouse::addPackages(const vector<Package*> &packs)
{
    for(Package* p : packs)
    {
        undeliveredPackages.push_back(p);
    }
    sortPackages();
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
Truck& Warehouse::makeTruck(const double w)
{
    trucks.push_back(new Truck(w));
}

//! loadTrucks()
/*!
 * Uses some sort of algorithm to decide which packages go into each truck. It must divide up the map and then prioritize packages to be loaded and then add them to the trucks in the fleet. For now, it only loads one truck.
 * \return vector<string*> the order that packages must be delivered.
 */
vector<string*> Warehouse::loadTrucks()
{
    vector<Client*> stops;
    vector<string*> directions;
    vector<vector<unsigned int> > adjMat;           //adjacency matrixa
    vector<int> order;

    double weight = trucks[0]->getWeight();         //working weight limit
    auto packIter = undeliveredPackages.begin();    //iterator for vector of packages
    
    // Greedily load the truck with all of the highest priority packages.
    while(weight > 0)
    {
        double w = (*packIter)->getWeight();
        weight -= w;
        if(weight > 0)
        {
            loadedPackages.push_back(*packIter);
            packIter++;
            undeliveredPackages.erase(undeliveredPackages.begin());
        }
    }
    
    //adds the warehouse to the vector
    stops.push_back(myAddress);

    //Make the adjacency matrix
    adjMat = makeMatrix(loadedPackages);

    //Generate the Greedy Route
    order = greedyRoute(adjMat);

    //Add the warehouse to the end of the route.
    order.push_back(0);

    //Get distance and time as a pair of ints <blocks, minutes>
    pair<int,int> dt = getDistanceAndTime(order, adjMat);
    
    vector<Package*> toUnload;
    //Determine time of route and if it can be completed in one shift.
    while(dt.second > (8*60))
    {
        toUnload.push_back(loadedPackages[order[order.size()-2]]);
        order.erase(order.begin() + order.size()-2);
    }
    
    this->addPackages(toUnload);
    for(Package* p : toUnload)
    {
        loadedPackages.erase(remove(loadedPackages.begin(), loadedPackages.end(), p), loadedPackages.end());
    }

    stops.push_back(myAddress);
    getStops(stops, loadedPackages);
    stops.push_back(myAddress);

    makeDirections(order, stops, directions);
    for(string* d : directions)
    {
        cout << *d << endl;
    }
}

//! dispatchTrucks()
/*!
 * In essence, this method should force a day of deliveries to go by. It should empty out all trucks, deliver all packages, increment age of all packages still in the warehouse after the trucks leave, and set everything up for the next day.
 */
void Warehouse::dispatchTrucks()
{
    //TODO - Implement this business
}

/**************STATIC METHODS*****************/

static void getStops(vector<Client*> &stops, vector<Package*> &packages)
{
    cout << packages.size();
    for(Package* p : packages)
    {
        stops.push_back(p->getReceiver());
    }
}

static void makeDirections(vector<int> &order, vector<Client*> &stops, vector<string*> &directions)
{
    string direct;
    for(int i = 0; i < order.size(); i++)
    {
        direct = stops[order[i]]->getAddress();
        directions.push_back(&direct);
    }
}

//! makeMatrix()
/*! Generates an adjacency matrix for the vector of stops
 * \param &stops vecotr of pointers to clients
 * \return vector<vector<unsigned int> > the adjacency matrix
 */
static vector<vector<unsigned int> > makeMatrix(vector<Package*> &stops)
{
    pair<int, int> receiver1, receiver2;
    int numStops = stops.size();
    vector<vector<unsigned int> > matrix(numStops, vector<unsigned int>(numStops,0));

    for(int i = 0; i < numStops; i++)
    {
        pair<int,int> r = stops[i]->getReceiver()->getCoords();
        cout << "" << r.first << "," << r.second << "\n";
        for(int j = 0; j < numStops; j++)
        {
            receiver1 = stops[i]->getReceiver()->getCoords();
            receiver2 = stops[j]->getReceiver()->getCoords();
            matrix[i][j] = abs(receiver1.first - receiver2.first) + 
                abs(receiver1.second - receiver2.second);
        }
    }
    return matrix;
}

//! greedyRoute()
/*! A greedy algorithm which generates a route as a vector of indices for the sotps list
 * \param &matrix the adjacency matrix
 * \return vector<int> a list of indices for the client list
 */
static vector<int> greedyRoute(vector<vector<unsigned int> > &matrix)
{
    int numStops = matrix.size();
    vector<bool> visited(numStops, false);

    unsigned int start = 0;
    visited[start] = true;
    int numVisited = 0;
    
    vector<int> order;
    order.push_back(start);
    int distance = 0;

    while(numVisited < numStops)
    {
        unsigned int shortest = numeric_limits<unsigned int>::max();
        int next = 0;

        for (int i = 0; i < numStops; i++)
        {
            if(i != start && !visited[i] && matrix[start][i] < shortest)
            {
                shortest = matrix[start][i];
                next = i;
            }
        }
        order.push_back(next);
        visited[next] = true;
        start = next;
        numVisited++;
    }
    order.push_back(0);
    return order;
}

static pair<int,int> getDistanceAndTime(vector<int> &order, vector<vector<unsigned int> > &matrix)
{
    int numStops = order.size();
    int distance = 0;
    int time = 0;


    for(int i = 0; i < numStops-1; i++)
    {
        int nextLeg = matrix[order[i]][order[i+1]];
        distance += nextLeg;
        time += (nextLeg + 5);
    }
    
    return make_pair(distance,time);
}

