#ifndef NUOSTL_CORE_DATA_TYPES_NUO_PAIR_HPP_
#define NUOSTL_CORE_DATA_TYPES_NUO_PAIR_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>

namespace nuostl {

template<typename T1, typename T2>
class nuo_pair {
private:
    void nuo_type_check(const nuo_pair& np) const {
        static_assert(
            std::is_same<decltype(first), decltype(np.first)>::value,
            "Error: Type of 'first' must be the same!"
        );

        static_assert(
            std::is_same<decltype(second), decltype(np.second)>::value,
            "Error: Type of 'second' must be the same!"
        );
    }
public:
    T1 first;
    T2 second;

    /* Constructor */
    constexpr nuo_pair() noexcept(
        std::is_nothrow_default_constructible_v<T1> &&
        std::is_nothrow_default_constructible_v<T2>
    ) : first(), second() {}
    constexpr nuo_pair(const T1& _first, const T2& _second) noexcept(
        std::is_nothrow_copy_constructible_v<T1> &&
        std::is_nothrow_copy_constructible_v<T2>
    ) :
        first(_first), second(_second) {}

    /* Destructor */
    ~nuo_pair() = default;

    /* Copy Constructor */
    constexpr nuo_pair(const nuo_pair&) = default;
    constexpr nuo_pair(nuo_pair&&) = default;

    /* Operator */
    /* Group 0 */
    nuo_pair& operator=(const nuo_pair&) = default;
    nuo_pair& operator=(nuo_pair&&) = default;

    /* Group 1 */
    bool operator<(const nuo_pair& np) const;
    bool operator<=(const nuo_pair& np) const;
    bool operator>(const nuo_pair& np) const;
    bool operator>=(const nuo_pair& np) const;
    bool operator==(const nuo_pair& np) const;
    bool operator!=(const nuo_pair& np) const;

    /* Group 2 */
    nuo_pair operator+(const nuo_pair& np) const;
    nuo_pair operator-(const nuo_pair& np) const;
    nuo_pair operator*(const nuo_pair& np) const;
    nuo_pair operator/(const nuo_pair& np) const;

    /* Group 3 */
    nuo_pair& operator+=(const nuo_pair& np);
    nuo_pair& operator-=(const nuo_pair& np);
    nuo_pair& operator*=(const nuo_pair& np);
    nuo_pair& operator/=(const nuo_pair& np);

    void swap(nuo_pair& np) noexcept (
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

/* ------------------------------ */
/* Out-of-class template definitions */

/* Group 1 */
template<typename T1, typename T2>
bool nuo_pair<T1, T2>::operator<(const nuo_pair& np) const {
    nuo_type_check(np);
    if (first == np.first) return second < np.second;
    else return first < np.first;
}

template<typename T1, typename T2>
bool nuo_pair<T1, T2>::operator<=(const nuo_pair& np) const {
    nuo_type_check(np);
    if (first == np.first) return second <= np.second;
    else return first < np.first;
}

template<typename T1, typename T2>
bool nuo_pair<T1, T2>::operator>(const nuo_pair& np) const {
    nuo_type_check(np);
    if (first == np.first) return second > np.second;
    else return first > np.first;
}

template<typename T1, typename T2>
bool nuo_pair<T1, T2>::operator>=(const nuo_pair& np) const {
    nuo_type_check(np);
    if (first == np.first) return second >= np.second;
    else return first > np.first;
}

template<typename T1, typename T2>
bool nuo_pair<T1, T2>::operator==(const nuo_pair& np) const {
    nuo_type_check(np);
    return first == np.first && second == np.second;
}

template<typename T1, typename T2>
bool nuo_pair<T1, T2>::operator!=(const nuo_pair& np) const {
    nuo_type_check(np);
    return !(first == np.first && second == np.second);
}

/* Group 2 */
template<typename T1, typename T2>
nuo_pair<T1, T2> nuo_pair<T1, T2>::operator+(const nuo_pair& np) const {
    nuo_type_check(np);
    return nuo_make_pair(first + np.first, second + np.second);
}

template<typename T1, typename T2>
nuo_pair<T1, T2> nuo_pair<T1, T2>::operator-(const nuo_pair& np) const {
    nuo_type_check(np);
    return nuo_make_pair(first - np.first, second - np.second);
}

template<typename T1, typename T2>
nuo_pair<T1, T2> nuo_pair<T1, T2>::operator*(const nuo_pair& np) const {
    nuo_type_check(np);
    return nuo_make_pair(first * np.first, second * np.second);
}

template<typename T1, typename T2>
nuo_pair<T1, T2> nuo_pair<T1, T2>::operator/(const nuo_pair& np) const {
    nuo_type_check(np);
    return nuo_make_pair(first / np.first, second / np.second);
}

/* Group 3 */
template<typename T1, typename T2>
nuo_pair<T1, T2>& nuo_pair<T1, T2>::operator+=(const nuo_pair& np) {
    nuo_type_check(np);
    first += np.first, second += np.second;
    return *this;
}

template<typename T1, typename T2>
nuo_pair<T1, T2>& nuo_pair<T1, T2>::operator-=(const nuo_pair& np) {
    nuo_type_check(np);
    first -= np.first, second -= np.second;
    return *this;
}

template<typename T1, typename T2>
nuo_pair<T1, T2>& nuo_pair<T1, T2>::operator*=(const nuo_pair& np) {
    nuo_type_check(np);
    first *= np.first, second *= np.second;
    return *this;
}

template<typename T1, typename T2>
nuo_pair<T1, T2>& nuo_pair<T1, T2>::operator/=(const nuo_pair& np) {
    nuo_type_check(np);
    first /= np.first, second /= np.second;
    return *this;
}

} /* namespace nuostl */

namespace std {

/* for left value */
template<size_t N, typename T1, typename T2>
constexpr auto& get(nuostl::nuo_pair<T1, T2>& p) noexcept {
    static_assert(N < 2, "Index out of range in nuo_pair::get");
    if constexpr (N == 0) return p.first;
    else return p.second;
}

/* for const left value */
template<size_t N, typename T1, typename T2>
constexpr const auto& get(const nuostl::nuo_pair<T1, T2>& p) noexcept {
    static_assert(N < 2, "Index out of range in nuo_pair::get");
    if constexpr (N == 0) return p.first;
    else return p.second;
}

/* for right value */
template<size_t N, typename T1, typename T2>
constexpr auto&& get(nuostl::nuo_pair<T1, T2>&& p) noexcept {
    static_assert(N < 2, "Index out of range in nuo_pair::get");
    if constexpr (N == 0) return std::move(p.first);
    else return std::move(p.second);
}

template<typename T1, typename T2>
struct tuple_size<nuostl::nuo_pair<T1, T2>> :
    std::integral_constant<size_t, 2> {};

template<size_t N, typename T1, typename T2>
struct tuple_element<N, nuostl::nuo_pair<T1, T2>> {
    static_assert(N < 2, "tuple_element index out of range for nuo_pair");
    using type = std::conditional_t<N == 0, T1, T2>;
};

} /* namespace std */

#endif