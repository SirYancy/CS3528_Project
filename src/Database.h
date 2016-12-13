#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <utility>
using namespace std;



class Database
{

    public:
        Database();

        Database( const string &sqlqurrey, const string &clientid, const string &nam, const string &add, const string &st, const string &cit, const string &z);

        void setConnection(const string&);

        //! Getter for connection
        string getConnection() const {return sql;};


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


        //! To String
        /*! Converts client information to string for debugging output.
         * \return String containing client info for std::cout
         */
        string toString() const;




        unsigned int getID() {return ID;};



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


        //! ID of client
        int ID;

        //! Database Connection

        string sql = "";



};
#endif // DATABASE_H
