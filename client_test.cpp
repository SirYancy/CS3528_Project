#include <iostream>
#include <iomanip>
#include "Client.h"
#include "Package.h"
using namespace std;

int main()
{
    Client c1{"Superman", "123 1st St. NW, Bemidji, MN, 56601"};
    Client c2{"Batman", "444 12th Ave. SE, Bemidji, MN 56601"};
    
    Client* c1ptr = &c1;
    Client* c2ptr = &c2;

    Package p1{c1ptr, c2ptr, 10.2, Package::TWO_DAY};

    cout << p1 << endl;
    cout << c1.toString();
    cout << c2.toString();

    return 0;
}
