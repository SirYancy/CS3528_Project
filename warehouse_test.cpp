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
#include <limits>
#include "Utils.h"
#include "Client.h"
#include "Package.h"
#include "Truck.h"
#include "Warehouse.h"

using namespace std;

void getClients(const string &filename, vector<Client*> &clients);
void makePackages(vector<Client*> clients, vector<Package*> &packages, int numPackages);
Client* getRandomClient(vector<Client*> clients);

int main()
{
    Warehouse* w = new Warehouse();
    vector<Client*> clients;
    vector<Package*> packages;

    getClients("clients.txt", clients);
    makePackages(clients, packages, 40);

    for (Client* c : clients)
    {
        w->addClient(c);
    }

    w->addPackages(packages);
    w->makeTruck(100);
    
    vector<Package*> packs = w->getUndelivered();
    for(Package* p : packs)
    {
        cout << p->getPriorityString() << endl;
    }
    

    return 0;
}

void getClients(const string &filename, vector<Client*> &clients) 
{
    ifstream file (filename);
    string clientString;

    if(file.is_open())
    {
        while(getline(file, clientString))
        {
            clients.push_back(new Client(clientString));
        }
    }
    file.close();

}

void makePackages(vector<Client*> clients, vector<Package*> &packages, int numPackages)
{
    for(int i = 0; i < numPackages; i++)
    {

        Client* sender = getRandomClient(clients);
        Client* receiver = getRandomClient(clients);

        float weight = rand() % 25;
        Priority priority = static_cast<Priority>(rand() % Priority::NONE);

        packages.push_back(new Package(sender, receiver, weight, priority));
    }
}

Client* getRandomClient(vector<Client*> clients)
{
    int rand_index = rand() % clients.size();
    auto clientIter = clients.begin();
    advance(clientIter, rand_index);
    return *clientIter;
}


