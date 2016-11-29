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
    ifstream clientFile ("clients.txt");
   
    getline(clientFile, clientString);
    Client* c1 = new Client(clientString);
    
    getline(clientFile, clientString);
    Client* c2 = new Client(clientString);

    getline(clientFile, clientString);
    Client* c3 = new Client(clientString);

    clientFile.close();
    
    Package* p1 = new Package(c1, c2, 1.5, Priority::OVERNIGHT);
    Package* p2 = new Package(c2, c1, 2.5, Priority::TWO_DAY);
    Package* p3 = new Package(c3, c1, 5.3, Priority::REGULAR);

    cout << "P1" << endl << *p1 << endl;
    cout << "P2" << endl << *p2 << endl;
    cout << "P3" << endl << *p3 << endl;

    cout << "P1 Priority: " << p1->getPriority() << endl;
    cout << "P2 Priority: " << p2->getPriority() << endl;
    cout << "P3 Priority: " << p3->getPriority() << endl << endl;

    cout << "p1 > p2: "  << (*p1 >  *p2) << endl;
    cout << "p1 == p2: " << (*p1 == *p2) << endl;
    cout << "p1 < p2: "  << (*p1 <  *p2) << endl;
    cout << "p2 == p2: " << (*p2 == *p2) << endl;
    cout << "p3 > p2: "  << (*p3 >  *p2) << endl;
    cout << "p3 > p1: "  << (*p3 >  *p1) << endl;
    cout << "p3 < p1: "  << (*p3 <  *p1) << endl;

    return 0;

}
