#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>
using namespace std;

//! Priority enum for package type
enum class Priority{
    REGULAR = 1,
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
        /*! Creates a package object to track client info, such as address and coordinates. Also track
         *  package age and IDs.
         * \param s Client pointer to sender object.
         * \param r Client pointer to receiver object.
         * \param w Weight of package in ounces.
         * \param p Priority enum of package (overnight, two-day, regular?)
         * \param ID ID number of package. Used in the future to build adjacency matrix.
         * \param packAge The age of the package in days. New packages are born at age 1.
         * \return NULL.
         */
        Package(Client* s, Client* r, const float w, const Priority p, unsigned int ID, int packAge);

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

        unsigned int getID() {return ID;};

        void setID(unsigned int id) {ID = id;};

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

        //! Package ID number.
        unsigned int ID;

        //! Package age in delivery days.
        /*! Starts life at 1 for first day. 2 is second day, etc.
         *  Used for packages not previous delivered. If a package is not delivered
         * (does not make the route cut) it's age is increased. This age is used to
         * add a weight to the algorithm to favor long delayed packages.
         */
        int age;

};

#endif //PACKAGE_H
