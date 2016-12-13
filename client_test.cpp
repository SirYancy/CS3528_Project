#include <iostream>
#include <iomanip>
#include "Client.h"
#include "Package.h"
#include "Utils.h"
#include "Genetic.h"
#include "Truck.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include <future>
#include <random>
#include <algorithm>
#include <ctime>

// Defaults for GA
#define GENERATIONS 5000
#define POPULATION 500
#define MAXTIME 60*8
#define MAXWEIGHT 16*2000
#define PACKAGE_LIMIT 1000

using namespace std;

// Seed nice random number generator.
std::mt19937 rng(std::random_device{}());

// Default warehouse address if none specified.
struct {
    string name = "WAREHOUSE";
    string address = "123 MAIN ST N";
    string city = "Bemidji";
    string state = "MN";
    string zip = "56601";
} warehouse;


int readFile(string fileName, vector<Package*> &packageList, unordered_map<std::string, Client*> &clientMap) {
    // Read a package list from a CSV file.
    // packageList to store packages into,
    // clientMap to store clients into.
    // Returns integer error code 1 for error, 0 for okay.

    // Vector of CSV tokens.
    vector<string> csvLine;

    // Current read in line from file.
    string line;

    // Current token
    string token;

    // Client string made from tokens.
    string clientStr;

    // Weight of package
    float weight;

    // Temp integer and age of package.
    int integer, age;

    // Starting client and package ID.
    unsigned int clientID = 0;
    unsigned int packageID = 0;

    // Package priority enum.
    Priority packagePriority;

    // Pointers of clients
    Client* senderPtr;
    Client* receiverPtr;

    // File handle
    ifstream file;

    // Attempt to open file.
    file.open(fileName);

    // If unable to open file, return error.
    if (!file) {
        return 1;
    }

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

        // Build key in uppercase
        clientStr = strUpper(csvLine[0] + " " + csvLine[1] + "," + csvLine[2] + "," + csvLine[3] + "," + csvLine[4] + " " + csvLine[5]);

        // Create an iterator to look if client exists
        unordered_map<std::string, Client*>::iterator it = clientMap.find(clientStr);

        // Check if client already exists.
        if (it != clientMap.end()) {
            // We found a client already, so grab the object pointer for package
            senderPtr = it->second;

        } else {
            clientID++;
            // No client found, create a new one that is persistent and will not disappear off the stack.
            senderPtr = new Client(csvLine[0] + " " + csvLine[1], csvLine[2], csvLine[3], csvLine[4], csvLine[5], clientID);

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

        } else {
            clientID++;
            // No client found, create a new one that is persistent and will not disappear off the stack.
            receiverPtr = new Client(csvLine[6] + " " + csvLine[7], csvLine[8], csvLine[9], csvLine[10], csvLine[11], clientID);

            // Place new client into map.
            clientMap.emplace(clientStr, receiverPtr);

        }

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

        // Make new persistent package that won't disappear off the stack. Get pointer.
        Package* packagePtr = new Package(senderPtr, receiverPtr, weight, packagePriority, packageID, age);

        // Increment for next package
        packageID++;

        // Add to package list
        packageList.push_back(packagePtr);

    }

    // Return good
    return 0;
}


