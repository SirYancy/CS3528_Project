#include <iostream>
#include <iomanip>
#include "Client.h"
#include "Package.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "Utils.h"
#include "Genetic.h"
#include <future>
#include <random>
#include <algorithm>

#define GENERATIONS 10000
#define POPULATION 1000
#define MAXTIME 60*8
#define MAXWEIGHT 16*2000
#define PACKAGE_LIMIT 200

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
    string address = "23 MAIN ST N";
    string city = "BEMIDJI";
    string state = "MN";
    string zip = "56601";
} warehouse;


void readFile(string fileName, vector<Package*> &packageList, unordered_map<std::string, Client*> &clientMap) {//vector<Client*> &clientList) {
    vector<string> csvLine;
    string line;
    string token;
    string clientStr;
    float weight;
    int integer, age;
    unsigned int clientID = 0;
    unsigned int packageID = 0;

    Priority packagePriority;

    // Pointers of clients
    Client* senderPtr;
    Client* receiverPtr;

    // Create a SS object to extract values
    ifstream file;
    file.open(fileName);

    // Toss first header line if using spreadsheet
    //getline(file, line);

    // Find highest ClientID already in use. For adding clients through multiple files.
    for (auto iter = clientMap.begin(); iter != clientMap.end(); ++iter) {
        if (iter->second->getID() > clientID) {
            clientID = iter->second->getID();
        }
    }

    // Get line from file until EOF
    while(getline(file, line)) {
        // Parsing stream
        stringstream fileLineStream(line);

        // Clear any remaining EOLs
        csvLine.clear();

        // Tokenize the line
        while(std::getline(fileLineStream, token, ',')) {
            csvLine.push_back(token);
        }

        /*
        cout << "Read tokens: ";
        for (unsigned int i = 0; i < csvLine.size(); i++) {
            cout << csvLine[i] << " ";

        }
        cout << endl;
        */

        // Build key in uppercase
        clientStr = strUpper(csvLine[0] + " " + csvLine[1] + "," + csvLine[2] + "," + csvLine[3] + "," + csvLine[4] + " " + csvLine[5]);

        // Debugging output.
        //cout << clientStr << endl;

        // Create an iterator to look if client exists
        unordered_map<std::string, Client*>::iterator it = clientMap.find(clientStr);

        // Check if client already exists.
        if (it != clientMap.end()) {
            // We found a client already, so grab the object pointer for package
            senderPtr = it->second;
            //cout << "\r\nFound sender: " << senderPtr << endl;

        } else {
            clientID++;
            // No client found, create a new one that is persistent and will not disappear off the stack.
            senderPtr = new Client(csvLine[0] + " " + csvLine[1], csvLine[2], csvLine[3], csvLine[4], csvLine[5], clientID);
            //cout << "\r\nNot found sender, new ptr: " << senderPtr << endl;

            // Place the new client into the map
            clientMap.emplace(clientStr, senderPtr);

        }

        // Need line for receiver
        clientStr = strUpper(csvLine[6] + " " + csvLine[7] + ", " + csvLine[8] + ", " + csvLine[9] + ", " + csvLine[10] +" " + csvLine[11]);

        // Find if receiver exists
        it = clientMap.find(clientStr);

        if (it != clientMap.end()) {
            // We found a client already, so grab the object pointer.
            receiverPtr = it->second;
            //cout << receiverPtr << endl;

        } else {
            clientID++;
            // No client found, create a new one that is persistent and will not disappear off the stack.
            receiverPtr = new Client(csvLine[6] + " " + csvLine[7], csvLine[8], csvLine[9], csvLine[10], csvLine[11], clientID);
            //cout << "Receiver not found, new ptr: " << receiverPtr << endl;

            // Place new client into map.
            clientMap.emplace(clientStr, receiverPtr);

        }

        //cout << "Weight: " << csvLine[12] << endl;
        // Need to load for conversion to weight.
        stringstream conversion(csvLine[12]);

        // Get weight
        conversion >> weight;

        // Load for conversion to priority
        stringstream convertPriority(csvLine[13]);

        // Convert to integer for priority
        convertPriority >> integer;

        // Get package priority by typecasting.
        packagePriority = static_cast<Priority>(integer);

        stringstream convertAge(csvLine[14]);

        // Age of package
        convertAge >> age;
        //std::cout << "Input package priority int: " << integer << " Priority: " << packagePriority << std::endl;

        // Make new persistent package that won't disappear off the stack. Get pointer.
        Package* packagePtr = new Package(senderPtr, receiverPtr, weight, packagePriority, packageID, age);


        packageID++;

        // Add to package list
        packageList.push_back(packagePtr);



    }
}


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
    vector<string> firstNames;
    vector<string> lastNames;
    vector<string> clusterAddresses;


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

