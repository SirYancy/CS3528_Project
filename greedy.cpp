#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include <ctime>
#include "Utils.h"
#include "Client.h"
#include "Package.h"
#include "Truck.h"

using namespace std;

void getClients(const string &filename, unordered_map<string, Client*> &clientMap);
void makePackages(unordered_map<string, Client*> &clientMap, vector<Package*> &packages, int numPackages);
Client* getRandomClient(unordered_map<string, Client*> &clientMap);
vector<vector<unsigned int> > makeMatrix(vector<Package*> &packages);

int main()
{
    srand(time(NULL));
    unordered_map<string, Client*> clientMap;
    vector<Package*> packages;
    vector<Truck*> trucks;
    vector<vector<unsigned int> > matrix;

    getClients("clients.txt", clientMap);



    makePackages(clientMap, packages, 25);
   /* 
    for(Package* pack: packages)
    {
        cout << *pack << endl;
    }
    */
    matrix = makeMatrix(packages);
    
    cout << "Matrix size: " << matrix.size() << "x" << matrix[0].size() << endl;

    for(int i = 0; i < matrix.size(); i++)
    {
        for(int j = 0; j < matrix.size(); j++)
        {
            cout << setw(3) << matrix[i][j] << " ";
        }
        cout << endl;
    }
  
    //Client* origin = new Client("WAREHOUSE", "123 MAIN ST. N", "BEMIDJI", "MN", "56601");
    //clientMap.emplace(origin->getName(), origin);

    //packageAdjMatrix = makeMatrix(packages);


}

void getClients(const string &filename, unordered_map<string, Client*> &clientMap)
{
    ifstream file (filename);
    string clientString;

    if(file.is_open())
    {
        while(getline(file, clientString))
        {
            for(auto & ch: clientString)
                ch = toupper(ch); 
            Client* c = new Client(clientString); 
            clientMap.emplace(c->getName(), c);
        }
    }

}

void makePackages(unordered_map<string, Client*> &clientMap, vector<Package*> &packages, int numPackages)
{
    for(int i = 0; i < numPackages; i++)
    {

        Client* sender = getRandomClient(clientMap);
        Client* receiver = getRandomClient(clientMap);

        float weight = rand() % 25;
        Priority priority = static_cast<Priority>(rand() % Priority::OVERNIGHT+1);

        packages.push_back(new Package(sender, receiver, weight, priority));
    }
}

vector<vector<unsigned int> > makeMatrix(vector<Package*> &packages)
{
    pair<int, int> receiver1, receiver2;
    int numPackages = packages.size();
    vector<vector<unsigned int> > matrix(numPackages, vector<unsigned int>(numPackages,0));
    
    for(int i = 0; i < numPackages; i++)
    {
        for(int j = 0; j < numPackages; j++)
        {
            receiver1 = packages[i]->getReceiver()->getCoords();
            receiver2 = packages[j]->getReceiver()->getCoords();
            cout << "(" << i << "," << j << ")" << endl;
            cout << receiver1.first << ":" << receiver1.second << endl;
            cout << receiver2.first << ":" << receiver2.second << endl;


            matrix[i][j] = abs(receiver1.first - receiver2.first) + 
                           abs(receiver1.second - receiver2.second);
        }
    }
    return matrix;
}

Client* getRandomClient(unordered_map<string, Client*> &clientMap)
{
    int rand_index = rand() % clientMap.size();
    auto clientIter = clientMap.begin();
    advance(clientIter, rand_index);
    return clientIter->second;
}
