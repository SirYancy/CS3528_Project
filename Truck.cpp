#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "Truck.h"
#include "Package.h"
#include "Client.h"
#include "Utils.h"

using namespace std;

//! Initialize truck count
int Truck::truckCount = 0;

Truck::Truck(const double w)
    :weight(w), truckID(++truckCount)
{}

int Truck::getTruckID() const
{
    return truckID;
}

double Truck::getWeight() const
{
    return weight;
}

vector<Package*> Truck::getPackages() const
{
    return packages;
}

string Truck::getDirections() const
{
    return directions;
}

void Truck::processPackages() {
    // Start with blank string
    directions = "";

    // Loop through route adding client receiver information to string.
    for (auto iter = packages.begin(); iter != packages.end(); ++iter) {
        directions += (*iter)->getReceiver()->getName() + "\r\n";
        directions += (*iter)->getReceiver()->getAddress() + "\r\n";
        directions += (*iter)->getReceiver()->getCity() + ", " + (*iter)->getReceiver()->getState() + " " + (*iter)->getReceiver()->getZip() + "\r\n\r\n";
    }

    // Don't forget to tell the automaton to return to the warehouse.
    directions += "WAREHOUSE\r\n";
}


void Truck::setDirections(string direct)
{
    directions = direct;
}

void Truck::addPackage(Package *pack)
{
    if (currentWeight + pack->getWeight() > weight)
    {
        throw invalid_argument("Package doesn't fit");
    }
    packages.push_back(pack);
    currentWeight += pack->getWeight();
}

void Truck::addPackageVector(vector<Package* >* packs) {
    packages = *packs;
}

void Truck::deliverPackage(Package *pack)
{
/*    if(pack->getDelivered())
    {
        throw invalid_argument("Package is already delivered.");
    }
    vector<Package*>::iterator pos = find(packages.begin(), packages.end(), pack);
    if (pos != packages.end())
    {
        pack->deliver();
        packages.erase(pos);
    }
    else
    {
        throw invalid_argument("That package is not on this truck and can't be delivered");
    }
    */
}

string Truck::toString() const
{
    ostringstream output;

    output << endl
        << "Truck Number: " << truckID << endl
        << "Weight Capacity: " << weight << endl
        << "Current Weight: " << currentWeight << endl;
    output << "Current load of packages: " << endl;
    for(Package* p : packages)
    {
        output << "   Sender: " << p->getSender()->getName() << endl
               << " Receiver: " << p->getReceiver()->getName() << endl
               << "   Weight: " << p->getWeight() << endl
               << " Priority: " << p->getPriorityString() << endl;
    }
    return output.str();
}
