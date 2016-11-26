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
    string clientString;
    ifstream clientFile ("address.txt");
   
    if(clientFile.is_open())
    {
        while(getline(clientFile, clientString))
        {
            Client c{clientString};
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