void dumpClients(unordered_map<std::string, Client*> &ClientMap, vector<Package*> &Packages) {
    // Dump a bunch of client info to console
    // ClientMap map of client objects.
    // Packages is vector of packages to iterate over.

    cout << "***** CLIENTS *****" << endl;
    int index = 0;

    // Iterate over all clients, dumping relevant information.
    for (auto iter = ClientMap.begin(); iter != ClientMap.end(); ++iter) {
        cout << endl << "*** CLIENT " << iter->second->getID() << " ***" << endl;
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
    // Dump a bunch of package info to console
    // ClientMap map of client objects.
    // Packages is vector of packages to iterate over.

    cout << "***** PACKAGES *****" << endl;
    // Dump senders and receivers of packages to console.
    for (vector<Package*>::iterator iter = Packages.begin(); iter != Packages.end(); ++iter) {
        cout << endl << "*** PACKAGE ***" << endl;
        //cout << "Package sender ptr: " << (*iter)->getPointer() << endl;
        cout << "Sender: " << (*iter)->getSender()->getName() << endl;
        cout << "Receiver: " << (*iter)->getReceiver()->getName() << endl;

    }

    cout << endl << endl;
}

vector<vector<unsigned int> > makeMatrix(unordered_map<std::string, Client*> &ClientMap, vector<Package*> &Packages) {
    // Create and return Manhattan adjacency matrix.
    // ClientMap is all clients in this potential package pool.
    // Packages is all potential packages to be delivered.
    // Returns a matrix of Manhattan distances indexed by client ID for receivers.

    // Coordinates of client 1 and client 2.
    pair<int, int> coord_1, coord_2;

    // How many clients
    unsigned int clientSize = ClientMap.size();
    // Client ID/index.
    unsigned int client_1_ID, client_2_ID;

    // Create the matrix (again we created the matrix!)
    vector<vector<unsigned int> > matrix (clientSize, vector<unsigned int> (clientSize, 0));

    // Manhattan x and y.
    int manhattan_x, manhattan_y;

    // Iterate over clients
    for (auto iter_1 = ClientMap.begin(); iter_1 != ClientMap.end(); ++iter_1) {

        // Get client 1 coordinates.
        coord_1 = iter_1->second->getCoords();
        // Get client 1 ID/index
        client_1_ID = iter_1->second->getID();

        // Iterate over all other clients.
        for (auto iter_2 = ClientMap.begin(); iter_2 != ClientMap.end(); ++iter_2) {
            // Client 2 coordinates.
            coord_2 = iter_2->second->getCoords();
            // Client 2 ID/index
            client_2_ID = iter_2->second->getID();

            // Find Manhattan X and Y
            manhattan_x = (coord_1.first - coord_2.first);
            manhattan_y = (coord_1.second - coord_2.second);

            // Convert to positive distances
            if (manhattan_x < 0) {
                manhattan_x *= -1;
            }

            if (manhattan_y < 0) {
                manhattan_y *= -1;
            }

            // Enter into the matrix.
            matrix[client_1_ID][client_2_ID] = manhattan_x + manhattan_y;
        }
    }

    return matrix;
}

vector< pair<vector<Package* >, Genetic::geneInfo> > simulationIsolation(vector<Package* > Packages, vector<vector<unsigned int> > matrix, unsigned int numOfGenerations, unsigned int population, unsigned int shiftTime, unsigned int weight) {
    // Run an isolated evolution simulation. Used in threading to work around the "each thread is its own universe" problem.
    // Requires:
    // Packages: Package pool list
    // Matrix: Adjacency matrix
    // numOfGenerations: How many generations.
    // population: Number of individuals per generation.
    // shiftTime: Maximum shift time allowable. Minutes.
    // weight: Maximum allowable weight.
    //
    // Returns:
    // Returns a full population vector to later processing in simulationIntermingle or simulationCrossover.

    // Resulting returned population
    vector< pair<vector<Package* >, Genetic::geneInfo> > result;


    // Mutation values
    mutation_struct mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.001;
    mutation.insertNew  = 0.001;
    mutation.inversion  = 0.005;
    mutation.swapOut    = 0.001;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    // Genetic object creation, some defaults.
    Genetic GA(Packages, matrix, weight, PACKAGE_LIMIT, population, 5, 1, shiftTime, numOfGenerations, mutation);
    // Create initial population
    GA.initPopulation();

    // Evolved the routes.
    result = GA.evolve_threads();

    // Return the results.
    return result;

}

vector< pair<vector<Package* >, Genetic::geneInfo> >  simulationIntermingle(vector<Package* > Packages, vector<vector<unsigned int> > matrix, vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPop, unsigned int numOfGenerations, unsigned int shiftTime, unsigned int weight) {
    // Run an intermingled evolution simulation. Used in threading to work around the "each thread is its own universe" problem.
    // Requires:
    // Packages: Package pool list
    // Matrix: Adjacency matrix
    // mixedPop: Population of intermingled individuals from other isolated islands.
    // numOfGenerations: How many generations.
    // population: Number of individuals per generation.
    // shiftTime: Maximum shift time allowable. Minutes.
    // weight: Maximum allowable weight.
    //
    // Returns:
    // Returns a full population vector to later processing in simulationIntermingle or simulationCrossover.

    // Resulting returned population
    vector< pair<vector<Package* >, Genetic::geneInfo> > resultPopulation;

    // Mutation values
    mutation_struct mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.001;
    mutation.insertNew  = 0.001;
    mutation.inversion  = 0.005;
    mutation.swapOut    = 0.001;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    // Genetic object creation, some defaults.
    Genetic GA(Packages, matrix, weight, PACKAGE_LIMIT, mixedPop.size(), 5, 1, shiftTime, numOfGenerations, mutation);

    // Load the already established population
    GA.loadPopulation(mixedPop);

    // Evolve!
    resultPopulation = GA.evolve_threads();

    // Return intermingled population.
    return resultPopulation;

}

pair<vector<Package* >, vector<double> > simulationCrossover(vector<Package* > Packages, vector<vector<unsigned int> > matrix, vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPop, unsigned int numOfGenerations, unsigned int shiftTime, unsigned int weight) {
    // Run an intermingled evolution simulation. Used in threading to work around the "each thread is its own universe" problem.
    // Requires:
    // Packages: Package pool list
    // Matrix: Adjacency matrix
    // mixedPop: Population of intermingled individuals from other isolated islands.
    // numOfGenerations: How many generations.
    // population: Number of individuals per generation.
    // shiftTime: Maximum shift time allowable. Minutes.
    // weight: Maximum allowable weight.
    //
    // Returns:
    // Returns the best fit individual.

    // Resulting returned fittest
    vector<Package * > resultFitest;

    // Fitness of fittest
    vector<double > fit;

    // Mutation parameters
    mutation_struct mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.001;
    mutation.insertNew  = 0.001;
    mutation.inversion  = 0.005;
    mutation.swapOut    = 0.001;
    mutation.swapWithin = 0.02;
    mutation.elite      = 0.01;

    // Genetic object creation, some defaults.
    Genetic GA(Packages, matrix, weight, PACKAGE_LIMIT, mixedPop.size(), 5, 1, shiftTime, numOfGenerations, mutation);

    // Load the already established population
    GA.loadPopulation(mixedPop);

    // Evolve and get fittest
    resultFitest = GA.evolve();

    // What was the fitness of this route?
    fit = GA.fitness(&resultFitest);

    // Return fittest and fitness.
    return make_pair(resultFitest, fit);


}

pair<vector<Package* >, vector<double> > runSimulationMixed(vector<Package* > Packages, vector<vector<unsigned int> > matrix, unsigned int population, unsigned int generations, unsigned int shiftTime, unsigned int weight) {
    // Run multiple threaded evolutions. Threading creates copies of the universe so we need to call functions and fetch results later.
    // Requires:
    // Packages: Package pool list
    // Matrix: Adjacency matrix
    // generation: How many generations.
    // population: Number of individuals per generation.
    // shiftTime: Maximum shift time allowable. Minutes.
    // weight: Maximum allowable weight.
    //
    // Returns:
    // Returns the best fit individual.

    // Genome population for mixed populations, returned populations, and threaded populations.
    vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPopulation, returnedPopulation, newPop1, newPop2, newPop3, newPop4;

    // The best selected route
    pair<vector<Package* >, vector<double> > selected;

    std::cout << std::endl << "Running isolated islands..." << std::endl << std::endl;

    // Launch multiple asynchronous threads. It seems that one cannot call object methods and get object copies.
    // Launching from within an object requires pointers to that object, leading to collisions and race conditions.
    // Here we call a wrapper, that creates it's own GA object and runs the simulation. Hence, every simulation should be
    // it's own unique snowflake.
    // We launch simulationIsolation which will return the entire population for mixing.
    auto f1 = std::async(std::launch::async, simulationIsolation, Packages, matrix, generations, population, shiftTime, weight);
    auto f2 = std::async(std::launch::async, simulationIsolation, Packages, matrix, generations, population, shiftTime, weight);
    auto f3 = std::async(std::launch::async, simulationIsolation, Packages, matrix, generations, population, shiftTime, weight);
    auto f4 = std::async(std::launch::async, simulationIsolation, Packages, matrix, generations, population, shiftTime, weight);

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

    std::cout << std::endl << "Intermingling islands..." << std::endl << std::endl;

    // Mix the population up, and run a combined simulation (eg, land bridges allow population to intermingle and breed, allowing the fittest to survive.)
    mixedPopulation = simulationIntermingle(Packages, matrix, mixedPopulation, generations / 4, shiftTime, weight);

    // Shuffle up the big intermixed return population for parsing to isolated islands again.
    std::shuffle(mixedPopulation.begin(), mixedPopulation.end(), rng);

    // Repopulate the isolated island threads from the mixed genomes.
    while(mixedPopulation.size() > 0) {
        if (mixedPopulation.size() > 0) {
            newPop1.push_back(mixedPopulation.back());
            mixedPopulation.pop_back();
        }
        if (mixedPopulation.size() > 0) {
            newPop2.push_back(mixedPopulation.back());
            mixedPopulation.pop_back();
        }
        if (mixedPopulation.size() > 0) {
            newPop3.push_back(mixedPopulation.back());
            mixedPopulation.pop_back();
        }

        if (mixedPopulation.size() > 0) {
            newPop4.push_back(mixedPopulation.back());
            mixedPopulation.pop_back();
        }
    }

    std::cout << std::endl << "Running isolated islands again..." << std::endl << std::endl;

    // Launch evolution again, with the mixed up and shuffled isolated populations. This time
    // We return another entire population from the islands.
    auto c1 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop1, generations, shiftTime, weight);
    auto c2 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop2, generations, shiftTime, weight);
    auto c3 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop3, generations, shiftTime, weight);
    auto c4 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop4, generations, shiftTime, weight);

    // Get the resulting populations.
    res1 = c1.get();
    res2 = c2.get();
    res3 = c3.get();
    res4 = c4.get();

    // Clear our old intermingled population.
    mixedPopulation.clear();

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

    std::cout << std::endl << "Intermingling islands..." << std::endl << std::endl;

    // Mix the population up, and run a combined simulation (eg, land bridges allow population to intermingle.)
    // This time we will only receive the fittest individual.
    selected = simulationCrossover(Packages, matrix, mixedPopulation, generations / 2, shiftTime, weight);

    return selected;
}

