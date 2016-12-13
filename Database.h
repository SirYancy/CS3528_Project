#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <utility>
#include <map>
using namespace std;

typedef struct {
    string name;
    string addr;
    string city;
    string state;
    string zip;
    int clientID;
} client_struct;



class Database
{

public:
	Database() {}

	Database(const string &filename);




	void openDatabase();
	void closeDatabase();




private:

	string sql = "";

    //! Database handle
	void* db;

    //! Database filename
    string file;


};
#endif // DATABASE_H
