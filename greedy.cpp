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

using namespace std;

void getClients(const string &filename, unordered_map<string, Client*> &clientMap);
void makePackages(unordered_map<string, Client*> &clientMap, vector<Package*> &packages, int numPackages);
void getStops(vector<Client*> &destinations, vector<Package*> &packages);
Client* getRandomClient(unordered_map<string, Client*> &clientMap);
vector<vector<unsigned int> > makeMatrix(vector<Client*> &stops);
vector<int> greedyRoute(vector<vector<unsigned int> > &matrix);

int main()
{
    srand(time(NULL));
    unordered_map<string, Client*> clientMap;
    vector<Package*> packages;
    vector<Client*> stops;
    vector<Truck*> trucks;
    vector<vector<unsigned int> > matrix;
    vector<int> order;

    Client* origin = new Client("WAREHOUSE", "123 MAIN ST. N", "BEMIDJI", "MN", "56601");
    clientMap.emplace(origin->getName(), origin);

    getClients("clients.txt", clientMap);
    stops.push_back(origin);

    makePackages(clientMap, packages, 40);
    getStops(stops, packages);

    matrix = makeMatrix(stops);

    cout << "Matrix size: " << matrix.size() << "x" << matrix[0].size() << endl;
    for(int i = 0; i < matrix.size(); i++)
    {
        for(int j = 0; j < matrix.size(); j++)
        {
            cout << setw(3) << matrix[i][j] << " ";
        }
        cout << endl;
    }

    order = greedyRoute(matrix);

    for(int i: order)
    {
        cout << i << endl;
    }

    
    ofstream stopFile("plotstops.py");
    stopFile << "import matplotlib.pyplot as plt\n";
    for(Client* c : stops)
    {
        pair<int,int> coords = c->getCoords();
        ostringstream output;
        output << "plt.plot(" << coords.first << ", " << coords.second << ", 'o')\n";
        cout << output.str();
        stopFile << output.str();
    }
    stopFile << "plt.show()\n";
    stopFile.close();

    ofstream routeFile("plotroute.py");
    routeFile << "import matplotlib.pyplot as plt\n";
    for(int i = 0; i < order.size()-2; i++)
    {
        pair<int,int> c1 = stops[i]->getCoords();
        pair<int,int> c2 = stops[i+1]->getCoords();
        ostringstream output;
        output << "plt.plot([" << c1.first << ", " << c2.first << "], ["
            << c1.second << ", " << c2.second << "], 'k-', lw=2)\n";
        cout << output.str();
        routeFile << output.str();
    }
    routeFile << "plt.show()\n";
    routeFile.close();
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
    file.close();

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
void getStops(vector<Client*> &stops, vector<Package*> &packages)
{
    cout << packages.size();
    for(Package* p : packages)
    {
        stops.push_back(p->getReceiver());
    }
}

vector<vector<unsigned int> > makeMatrix(vector<Client*> &stops)
{
    pair<int, int> receiver1, receiver2;
    int numStops = stops.size();
    vector<vector<unsigned int> > matrix(numStops, vector<unsigned int>(numStops,0));

    for(int i = 0; i < numStops; i++)
    {
        pair<int,int> r = stops[i]->getCoords();
        cout << "" << r.first << "," << r.second << "\n";
        for(int j = 0; j < numStops; j++)
        {
            receiver1 = stops[i]->getCoords();
            receiver2 = stops[j]->getCoords();
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

vector<int> greedyRoute(vector<vector<unsigned int> > &matrix)
{
    int numStops = matrix.size();
    vector<bool> visited(numStops, false);

    unsigned int start = 0;
    visited[start] = true;
    int numVisited = 0;
    
    vector<int> order;
    order.push_back(start);

    while(numVisited < numStops)
    {
        unsigned int shortest = numeric_limits<unsigned int>::max();
        int next = 0;

        for (int i = 0; i < numStops; i++)
        {
            if(i != start && !visited[i] && matrix[start][i] < shortest)
            {
                shortest = matrix[start][i];
                next = i;
            }
        }
        order.push_back(next);
        visited[next] = true;
        start = next;
        numVisited++;
    }
    return order;
}
