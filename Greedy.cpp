#include <stdlib.h>
#include <functional>
#include <string>
#include <cmath>
#include <random>
#include <ctime>
#include <limits>
#include <vector>
#include "Greedy.h"
#include "Utils.h"
#include "Client.h"
#include "Truck.h"
#include "Client.h"
#include "Package.h"
#include "Truck.h"
#include "Warehouse.h"

using namespace std;

Greedy::Greedy(){}

void Greedy::setPackages(const vector<Package*> &p)
{
    packages = p;
}

vector<Package*> Greedy::getPackages() const
{
    return packages;
}

vector<Package*> Greedy::getUnload() const
{
    return unload;
}

void Greedy::setOrigin(Client* c)
{
    origin = c;
}

Client* Greedy::getOrigin() const
{
    return origin;
}

void Greedy::makeRoute()
{
    getStops();
    makeMatrix();
    greedyRoute();

    verifyTime();

    makeDirections();
}

vector<string*> Greedy::getDirections() const
{
    return directions;
}

int Greedy::getDistance() const
{
    return distance;
}

int Greedy::getTime() const
{
    return time;
}

void Greedy::getStops()
{
    stops.push_back(origin);  //Because of this, indices for 
    //stops and packages are different by 1

    for(Package* p: packages)
    {
        stops.push_back(p->getReceiver());
    }
}

void Greedy::makeMatrix()
{
    pair<int,int> receiver1, receiver2;
    int numStops = stops.size();
    matrix = vector<vector<unsigned int> >(numStops, vector<unsigned int>(numStops,0));

    for(int i = 0; i < numStops; i++)
    {
        pair<int,int> r = stops[i]->getCoords();
        for(int j = 0; j < numStops; j++)
        {
            receiver1 = stops[i]->getCoords();
            receiver2 = stops[j]->getCoords();
            matrix[i][j] = abs(receiver1.first - receiver2.first) + abs(receiver1.second - receiver2.second);
        }
    }
}

void Greedy::greedyRoute()
{
    int numStops = matrix.size();
    vector<bool> visited(numStops, false);

    unsigned int start = 0;
    visited[start] = true;
    int numVisited = 0;

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
}

void Greedy::getDistanceAndTime()
{
    int numStops = order.size();
    distance = 0;
    time = 0;

    for(int i = 0; i < numStops; i++)
    {
        int nextLeg = matrix[order[i]][order[i+1]];
        distance += nextLeg;
        time += (nextLeg + 5);
    }
}

void Greedy::makeDirections()
{
    string originAddress = origin->getAddress();
    directions.push_back(&originAddress);
    for(Client* c : stops)
    {
        string address = c->getAddress();
        directions.push_back(&address);
    }
    directions.push_back(&originAddress);
}

void Greedy:: verifyTime()
{
    getDistanceAndTime();

    while(time > (8*60))
    {
        int index = order.end()[-2];
        packages.erase(packages.begin() + index);
        stops.erase(stops.begin() + index-1);
        order.erase(order.begin() + order.size()-2);
        getDistanceAndTime();
    }
}
