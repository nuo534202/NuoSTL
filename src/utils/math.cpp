#include "utils/math.hpp"

namespace nuostl
{

int128 gcd(int128 a, int128 b)
{
    return a == 0 ? b : gcd(b % a, a);
}

}   /* namespace nuostl */