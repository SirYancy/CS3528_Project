#include "Genetic.h"
//#include "Client.h"
//#include "Package.h"
#include <vector>
#include <iostream>

Genetic::Genetic(std::vector<Package*>* packs,
                 std::vector<vector<unsigned int> >* matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive)
                 : packages(packs), adMatrix(matrix), weightLimit(weight), packageLimit(packLimit), popNum(population), stopTime(stops), driveTime(drive)

{

}

Genetic::~Genetic()
{
    //dtor
}

void Genetic::initPopulation() {
    std::vector<Package* > individual;

    float totalWeight = 0;
    unsigned int totalPackages = 0;

    unsigned int numOfPackages = (*packages).size();
    Package* currentPackage;
    unsigned int randomPackage;
    bool present = false;

    for (unsigned int i = 0; i < popNum; i++) {
        // Clear individual
        individual.clear();

        // Loop while we still have packages to place, and we're under the limits.
        while (totalPackages < numOfPackages && totalPackages << packageLimit && totalWeight < weightLimit) {


            // Random index to packages
            randomPackage = rand() % numOfPackages;

            // Get the package pointer
            currentPackage = (*packages)[randomPackage];

            // Assume not present in current individual
            present = false;

            // Loop over the individual looking for duplicate package
            for (std::vector<Package*>::iterator iter = individual.begin(); iter != individual.end(); ++iter) {
                // Check duplicate
                if (*iter == currentPackage) {
                    present = true;
                    std::cout << "Package already present: " << currentPackage << std::endl;
                }
            }

            // If not a duplicate
            if (present == false) {
                // Over truck limit if added?
                if (currentPackage->getWeight() + totalWeight <= weightLimit) {
                    // Add package to individual
                    individual.push_back(currentPackage);
                    std::cout << "Adding package ptr: " << currentPackage << std::endl;
                    packages++;
                    totalWeight += currentPackage->getWeight();
                    std::cout << "Current weight: " << totalWeight << endl;
                }

            }
        }
        //genes.push_back(individual);
    }
}
