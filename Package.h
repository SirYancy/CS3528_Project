#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>
using namespace std;

//! Priority enum for package type
enum Priority{
    REGULAR = 1,
    TWO_DAY = 2,
    OVERNIGHT = 3
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
        Package(Client* s, Client* r, const float w, const Priority p);

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
        float getWeight() const;

        //! Priority setter
        void setPriority(const Priority);
        //! Priority getter
        Priority getPriority() const;
        //! Priority string getter
        string getPriorityString() const;

        Package* getPointer() {return this;};

    private:
        //! Pointer to sender client
        Client *sender;
        //! Pointer to receiver client
        Client *receiver;
        //! Weight of package in ounces
        float weight;
        //! Priority enum of package
        Priority priority;

        //! Package delivered?
        bool Delivered = false;

};

#endif //PACKAGE_H
