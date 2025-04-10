#include "types.h"

#include <iostream>

std::ostream &operator<<(std::ostream &os, Gender gender)
{
    switch (gender)
    {
    case Gender::MALE:
        os << "Male";
        break;
    case Gender::FEMALE:
        os << "Female";
        break;
    default:
        os << "Unknown";
        break;
    }
    return os;
}
