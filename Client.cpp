#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Client.h"
#include "Package.h"
using namespace std;

//! Constructor
/*!
 \param &name reference to client's name
 \param &address reference to client's address
*/
Client::Client(
        const string &name,
        const string &address)
    : name(name), address(address) {}

//! Setter for name
/*!
 \param n reference to name string
 */
void Client::setName(const string& n){
    name = n;
}
//! Getter for name
/*!
 \return name
 */
string Client::getName() const{
    return name;
}

//! Setter for address
/*!
 \param a reference to address string
 */
void Client::setAddress(const string& a){
    address = a;
}
//! Getter for address
/*!
 \return Street Address
 */
string Client::getAddress() const{
    return address;
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
    output << endl
        << "Name: " << name << endl
        << "Address: " << address << endl
        << "Sent Packages:" << endl;
    for(Package* p : sentPackages)
    {
        output << " Received by: " << p->getSender()->getName() << endl
            << "  Weight: " << p->getWeight() << " Priority: " << p->getPriorityString() << endl;
    }
    output << "Received Packages: " << endl;
    for(Package* p : receivedPackages)
    {
        output << " Sent by: " << p->getReceiver()->getName() << endl
            << "  Weight: " << p->getWeight() << " Priority: " << p->getPriorityString() << endl;
    }
    return output.str();
}


//! Insertion Operator
/*!
 \param output output stream
 \param client client object to print
 \return output stream
 */
ostream& operator<<(ostream& output, const Client& client)
{
    output << client.name << endl << client.address << endl;
    return output;
}
