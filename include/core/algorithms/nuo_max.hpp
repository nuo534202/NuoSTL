#ifndef NUOSTL_CORE_ALGORITHMS_NUO_MAX_HPP_
#define NUOSTL_CORE_ALGORITHMS_NUO_MAX_HPP_

#include <concepts>

namespace nuostl {

template<typename T>
constexpr const T& nuo_max(const T& a, const T& b) {
    if constexpr (requires (const T& x, const T& y) {
        { x < y } -> std::convertible_to<bool>;
    })
    {
        return (a < b) ? b : a;
    } else if constexpr (requires (const T& x, const T& y) {
        { x > y } -> std::convertible_to<bool>;
    })
    {
        return (a > b) ? a : b;
    } else
    {
        /* unreachable, for well-formedness */
        static_assert(!sizeof(T), "nuo_max: T must define operator< or operator>");
        return a;
    }
}

}   /* namespace nuostl */

#endif