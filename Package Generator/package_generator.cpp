#include <iostream>
#include <iomanip>
//#include "Client.h"
#include "../Package.h"
#include <functional>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "../Utils.h"
#include <random>
#include <algorithm>
#include <vector>

using namespace std;

// Seed nice random number generator.
std::mt19937 rng(std::random_device{}());

typedef struct {

    // Filename to open
    string fileName;

    // Population of names to draw from.
    // 10 gives 10 first names and 10 last names, 10*10 = 100 combinations.
    unsigned int population;

    // Number of random packages
    unsigned int num;

    // Maximum building address
    int maxAddress;

    // Maximum streets each direction (20 = 20th St/Ave is furthest out.)
    unsigned int maxStreets;

    // Max random weight
    float maxWeight;

    // Priority weights of REG, TWO, OVER.
    // One can place positive probabilities or weights.
    // [0.1, 0.4, 0.5] would be 10% regular, 40% two-day and 50% overnights.
    // [1, 4, 5] would give the same percentages
    // [10, 40, 50] should as well.
    float priority[3];

    // Clustering creation.
    // Number of clusters (min[0], max[1]), package num in cluster (min [2], max[3]),
    // Radius [4] and standard deviation [5] in building addresses
    int cluster[6];

} randomPackageEnum;

struct {
    string name = "WAREHOUSE";
    string address = "2240 21 ST NW";
    string city = "BEMIDJI";
    string state = "MN";
    string zip = "56601";
} warehouse;

string parseStreet(unsigned int street, unsigned int addressNum) {
    // Coin flip
    std::uniform_int_distribution<int> coinFlip(0, 1);

    // Cardinal direction
    std::uniform_int_distribution<int> cardinalUniform(0, 3);

    string address = "";

    // Toss up if road is street or avenue
    unsigned int randomNum = coinFlip(rng);


    // What appropriate street name do we have?
    if (street == 0) {
        // No 0 street or avenue, choose main or central
        if (randomNum == 0) {
            address += "Main St ";

            // North or South?
            if (coinFlip(rng) == 0) {
                address += "N";
            } else {
                address += "S";
            }
        } else {
            address += "Central Ave ";
            // East or West?
            if (coinFlip(rng) == 0) {
                address += "W";
            } else {
                address += "E";
            }
        }

        // Add appropriate suffix
    } else if (street == 1) {

        address += std::to_string(street) + "st ";
    } else if (street == 2) {

        address += std::to_string(street) + "nd ";
    } else if (street == 3) {

        address += std::to_string(street) + "rd ";
    } else {

        address += std::to_string(street) + "th ";
    }

    // If we did not have a main or central
    if (street != 0) {
        // Choose street or avenue
        if (randomNum == 0) {
            address += "Street ";
        } else {
            address += "Avenue ";
        }

        // Since we did not pick main street or central avenue,
        // We need to choose our quadrant
        randomNum = cardinalUniform(rng);

        if (randomNum == 0) {
            address += "NW";
        } else if (randomNum == 1) {
            address += "NE";
        } else if (randomNum == 2) {
            address += "SE";
        } else if (randomNum == 3) {
            address += "SW";
        }
    }

    return address;
}


string streetAddressUniform(const randomPackageEnum randomConsts) {
    // Holds random ints for address creation
    unsigned int randomNum;

    // Holds random floats for probabilities and weights
    float randomFloat;

    // Street number
    unsigned int street;

    // Round any odd populations down
    unsigned int halfPop = randomConsts.population / 2;

    // Uniform distribution of integers for streets
    std::uniform_int_distribution<int> streetsUniform(0, randomConsts.maxStreets);

    // Uniform distribution of integers for address numbers
    std::uniform_int_distribution<int> addressUniform(1, randomConsts.maxAddress - 1);

    string address = "";

    // Random building number
    int addressNum = static_cast<int>(addressUniform(rng));

    address += std::to_string(addressNum) + " ";

    // Random street number
    street = streetsUniform(rng);

    address += parseStreet(street, addressNum);
    // No variation for this simulation
    address += ",Bemidji,MN,56601";

    return address;
}

