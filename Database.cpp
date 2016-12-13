#include "Database.h"
#include "stdio.h"
#include "sqlite3.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Client.h"
#include "Package.h"
#include <algorithm>
#include <locale>
#include <cctype>
#include <functional>
#include <fstream>
#include <unordered_map>
#include "Utils.h"

using namespace std;

//!Sqlite call back function

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

//!changing an int to string
string int_to_str(int n) {
	stringstream s;
	s << n;
	return s.str();
}

//!Sqlite callback function count
static int callback_count(void *count, int argc, char **argv, char **azColName) {
	int* c = (int*)count;
	*c = atoi(argv[0]);
	return 0;
}

Database::Database(const string &filename) {
	file = filename;

	openDatabase();

	ifstream in;
	in.open(filename);

	vector<string> data;

	char *zErrMsg = 0;

//!If file is open
	if (in.is_open()) {
		int idc = 0;

		sqlite3_exec((sqlite3*)db, "SELECT COUNT(*) FROM ClientInfo", callback_count, &idc, &zErrMsg);

		while (in.good()) {
			string line;
			getline(in, line);

			stringstream ss(line);
			string token;

			while (std::getline(ss, token, ',')) {
				data.push_back(token);
			}

			int t = 0;
			string first = data[t++];
			string last = data[t++];
			string addr = data[t++];
			string city = data[t++];
			string state = data[t++];
			string zip = data[t++];

			sql = "INSERT INTO ClientInfo (clientid,firstname, lastname, address, city, state, zipcode) VALUES ('" +
				int_to_str(idc++) + "','" + first + "','" + last + "','" + addr + "','" + city + "','" + state + "','" + zip + "')";

			int rc = sqlite3_exec((sqlite3*)db, sql.c_str(), callback, 0, &zErrMsg);

			if (rc == 1) {
				rc = sqlite3_exec((sqlite3*)db, "CREATE TABLE ClientInfo(clientid PRIMARY KEY, firstname VARCHAR(100), lastname VARCHAR(100), address VARCHAR(100), city VARCHAR(100), state VARCHAR(100), zipcode VARCHAR(100))", callback, 0, &zErrMsg);
				rc = sqlite3_exec((sqlite3*)db, sql.c_str(), callback, 0, &zErrMsg);
			}

			if (rc != SQLITE_OK) {
				fprintf(stderr, "SQL error: %s\n", zErrMsg);
				sqlite3_free(zErrMsg);
			}
			else {
				fprintf(stdout, "Records created successfully\n");
			}
		}
		sql = "SELECT * FROM ClientInfo where clientid = '2'";

		int rc = sqlite3_exec((sqlite3*)db, sql.c_str(), callback, 0, &zErrMsg);


		sqlite3_close((sqlite3*)db);
	}
	in.close();

	closeDatabase();
}


void Database::closeDatabase() {
    // Close database
    sqlite3_close((sqlite3*)db);
}

void Database::openDatabase() {
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char *SqlInfo;

   /* Open database */
   sqlite3* dbb = (sqlite3*)db;
   rc = sqlite3_open("client.db", &dbb);
   db = dbb;
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg((sqlite3*)db));
      //return(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
}

