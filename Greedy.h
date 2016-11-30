#ifndef GREEDY_H
#define GREEDY_H

#include <stdlib.h>
#include <functional>
#include <string>
#include <cmath>
#include <random>
#include <ctime>
#include <limits>
#include <vector>
#include "Utils.h"
#include "Client.h"
#include "Truck.h"
#include "Client.h"
#include "Package.h"
#include "Truck.h"
#include "Warehouse.h"

class Package;
class Client;

using namespace std;

class Greedy
{
    public:
        Greedy();

        void setPackages(const vector<Package*>&);
        vector<Package*> getPackages() const;
        vector<Package*> getUnload() const;
        void setOrigin(Client*);
        Client* getOrigin() const;
        void makeRoute();
        vector<string*> getDirections() const;
        int getDistance() const;
        int getTime() const;


    private:
        //! The address of the main warehouse
        Client* origin;
        //! Packages loaded
        vector<Package*> packages;
        //! Packages that couldn't fit the time
        vector<Package*> unload;
        //! The client stops from the packages loaded
        vector<Client*> stops;
        //! Adjacency Matrix
        vector<vector<unsigned int> > matrix;
        //! The order of the indices in the packages
        vector<int> order;
        //! strings that represent the text directions for the truck
        vector<string*> directions;
        //! Distance of route
        int distance;
        //! Time of route
        int time;

        void getStops();
        void makeMatrix();
        void greedyRoute();
        void makeDirections();
        void getDistanceAndTime();
        void verifyTime();

};

#endif //GREEDY_H