vector<string > streetAddressCluster(const randomPackageEnum randomConsts) {
    // Number of clusters (min[0], max[1]), package num in cluster (min [2], max[3]),
    // Radius [4] and standard deviation [5] in building addresses

    // Holds random ints for address creation
    int randomNum;
    int addressNum;

    int clusterAddress = 0;
    int clusterStreet = 0;

    // Holds random floats for probabilities and weights
    float randomFloat;

    // Street number
    int street;

    // Round any odd populations down
    int halfPop = randomConsts.population / 2;

    // Uniform distribution of integers for streets
    std::uniform_int_distribution<int> streetsUniform(0, randomConsts.maxStreets);

    // Uniform distribution of integers for address numbers
    std::uniform_int_distribution<int> addressUniform(1, randomConsts.maxAddress);

    // Number of clusters to create
    std::uniform_int_distribution<int> numClusterUniform(randomConsts.cluster[0], randomConsts.cluster[1]);

    // Number of packages to create in cluster
    std::uniform_int_distribution<int> numClusterPackages(randomConsts.cluster[2], randomConsts.cluster[3]);

    // Uniform distribution around cluster point.
    std::normal_distribution<double> clusterNormal(randomConsts.cluster[4],randomConsts.cluster[5]);

    vector<string> returnAddresses;
    string tempAddress;

    for (int cluster = 0; cluster < numClusterUniform(rng); ++cluster) {

        // Get the clusters street center
        clusterStreet = streetsUniform(rng);

        clusterAddress = addressUniform(rng);

        for (int individualPackage = 0; individualPackage < numClusterPackages(rng); ++individualPackage) {

            tempAddress = "";

            // Random building number
            addressNum = static_cast<int>(clusterNormal(rng)) + clusterAddress;

            tempAddress += std::to_string(addressNum) + " ";

            // Random street number
            street = static_cast<int>(clusterNormal(rng))/100 + clusterStreet;

            tempAddress += parseStreet(street, addressNum);
            // No variation for this simulation
            tempAddress += ",Bemidji,MN,56601";

            returnAddresses.push_back(tempAddress);
        }
    }

    return returnAddresses;
}

