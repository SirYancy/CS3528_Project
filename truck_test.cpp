#include <iostream>
#include <iomanip>
#include <string>
#include "Client.h"
#include "Package.h"
#include "Truck.h"

using namespace std;

int main()
{
    Truck t{25};
    Truck t2{40};
    
    Priority p = Priority::TWO_DAY;

    cout << t.toString() << endl;
    cout << t2.toString() << endl;

    Client c1{"Superman", "123 1st St. NW", "Bemidji", "MN",  "56601"};
    Client c2{"Batman", "444 12th Ave. SE", "Bemidji", "MN", "56601"};

    Package p1{&c1, &c2, 10.2, p};

    t.addPackage(&p1);

    cout << t.toString() << endl;

    return 0;
}
