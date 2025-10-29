#ifndef NUOSTL_CORE_UTILS_MATH_HPP_
#define NUOSTL_CORE_UTILS_MATH_HPP_

#include <type_traits>

#include "nuo_typedefs.hpp"

namespace nuostl
{

template<typename T>
constexpr T nuo_bit_ceil(T x)
{
    static_assert(std::is_integral_v<T>, "bit_ceil requires an integral type");

    if (x <= 0)
        return static_cast<T>(0);

    if (x < 2)
        return static_cast<T>(1);

    if ((x & (x - 1)) == 0)
        return x;
    
    using UT = std::make_unsigned_t<T>;
    UT ux = static_cast<UT>(x);
    
    ux--;
    ux |= (ux >> 1);
    ux |= (ux >> 2);
    ux |= (ux >> 4);
    if constexpr (sizeof(UT) > 1)
        ux |= (ux >> 8);
    if constexpr (sizeof(UT) > 2)
        ux |= (ux >> 16);
    if constexpr (sizeof(UT) > 4)
        ux |= (ux >> 32);
    ux++;

    return static_cast<T>(ux);
}

}   /* namespace nuostd */

#endif