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

//! Constructor
/*!
 * \param w Weight limit for this truck
 */
Truck::Truck(const double w)
    :weight(w), truckID(++truckCount)
{}

//! Gets this truck's ID number
/*!
 * return int 
 */
int Truck::getTruckID() const
{
    return truckID;
}

//! Gets truck's weight limit
/*!
 * \return double
 */
double Truck::getWeight() const
{
    return weight;
}

double Truck::getCurrentWeight() const
{
    return currentWeight;
}

//! Gets truck's currently loaded packages
/*!
 * return vector<Package*>
 */
vector<Package*> Truck::getPackages() const
{
    return packages;
}

//! Get directions assigned to this truck
/*!
 * \return vector<string*>
 */
vector<string> Truck::getDirections() const
{
    return directions;
}

//! Setter for directions vector
/*!
 * \param direct pointer to directions vector
 */
void Truck::setDirections(vector<string> direct)
{
    directions = direct;
}

//! Add a package to the truck
/*!
 * Adds a package to the truck. If this would go over the weight limit of the truck, then it throws an error. It also adds the weight of the package to the truck if the package fits.
 * \param *pack Pointer to package
 */
void Truck::addPackage(Package* pack)
{
    if (currentWeight + pack->getWeight() > weight)
    {
        throw invalid_argument("Package doesn't fit");
    }
    packages.push_back(pack);
    currentWeight += pack->getWeight();
}

//! Removes package from truck (no flag)
/*!
 * Removes package from the truck, leaves it unaltered. Has some small error correction. Throws error if package is not on the truck.
 * \param pack a pointer to a Package object
 */
void Truck::removePackage(Package* pack)
{
   if(find(packages.begin(), packages.end(), pack) != packages.end())
   {
       packages.erase(remove(packages.begin(), packages.end(), pack), packages.end());
   }
   else
   {
       throw invalid_argument("Package not on truck");
   }
}

//! Delivers package
/*!
 * sets delivered flag to true if this package is in the vector
 */
void Truck::deliverPackage(Package *pack)
{
    if(pack->getDelivered())
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
}

//! Delivers all packages
/*!
 * Delivers all packages, sets their delivered flags, resets weight, reinitializes packages vector.
 */
void Truck::deliverPackages()
{
    for(Package* p: packages)
    {
        p->deliver();
    }
    packages = vector<Package*>();
    weight = 0;
}

//! To string
/*!
 * \return string A string representation of the truck and its contents
 */
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
