#include <string>
#include <vector>
#include <sstream>
#include "Package.h"
#include "Client.h"

using namespace std;

//! Constructor
/*!
 \param s Sender Client
 \param r Receiver Client
 \param w Package weight
 \param p Package priority
 */
Package::Package(
        Client *s,
        Client *r,
        const double w,
        const Package::Priority p)
    : sender(s), receiver(r), priority(p)
{ 
    setWeight(w);
}

//! Destructor
Package::~Package()
{
    delete sender;
    delete receiver;
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
double Package::getWeight() const
{
    return weight;
}

//! Package Priority setter
/*!
 \param p Priority enum
 */
void Package::setPriority(const Package::Priority p)
{
    priority = p;
}
//! Package priority getter
/*!
 \return Package priority
 */
Package::Priority Package::getPriority() const
{
    return priority;
}


