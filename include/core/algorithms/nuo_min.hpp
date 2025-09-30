#ifndef NUOSTL_CORE_ALGORITHMS_NUO_MIN_HPP_
#define NUOSTL_CORE_ALGORITHMS_NUO_MIN_HPP_

#include <concepts>
#include <initializer_list>
#include <iterator>

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

template<typename T, typename... Ts>
constexpr const T nuo_min(const T& a, const T& b, const Ts&... rest) {
    const T& ans = nuo_min(a, b);
    if (sizeof...(rest) == 0) {
        return ans;
    } else {
        return nuo_min(ans, rest...);
    }
}

template<typename T>
constexpr T nuo_min(std::initializer_list<T> ilist) {
    if (ilist.size() == 0)
        return T{};
    
    auto it = ilist.begin();
    T ans = *it;
    it++;
    for (; it != ilist.end(); it++)
        ans = nuo_min(ans, *it);

    return ans;
}

template<typename Iter>
constexpr auto nuo_min(Iter first, Iter last)
        -> typename std::iterator_traits<Iter>::value_type {
    using T = typename std::iterator_traits<Iter>::value_type;
    if (first == last)
        return T{};

    T ans = *first;
    first++;
    for (; first != last; first++)
        ans = nuo_min(ans, *first);

    return ans;
}

}   /* namespace nuostl*/

#endif