void randomPackages(const randomPackageEnum randomConsts) {
    // Holds our names from files
    std::vector<string> firstNames;
    std::vector<string> lastNames;
    std::vector<string> clusterAddresses;


    // Output file stream
    ofstream file;

    // Input file stream
    ifstream names;

    // Holds lines and names from files
    string line;
    string name;
    string sender, receiver;

    double weight;

    // Buffer to extract names from files.
    stringstream buffer;

    // Round any odd populations down
    unsigned int halfPop = randomConsts.population / 2;

    // Uniform distribution of integers for streets
    std::uniform_int_distribution<int> streetsUniform(0, randomConsts.maxStreets);

    // Uniform distribution of integers for address numbers
    std::uniform_int_distribution<int> addressUniform(1, randomConsts.maxAddress);

    // Coin flip
    std::uniform_int_distribution<int> coinFlip(0, 1);

    // Cardinal direction
    std::uniform_int_distribution<int> cardinalUniform(0, 3);


    // Normally distributed package weight around the center of maximum weight. Standard deviation is 1/3 of that half point.
    std::normal_distribution<double> weightNormal(randomConsts.maxWeight / 2, randomConsts.maxWeight / 6);

    // Holds random ints for address creation
    unsigned int randomNum;

    // Holds random floats for probabilities and weights
    float randomFloat;

    // Street number
    unsigned int street;

    // Accumulated P or weights for priority package generation.
    float accumulated_P = 0;


    // Sum all weights or probabilities
    for (unsigned int i = 0; i < 3; i++) {
        accumulated_P += randomConsts.priority[i];
    }

    std::uniform_real_distribution<double> priorityUniform(0, accumulated_P);

    // Open file
    names.open("dist.all.last");

    // Extract up to population last names
    for(unsigned int i = 0; i < halfPop * 2; i++) {
        getline(names, line);
        buffer.str(line);
        buffer >> name;
        lastNames.push_back(name);
    }

    names.close();

    names.open("dist.female.first");

    // Extract 50% female first names
    for(unsigned int i = 0; i < halfPop; i++) {
        getline(names, line);
        buffer.str(line);
        buffer >> name;
        firstNames.push_back(name);
    }

    names.close();

    names.open("dist.male.first");

    // Extract 50% male first names.
    for(unsigned int i = 0; i < halfPop; i++) {
        getline(names, line);
        buffer.str(line);
        buffer >> name;
        firstNames.push_back(name);
    }

    names.close();

    // Uniform integer distribution for names.
    std::uniform_int_distribution<int> nameUniform(0, lastNames.size() - 1);

    // Get cluster addresses
    clusterAddresses = streetAddressCluster(randomConsts);

    // Start on creating random packages
    // Open file
    file.open(randomConsts.fileName);

    // Need to create clusters of packages
    for (auto iter = clusterAddresses.begin(); iter != clusterAddresses.end(); ++iter) {
        // Need a sender and receiver
        sender = streetAddressUniform(randomConsts);

        // Grab receiver.
        receiver = (*iter);

        file << firstNames[nameUniform(rng)] << "," << lastNames[nameUniform(rng)] << "," << sender << "," << firstNames[nameUniform(rng)] << "," << lastNames[nameUniform(rng)] << "," << receiver << ",";
        // Weight of package. Apply tenth ounce truncation by multiplying
        //unsigned int weight = weightNormal;

        weight = weightNormal(rng);

        while (weight < 0) {
            weightNormal(rng);
        }

        file << weight << ',';

        // Choose a random float up to accumulated_P
        randomFloat = priorityUniform(rng);

        // Start at index 0
        unsigned int index = 0;

        // Subtract priority weights from random until we find one that is less than our random number.
        while(index < 3) {
            if (randomFloat < randomConsts.priority[index]) {
                break;
            } else {
                randomFloat -= randomConsts.priority[index];
            }

            // Not found yet, move on.
            index++;
        }

        // Found our priority, since it is an enum, just output integer (moved enum up by 1 to avoid divide by zero).
        // Additionally, all packages start life on age = 1.
        file << index + 1 << ",1" << std::endl;

    }

    // Loop for number of desired packages
    for (unsigned int i = 0; i < randomConsts.num - clusterAddresses.size(); ++i) {
        // Need a sender and receiver
        sender = streetAddressUniform(randomConsts);
        receiver = streetAddressUniform(randomConsts);

        file << firstNames[nameUniform(rng)] << "," << lastNames[nameUniform(rng)] << "," << sender << "," << firstNames[nameUniform(rng)] << "," << lastNames[nameUniform(rng)] << "," << receiver << ",";
        // Weight of package. Apply tenth ounce truncation by multiplying
        //unsigned int weight = weightNormal;

        weight = weightNormal(rng);

        while (weight < 0) {
            weightNormal(rng);
        }

        file << weight << ',';

        // Start Russian roulette selection.
        // Choose a random float up to accumulated_P
        randomFloat = priorityUniform(rng);

        // Start at index 0
        unsigned int index = 0;

        // Subtract priority weights from random until we find one that is less than our random number.
        while(index < 3) {
            if (randomFloat < randomConsts.priority[index]) {
                break;
            } else {
                randomFloat -= randomConsts.priority[index];
            }

            // Not found yet, move on.
            index++;
        }

        // Found our priority, since it is an enum, just output integer (moved enum up by 1 to avoid divide by zero).
        // Additionally, all packages start life on age = 1.
        file << index + 1 << ",1" << std::endl;

    }

    file.close();

}

int main(int argc, char *argv[]) {

    // *Filename to open
    // *Population of names to draw from.
    // 10 gives 10 first names and 10 last names, 10*10 = 100 combinations.
    // *Number of random packages
    // Maximum building address
    // Maximum streets each direction (20 = 20th St/Ave is furthest out.)
    // Max random weight
    // Priority weights of REG, TWO, OVER.
    // One can place positive probabilities or weights.
    // [0.1, 0.4, 0.5] would be 10% regular, 40% two-day and 50% overnights.
    // [1, 4, 5] would give the same percentages
    // [10, 40, 50] should as well.
    // Clustering creation.
    // Number of clusters (min[0], max[1]), package num in cluster (min [2], max[3]),
    // Radius [4] and standard deviation [5] in building addresses
    
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " filename [options]" << std::endl;
        std::cout << "Options" << std::endl;
        std::cout << "=======" << std::endl;
        std::cout << "-pop #             Population of clients to produce" << std::endl;
        std::cout << "-n #               Number of packages to produce" << std::endl;
        std::cout << "-a #               Max building address (eg, 2000 1st St.)" << std::endl;
        
        
    }
    randomPackageEnum generatePackages = {"Cluster6.csv", 200, 200, 4000, 40, 320, {12, 4, 1}, {5,10,10,20, 1000, 200}};

    // Not guaranteed unique yet (eg, may send package to self, but with different address with small population.)
    //randomPackages(generatePackages);

    return 0;
}
