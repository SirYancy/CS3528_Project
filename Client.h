#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
using namespace std;

class Package;

//! Client class
/*!
 * This client class defines a citizen of the town who can send and receive packages. Each person has a name, address, and vectors representing records of all packages sent and received.
 */
class Client{
    //! Insertion Operator override
    friend std::ostream& operator<<(std::ostream&, const Client&);
        
    public:
        //! Client basic constructor
        /*!
         * Basic constructor takes strings for name and address.
         */
        Client(const string&,
                const string&);

        //! Setter for name
        void setName(const string&);
        //! Getter for name
        string getName() const;

        //! Setter for address
        void setAddress(const string&);
        //! Getter for address
        string getAddress() const;

        //! Add a sent package
        void sendPackage(Package *pack);
        //! Get sent packages
        vector<Package*> getSentPackages() const;

        //! Add a received package
        void receivePackage(Package *pack);
        //! Get received packages
        vector<Package*> getReceivedPackages() const;

            private:
        //! Client name
        string name;
        //! Client address
        string address;
        //! Vector of packages sent by this client
        vector<Package*> sentPackages;
        //! Vector of packages received by this client
        vector<Package*> receivedPackages;
};

#endif //CLIENT_H
