#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>

using namespace std;

string makeAddress()
{
    string roads [] = {"Ave.", "St."};

    pair<int,int> c (rand() % -41 + (-20), rand() % 41 + (-20));
    ostringstream address;

    string quadrant = "";
    if(c.second < 0)
        quadrant += "S";
    else if( c.second > 0)
        quadrant += "N";
    if(c.first < 0)
        quadrant += "W";
    else if(c.first > 0)
        quadrant += "E";

    int r = rand() % 1;

    ostringstream street;

    if(c.first == 0)
        street << abs(c.second * 100 + (rand() % 99)) << " Central Ave.";
    else if(c.second == 0)
        street << abs(c.first * 100 + (rand() % 99)) << " Main St.";
    else
    {
        if(r == 0)
        {
            street << abs(c.first * 100 + (rand() % 99)) << " " << abs(c.second) << " " << roads[r];
        }
        else
        {
            street << abs(c.second * 100 + (rand() % 99)) << " " << abs(c.first) << " " << roads[r];
        }
    }

    address << street.str() << " " << quadrant << ", Bemidji, MN, 56601";

    return address.str();
}



int main()
{
    string name;
    ifstream names ("names.txt");
    ofstream addresses ("address.txt");

    if(names.is_open() && addresses.is_open())
    {
        while( getline(names, name) )
        {
            addresses << name << ", " << makeAddress() << endl;
        }
        names.close();
        addresses.close();
    }
    else
    {
        cout << "Unable to open a file";
    }
    return 0;
}
