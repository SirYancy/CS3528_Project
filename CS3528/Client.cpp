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

Client::Client(const string &nam, const string &add, const string &cit, const string &st, const string &z) {

    // Trim whitespace
    name = trim_copy(nam);
    address = trim_copy(add);
    city = trim_copy(cit);
    state = trim_copy(st);
    zip = trim_copy(z);
}

//! Setter for name
/*!
 \param n reference to name string
 */
void Client::setName(const string& n){
    name = n;
}

//! Setter for address
/*!
 \param a reference to address string
 */
void Client::setAddress(const string &a, const string &c, const string &s, const string &z){

    address = a;
    city = c;
    state = s;
    zip = z;

}

//! Add a sent package
/*!
 \param pack pointer to package
 */
void Client::sendPackage(Package *pack){
    sentPackages.push_back(pack);
}
//! Get sent packages
/*!
 \return Vector of sent packages
 */
vector<Package *> Client::getSentPackages() const{
    return sentPackages;
}

//! Add a received package
/*!
 \param pack pointer to package
 */
void Client::receivePackage(Package *pack){
    receivedPackages.push_back(pack);
}
//! Get received packages
/*!
 \return Vector of received packages
 */
vector<Package *> Client::getReceivedPackages() const
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
    output << "Client: " << name << endl;
    output << address << endl;
    output << city << ", " << state << " " << zip << endl;
    output << endl << "Sent Packages:" << endl;

    for(Package* p : sentPackages)
    {
        output  << " Received by: " << p->getReceiver()->getName() << endl
                << "      Weight: " << p->getWeight() << endl
                << "    Priority: " << p->getPriorityString() << endl;
    }
    output << endl << "Received Packages: " << endl;
    for(Package* p : receivedPackages)
    {
        output  << " Sent by: " << p->getSender()->getName() << endl;
        output  << "  Weight: " << p->getWeight() << endl << " Priority: " << p->getPriorityString() << endl;
    }
    return output.str();
}

string Client::hashable() {
    std::locale loc;

    string value;

    for (unsigned int i = 0; i < name.length(); ++i) {
        value += std::toupper(name.at(i), loc) ;
    }

    value += coords.first + "," + coords.second;
    return value;
}


//! Insertion Operator
/*!
 \param output output stream
 \param client client object to print
 \return output stream
 */
ostream& operator<<(ostream& output, const Client& client)
{
    output << client.name << endl << client.address << endl << client.city << ", " << client.state << " " << client.zip << endl;
    return output;
}
