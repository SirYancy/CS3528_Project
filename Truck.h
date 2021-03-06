#ifndef TRUCK_H
#define TRUCK_H

#include <string>
#include <vector>

using namespace std;

class Package;
class Client;

//! Truck Class
/*!
 * This Truck class defines a truck. It is responsible for holding a list of all packages in its delivery queue and a list of instructions for delivering those packages.
 */
class Truck{
    public:
        //! A count of the total number of trucks on the road.
        static int truckCount;
        //! Truck constructor
        /*!
         * \param w Weight limit
         */
        Truck(const double w);

        //! Getter for Truck ID Number
        /*!
         * \return int Truck ID number
         */
        int getTruckID() const;

        //! Getter for truck's weight limit
        /*!
         * \return double Truck's weight limit
         */
        double getWeight() const;

        //! Getter for package vector
        /*
         * \return packages a vector containing pointers to packages loaded in the truck
         */
        vector<Package*> getPackages() const;

        //! Getter for directions vector
        /*!
         * \return A string containing the truck's current list of directions.
         */
        string getDirections() const;

        //! Setter for directions vector
        /*!
         * \param direct A string of the truck's assigned directions
         */
        void setDirections(string direct);

        //! Adds a package to the truck
        /*!
         * Adds a package to the truck and adds its weight to the current load
         * \param pack A package object
         */
        void addPackage(Package*);

        //! Add package route to truck
        /*! \param packs Package route vector to add to truck.
         */
        void addPackageVector(vector<Package* >* packs);

        //! Processes packages held by the truck into addresses.
        /*! Processes the packages held by the truck.
         * Currently converts packages into vector string for print out.
         */
        void processPackages();

        //! Delivers a package
        /*!
         * Removes a package from the truck, subtracts its weight, and sets the package's delivered flag.
         */
        void deliverPackage(Package *pack);

        //! Returns a string representation of the truck and its contents
        string toString() const;

    private:
        //! This truck's identification number
        const int truckID;

        //! This truck's weight limit
        double weight;

        //! The current weight of the truck's load of packages
        double currentWeight = 0;

        //! Vector storing the truck's current load of packages
        vector<Package* > packages;

        //! Vector of strings storing the directions for this truck
        string directions;

};
#endif //TRUCK_H
