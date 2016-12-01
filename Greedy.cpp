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
    makeDirections();
}

vector<string> Greedy::getDirections() const
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
    order = vector<int>();
    int numStops = matrix.size();
    int timeLimit = 8 * 60;
    vector<bool> visited(numStops, false);

    order.push_back(0);
    unsigned int start = 0;
    visited[start] = true;
    int numVisited = 0;

    distance = 0;
    time = 0;
    int thisDistance = 0;

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
                thisDistance = shortest;
            }
        }
        order.push_back(next);
        visited[next] = true;
        start = next;
        numVisited++;
        time += thisDistance + 5;
        distance += thisDistance;
        if(time + matrix[0][start] >= timeLimit)
            break;
    }
    thisDistance = matrix[0][start];
    time += thisDistance + 5;
    distance += thisDistance;
}

void Greedy::makeDirections()
{
    string originAddress = origin->getAddress();
    for (int i = 0; i < order.size(); i++)
    {
        Client* c = stops[order[i]];
        string address = c->getName() + "\n" + c->getAddress() + "\n";
        directions.push_back(address);
    }
}

