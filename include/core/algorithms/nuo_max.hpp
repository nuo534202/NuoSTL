#ifndef NUOSTL_CORE_ALGORITHMS_NUO_MAX_HPP_
#define NUOSTL_CORE_ALGORITHMS_NUO_MAX_HPP_

#include <concepts>
#include <initializer_list>
#include <iterator>

namespace nuostl {

template<typename T>
constexpr const T& nuo_max(const T& a, const T& b) {
    if constexpr (requires(const T& x, const T& y) {
                                    { x < y } -> std::convertible_to<bool>;
                                }) {
        return (a < b) ? b : a;
    } else if constexpr (requires(const T& x, const T& y) {
            { x > y } -> std::convertible_to<bool>;
        })
    {
        return (a > b) ? a : b;
    } else {
        /* unreachable, for well-formedness */
        static_assert(!sizeof(T),
            "nuo_max: T must define operator< or operator>");
        return a;
    }
}

template<typename T, typename... Ts>
constexpr const T nuo_max(const T& a, const T& b, const Ts&... rest) {
    const T& ans = nuo_max(a, b);
    if constexpr (sizeof...(rest) == 0) {
        return ans;
    } else {
        return nuo_max(ans, rest...);
    }
}

template<typename T>
constexpr T nuo_max(std::initializer_list<T> ilist) {
    if (ilist.size() == 0) {
        return T{};
    }
    auto it = ilist.begin();
    T best = *it;
    it++;
    for (; it != ilist.end(); it++) {
        best = nuo_max(best, *it);
    }
    return best;
}

template<typename Iter>
constexpr auto nuo_max(Iter first, Iter last)
        -> typename std::iterator_traits<Iter>::value_type {
    using T = typename std::iterator_traits<Iter>::value_type;
    if (first == last) {
        return T{};
    }
    T best = *first;
    first++;
    for (; first != last; ++first) {
        best = nuo_max(best, *first);
    }
    return best;
}

} /* namespace nuostl */

#endif