void dumpClients(unordered_map<std::string, Client*> &ClientMap, vector<Package*> &Packages) {
    cout << "***** CLIENTS *****" << endl;
    int index = 0;
    for (auto iter = ClientMap.begin(); iter != ClientMap.end(); ++iter) {
        cout << endl << "*** CLIENT " << iter->second->getID() << " ***" << endl;
        //cout << "Client ptr: " << iter->second << endl;
        cout << iter->second->getName() << endl;
        cout << iter->second->getAddress() << endl;
        cout << iter->second->getCity() << ", " << iter->second->getState() << " " << iter->second->getZip() << endl;
        cout << "Coords: (" << iter->second->getCoords().first << ", " << iter->second->getCoords().second << ")" << endl;
        vector<Package*> temp = iter->second->getReceivedPackages();
        cout << endl << "Received packages: " << endl;
        for (auto& packPtr : temp) {
            cout << packPtr->getSender()->getName() << endl;
        }

        temp = iter->second->getSentPackages();
        cout << endl << "Sent packages: " << endl;
        for (auto& packPtr : temp) {
            cout << packPtr->getReceiver()->getName() << endl;
        }
        index++;
    }
}

void dumpPackages(unordered_map<std::string, Client*> &ClientMap, vector<Package*> &Packages) {
    cout << "***** PACKAGES *****" << endl;
    for (vector<Package*>::iterator iter = Packages.begin(); iter != Packages.end(); ++iter) {
        cout << endl << "*** PACKAGE ***" << endl;
        //cout << "Package sender ptr: " << (*iter)->getPointer() << endl;
        cout << "Sender: " << (*iter)->getSender()->getName() << endl;
        cout << "Receiver: " << (*iter)->getReceiver()->getName() << endl;

    }

    cout << endl << endl;
}

vector<vector<unsigned int> > makeMatrix(unordered_map<std::string, Client*> &ClientMap, vector<Package*> &Packages) {
    pair<int, int> coord_1, coord_2;

    unsigned int clientSize = ClientMap.size();
    unsigned int client_1_ID, client_2_ID;

    vector<vector<unsigned int> > matrix (clientSize, vector<unsigned int> (clientSize, 0));
    int manhattan_x, manhattan_y;

    for (auto iter_1 = ClientMap.begin(); iter_1 != ClientMap.end(); ++iter_1) {
        //vector<unsigned int> row;

        coord_1 = iter_1->second->getCoords();
        client_1_ID = iter_1->second->getID();
        for (auto iter_2 = ClientMap.begin(); iter_2 != ClientMap.end(); ++iter_2) {
            coord_2 = iter_2->second->getCoords();
            client_2_ID = iter_2->second->getID();
            //cout << "(" << coord_1.first << "," << coord_1.second << ") (" << coord_2.first << "," << coord_2.second << ") = ";
            manhattan_x = (coord_1.first - coord_2.first);
            manhattan_y = (coord_1.second - coord_2.second);

            if (manhattan_x < 0) {
                manhattan_x *= -1;
            }

            if (manhattan_y < 0) {
                manhattan_y *= -1;
            }

            matrix[client_1_ID][client_2_ID] = manhattan_x + manhattan_y;
            //cout << manhattan_x + manhattan_y << endl;
        }
    }

    return matrix;
}

vector< pair<vector<Package* >, Genetic::geneInfo> > simulationIsolation(vector<Package* > Packages, vector<vector<unsigned int> > matrix) {
    vector< pair<vector<Package* >, Genetic::geneInfo> > result;
    vector<float > fit;

    mutation_enum mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.001;
    mutation.insertNew  = 0.001;
    mutation.inversion  = 0.001;
    mutation.swapOut    = 0.001;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, PACKAGE_LIMIT, POPULATION, 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.initPopulation();
    result = GA.evolve_threads();
    //fit = GA.fitness(result);
    //return make_pair(result, fit);
    return result;

}

