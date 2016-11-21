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
         * \param nam Name of client.
         * \param add Street address for client.
         * \param cit City of client
         * \param st State of client
         * \param z ZIP code of client. Note string usage for international packages.
         */
        Client(const string &nam, const string &add, const string &cit, const string &st, const string &z);

        //! Setter for name
        void setName(const string&);

        //! Getter for name
        string getName() const {return name;};

        //! Setter for address
        /*! Updates address for client.
         * \param add Street address for client.
         * \param cit City of client
         * \param st State of client
         * \param z ZIP code of client. Note string usage for international packages.
         */
        void setAddress(const string &add, const string &cit, const string &st, const string &z);

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
        /*! Converts client information to string for debugging output.
         * \return String containing client info for std::cout
         */
        string toString() const;

        //! Returns coordinate pair of client.
        /*! \return Coordinate pair of client on City grid.
         */
        pair<int, int> getCoords() {return coordinates;};

        Client* getPointer() {return this;};

    private:
        //! Parses address into coordinates
        /*! Parses the address stored in object variables to Cartesian coordinates.
         * Main Street runs East/West at \f$Y=0\f$.
         * Central Avenue runs North/South at \f$X=0\f$.
         * All avenues are N/S, while all streets are E/W.
         * Building numbers are modulus 100, and rounded up or down to the nearest integer.
         */
        void parseAddress();

        //! Client name
        string name = "TEST";

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
