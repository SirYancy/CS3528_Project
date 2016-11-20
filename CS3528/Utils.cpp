#include "Utils.h"
#include <string>
#include <locale>

std::string strUpper(const std::string& s) {
    std::locale loc;

    std::string value;

    for (unsigned int i = 0; i < s.length(); ++i) {
        value += std::toupper(s.at(i), loc) ;
    }

    //value += coords.first + "," + coords.second;
    return value;
}

