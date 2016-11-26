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

using namespace std;

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
    int integer;
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
        //std::cout << "Input package priority int: " << integer << " Priority: " << packagePriority << std::endl;

        // Make new persistent package that won't disappear off the stack. Get pointer.
        Package* packagePtr = new Package(senderPtr, receiverPtr, weight, packagePriority, packageID);

        packageID++;

        // Add to package list
        packageList.push_back(packagePtr);

    }
}

void randomPackages(const randomPackageEnum& randomConsts) {
    // Holds our names from files
    vector<string> firstNames;
    vector<string> lastNames;

    // Output file stream
    ofstream file;

    // Input file stream
    ifstream names;

    // Holds lines and names from files
    string line;
    string name;

    // Buffer to extract names from files.
    stringstream buffer;

    // Round any odd populations down
    unsigned int halfPop = randomConsts.population / 2;

    // Holds random ints for address creation
    unsigned int randomNum;

    // Holds random floats for probabilities and weights
    float randomFloat;

    // Street number
    unsigned int street;

    // Accumulated P or weights for priority package generation.
    float accumulated_P;

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

    // Start on creating random packages
    // Open file
    file.open(randomConsts.fileName);

    // Loop for number of desired packages
    for (unsigned int i = 0; i < randomConsts.num; i++) {

        // Need a sender and receiver
        for (unsigned int j = 0; j < 2; j++) {
            // Random name
            file << firstNames[rand() % randomConsts.population] << ',' << lastNames[rand() % randomConsts.population] << ',';

            // Random building number
            file << (rand() % randomConsts.maxAddress) + 1 << ' ';

            // Random street number
            street = rand() % randomConsts.maxStreets;

            // Toss up if road is street or avenue
            randomNum = rand() % 2;

            // What appropriate street name do we have?
            if (street == 0) {
                // No 0 street or avenue, choose main or central
                if (randomNum == 0) {
                    file << "Main St ";

                    // North or South?
                    if (rand() % 2 == 0) {
                        file << "N";
                    } else {
                        file << "S";
                    }
                } else {
                    file << "Central Ave ";
                    // East or West?
                    if (rand() % 2 == 0) {
                        file << "W";
                    } else {
                        file << "E";
                    }
                }

                // Add appropriate suffix
            } else if (street == 1) {
                file << street << "st ";
            } else if (street == 2) {
                file << street << "nd ";
            } else if (street == 3) {
                file << street << "rd ";
            } else {
                file << street << "th ";
            }

            // If we did not have a main or central
            if (street != 0) {
                // Choose street or avenue
                if (randomNum == 0) {
                    file << "Street ";
                } else {
                    file << "Avenue ";
                }

                // Since we did not pick main street or central avenue,
                // We need to choose our quadrant
                randomNum = rand() % 4;

                if (randomNum == 0) {
                    file << "NW";
                } else if (randomNum == 1) {
                    file << "NE";
                } else if (randomNum == 2) {
                    file << "SE";
                } else if (randomNum == 3) {
                    file << "SW";
                }
            }

            // No variation for this simulation
            file << ",Bemidji,MN,56601,";
        }
        // Weight of package. Apply tenth ounce truncation by multiplying
        unsigned int weight = static_cast<unsigned int>(floor(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * randomConsts.maxWeight * 10));

        // Truncate weight to a tenth of an ounce.
        file << weight/10 << ',';

        // Start Russian roulette selection.
        // Accumulator is zero
        accumulated_P = 0;

        // Sum all weights or probabilities
        for (unsigned int i = 0; i < 3; i++) {
            accumulated_P += randomConsts.priority[i];
        }

        // Choose a random float up to accumulated_P
        randomFloat = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * accumulated_P;

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
        file << index + 1 << endl;

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

vector< pair<vector<Package* >, float> > simulationIsolation(vector<Package* > Packages, vector<vector<unsigned int> > matrix) {
    vector< pair<vector<Package* >, float> > result;
    vector<float > fit;

    mutation_enum mutation;
    mutation.crossOver  = 70;
    mutation.deleteOld  = 20;
    mutation.insertNew  = 25;
    mutation.inversion  = 20;
    mutation.swapOut    = 25;
    mutation.swapWithin = 25;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, 200, POPULATION, 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.initPopulation();
    result = GA.evolve_threads();
    //fit = GA.fitness(result);
    //return make_pair(result, fit);
    return result;

}

pair<vector<Package* >, vector<float> > simulationCrossover(vector<Package* > Packages, vector<vector<unsigned int> > matrix, vector< pair<vector<Package* >, float> > mixedPop) {
    vector<Package * > result;
    vector<float > fit;

    mutation_enum mutation;
    mutation.crossOver  = 70;
    mutation.deleteOld  = 20;
    mutation.insertNew  = 25;
    mutation.inversion  = 20;
    mutation.swapOut    = 25;
    mutation.swapWithin = 25;
    mutation.elite      = 0.01;

    Genetic GA(Packages, matrix, MAXWEIGHT, 200, POPULATION, 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.loadPopulation(mixedPop);
    result = GA.evolve();
    fit = GA.fitness(result);
    return make_pair(result, fit);
    //return result;

}

pair<vector<Package* >, vector<float> > runSimulationMixed(vector<Package* > Packages, vector<vector<unsigned int> > matrix) {
    vector< pair<vector<Package* >, float> > mixedPopulation;
    pair<vector<Package* >, float> selected;

    vector< pair<vector<Package* >, float> > newPop1, newPop2, newPop3, newPop4;

    // Launch multiple asncronous threads. It seems that one cannot call object methods and get object copies.
    // Launching from within an object requires pointers to that object, leading to collisions and race conditions.
    // Here we call a wrapper, that creates it's own GA object and runs the simulation. Hence, every simulation should be
    // it's own unique snowflake.
    auto f1 = std::async(std::launch::async, simulationIsolation, Packages, matrix);
    auto f2 = std::async(std::launch::async, simulationIsolation, Packages, matrix);
    auto f3 = std::async(std::launch::async, simulationIsolation, Packages, matrix);
    auto f4 = std::async(std::launch::async, simulationIsolation, Packages, matrix);


    auto res1 = f1.get();
    auto res2 = f2.get();
    auto res3 = f3.get();
    auto res4 = f4.get();

    unsigned int resultSize1 = res1.size();
    unsigned int resultSize2 = res2.size();
    unsigned int resultSize3 = res3.size();
    unsigned int resultSize4 = res4.size();

    for (vector< pair<vector<Package* >, float> >::iterator iter = res1.begin(); iter != res1.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    for (vector< pair<vector<Package* >, float> >::iterator iter = res2.begin(); iter != res2.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    for (vector< pair<vector<Package* >, float> >::iterator iter = res3.begin(); iter != res3.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    for (vector< pair<vector<Package* >, float> >::iterator iter = res4.begin(); iter != res4.end(); ++iter) {
        mixedPopulation.push_back(*iter);
    }

    // Fill selection vector with 0s for size of population.
    //vector<unsigned int > roundRobinSelection {mixedPopulation.size(), 0};
    // Fill vector with 0, 1, 2, ..., n
    //std::iota(roundRobinSelection.begin(), roundRobinSelection.end(), 0);

    // Seed nice random number generator.
    std::mt19937 rng(std::random_device{}());

    // Shuffle up the indices.
    std::shuffle(mixedPopulation.begin(), mixedPopulation.end(), rng);

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

    vector<pair<vector<Package* >, vector<float> > > bestMixed;
    bestMixed.push_back(c1.get());
    bestMixed.push_back(c2.get());
    bestMixed.push_back(c3.get());
    bestMixed.push_back(c4.get());

    unsigned int index = 0;
    float bestSoFar = 0;

    for (unsigned int i = 0; i < bestMixed.size(); ++i) {
        if (bestMixed[i].second[0] > bestSoFar) {
            bestSoFar = i;
        }
    }

    return bestMixed[bestSoFar];

}

int main() {

    //srand(time(0));
    srand(201);
    vector<Package* > Packages;
    //vector<Client*> Clients;
    unordered_map<std::string, Client*> ClientMap;
    vector<vector<unsigned int> > matrix;
    pair<vector<Package* >, vector<float> > best;



                                        // fileName, population, num, maxAddress, maxStreets, maxWeight, priority[REG, TWO, OVER]
    randomPackageEnum generatePackages = {"Test2.csv", 200, 100, 2000, 20, 160, {4, 2, 1}};

    // Not guaranteed unique yet (eg, may send package to self, but with different address with small population.)
    //randomPackages(generatePackages);

    // Make warehouse the origin.
    Client* originPtr = new Client(warehouse.name, warehouse.address, warehouse.city, warehouse.state, warehouse.zip, 0);

    ClientMap.emplace(warehouse.name + "," + warehouse.address + "," + warehouse.city + "," + warehouse.state + " " + warehouse.zip, originPtr);

    readFile("Test2.csv", Packages, ClientMap);

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
    /*
    vector< pair<vector<Package* >, float> > result;
    vector<float > fit;

    mutation_enum mutation;
    mutation.crossOver  = 60;
    mutation.deleteOld  = 5;
    mutation.insertNew  = 25;
    mutation.inversion  = 20;
    mutation.swapOut    = 25;
    mutation.swapWithin = 25;
    mutation.elite      = 0;

    Genetic GA(Packages, matrix, MAXWEIGHT, 200, POPULATION, 5, 1, MAXTIME, GENERATIONS, mutation);
    GA.initPopulation();
    GA.evolve();
*/
    std::cout << std::endl << "Best OVERALL -> Fit: " << best.second[0] << " Pri: " << best.second[1] << " D: " << best.second[2] << " T: " << best.second[3] << "/" << MAXTIME << " W: " << best.second[4] << "/" << MAXWEIGHT << std::endl;

    // Output file stream
    ofstream file;

    int xmin = 0;
    int xmax = 0;
    int ymin = 0;
    int ymax = 0;

    file.open("route.gnu");
    for (vector<Package* >::iterator iter = Packages.begin(); iter!=Packages.end(); ++iter) {
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

        file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
    }

    file << std::endl << std::endl;

    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    for (vector<Package* >::iterator iter = best.first.begin(); iter!=best.first.end(); ++iter) {
        file << (*iter)->getReceiver()->getCoords().first << " " << (*iter)->getReceiver()->getCoords().second << std::endl;
    }
    file << originPtr->getCoords().first << " " << originPtr->getCoords().second << std::endl;

    file.close();

    file.open("gnugraph");

    file << "set style line 1 lc rgb \'#0060ad\' lt 1 lw 2 pt 7 ps 1.5 # --- blue" << std::endl;
    file << "set style line 2 lc rgb \'#dd181f\' lt 1 lw 2 pt 5 ps 1.5 # --- red" << std::endl;

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
    file << "set label \"Pop: " << POPULATION << ", Gen: " << GENERATIONS << ", Fit: " << best.second[0] << "\"  at graph 0.8, 0.05" << std::endl;
    file << "set label \"Pri: " << best.second[1] << ", T: " << best.second[3] << "/" << MAXTIME << ", P: " << best.first.size() - 2 << "/" << Packages.size() << "\" at graph 0.8, graph 0.03" << std::endl;

    file << "plot \'route.gnu\' index 0 with points ls 1 title \'Packages\', \'route.gnu\' index 1 with linespoints ls 2 title \'Route\'" << std::endl;
    //file << "pause -1" << std::endl;
    file.close();
/*
    cout << "This works?!?!" << endl;

    Client c1{"Superman", "1587 Main Street E", "Bemidji", "MN", "56601"};

    //cout << "X: " << c1.getCoords().first << " Y: " << c1.getCoords().second << endl;
    Client c2{"Batman", "444 12th Ave. SE", "Bemidji", "MN", "56601"};

    //Client* c1ptr = &c1;
    //Client* c2ptr = &c2;

    // We remove the intermediate step above.
    // Conceptually easier, but a pointer might be what we use anyway.
    Package p1{&c1, &c2, 10.2, Priority::TWO_DAY};
    //Package p2{&c2, &c1, 22.2, Priority::OVERNIGHT};

    c2.sendPackage(&p1);
    c1.receivePackage(&p1);
    //cout << p1 << endl;
    cout << c1.toString();
    cout << c2.toString();

*/

    return 0;
}
