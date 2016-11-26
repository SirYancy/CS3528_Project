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
#include "Truck.h"

using namespace std;

unordered_map getClients(const string filename);
vector<Package*> makePackages(unordered_map<string, Client*> clientMap);

int main()
{
    unordered_map<string, Client*> clientMap;
    vector<Package*> packages;
    vector<Truck*> trucks;
    vector<vector<unsigned int> > packageAdjMatrix;

    clientMap = getClients("clients.txt");
    packages = makePackages(clientMap);

    Client* origin = new Client("WAREHOUSE", "123 MAIN ST. N", "BEMIDJI", "MN", "56601");
    clientMap.emplace(origin->getName(), origin);

    
    
}

