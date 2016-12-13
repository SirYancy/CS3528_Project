#include "Utils.h"
#include <string>
#include <locale>

std::string strUpper(const std::string& s) {
    std::locale loc;

    // Current uppercased value.
    std::string uppered;

    // Iterate over string and convert each character.
    for (unsigned int i = 0; i < s.length(); ++i) {
        uppered += std::toupper(s.at(i), loc) ;
    }

    return uppered;
}

