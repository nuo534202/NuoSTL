#include "core/data_types/nuo_pair.hpp"

#include <type_traits>

#include "nuo_typedefs.hpp"

namespace nuostl {

/* nuo_pair */

/* private*/
template<typename T1, typename T2>
void nuo_pair<T1, T2>::nuo_type_check(const nuo_pair& np) const {
    static_assert(
        std::is_same<decltype(first), decltype(np.first)>::value,
        "Error: Type of 'first' must be the same!"
    );

    static_assert(
        std::is_same<decltype(second), decltype(np.second)>::value,
        "Error: Type of 'second' must be the same!"
    );
}

/* public */

/* Operator */

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
    return first != np.first && second != np.second;
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