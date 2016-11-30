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
#define PACKAGE_LIMIT 1000

using namespace std;

// Seed nice random number generator.
std::mt19937 rng(std::random_device{}());

struct {
    string name = "WAREHOUSE";
    string address = "2240 21 ST NW";
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
        }
    }

    return matrix;
}

vector< pair<vector<Package* >, Genetic::geneInfo> > simulationIsolation(vector<Package* > Packages, vector<vector<unsigned int> > matrix) {
    vector< pair<vector<Package* >, Genetic::geneInfo> > result;
    vector<float > fit;

    mutation_enum mutation;
    mutation.crossOver  = 0.70;
    mutation.deleteOld  = 0.002;
    mutation.insertNew  = 0.002;
    mutation.inversion  = 0.005;
    mutation.swapOut    = 0.002;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, PACKAGE_LIMIT, POPULATION, 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.initPopulation();
    result = GA.evolve_threads();

    return result;

}

vector< pair<vector<Package* >, Genetic::geneInfo> >  simulationIntermingle(vector<Package* > Packages, vector<vector<unsigned int> > matrix, vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPop) {
    vector< pair<vector<Package* >, Genetic::geneInfo> > resultPopulation;

    vector<double > fit;

    mutation_enum mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.002;
    mutation.insertNew  = 0.002;
    mutation.inversion  = 0.005;
    mutation.swapOut    = 0.002;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, PACKAGE_LIMIT, mixedPop.size(), 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.loadPopulation(mixedPop);
    resultPopulation = GA.evolve_threads();
    return resultPopulation;

}

pair<vector<Package* >, vector<double> > simulationCrossover(vector<Package* > Packages, vector<vector<unsigned int> > matrix, vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPop) {
    vector<Package * > resultFitest;
    vector<double > fit;

    mutation_enum mutation;
    mutation.crossOver  = 0.90;
    mutation.deleteOld  = 0.002;
    mutation.insertNew  = 0.002;
    mutation.inversion  = 0.005;
    mutation.swapOut    = 0.002;
    mutation.swapWithin = 0.005;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, PACKAGE_LIMIT, mixedPop.size(), 5, 1, MAXTIME, GENERATIONS, mutation);

    GA.loadPopulation(mixedPop);
    resultFitest = GA.evolve();
    fit = GA.fitness(&resultFitest);
    return make_pair(resultFitest, fit);


}

pair<vector<Package* >, vector<double> > runSimulationMixed(vector<Package* > Packages, vector<vector<unsigned int> > matrix) {
    vector< pair<vector<Package* >, Genetic::geneInfo> > mixedPopulation, returnedPopulation, newPop1, newPop2, newPop3, newPop4;
    pair<vector<Package* >, vector<double> > selected;

    std::cout << std::endl << "Running isolated islands..." << std::endl << std::endl;
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

    std::cout << std::endl << "Intermingling islands..." << std::endl << std::endl;

    // Mix the population up, and run a combined simulation (eg, land bridges allow population to intermingle.
    mixedPopulation = simulationIntermingle(Packages, matrix, mixedPopulation);

    // Shuffle up the big intermixed population.
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

    std::cout << std::endl << "Running isolated islands again..." << std::endl << std::endl;

    // Launch evolution again, with the mixed up and shuffled isolated populations. This time
    // we return only the fittest individual genome.
    auto c1 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop1);
    auto c2 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop2);
    auto c3 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop3);
    auto c4 = std::async(std::launch::async, simulationIntermingle, Packages, matrix, newPop4);

    res1 = c1.get();
    res2 = c2.get();
    res3 = c3.get();
    res4 = c4.get();

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

    // Mix the population up, and run a combined simulation (eg, land bridges allow population to intermingle.
    selected = simulationCrossover(Packages, matrix, mixedPopulation);
    /*
    // Grab the best route and stuff in here
    vector<pair<vector<Package* >, vector<double> > > bestMixed;
    bestMixed.push_back(c1.get());
    bestMixed.push_back(c2.get());
    bestMixed.push_back(c3.get());
    bestMixed.push_back(c4.get());

    // Which route is the bestest!
    unsigned int bestSoFar = 0;
    // Best fitness score?
    double bestScore = 0;

    for (unsigned int i = 0; i < bestMixed.size(); ++i) {
        if (bestMixed[i].second[0] > bestScore) {
            bestSoFar = i;
            bestScore = bestMixed[i].second[0];
        }
    }

    // Return best of the isolated and mixed populations.
    return bestMixed[bestSoFar];
*/
    return selected;
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




    // Make warehouse the origin.
    Client* originPtr = new Client(warehouse.name, warehouse.address, warehouse.city, warehouse.state, warehouse.zip, 0);

    ClientMap.emplace(warehouse.name + "," + warehouse.address + "," + warehouse.city + "," + warehouse.state + " " + warehouse.zip, originPtr);

    readFile("Cluster5.csv", Packages, ClientMap);

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
