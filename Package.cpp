#include <string>
#include <vector>
#include <sstream>
#include "Package.h"
#include "Client.h"
#include <iostream>

using namespace std;


Package::Package(Client* s, Client* r, float w, Priority p, unsigned int id, int packAge) : sender(s), receiver(r), priority(p), ID(id), age(packAge) {
    //cout << "Package this: " << this << endl;
    setWeight(w);

    s->sendPackage(this);
    r->receivePackage(this);
}

void Package::setSender(Client *s)
{
    sender = s;
}

Client* Package::getSender() const
{
    return sender;
}

void Package::setReceiver(Client *r)
{
    receiver = r;
}

Client* Package::getReceiver() const
{
    return receiver;
}

void Package::setWeight(const double w)
{
    if(w < 0.0)
    {
        throw invalid_argument("No package has negative weight");
    }
    weight = w;
}

float Package::getWeight() const
{
    return weight;
}

void Package::setPriority(const Priority p)
{
    priority = p;
}

Priority Package::getPriority() const
{
    //cout << "Package priority: " << static_cast<int>(priority) << endl;
    return priority;
}


string Package::getPriorityString() const {

    string value;

    // Figure out string to return
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
