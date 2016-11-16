#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>
using namespace std;

class Client;

//! Package class
/*!
 * This Package class defines a package which was sent by a Client and received by a Client. Each package has a priority, a weight, and two clients (a sender and receiver).
 */
class Package{
    public:
        enum Priority{
            REGULAR,
            TWO_DAY,
            OVERNIGHT
        };

        //! Constructor
        Package(Client*, Client*, const double, const Priority);
 
        //! Destructor
        ~Package();

        //! Sender setter
        void setSender(Client*);
        //! Sender getter
        Client* getSender() const;

        //! Receiver setter
        void setReceiver(Client*);
        //! Receiver getter
        Client* getReceiver() const;

        //! Weight setter
        void setWeight(const double);
        //! Weight getter
        double getWeight() const;

        //! Priority setter
        void setPriority(const Priority);
        //! Priority getter
        Priority getPriority() const;

    private:
        Client *sender;
        Client *receiver;
        double weight;
        Priority priority;
};

#endif //PACKAGE_H
