#include "Move.h"
#include <sstream>

std::string Move::toString() const {
    std::ostringstream oss;
    char fileFrom = 'a' + (from % 8);
    char rankFrom = '1' + (from / 8);
    char fileTo = 'a' + (to % 8);
    char rankTo = '1' + (to / 8);
    oss << fileFrom << rankFrom << fileTo << rankTo;
    if (promotion) oss << promotion;
    return oss.str();
}
