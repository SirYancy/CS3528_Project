#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>
using namespace std;

//! Priority enum for package type
enum Priority{
    REGULAR,
    TWO_DAY,
    OVERNIGHT
};

class Client;

//! Package class
/*!
 * This Package class defines a package which was sent by a Client and received by a Client. Each package has a priority, a weight, and two clients (a sender and receiver).
 */
class Package{
    //! Insertion Operator Override
    friend std::ostream& operator<<(std::ostream&, const Package&);

    public:


        //! Constructor
        Package(Client*, Client*, const double, const Priority);

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
        //! Priority string getter
        string getPriorityString() const;

    private:
        //! Pointer to sender client
        Client *sender;
        //! Pointer to receiver client
        Client *receiver;
        //! Weight of package in ounces
        double weight;
        //! Priority enum of package
        Priority priority;

        //! Package delivered?
        bool Delivered = false;

};

#endif //PACKAGE_H