pair<vector<Package* >, vector<double> > simulationCrossover(vector<Package* > Packages, vector<vector<unsigned int> > matrix, vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPop) {
    vector<Package * > result;
    vector<double > fit;

    mutation_enum mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.001;
    mutation.insertNew  = 0.001;
    mutation.inversion  = 0.001;
    mutation.swapOut    = 0.001;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, PACKAGE_LIMIT, POPULATION, 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.loadPopulation(mixedPop);
    result = GA.evolve();
    fit = GA.fitness(&result);
    return make_pair(result, fit);
    //return result;

}

pair<vector<Package* >, vector<double> > runSimulationMixed(vector<Package* > Packages, vector<vector<unsigned int> > matrix) {
    vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPopulation;
    pair<vector<Package* >, double> selected;

    vector< pair<vector<Package* >, Genetic::geneInfo> > newPop1, newPop2, newPop3, newPop4;

    // Launch multiple asncronous threads. It seems that one cannot call object methods and get object copies.
    // Launching from within an object requires pointers to that object, leading to collisions and race conditions.
    // Here we call a wrapper, that creates it's own GA object and runs the simulation. Hence, every simulation should be
    // it's own unique snowflake.
    // We launch simulationIsolation which will return the entire population for mixing.
    auto f1 = std::async(std::launch::async, simulationIsolation, Packages, matrix);
    auto f2 = std::async(std::launch::async, simulationIsolation, Packages, matrix);
    auto f3 = std::async(std::launch::async, simulationIsolation, Packages, matrix);
    auto f4 = std::async(std::launch::async, simulationIsolation, Packages, matrix);

    // Wait for threads to finish, then fetch the returned result with get()
    auto res1 = f1.get();
    auto res2 = f2.get();
    auto res3 = f3.get();
    auto res4 = f4.get();

    // Process all results by conglomerating them into one big population.
    for (vector< pair<vector<Package* >, Genetic::geneInfo> >::iterator iter = res1.begin(); iter != res1.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    for (vector< pair<vector<Package* >, Genetic::geneInfo> >::iterator iter = res2.begin(); iter != res2.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    for (vector< pair<vector<Package* >, Genetic::geneInfo> >::iterator iter = res3.begin(); iter != res3.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    for (vector< pair<vector<Package* >, Genetic::geneInfo> >::iterator iter = res4.begin(); iter != res4.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }


    // Shuffle up the big population.
    std::shuffle(mixedPopulation.begin(), mixedPopulation.end(), rng);

    // Repopulate the isolated island threads from the mixed genomes.
    while(mixedPopulation.size() > 0) {
        if (mixedPopulation.size() > 0) {
            newPop1.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }
        if (mixedPopulation.size() > 0) {
            newPop2.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }
        if (mixedPopulation.size() > 0) {
            newPop3.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }

        if (mixedPopulation.size() > 0) {
            newPop4.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }
    }

    // Launch evolution again, with the mixed up and shuffled isolated populations. This time
    // we return only the fittest individual genome.
    auto c1 = std::async(std::launch::async, simulationCrossover, Packages, matrix, newPop1);
    auto c2 = std::async(std::launch::async, simulationCrossover, Packages, matrix, newPop2);
    auto c3 = std::async(std::launch::async, simulationCrossover, Packages, matrix, newPop3);
    auto c4 = std::async(std::launch::async, simulationCrossover, Packages, matrix, newPop4);

/*
    auto res1 = f1.get();
    auto res2 = f2.get();
    auto res3 = f3.get();
    auto res4 = f4.get();
    */

    // Grab the best route and stuff in here
    vector<pair<vector<Package* >, vector<double> > > bestMixed;
    bestMixed.push_back(c1.get());
    bestMixed.push_back(c2.get());
    bestMixed.push_back(c3.get());
    bestMixed.push_back(c4.get());

    // Which route is the bestest!
    float bestSoFar = 0;

    for (unsigned int i = 0; i < bestMixed.size(); ++i) {
        if (bestMixed[i].second[0] > bestSoFar) {
            bestSoFar = i;
        }
    }

    // Return best of the isolated and mixed populations.
    return bestMixed[bestSoFar];

}

void createGraphFile(vector<Package* >* Packages, pair<vector<Package* >, vector<double> >* best, Client* originPtr) {
    // Output file stream
    ofstream file;

    int xmin = 0;
    int xmax = 0;
    int ymin = 0;
    int ymax = 0;
    int routeIndex = -1, overnightIndex = -1, twodayIndex = -1, regularIndex = -1, depotIndex = -1;
    int packages = 0;
    int index = 0;

    file.open("route.gnu");

    // Index 0, OVERNIGHT
    file << "#OVERNIGHT" << std::endl;
    for (vector<Package* >::iterator iter = Packages->begin(); iter!=Packages->end(); ++iter) {
        if ((*iter)->getReceiver()->getCoords().first < xmin) {
            xmin = (*iter)->getReceiver()->getCoords().first;
        }

        if ((*iter)->getReceiver()->getCoords().first > xmax) {
            xmax = (*iter)->getReceiver()->getCoords().first;
        }

        if ((*iter)->getReceiver()->getCoords().second < ymin) {
            ymin = (*iter)->getReceiver()->getCoords().second;
        }

        if ((*iter)->getReceiver()->getCoords().second > ymax) {
            ymax = (*iter)->getReceiver()->getCoords().second;
        }

        if ((*iter)->getPriority() == Priority::OVERNIGHT) {
            file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
            ++packages;
        }
    }

    // Did we have any packages of overnight?
    if (packages != 0) {
        overnightIndex = index;
        ++index;
        file << std::endl << std::endl;
        packages = 0;
    }

    // Index 1, TWO_DAY
    file << "#TWO-DAY" << std::endl;
    for (vector<Package* >::iterator iter = Packages->begin(); iter!=Packages->end(); ++iter) {
        if ((*iter)->getReceiver()->getCoords().first < xmin) {
            xmin = (*iter)->getReceiver()->getCoords().first;
        }

        if ((*iter)->getReceiver()->getCoords().first > xmax) {
            xmax = (*iter)->getReceiver()->getCoords().first;
        }

        if ((*iter)->getReceiver()->getCoords().second < ymin) {
            ymin = (*iter)->getReceiver()->getCoords().second;
        }

        if ((*iter)->getReceiver()->getCoords().second > ymax) {
            ymax = (*iter)->getReceiver()->getCoords().second;
        }

        if ((*iter)->getPriority() == Priority::TWO_DAY) {
            file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
            ++packages;
        }
    }

    // Did we have any packages of twoday?
    if (packages != 0) {
        twodayIndex = index;
        ++index;
        file << std::endl << std::endl;
        packages = 0;
    }

    // Index 2, REGULAR
    file << "#REGULAR" << std::endl;
    for (vector<Package* >::iterator iter = Packages->begin(); iter!=Packages->end(); ++iter) {
        if ((*iter)->getReceiver()->getCoords().first < xmin) {
            xmin = (*iter)->getReceiver()->getCoords().first;
        }

        if ((*iter)->getReceiver()->getCoords().first > xmax) {
            xmax = (*iter)->getReceiver()->getCoords().first;
        }

        if ((*iter)->getReceiver()->getCoords().second < ymin) {
            ymin = (*iter)->getReceiver()->getCoords().second;
        }

        if ((*iter)->getReceiver()->getCoords().second > ymax) {
            ymax = (*iter)->getReceiver()->getCoords().second;
        }

        if ((*iter)->getPriority() == Priority::REGULAR) {
            file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
            ++packages;
        }
    }


    // Did we have any packages of regular?
    if (packages != 0) {
        regularIndex = index;
        ++index;
        file << std::endl << std::endl;
        packages = 0;
    }

    // Index 3, Depot
    file << "#Depot" << std::endl;
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    depotIndex = index;
    ++index;
    file << std::endl << std::endl;

    // Index 4, route
    file << "#Evolved route" << std::endl;
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    for (vector<Package* >::iterator iter = best->first.begin(); iter!=best->first.end(); ++iter) {
        file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
        ++packages;
    }
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    routeIndex = index;
    file.close();

    file.open("gnugraph");

    // Regular packages, green circles
    file << "set style line 1 lc rgb \'#008000\' lt 1 lw 2 pt 7 ps 1.5 # --- green" << std::endl;
    // Two-day packages, blue squares
    file << "set style line 2 lc rgb \'blue\' lt 1 lw 2 pt 5 ps 1.5 # --- blue" << std::endl;
    // Overnight packages, red triangles
    file << "set style line 3 lc rgb \'red\' lt 1 lw 2 pt 9 ps 1.5 # --- red" << std::endl;
    // Depot, yellow pentagon
    file << "set style line 4 lc rgb \'black' lt 1 lw 2 pt 15 ps 4 # --- black" << std::endl;

    // Route, orange.
    file << "set style line 5 lc rgb \'orange' lt 1 lw 3 pt 1 ps 1.5 # --- orange" << std::endl;

    file << "set xrange [" << xmin - 5 << ":" << xmax + 5 << "]" << std::endl;
    file << "set yrange [" << ymin - 5 << ":" << ymax + 5 << "]" << std::endl;

    file << "set terminal pngcairo size 1920,1080" << std::endl;

    file << "set term \'pngcairo\'" << std::endl;
    file << "set output \'route.png\'" << std::endl;
    file << "set style line 100 lt 1 lc rgb \"gray\" lw 2" << std::endl;
    file << "set style line 101 lt 0.5 lc rgb \"gray\" lw 2" << std::endl;

    file << "set grid mytics ytics ls 100, ls 101" << std::endl;
    file << "set grid mxtics xtics ls 100, ls 101" << std::endl;
    file << "set mxtics 10" << std::endl;
    file << "set mytics 10" << std::endl;
    file << "set label \"Pop: " << POPULATION << ", Gen: " << GENERATIONS << ", Fit: " << best->second[0] << "\"  at graph 0.8, 0.05" << std::endl;
    file << "set label \"Pri: " << best->second[1] << ", T: " << best->second[3] << "/" << MAXTIME << ", P: " << best->first.size() - 2 << "/" << Packages->size() << "\" at graph 0.8, graph 0.03" << std::endl;

    file << "plot \'route.gnu\' index " << std::to_string(routeIndex) << " with lines ls 5 title \'Route\',\\" << std::endl;
    if (overnightIndex != -1) {
        file << "\'route.gnu\' index " << std::to_string(overnightIndex) << " with points ls 3 title \'Overnight\',\\" << std::endl;
    }

    if (twodayIndex != -1) {
        file << "\'route.gnu\' index " << std::to_string(twodayIndex) << " with points ls 2 title \'Two-day\',\\" << std::endl;
    }

    if (regularIndex != -1) {
        file << "\'route.gnu\' index " << std::to_string(regularIndex) << " with points ls 1 title \'Regular\',\\" << std::endl;
    }

    file << "\'route.gnu\' index " << std::to_string(depotIndex) << " with points ls 4 title \'Depot\'" << std::endl;
    //file << "pause -1" << std::endl;
    file.close();
}

int main() {

    srand(time(0));
    //srand(201);
    vector<Package* > Packages;
    //vector<Client*> Clients;
    unordered_map<std::string, Client*> ClientMap;
    vector<vector<unsigned int> > matrix;
    pair<vector<Package* >, vector<double> > best;


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
    randomPackageEnum generatePackages = {"Cluster3.csv", 200, 300, 4000, 40, 320, {2, 2, 1}, {10,20,10,20, 1000, 400}};

    // Not guaranteed unique yet (eg, may send package to self, but with different address with small population.)
    //randomPackages(generatePackages);

    // Make warehouse the origin.
    Client* originPtr = new Client(warehouse.name, warehouse.address, warehouse.city, warehouse.state, warehouse.zip, 0);

    ClientMap.emplace(warehouse.name + "," + warehouse.address + "," + warehouse.city + "," + warehouse.state + " " + warehouse.zip, originPtr);

    readFile("Cluster3.csv", Packages, ClientMap);

    matrix = makeMatrix(ClientMap, Packages);

    //dumpClients(ClientMap, Packages);

    /*
    for (unsigned int i = 0; i < matrix.size(); i++) {
        for (unsigned int j = 0; j < matrix[0].size(); j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
    */

    std::cout << "Evolving best route, please wait..." << std::endl;


    best = runSimulationMixed(Packages, matrix);

    std::cout << std::endl << "Best OVERALL -> Fit: " << best.second[0] << " Pri: " << best.second[1] << " D: " << best.second[2] << " T: " << best.second[3] << "/" << MAXTIME << " W: " << best.second[4] << "/" << MAXWEIGHT << std::endl;

    createGraphFile(&Packages, &best, originPtr);

    return 0;
}
