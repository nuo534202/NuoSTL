#pragma once

#include <cstddef>
#include <type_traits>

#include "utils/nuo_util.hpp"

namespace nuostl
{

/* The standard library declaration is
   template <typename T, T... Values> struct integer_sequence;
   cppreference.com/w/cpp/utility/integer_sequence.html */
template <typename T, T... Values>
struct NuoIntegerSequence
{
  using value_type = T;

  static constexpr std::size_t size() noexcept
  {
    return sizeof...(Values);
  }
};

template <std::size_t... Values>
using NuoIndexSequence = NuoIntegerSequence<std::size_t, Values...>;

namespace detail
{

template <typename T, typename First, typename Second, T Offset>
struct NuoIntegerSequenceConcat;

template <typename T, T... First, typename Second, T Offset>
struct NuoIntegerSequenceConcat<
  T, NuoIntegerSequence<T, First...>, Second, Offset>
{
  template <T... SecondValues>
  static NuoIntegerSequence<
    T, First..., static_cast<T>(SecondValues + Offset)...>
  Apply(NuoIntegerSequence<T, SecondValues...>);

  using type = decltype(Apply(Second()));
};

template <typename T, T N, bool Odd = (N % 2 != 0), bool Zero = (N == 0)>
struct NuoMakeIntegerSequenceImpl;

template <typename T, T N>
struct NuoMakeIntegerSequenceImpl<T, N, false, true>
{
  using type = NuoIntegerSequence<T>;
};

template <typename T, T N>
struct NuoMakeIntegerSequenceImpl<T, N, false, false>
{
  using half = typename NuoMakeIntegerSequenceImpl<
    T, static_cast<T>(N / 2)>::type;
  using type = typename NuoIntegerSequenceConcat<
    T, half, half, static_cast<T>(half::size())>::type;
};

template <typename T, T N>
struct NuoMakeIntegerSequenceImpl<T, N, true, false>
{
  using previous = typename NuoMakeIntegerSequenceImpl<
    T, static_cast<T>(N - 1)>::type;
  using type = typename NuoIntegerSequenceConcat<
    T, previous, NuoIntegerSequence<T, static_cast<T>(N - 1)>,
    static_cast<T>(0)>::type;
};

} /* namespace detail */

template <typename T, T N>
using NuoMakeIntegerSequence =
  typename detail::NuoMakeIntegerSequenceImpl<T, N>::type;

template <std::size_t N>
using NuoMakeIndexSequence = NuoMakeIntegerSequence<std::size_t, N>;

template <typename... Types>
using NuoIndexSequenceFor = NuoMakeIndexSequence<sizeof...(Types)>;

/* The standard library declaration is
   template <typename T, typename U = T>
   constexpr T exchange(T& obj, U&& new_value);
   cppreference.com/w/cpp/utility/exchange.html */
template <typename T, typename U = T>
constexpr T NuoExchange(T& object, U&& new_value)
{
  T old_value = NuoMove(object);
  object = NuoForward<U>(new_value);
  return old_value;
}

/* The standard library declaration is
   template <typename T> constexpr add_const_t<T>& as_const(T& t) noexcept;
   template <typename T> void as_const(const T&&) = delete;
   cppreference.com/w/cpp/utility/as_const.html */
template <typename T>
constexpr std::add_const_t<T>& NuoAsConst(T& value) noexcept
{
  return value;
}

template <typename T>
void NuoAsConst(const T&&) = delete;

/* The standard library declaration is
   template <typename T> add_rvalue_reference_t<T> declval() noexcept;
   cppreference.com/w/cpp/utility/declval.html */
template <typename T>
std::add_rvalue_reference_t<T> NuoDeclval() noexcept;

} /* namespace nuostl */
