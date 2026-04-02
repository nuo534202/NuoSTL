#pragma once

#include <type_traits>

namespace nuostl
{

/* helper struct */
template <typename T, T v>
class NuoIntegralConstant
{
public:
  static constexpr T kValue = v;
};

template <bool bv>
using NuoBoolConstant = NuoIntegralConstant<bool, bv>;

using NuoTrueType = NuoBoolConstant<true>;
using NuoFalseType = NuoBoolConstant<false>;

/* type traits */

/* forward declaration */
template <typename T1, typename T2>
struct NuoPair;

template <typename T>
class NuoIsPair : public NuoFalseType {};

template <typename T1, typename T2>
class NuoIsPair<NuoPair<T1, T2>> : public NuoTrueType {};

} /* namespace nuostl */