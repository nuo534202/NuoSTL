#ifndef NUOSTL_CORE_ALGORITHMS_NUO_MIN_HPP_
#define NUOSTL_CORE_ALGORITHMS_NUO_MIN_HPP_

#include <concepts>

namespace nuostl {

template<typename T>
constexpr const T& nuo_min(const T& a, const T& b) {
    if constexpr (requires (const T& x, const T& y) {
        { x < y } -> std::convertible_to<bool>;
    })
    {
        return (b < a) ? b : a;
    } else if constexpr (requires (const T& x, const T& y) {
        { x > y } -> std::convertible_to<bool>;
    })
    {
        return (a > b) ? b : a;
    } else
    {
        /* unreachable, for well-formedness */
        static_assert(!sizeof(T), "nuo_min: T must define operator< or operator>");
        return a;
    }
}

}   /* namespace nuostl*/

#endif