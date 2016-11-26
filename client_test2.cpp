#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "Utils.h"
#include "Client.h"
#include "Package.h"

using namespace std;

int main()
{
    vector<Client> clients;
    string address;
    ifstream addresses ("address.txt");
   
    if(addresses.is_open())
    {
        while(getline(addresses, address))
        {
            Client c{address};
            cout << c.toString() << endl;
            clients.push_back(c);
        }
    }
   
    for(Client c: clients)
    {
        cout << c.toString() << endl;
    }

    return 0;
}
