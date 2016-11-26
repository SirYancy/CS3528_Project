#include <string>
#include <vector>
#include <sstream>
#include "Package.h"
#include "Client.h"
#include <iostream>

using namespace std;

//! Constructor
/*!
 \param s Sender Client
 \param r Receiver Client
 \param w Package weight
 \param p Package priority
 */
Package::Package(Client* s, Client* r, float w, Priority p, unsigned int id, int packAge) : sender(s), receiver(r), priority(p), ID(id), age(packAge) {
    //cout << "Package this: " << this << endl;
    setWeight(w);

    s->sendPackage(this);
    r->receivePackage(this);
}

//! Sender setter
/*!
 \param s pointer to sender client
 */
void Package::setSender(Client *s)
{
    sender = s;
}
//! Sender getter
/*!
 \return pointer to sender client
 */
Client* Package::getSender() const
{
    return sender;
}

//! Receiver setter
/*!
 \param r pointer to receiver client
 */
void Package::setReceiver(Client *r)
{
    receiver = r;
}

//! Receiver getter
/*!
 \return pointer to receiver client
 */
Client* Package::getReceiver() const
{
    return receiver;
}
//! Package weight setter
/*!
 * Throws error on negative weight.
 \param w Package weight
 */
void Package::setWeight(const double w)
{
    if(w < 0.0)
    {
        throw invalid_argument("No package has negative weight");
    }
    weight = w;
}
//! Package Weight getter
/*!
 \return Package weight
 */
float Package::getWeight() const
{
    return weight;
}

//! Package Priority setter
/*!
 \param p Priority enum
 */
void Package::setPriority(const Priority p)
{
    priority = p;
}
//! Package priority getter
/*!
 \return Package priority
 */
Priority Package::getPriority() const
{
    //cout << "Package priority: " << static_cast<int>(priority) << endl;
    return priority;
}

//! Priority string getter
/*!
 * Priority is an enum and hence is actually an int. Here's a nice way to print the string value.
 \return string of priority
 */
string Package::getPriorityString() const {
    string value;

    switch(priority) {
        case Priority::REGULAR:
            value = "Regular";
            break;
        case Priority::TWO_DAY:
            value = "Two Day";
            break;
        case Priority::OVERNIGHT:
            value = "Overnight";
            break;
    }
    return value;
}

//! Insertion Operator
/*!
 \param output output stream
 \param package package object to print
 \return output stream
 */
ostream& operator<<(ostream& output, const Package& pack)
{
    output << "Sender:" << endl
        << *pack.sender << endl
        << "Receiver:" << endl
        << *pack.receiver << endl
        << "Weight: " << pack.weight << endl
        << "Priority: " << pack.getPriorityString() << endl;
    return output;
}
