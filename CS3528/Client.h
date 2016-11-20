#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include <utility>

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
         * Basic constructor.
         * \param name Name of client.
         * \param address Street address for client.
         * \param city City of client
         * \param state State of client
         * \param zip ZIP code of client. Note string usage for international packages.
         */
        Client(const string &name, const string &address, const string &city, const string &state, const string &zip);

        //! Setter for name
        void setName(const string&);

        //! Getter for name
        string getName() const {return name;};

        //! Setter for address
        /*! Updates address for client.
         * \param address Street address for client.
         * \param city City of client
         * \param state State of client
         * \param zip ZIP code of client. Note string usage for international packages.
         */
        void setAddress(const string &address, const string &city, const string &state, const string &zip);

        //! Getter for address
        /*! \return String of street address
         */
        string getAddress() const {return address;};

        //! Getter for city
        string getCity() const {return city;};

        //! Getter for state
        string getState() const {return state;};

        //! Getter for zip
        string getZip() const {return zip;};

        //! Add a sent package
        void sendPackage(Package *pack);

        //! Get sent packages
        vector<Package*> getSentPackages() const;

        //! Add a received package
        void receivePackage(Package *pack);
        //! Get received packages
        vector<Package*> getReceivedPackages() const;

        //! To String
        string toString() const;

        void parseAddress();

        pair<int, int> getCoords() {return coordinates;};

    private:
        //! Client name
        string name = "";

        //! Client street address
        string address = "";

        //! Client city
        string city = "";

        //! Client state
        string state = "";

        //! Client zip. Note as string due to possible upgrades to none numeric zips (eg, UK.)
        string zip = "";


        //! Vector of packages sent by this client
        vector<Package*> sentPackages;
        //! Vector of packages received by this client
        vector<Package*> receivedPackages;

        //! Coordinate pair of address in the city.
        pair<int, int> coordinates;
};

#endif //CLIENT_H