void createGraphFile(vector<Package* >* Packages, pair<vector<Package* >, vector<double> >* best, Client* originPtr, unsigned int population, unsigned int generations, unsigned int shiftTime, float weight) {
    // Output a gnugraph file for visualizing the fittest route.
    // Requires:
    // Packages: List of packages in potential delivery pool.
    // best: Best, fittest route with fitness info.
    // originPtr: Client origin pointer for warehouse address.
    // generation: How many generations.
    // population: Number of individuals per generation.
    // shiftTime: Maximum shift time allowable. Minutes.
    // weight: Maximum allowable weight.

    // Output file stream
    ofstream file;

    // Keep track of extent of graph
    int xmin = 0;
    int xmax = 0;
    int ymin = 0;
    int ymax = 0;

    // How many routes, and packages are we plotting?
    int routeIndex = -1, overnightIndex = -1, twodayIndex = -1, regularIndex = -1, depotIndex = -1;

    // Number of packages
    int packages = 0;

    // Index for gnuplot files assigning line and point attributes.
    int index = 0;

    // Get date and time right now
    auto now = time(nullptr);

    // Convert to string
    auto dateTime = *std::localtime(&now);

    // Open standard file
    file.open("route.gnu");

    // Index 0, OVERNIGHT
    file << "#OVERNIGHT" << std::endl;

    // These priority counters are highly inefficient and should be combined...

    // Look through all packages and find minimum and maximum coordinates.
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

        // Count overnight packages. Guards against throwing out indices off for gnuplot. Output coordinates as well.
        if ((*iter)->getPriority() == Priority::OVERNIGHT) {
            file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
            ++packages;
        }
    }

    // Did we have any packages of overnight?
    if (packages != 0) {
        // Assign this index to overnights.
        overnightIndex = index;
        ++index;
        file << std::endl << std::endl;
        packages = 0;
    }

    // Index 1, TWO_DAY
    file << "#TWO-DAY" << std::endl;
    // Look through all packages and find minimum and maximum coordinates.
    for (vector<Package* >::iterator iter = Packages->begin(); iter!=Packages->end(); ++iter) {

        // Count two-day packages. Guards against throwing out indices off for gnuplot. Output coordinates as well.
        if ((*iter)->getPriority() == Priority::TWO_DAY) {
            file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
            ++packages;
        }
    }

    // Did we have any packages of two-day?
    if (packages != 0) {
        // Assign index.
        twodayIndex = index;
        ++index;
        file << std::endl << std::endl;
        packages = 0;
    }

    // Index 2, REGULAR
    file << "#REGULAR" << std::endl;
    // Iterate and count... again.
    for (vector<Package* >::iterator iter = Packages->begin(); iter!=Packages->end(); ++iter) {

        // Regular package count. Output coordinates as well.
        if ((*iter)->getPriority() == Priority::REGULAR) {
            file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
            ++packages;
        }
    }


    // Did we have any packages of regular?
    if (packages != 0) {
        // Assign index.
        regularIndex = index;
        ++index;
        file << std::endl << std::endl;
        packages = 0;
    }

    // Index 3, Depot
    file << "#Depot" << std::endl;
    // Depot coordinates.
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    // Depot uses this index.
    depotIndex = index;
    ++index;
    file << std::endl << std::endl;

    // Index 4, route
    file << "#Evolved route" << std::endl;
    // Begin route at warehouse coords.
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    // Iterate over route, output the routes coords.
    for (vector<Package* >::iterator iter = best->first.begin(); iter!=best->first.end(); ++iter) {
        file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
        ++packages;
    }

    // Make route go back to warehouse.
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    // Index route for gnuplot.
    routeIndex = index;

    // Close data file.
    file.close();

    // Open script file
    file.open("gnugraph");

    // Build script for gnuplot.
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

    // Min/max extent of plot.
    file << "set xrange [" << xmin - 5 << ":" << xmax + 5 << "]" << std::endl;
    file << "set yrange [" << ymin - 5 << ":" << ymax + 5 << "]" << std::endl;

    // PNG canvas size
    file << "set terminal pngcairo size 1920,1080" << std::endl;

    // PNG output
    file << "set term \'pngcairo\'" << std::endl;
    file << "set output \'route.png\'" << std::endl;

    // Grid line style
    file << "set style line 100 lt 1 lc rgb \"gray\" lw 2" << std::endl;
    file << "set style line 101 lt 0.5 lc rgb \"gray\" lw 2" << std::endl;

    // We want axis and grid ticks
    file << "set grid mytics ytics ls 100, ls 101" << std::endl;
    file << "set grid mxtics xtics ls 100, ls 101" << std::endl;
    file << "set mxtics 10" << std::endl;
    file << "set mytics 10" << std::endl;

    // Label info in corners of plot
    file << "set label \"Pop: " << population << ", Gen: " << generations << ", Fit: " << best->second[0] << " Pri: " << best->second[1] << "\"  at graph 0.8, 0.05" << std::endl;
    file << "set label \"T: " << best->second[3] << "/" << shiftTime << ", P: " << best->first.size() << "/" << Packages->size() << " W: " << best->second[4] << "/" << weight << "\" at graph 0.8, graph 0.03" << std::endl;
    file << "set label \"Date/Time: " << std::put_time(&dateTime, "%m-%d-%Y %H:%M:%S") << "\"  at graph 0.05, 0.05" << std::endl;

    // State our file containing the data and assign indices to packages and routes.
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

