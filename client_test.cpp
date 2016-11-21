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

void readFile(string fileName, vector<Package> &packageList, vector<Client*> &clientList) {
    vector<string> csvLine;
    string line;
    string token;
    string clientName;
    float weight;
    int integer;
    Priority packagePriority;

    // Create a SS object to extract values
    ifstream file;
    file.open(fileName);

    // Toss first header line
    //getline(file, line);
    int index = 0;
    // Get line from file
    while(getline(file, line)) {
        index++;
        cout << "Line " << index << ": " << line << endl;
        //fileLineStream.flush();
        stringstream fileLineStream(line);

        csvLine.clear();

        while(std::getline(fileLineStream, token, ',')) {
            csvLine.push_back(token);
        }

        cout << "Read tokens: ";
        for (unsigned int i = 0; i < csvLine.size(); i++) {
            cout << csvLine[i] << " ";

        }
        cout << endl;

        clientName = csvLine[0] + " " + csvLine[1];
        // Create a persistent client and reference via pointer. Must call delete when done to free memory.
        Client* senderPtr = new Client(csvLine[0] + " " + csvLine[1], csvLine[2], csvLine[3], csvLine[4], csvLine[5]);
        Client* receiverPtr = new Client(csvLine[6] + " " + csvLine[7], csvLine[8], csvLine[9], csvLine[10], csvLine[11]);
        /*
        clientList.push_back(senderPtr);
        clientList.push_back(receiverPtr);
        */

        clientList.push_back(senderPtr);
        clientList.push_back(receiverPtr);

        cout << "Weight: " << csvLine[12] << endl;
        stringstream conversion(csvLine[12]);
        conversion >> weight;

        conversion.str(csvLine[13]);
        conversion >> integer;
        packagePriority = static_cast<Priority>(integer);
        /*Package* packagePtr = new Package(senderPtr, receiverPtr, weight, packagePriority);
        packageList.push_back(packagePtr);
        */

        packageList.emplace_back(Package(senderPtr, receiverPtr, weight, packagePriority));

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

        // Found our priority, since it is an enum, just output integer.
        file << index << endl;

    }

    file.close();

}

int main() {

    vector<Package> Packages;
    vector<Client*> Clients;

    // fileName, population, num, maxAddress, maxStreets, maxWeight, priority[REG, TWO, OVER]
    randomPackageEnum generatePackages = {"Test1.csv", 10, 2, 2000, 20, 1600, {4, 2, 1}};

    randomPackages(generatePackages);

    readFile("Test1.csv", Packages, Clients);

    cout << "***** PACKAGES *****" << endl;
    for (unsigned int i = 0; i < Packages.size(); i++) {
        cout << "*** PACKAGE " << i << " ***" << endl;
        cout << Packages[i] << endl;
    }
    cout << endl;

    cout << "***** CLIENTS *****" << endl;

    /**********************
    * Cannot print out senders and receivers!
    * Tried many variations with pointers, references and iterators.
    **********************/
    for (auto& clientPtr : Clients) {
        cout << "*** CLIENT ***" << endl;
        //cout << Clients[i] << endl;
        vector<Package*> temp = clientPtr->getReceivedPackages();
        for (auto& packPtr : temp) {
            //Client* tempClient = packPtr->getSender();
            cout << "Sent: " << (*packPtr).getSender()->getName() << endl;
        }
        cout << &temp << endl;
    }

    cout << "This works?!?!" << endl;

    Client c1{"Superman", "1587 Main Street E", "Bemidji", "MN", "56601"};

    //cout << "X: " << c1.getCoords().first << " Y: " << c1.getCoords().second << endl;
    Client c2{"Batman", "444 12th Ave. SE", "Bemidji", "MN", "56601"};

    //Client* c1ptr = &c1;
    //Client* c2ptr = &c2;

    // We remove the intermediate step above.
    // Conceptually easier, but a pointer might be what we use anyway.
    Package p1{&c1, &c2, 10.2, Priority::TWO_DAY};
    Package p2{&c2, &c1, 22.2, Priority::OVERNIGHT};


    //cout << p1 << endl;
    //cout << c1.toString();
    //cout << c2.toString();


    return 0;
}
