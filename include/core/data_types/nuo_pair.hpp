#ifndef NUOSTL_CORE_DATA_TYPES_NUO_PAIR_HPP_
#define NUOSTL_CORE_DATA_TYPES_NUO_PAIR_HPP_

#include <stddef.h>
#include <type_traits>
#include <utility>

namespace nuostl {

template<typename T1, typename T2>
class nuo_pair {
public:
    T1 first;
    T2 second;

    /* Constructor */
    constexpr nuo_pair() : first(), second() {}
    constexpr nuo_pair(const T1& _first, const T2& _second) :
        first(_first), second(_second) {}

    /* Destructor */
    ~nuo_pair() = default;

    /* Copy Constructor */
    constexpr nuo_pair(const nuo_pair&) = default;
    constexpr nuo_pair(nuo_pair&&) = default;

    /* Operator */
    nuo_pair& operator=(const nuo_pair&) = default;
    nuo_pair& operator=(nuo_pair&&) = default;

    bool operator<(const nuo_pair& np) const;
    bool operator<=(const nuo_pair& np) const;
    bool operator>(const nuo_pair& np) const;
    bool operator>=(const nuo_pair& np) const;
    bool operator==(const nuo_pair& np) const;
    bool operator!=(const nuo_pair& np) const;

    nuo_pair operator+(const nuo_pair& np) const;
    nuo_pair operator-(const nuo_pair& np) const;
    nuo_pair operator*(const nuo_pair& np) const;
    nuo_pair operator/(const nuo_pair& np) const;

    nuo_pair& operator+=(const nuo_pair& np);
    nuo_pair& operator-=(const nuo_pair& np);
    nuo_pair& operator*=(const nuo_pair& np);
    nuo_pair& operator/=(const nuo_pair& np);

    const void swap(nuo_pair& np) noexcept (
        noexcept(std::swap(first, np.first)) &&
        noexcept(std::swap(second, np.second))
    ) {
        std::swap(first, np.first), std::swap(second, np.second);
    }
};

template<typename T1, typename T2>
nuo_pair<T1, T2> nuo_make_pair(T1 _first, T2 _second) {
    return nuo_pair<T1, T2>(_first, _second);
}

/* Todo: get */

} /* namespace */

namespace std {

template<typename T1, typename T2>
struct tuple_size<nuostl::nuo_pair<T1, T2>> :
    std::integral_constant<size_t, 2> {};

template<size_t N, typename T1, typename T2>
struct tuple_element<N, nuostl::nuo_pair<T1, T2>> {
    static_assert(N < 2, "tuple_element index out of range for nuo_pair");
    using type = std::conditional_t<N == 0, T1, T2>;
};

} /* namespace */

#endif