void printHelp(char *argv[]) {
    std::cout << "Usage: " << argv[0] << " filename -warehouse \"warehouse address\" [options]" << std::endl << std::endl;
    std::cout << std::endl;
    std::cout << "This program calculates the most efficient route for a package delivery service" << std::endl;
    std::cout << "given a CSV file of packages to be sent. It outputs a set of delivery addresses" << std::endl;
    std::cout << "to the console and file \"directions.out\" Additionally, a gnuPlot file is output" << std::endl;
    std::cout << "for visual reference and can be invoked via \"gnuplot gnugraph\" to output \"route.png\"" << std::endl << std::endl;
    std::cout << "Options" << std::endl;
    std::cout << "=======" << std::endl;
    std::cout << "filename                        CSV file of packages from delivery clients" << std::endl;
    std::cout << "-warehouse \"warehouse addr\"     Address string of warehouse. eg \"123 Main St N\" (no valid street checking, yet)" << std::endl;
    std::cout << "-pop #                          Population size in GA to use" << std::endl;
    std::cout << "-g #                            Number of generations in GA" << std::endl;
    std::cout << "-t #                            Max minutes in shift (experimental)" << std::endl;
    std::cout << "-w #                            Max weight capacity in ounces (experimental)" << std::endl;
}

int main(int argc, char *argv[]) {
    // Probably not needed anymore.
    srand(time(0));

    // Package list
    vector<Package* > Packages;

    // Client map holding all sending and receiving clients.
    unordered_map<std::string, Client*> ClientMap;
    // Manhattan adjacency matrix
    vector<vector<unsigned int> > matrix;
    // Best fit individual route
    pair<vector<Package* >, vector<double> > best;

    // Return value of function calls.
    int returnValue;

    // Initial commandline parameters.
    int population = 0;
    int generations = 0;
    int weight = 0;
    int shiftTime = 0;

    // Files in and out
    ifstream file;
    ofstream outFile;

    // String to hold filenames
    string inputFile;
    string temp;

    // Make sure we had at least the program name, input file and warehouse street address.
    if (argc <= 3) {
        printHelp(argv);
        // Exit program
        return 1;
    } else {
        // String stream to hold filename
        istringstream inputFileSS(argv[1]);

        try {
            // Extract filename of input file.
            if (!(inputFileSS>>inputFile)) {
                // Error
                std::cout << "Error parsing filename to string" << std::endl;
                printHelp(argv);
                return 1;
            } else if (inputFile == "-help" || inputFile == "--help" || inputFile == "-h" || inputFile == "--h") {
                // We got a help request
                printHelp(argv);
                return 0;
            }

            // Attempt to open the input file.
            file.open(inputFile);

            // Can we open it?
            if (!file) {
                std::cout << "File " << inputFile << " does not exist!" << std::endl;
                return 1;
            }

            // Close file until later
            file.close();

        }


        catch (exception& e) {
            // Caught badness
            std::cout << "Invalid file parameter: " << argv[2] << std::endl;
            std::cout << "Exception: " << e.what() << std::endl;
            return 1;
        }

        // Parse the rest of the options.
        for (int i = 2; i < argc; ++i) {
            // Extracting command switches
            istringstream cmdSwitch(argv[i]);
            string strSwitch;

            // Problems extracting arguments?
            if (!(cmdSwitch >> strSwitch)) {
                std::cout << "Bad switch: " << argv[i] << std::endl;
                return 1;
            }

            // Population switch
            if (strSwitch == "-pop") {
                try {
                    // Get population
                    istringstream ss(argv[i+1]);
                    if (!(ss >> population)) {
                        std::cout << "Invalid number for population: " << argv[i+1] << std::endl;
                        population = 0;
                        return 1;
                    }

                    // Check good population.
                    if (population < 0) {
                        std::cout << std::endl << "*** Negative population is not allowed! ***" << std::endl;
                        population = 0;
                    }
                }
                catch (exception& e) {
                    // Caught bad arguments.
                    std::cout << "Invalid population parameter: " << argv[i + 1] << std::endl;
                    std::cout << "Exception: " << e.what() << std::endl;
                    return 1;
                }
            } else if (strSwitch == "-warehouse") {
                // Warehouse address switch
                istringstream ss(argv[i+1]);
                // Extract
                ss >> warehouse.address;
                // Replace default warehouse address
                while (ss >> temp) {

                    warehouse.address += " " + temp;
                }

            } else if (strSwitch == "-g") {
                // Generations switch
                try {
                    // Extract generations
                    istringstream ss(argv[i+1]);
                    if (!(ss >> generations)) {
                        std::cout << "Invalid number for generations: " << argv[i+1] << std::endl;
                        generations = 0;
                    }

                    if (generations < 0) {
                        std::cout << std::endl << "*** Negative generations are not allowed! ***" << std::endl;
                        generations = 0;
                    }
                }
                catch (exception& e) {
                    std::cout << "Invalid generations parameter: " << argv[i + 1] << std::endl;
                    std::cout << "Exception: " << e.what() << std::endl;
                    return 1;
                }
            } else if (strSwitch == "-t") {
                // Shift time switch
                try {
                    istringstream ss(argv[i+1]);
                    if (!(ss >> shiftTime)) {
                        std::cout << "Invalid number for maximum shift time: " << argv[i+1] << std::endl;
                        shiftTime = 0;
                    }

                    if (shiftTime < 0) {
                        std::cout << std::endl << "*** Negative shift times are not allowed! ***" << std::endl;
                        shiftTime = 0;
                    }
                }
                catch (exception& e) {
                    std::cout << "Invalid shift time parameter: " << argv[i + 1] << std::endl;
                    std::cout << "Exception: " << e.what() << std::endl;
                    return 1;
                }
            } else if (strSwitch == "-w") {
                // Weight switch
                try {
                    istringstream ss(argv[i+1]);
                    if (!(ss >> weight)) {
                        std::cout << "Invalid number for maximum weight limit: " << argv[i+1] << std::endl;
                        weight = 0;
                    }

                    if (weight < 0) {
                        std::cout << std::endl << "*** Negative weight limits are not allowed! ***" << std::endl;
                        weight = 0;
                    }
                }
                catch (exception& e) {
                    std::cout << "Invalid weight parameter: " << argv[i + 1] << std::endl;
                    std::cout << "Exception: " << e.what() << std::endl;
                    return 1;
                }
            }

        }
    }

    // Check for parameters that did not have anything passed into it.
    if (population == 0) {
        std::cout << std::endl << "*** No population given... using default population. ***" << std::endl;
        population = POPULATION;
    }

    if (generations == 0) {
        std::cout << std::endl << "*** No generations given... using default number of generations. ***" << std::endl;
        generations = GENERATIONS;
    }

    if (shiftTime == 0) {
        std::cout << std::endl << "*** No shift time given... using default shift time. ***" << std::endl;
        shiftTime = MAXTIME;
    }

    if (weight == 0) {
        std::cout << std::endl << "*** No generations given... using default maximum weight. ***" << std::endl;
        weight = MAXWEIGHT;
    }
    // Make warehouse the origin.
    Client* originPtr = new Client(warehouse.name, warehouse.address, warehouse.city, warehouse.state, warehouse.zip, 0);
    // Output warehouse coordinates.
    std::cout << warehouse.address << " X: " << originPtr->getCoords().first << " Y: " << originPtr->getCoords().second << std::endl;

    // Make warehouse the first client.
    ClientMap.emplace(warehouse.name + "," + warehouse.address + "," + warehouse.city + "," + warehouse.state + " " + warehouse.zip, originPtr);

    // Read in the new packages and clients.
    returnValue = readFile(inputFile, Packages, ClientMap);

    if (returnValue != 0) {
        std::cout << std::endl << "*** File specified client input file was not found... Exiting ***" << std::endl;
        return 1;
    }

    // Create the matrix...
    matrix = makeMatrix(ClientMap, Packages);


    std::cout << std::endl << "Evolving best route, please wait..." << std::endl;

    // Find best route
    best = runSimulationMixed(Packages, matrix, population, generations, shiftTime, weight);

    // Output bestness
    std::cout << std::endl << "Best OVERALL -> Fit: " << best.second[0] << " Pri: " << best.second[1] << " D: " << best.second[2] << " T: " << best.second[3] << "/" << MAXTIME << " W: " << best.second[4] << "/" << MAXWEIGHT << std::endl;

    // Create truck. Truck wants a weight, so we give it to them.
    Truck truck(best.second[4]);

    // Add route to truck
    truck.addPackageVector(&best.first);

    // Process the packages on the truck.
    truck.processPackages();

    // Get package delivery directions.
    string directions = truck.getDirections();


    // Print the directions
    std::cout << "Route Packages: " << best.first.size() << ", distance: " << best.second[2] << " blocks, weight: " <<  best.second[4] << " ounces." << std::endl;
    std::cout << std::endl << "Directions\r\n==========" << std::endl << directions << std::endl;

    // Output directions
    outFile.open("direction.out");

    // If we could open/create the file, output direction to file.
    if (outFile) {
        outFile << "Route Packages: " << best.first.size() << ", distance: " << best.second[2] << " blocks, weight: " <<  best.second[4] << " ounces." << std::endl;
        outFile << std::endl << "Directions\r\n==========" << std::endl << directions << std::endl;
    }

    // Close it like a good child
    outFile.close();

    // Create a pretty route graph.
    createGraphFile(&Packages, &best, originPtr, population, generations, shiftTime, weight);

    // DONE!
    return 0;
}
