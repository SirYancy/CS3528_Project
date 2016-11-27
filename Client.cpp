#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <locale>
#include <cctype>
#include <functional>
#include "Client.h"
#include "Package.h"
#include "Utils.h"

using namespace std;

int Client::count = 0;

Client::Client(): ID(++count) {
}

Client::Client(const string& clientString)
    : ID{++count}
{
    int nameLoc = clientString.find_first_of(",");

    name = clientString.substr(0, nameLoc);

    setAddress(clientString.substr(nameLoc+1));
}


Client::Client(const string &nam, const string &add, const string &cit, const string &st, const string &z)
    : ID{++count} 
{

    // Trim whitespace
    name = trim_copy(nam);
    address = trim_copy(add);
    city = trim_copy(cit);
    state = trim_copy(st);
    zip = trim_copy(z);

    // Parse address into coordinates.
    parseAddress();
}


void Client::setName(const string& n){
    name = trim_copy(n);
}


void Client::setAddress(const string& addString)
{
    stringstream    clientStream(addString);
    string          clientItem;
    vector<string>  clientVector;

    while(getline(clientStream, clientItem, ','))
    {
        clientVector.push_back(clientItem);
    }

    address =     trim_copy(clientVector[0]);
    city =        trim_copy(clientVector[1]);
    state =       trim_copy(clientVector[2]);
    zip =         trim_copy(clientVector[3]);

    parseAddress();
}

void Client::setAddress(const string &add, const string &cit, const string &st, const string &z){

    address = add;
    city = cit;
    state = st;
    zip = z;

    parseAddress();

}

//! Add a sent package
/*!
 \param pack pointer to package
 */
void Client::sendPackage(Package *pack){
    //cout << "Sent: " << pack << endl;
    sentPackages.push_back(pack);
}
//! Get sent packages
/*!
 \return Vector of sent packages
 */
vector<Package*> Client::getSentPackages() const{
    return sentPackages;
}

//! Add a received package
/*!
 \param pack pointer to package
 */
void Client::receivePackage(Package *pack){
    //cout << "Received: " << pack << endl;
    receivedPackages.push_back(pack);
}
//! Get received packages
/*!
 \return Vector of received packages
 */
vector<Package*> Client::getReceivedPackages() const
{
    return receivedPackages;
}

//! To String
/*!
 * Prints out client's name and address and also all packages associated with the client. Insertion operator only prints name and address.
 \return string representation of client and all sent and received packages
 */
string Client::toString() const{
    ostringstream output;

    output << endl;
    output << "ID: " << ID << endl;
    output << "Client: " << name << endl;
    output << address << endl;
    output << city << ", " << state << " " << zip << endl;
    output << "Coordinates: (" << coordinates.first << "," << coordinates.second << ")" << endl;
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
    int temp = 0;

    // Quadrant of city map. A negative number indicates South or West.
    // (Cartesian coordinate signs)
    int northSouth, eastWest;

    // Parser temp output
    string output;

    parser >> output;

    addressNum = stoi(output, nullptr, 10);

    parser >> output;

    output = strUpper(output);
    if (output == "CENTRAL") {
        // Central avenue runs North-South on X = 0
        coordinates.first = 0;
        // No east or west
        eastWest = 0;

        // No street number
        streetNum = 0;
    } else if (output == "MAIN") {
        // Main runs East-West on Y = 0
        coordinates.second = 0;
        // No north or south
        northSouth = 0;

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

    if (output == "N") {
        northSouth = 1;

        coordinates.second = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.second += 1;
        }

    } else if (output == "NW") {
        northSouth = 1;
        eastWest = -1;

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

    } else if (output == "W") {
        eastWest = -1;

        coordinates.first = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.first += 1;
        }

        // Negative X
        coordinates.first *= -1;

    } else if (output == "SW") {
        eastWest = -1;
        northSouth = -1;

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

    } else if (output == "S") {
        northSouth = -1;

        coordinates.second = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.second += 1;
        }

        // Negative Y
        coordinates.second *= -1;

    } else if (output == "SE") {
        northSouth = -1;
        eastWest = 1;

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

    } else if (output == "E") {
        eastWest = 1;

        coordinates.first = addressNum / 100;

        // Which side of the middle of the block are we?
        if ((addressNum % 100) > 50) {
            coordinates.first += 1;
        }

    } else if (output == "NE") {
        northSouth = 1;
        eastWest = 1;

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


//! Insertion Operator
/*!
 \param output output stream
 \param client client object to print
 \return output stream
 */
ostream& operator<<(ostream& output, const Client& client)
{
    output << client.name << endl << client.address << endl << client.city << ", " << client.state << ", " << client.zip << endl;
    return output;
}


