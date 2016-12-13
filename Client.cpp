#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Client.h"
#include "Package.h"
#include <algorithm>
#include <locale>
#include <cctype>
#include <functional>
#include "Utils.h"

using namespace std;
Client::Client() {
}

Client::Client(const string &nam, const string &add, const string &cit, const string &st, const string &z, const int &clientID) {

    // Trim whitespace
    name = trim_copy(nam);
    address = trim_copy(add);
    city = trim_copy(cit);
    state = trim_copy(st);
    zip = trim_copy(z);

    ID = clientID;

    // Parse address into coordinates.
    parseAddress();
}


void Client::setName(const string& n){
    name = n;
}


void Client::setAddress(const string &add, const string &cit, const string &st, const string &z){

    address = add;
    city = cit;
    state = st;
    zip = z;

}

void Client::sendPackage(Package *pack){
    //cout << "Sent: " << pack << endl;
    sentPackages.push_back(pack);
}

vector<Package*> Client::getSentPackages() const{
    return sentPackages;
}

void Client::receivePackage(Package *pack){
    //cout << "Received: " << pack << endl;
    receivedPackages.push_back(pack);
}

vector<Package*> Client::getReceivedPackages() const
{
    return receivedPackages;
}

string Client::toString() const{
    ostringstream output;

    output << endl;
    output << "Client: " << name << endl;
    output << address << endl;
    output << city << ", " << state << " " << zip << endl;
    output << endl << "Sent Packages:" << endl;

    for(unsigned int i = 0; i < sentPackages.size(); i++)
    {
        //cout << p << endl;
        output  << " Received by: " << sentPackages[i]->getReceiver()->getName() << endl
                << "      Weight: " << sentPackages[i]->getWeight() << endl
                << "    Priority: " << sentPackages[i]->getPriorityString() << endl;
    }
    output << endl << "Received Packages: " << endl;
    for(Package* p : receivedPackages)
    {
        output  << " Sent by: " << p->getSender()->getName() << endl;
        output  << "  Weight: " << p->getWeight() << endl << " Priority: " << p->getPriorityString() << endl;
    }
    return output.str();
}

void Client::parseAddress() {
    // Stream to parse individual address chunks
    stringstream parser(address);

    // Address house/building number
    int addressNum = 0;

    // Street number (12th, 1st, 106th)
    int streetNum = 0;

    // Avenue or street? Street = 1, Avenue = -1
    int aveStreet = 0;

    // Parser temp output
    string output;

    parser >> output;

    addressNum = stoi(output, nullptr, 10);

    parser >> output;

    output = strUpper(output);
    if (output == "CENTRAL") {
        // Central avenue runs North-South on X = 0
        coordinates.first = 0;


        // No street number
        streetNum = 0;
    } else if (output == "MAIN") {
        // Main runs East-West on Y = 0
        coordinates.second = 0;

        // No street number
        streetNum = 0;
    } else {
        // We are not on the axis, so need to know what block we're on
        streetNum = stoi(output, nullptr, 10);

    }

    // This will be an avenue or street.
    parser >> output;
    output = strUpper(output);

    if (output.find("AVE") != string::npos) {
        aveStreet = -1;
    } else if (output.find("ST") != string::npos) {
        aveStreet = 1;
    }

    // This should be a cardinal direction.
    parser >> output;
    output = strUpper(output);

    if (output == "N" || output == "NORTH") {
        coordinates.second = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.second += 1;
        }

    } else if (output == "NW" || output == "NORTHWEST") {
        // We have a Northern STREET
        if (aveStreet == 1) {
            // Positive Y
            coordinates.second = streetNum;

            coordinates.first = addressNum / 100;

            // Which side of the middle of the block are we?
            if ((addressNum % 100) > 50) {
                coordinates.first += 1;
            }

            // Negative X
            coordinates.first *= -1;

        } else if (aveStreet == -1) {
            // We have a Western AVENUE

            coordinates.first = streetNum * -1;

            coordinates.second = addressNum / 100;

            if ((addressNum % 100) > 50) {
                coordinates.second += 1;
            }

        }

    } else if (output == "W" || output == "WEST") {
        coordinates.first = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.first += 1;
        }

        // Negative X
        coordinates.first *= -1;

    } else if (output == "SW" || output == "SOUTHWEST") {
        // We have a Southern STREET
        if (aveStreet == 1) {
            // Negative Y
            coordinates.second = streetNum * -1;

            coordinates.first = addressNum / 100;

            // Which side of the middle of the block are we?
            if ((addressNum % 100) > 50) {
                coordinates.first += 1;
            }

            // Negative X
            coordinates.first *= -1;

        } else if (aveStreet == -1) {
            // We have a Western AVENUE
            // Negative X
            coordinates.first = streetNum * -1;

            coordinates.second = addressNum / 100;

            if ((addressNum % 100) > 50) {
                coordinates.second += 1;
            }

            // Negative Y
            coordinates.second *= -1;

        }

    } else if (output == "S" || output == "SOUTH") {
        coordinates.second = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.second += 1;
        }

        // Negative Y
        coordinates.second *= -1;

    } else if (output == "SE" || output == "SOUTHEAST") {
        // We have a Southern STREET
        if (aveStreet == 1) {
            // Positive Y
            coordinates.second = streetNum * -1;

            coordinates.first = addressNum / 100;

            // Which side of the middle of the block are we?
            if ((addressNum % 100) > 50) {
                coordinates.first += 1;
            }


        } else if (aveStreet == -1) {
            // We have a Eastern AVENUE

            coordinates.first = streetNum;

            coordinates.second = addressNum / 100;

            if ((addressNum % 100) > 50) {
                coordinates.second += 1;
            }

            // Negative Y
            coordinates.second *= -1;

        }

    } else if (output == "E" || output == "EAST") {
        coordinates.first = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.first += 1;
        }

    } else if (output == "NE" || output == "NORTHEAST") {
        // We have a Northern STREET
        if (aveStreet == 1) {
            // Positive Y
            coordinates.second = streetNum;

            coordinates.first = addressNum / 100;

            // Which side of the middle of the block are we?
            if ((addressNum % 100) > 50) {
                coordinates.first += 1;
            }

        } else if (aveStreet == -1) {
            // We have a Eastern AVENUE

            coordinates.first = streetNum;

            coordinates.second = addressNum / 100;

            if ((addressNum % 100) > 50) {
                coordinates.second += 1;
            }
        }
    }
}

ostream& operator<<(ostream& output, const Client& client)
{
    output << client.name << endl << client.address << endl << client.city << ", " << client.state << " " << client.zip << endl;
    return output;
}
