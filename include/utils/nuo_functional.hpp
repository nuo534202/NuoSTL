#pragma once

#include <stddef.h>

#include "nuo_typedefs.hpp"

namespace nuostl
{

/* function type class definition */

template <typename Arg, typename Result>
class NuoUnaryFunction
{
public:
  using argument_type = Arg;
  using result_type   = Result;
};

template <typename Arg1, typename Arg2, typename Result>
class NuoBinaryFunction
{
public:
  using argument1_type  = Arg1;
  using argument2_type  = Arg2;
  using result_type     = Result;;
};

/* arithmetic operation class */

template <typename T>
class NuoPlus : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x + y; }
};

template <typename T>
class NuoMinus : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x - y; }
};

template <typename T>
class NuoMultiplies : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x * y; }
};

template <typename T>
class NuoDivides : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x / y; };
};

template <typename T>
class NuoModulus : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x % y; }
};

template <typename T>
class NuoNegate : public NuoUnaryFunction<T, T>
{
public:
  T operator()(const T& x) const { return -x; }
};

/* identity element class */

template <typename T>
T NuoIdentityElement(NuoPlus<T>) { return T(0); }

template <typename T>
T NuoIdentityElement(NuoMultiplies<T>) { return T(1); }

/* compare operation class */

template <typename T>
class NuoEqualTo : public NuoBinaryFunction<T, T, bool>
{
public:
  bool operator()(const T& x, const T& y) const { return x == y; }
};

template <typename T>
class NuoNotEqualTo : public NuoBinaryFunction<T, T, bool>
{
public:
  T operator()(const T& x, const T& y) const { return x != y; }
};

template <typename T>
class NuoLess : public NuoBinaryFunction<T, T, bool>
{
  T operator()(const T& x, const T& y) const { return x < y; }
};

template <typename T>
class NuoGreater : public NuoBinaryFunction<T, T, bool>
{
public:
  T operator()(const T& x, const T& y) const { return x > y; }
};

template <typename T>
class NuoLessEqual : public NuoBinaryFunction<T, T, bool>
{
public:
  T operator()(const T& x, const T& y) const { return x <= y; }
};

template <typename T>
class NuoGreaterEqual : public NuoBinaryFunction<T, T, bool>
{
public:
  T operator()(const T& x, const T& y) const { return x >= y; }
};

/* logic and bitwise operation class */

template <typename T>
class NuoLogicalAnd : public NuoBinaryFunction<T, T, bool>
{
public:
  bool operator()(const T& x, const T& y) const { return x && y; }
};

template <typename T>
class NuoLogicalOr : public NuoBinaryFunction<T, T, bool>
{
public:
  bool operator()(const T& x, const T& y) const { return x || y; }
};

template <typename T>
class NuoLogicalNot : public NuoUnaryFunction<T, bool>
{
  bool operator()(const T& x) const { return !x; }
};

template <typename T>
class NuoBitwiseAnd : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x & y; }
};

template <typename T>
class NuoBitwiseOr : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x | y; }
};

template <typename T>
class NuoBitwiseXor : public NuoBinaryFunction<T, T, T>
{
public:
  T operator()(const T& x, const T& y) const { return x ^ y; }
};

template <typename T>
class NuoBitwiseNot : public NuoUnaryFunction<T, T>
{
public:
  T operator()(const T& x) const { return (~x); }
};

/* identity class */

template <typename T>
class NuoIdentity : public NuoUnaryFunction<T, T>
{
  const T& operator()(const T& x) const { return x; }
};

/* select pair class */

template <typename Pair>
class NuoSelectFirst :
  public NuoUnaryFunction<Pair, typename Pair::first_type>
{
public:
  const typename Pair::first_type&
  operator()(const Pair& p) const
  {
    return p.first;
  }
};

template <typename Pair>
class NuoSelectSecond :
  public NuoUnaryFunction<Pair, typename Pair::second_type>
{
public:
  const typename Pair::second_type&
  operator()(const Pair& p) const
  {
    return p.second;
  }
};

/* project class */

template <typename Arg1, typename Arg2>
class NuoProjectFirst : public NuoBinaryFunction<Arg1, Arg2, Arg1>
{
  Arg1 operator()(const Arg1& x, const Arg2& y) const { return x; }
};

template <typename Arg1, typename Arg2>
class NuoProjectSecond : public NuoBinaryFunction<Arg1, Arg2, Arg2>
{
public:
  Arg2 operator()(const Arg1& x, const Arg2& y) const { return y; }
};

/*****************************************************************************/
/* hash function class */

template <typename Key>
class NuoHash {};

template <typename T>
class NuoHash<T*>
{
public:
  size_t operator()(T* pt) const noexcept
  {
    return reinterpret_cast<size_t>(pt);
  }
};

#define NUOSTL_TRIVIAL_HASH_FUNC(Type) \
template <> \
class NuoHash<Type> \
{ \
public: \
  size_t operator()(Type val) const noexcept \
  { \
    return static_cast<size_t>(val); \
  } \
};

NUOSTL_TRIVIAL_HASH_FUNC(bool)

NUOSTL_TRIVIAL_HASH_FUNC(char)
NUOSTL_TRIVIAL_HASH_FUNC(signed char)
NUOSTL_TRIVIAL_HASH_FUNC(unsigned char)
NUOSTL_TRIVIAL_HASH_FUNC(wchar_t)
NUOSTL_TRIVIAL_HASH_FUNC(char16_t)
NUOSTL_TRIVIAL_HASH_FUNC(char32_t)

NUOSTL_TRIVIAL_HASH_FUNC(int16)
NUOSTL_TRIVIAL_HASH_FUNC(int32)
NUOSTL_TRIVIAL_HASH_FUNC(int64)
NUOSTL_TRIVIAL_HASH_FUNC(uint16)
NUOSTL_TRIVIAL_HASH_FUNC(uint32)
NUOSTL_TRIVIAL_HASH_FUNC(uint64)

#undef NUOSTL_TRIVIAL_HASH_FUNC

/* bitwise hash for float, double, long double */
inline size_t
NuoBitwiseHash(const unsigned char* num, size_t count)
{
#if (_MSC_VER && _WIN64) || ((__GNUC__ || __clang__) &&__SIZEOF_POINTER__ == 8)
  const size_t fnv_offset = 14695981039346656037ull;
  const size_t fnv_prime = 1099511628211ull;
#else
  const size_t fnv_offset = 2166136261u;
  const size_t fnv_prime = 16777619u;
#endif

  size_t result = fnv_offset;
  for (idx_t i = 0; i < count; i++)
  {
    result ^= (size_t)num[i];
    result *= fnv_prime;
  }
  return result;
}

template <>
class NuoHash<float>
{
public:
  size_t operator()(const float& val)
  {
    return val == 0.0f ? 0 :
      NuoBitwiseHash((const unsigned char*)&val, sizeof(float));
  }
};

template <>
class NuoHash<double>
{
public:
  size_t operator()(const double& val)
  {
    return val == 0.0f ? 0 :
      NuoBitwiseHash((const unsigned char*)&val, sizeof(double));
  }
};

template <>
class NuoHash<long double>
{
public:
  size_t operator()(const long double& val)
  {
    return val == 0.0f ? 0 :
      NuoBitwiseHash((const unsigned char*)&val, sizeof(long double));
  }
};

} /* namespace nuostl */