#pragma once

#include <cmath>
#include <limits>
#include <ostream>
#include <type_traits>

namespace nuostl
{

/* cppreference declaration: template< class T > class complex;
   https://en.cppreference.com/w/cpp/numeric/complex.html
   A complex number with the usual arithmetic and the standard mathematical
   functions, plus a few helpers std::complex does not provide. */
template <typename T>
class NuoComplex
{
public:
  using value_type = T;

  constexpr NuoComplex(const T& real = T(), const T& imag = T())
    : real_(real)
    , imag_(imag)
  {
  }

  template <typename U>
  constexpr NuoComplex(const NuoComplex<U>& other)
    : real_(static_cast<T>(other.real()))
    , imag_(static_cast<T>(other.imag()))
  {
  }

  constexpr T real() const
  {
    return real_;
  }

  constexpr T imag() const
  {
    return imag_;
  }

  constexpr void real(const T& value)
  {
    real_ = value;
  }

  constexpr void imag(const T& value)
  {
    imag_ = value;
  }

  constexpr NuoComplex& operator=(const T& value)
  {
    real_ = value;
    imag_ = T();
    return *this;
  }

  constexpr NuoComplex& operator+=(const T& value)
  {
    real_ += value;
    return *this;
  }

  constexpr NuoComplex& operator-=(const T& value)
  {
    real_ -= value;
    return *this;
  }

  constexpr NuoComplex& operator*=(const T& value)
  {
    real_ *= value;
    imag_ *= value;
    return *this;
  }

  constexpr NuoComplex& operator/=(const T& value)
  {
    real_ /= value;
    imag_ /= value;
    return *this;
  }

  constexpr NuoComplex& operator+=(const NuoComplex& other)
  {
    real_ += other.real_;
    imag_ += other.imag_;
    return *this;
  }

  constexpr NuoComplex& operator-=(const NuoComplex& other)
  {
    real_ -= other.real_;
    imag_ -= other.imag_;
    return *this;
  }

  constexpr NuoComplex& operator*=(const NuoComplex& other)
  {
    /* Both components are computed first so that value *= value works. */
    const T real = real_ * other.real_ - imag_ * other.imag_;
    const T imag = real_ * other.imag_ + imag_ * other.real_;
    real_ = real;
    imag_ = imag;
    return *this;
  }

  constexpr NuoComplex& operator/=(const NuoComplex& other)
  {
    const T denominator = other.real_ * other.real_ +
      other.imag_ * other.imag_;
    const T real = (real_ * other.real_ + imag_ * other.imag_) / denominator;
    const T imag = (imag_ * other.real_ - real_ * other.imag_) / denominator;
    real_ = real;
    imag_ = imag;
    return *this;
  }

private:
  T real_;
  T imag_;
};

/* scalar assignment and compound operators */

template <typename T>
constexpr NuoComplex<T> operator+(const NuoComplex<T>& value)
{
  return value;
}

template <typename T>
constexpr NuoComplex<T> operator-(const NuoComplex<T>& value)
{
  return NuoComplex<T>(-value.real(), -value.imag());
}

/* complex with complex */

template <typename T>
constexpr NuoComplex<T> operator+(const NuoComplex<T>& lhs,
                                  const NuoComplex<T>& rhs)
{
  NuoComplex<T> result(lhs);
  result += rhs;
  return result;
}

template <typename T>
constexpr NuoComplex<T> operator-(const NuoComplex<T>& lhs,
                                  const NuoComplex<T>& rhs)
{
  NuoComplex<T> result(lhs);
  result -= rhs;
  return result;
}

template <typename T>
constexpr NuoComplex<T> operator*(const NuoComplex<T>& lhs,
                                  const NuoComplex<T>& rhs)
{
  NuoComplex<T> result(lhs);
  result *= rhs;
  return result;
}

template <typename T>
constexpr NuoComplex<T> operator/(const NuoComplex<T>& lhs,
                                  const NuoComplex<T>& rhs)
{
  NuoComplex<T> result(lhs);
  result /= rhs;
  return result;
}

template <typename T>
constexpr bool operator==(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs)
{
  return lhs.real() == rhs.real() && lhs.imag() == rhs.imag();
}

template <typename T>
constexpr bool operator!=(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs)
{
  return !(lhs == rhs);
}

/* complex with a scalar, in both orders */

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator+(const NuoComplex<T>& lhs, const U& rhs)
{
  NuoComplex<T> result(lhs);
  result += static_cast<T>(rhs);
  return result;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator+(const U& lhs, const NuoComplex<T>& rhs)
{
  return rhs + lhs;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator-(const NuoComplex<T>& lhs, const U& rhs)
{
  NuoComplex<T> result(lhs);
  result -= static_cast<T>(rhs);
  return result;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator-(const U& lhs, const NuoComplex<T>& rhs)
{
  return NuoComplex<T>(static_cast<T>(lhs)) - rhs;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator*(const NuoComplex<T>& lhs, const U& rhs)
{
  NuoComplex<T> result(lhs);
  result *= static_cast<T>(rhs);
  return result;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator*(const U& lhs, const NuoComplex<T>& rhs)
{
  return rhs * lhs;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator/(const NuoComplex<T>& lhs, const U& rhs)
{
  NuoComplex<T> result(lhs);
  result /= static_cast<T>(rhs);
  return result;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr NuoComplex<T> operator/(const U& lhs, const NuoComplex<T>& rhs)
{
  return NuoComplex<T>(static_cast<T>(lhs)) / rhs;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr bool operator==(const NuoComplex<T>& lhs, const U& rhs)
{
  return lhs == NuoComplex<T>(static_cast<T>(rhs));
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr bool operator==(const U& lhs, const NuoComplex<T>& rhs)
{
  return rhs == lhs;
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr bool operator!=(const NuoComplex<T>& lhs, const U& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename U>
  requires std::is_arithmetic_v<U>
constexpr bool operator!=(const U& lhs, const NuoComplex<T>& rhs)
{
  return !(lhs == rhs);
}

/* mixed component types */

template <typename T, typename U>
constexpr NuoComplex<std::common_type_t<T, U>> operator+(
  const NuoComplex<T>& lhs, const NuoComplex<U>& rhs)
{
  using Result = NuoComplex<std::common_type_t<T, U>>;
  return Result(lhs) + Result(rhs);
}

template <typename T, typename U>
constexpr NuoComplex<std::common_type_t<T, U>> operator-(
  const NuoComplex<T>& lhs, const NuoComplex<U>& rhs)
{
  using Result = NuoComplex<std::common_type_t<T, U>>;
  return Result(lhs) - Result(rhs);
}

template <typename T, typename U>
constexpr NuoComplex<std::common_type_t<T, U>> operator*(
  const NuoComplex<T>& lhs, const NuoComplex<U>& rhs)
{
  using Result = NuoComplex<std::common_type_t<T, U>>;
  return Result(lhs) * Result(rhs);
}

template <typename T, typename U>
constexpr NuoComplex<std::common_type_t<T, U>> operator/(
  const NuoComplex<T>& lhs, const NuoComplex<U>& rhs)
{
  using Result = NuoComplex<std::common_type_t<T, U>>;
  return Result(lhs) / Result(rhs);
}

template <typename T, typename U>
constexpr bool operator==(const NuoComplex<T>& lhs, const NuoComplex<U>& rhs)
{
  using Result = NuoComplex<std::common_type_t<T, U>>;
  return Result(lhs) == Result(rhs);
}

template <typename T, typename U>
constexpr bool operator!=(const NuoComplex<T>& lhs, const NuoComplex<U>& rhs)
{
  return !(lhs == rhs);
}

/* component access and structure */

template <typename T>
constexpr T NuoReal(const NuoComplex<T>& value)
{
  return value.real();
}

template <typename T>
constexpr T NuoImag(const NuoComplex<T>& value)
{
  return value.imag();
}

template <typename T>
constexpr NuoComplex<T> NuoConj(const NuoComplex<T>& value)
{
  return NuoComplex<T>(value.real(), -value.imag());
}

template <typename T>
constexpr T NuoNorm(const NuoComplex<T>& value)
{
  return value.real() * value.real() + value.imag() * value.imag();
}

/* NuoAbs uses hypot so that overflow and infinite inputs behave like abs. */
template <typename T>
T NuoAbs(const NuoComplex<T>& value)
{
  return std::hypot(value.real(), value.imag());
}

template <typename T>
T NuoArg(const NuoComplex<T>& value)
{
  return std::atan2(value.imag(), value.real());
}

template <typename T>
NuoComplex<T> NuoPolar(const T& magnitude, const T& angle = T())
{
  return NuoComplex<T>(magnitude * std::cos(angle),
                       magnitude * std::sin(angle));
}

template <typename T>
NuoComplex<T> NuoProj(const NuoComplex<T>& value)
{
  if (std::isinf(value.real()) || std::isinf(value.imag()))
  {
    const T zero = T();
    return NuoComplex<T>(std::numeric_limits<T>::infinity(),
                         std::copysign(zero, value.imag()));
  }
  return value;
}

/* exponential family */

template <typename T>
NuoComplex<T> NuoExp(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  /* Non finite arguments follow the dedicated results of the C library. */
  if (std::isnan(imag))
  {
    if (real == infinity)
    {
      return NuoComplex<T>(infinity, nan);
    }
    if (real == -infinity)
    {
      return NuoComplex<T>(T(), T());
    }
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(imag))
  {
    if (std::isinf(real))
    {
      if (real > T())
      {
        /* Only the +i infinity limit has a dedicated result. */
        return imag > T() ? NuoComplex<T>(infinity, nan)
                          : NuoComplex<T>(nan, nan);
      }
      return NuoComplex<T>(T(), std::copysign(T(), imag));
    }
    return NuoComplex<T>(nan, nan);
  }

  if (std::isnan(real))
  {
    /* cexp(NaN + i0) keeps the signed zero of the imaginary part. */
    return imag == T() ? NuoComplex<T>(nan, std::copysign(T(), imag))
                       : NuoComplex<T>(nan, nan);
  }

  const T magnitude = std::exp(real);
  const T cosine = std::cos(imag);
  const T sine = std::sin(imag);
  if (std::isinf(magnitude))
  {
    /* The magnitude saturated, so signed zeros are kept instead of turning
       into NaN through an infinite product. */
    const T real_part = cosine == T() ? std::copysign(T(), cosine)
                                      : std::copysign(infinity, cosine);
    const T imag_part = sine == T() ? std::copysign(T(), sine)
                                    : std::copysign(infinity, sine);
    return NuoComplex<T>(real_part, imag_part);
  }
  return NuoComplex<T>(magnitude * cosine, magnitude * sine);
}

template <typename T>
NuoComplex<T> NuoLog(const NuoComplex<T>& value)
{
  return NuoComplex<T>(std::log(NuoAbs(value)), NuoArg(value));
}

template <typename T>
NuoComplex<T> NuoLog10(const NuoComplex<T>& value)
{
  const T divisor = std::log(T(10));
  return NuoLog(value) / divisor;
}

template <typename T>
NuoComplex<T> NuoSqrt(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  /* Infinite and NaN inputs follow the dedicated results of the C library. */
  if (std::isnan(imag))
  {
    if (real == infinity)
    {
      return NuoComplex<T>(infinity, nan);
    }
    if (real == -infinity)
    {
      return NuoComplex<T>(nan, infinity);
    }
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(imag))
  {
    /* csqrt(x + i infinity) is +infinity + i sign(imag) * infinity, and it
       also covers a NaN real part. */
    return NuoComplex<T>(infinity, std::copysign(infinity, imag));
  }
  if (std::isnan(real))
  {
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(real))
  {
    if (real > T())
    {
      return NuoComplex<T>(infinity, std::copysign(T(), imag));
    }
    return NuoComplex<T>(T(), std::copysign(infinity, imag));
  }

  const T magnitude = std::hypot(real, imag);
  const T positive = std::sqrt((magnitude + real) / T(2));
  T negative = (magnitude - real) / T(2);
  if (negative < T())
  {
    /* Rounding can make the term slightly negative for large real parts. */
    negative = T();
  }
  const T root = std::sqrt(negative);
  return std::signbit(imag) ? NuoComplex<T>(positive, -root)
                            : NuoComplex<T>(positive, root);
}

template <typename T>
NuoComplex<T> NuoPow(const NuoComplex<T>& base, const NuoComplex<T>& exponent)
{
  return NuoExp(exponent * NuoLog(base));
}

template <typename T>
NuoComplex<T> NuoPow(const NuoComplex<T>& base, const T& exponent)
{
  return NuoExp(exponent * NuoLog(base));
}

/* Mathematical constants used by the inverse functions. */
template <typename T>
inline constexpr T kNuoPi = static_cast<T>(
  3.1415926535897932384626433832795029L);
template <typename T>
inline constexpr T kNuoHalfPi = static_cast<T>(
  1.5707963267948966192313216916397514L);
template <typename T>
inline constexpr T kNuoQuarterPi = static_cast<T>(
  0.7853981633974483096156608458198757L);
template <typename T>
inline constexpr T kNuoThreeQuarterPi = static_cast<T>(
  2.3561944901923449288469825374596272L);

/* Multiplies a finite factor by a factor that may be infinite. An exact zero
   factor keeps a signed zero instead of turning into NaN. */
template <typename T>
T NuoSaturatingProduct(T factor, T magnitude)
{
  if (factor != T() && magnitude != T())
  {
    return factor * magnitude;
  }
  if (std::isnan(factor) || std::isnan(magnitude))
  {
    /* An exact zero paired with NaN stays NaN, as IEEE arithmetic says. */
    return factor * magnitude;
  }
  /* An exact zero paired with an infinity keeps a signed zero. */
  const bool negative = std::signbit(factor) != std::signbit(magnitude);
  return negative ? -T() : T();
}

/* trigonometric functions */

template <typename T>
NuoComplex<T> NuoSin(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (imag == T())
  {
    /* csin(x + i0) is sin x + i0, so the signed zero follows the argument. */
    return NuoComplex<T>(std::sin(real), std::copysign(T(), imag));
  }
  if (std::isnan(imag))
  {
    /* csin(+-0 + iNaN) is +-0 + iNaN. */
    return real == T() ? NuoComplex<T>(std::copysign(T(), real), nan)
                       : NuoComplex<T>(nan, nan);
  }
  if (std::isinf(imag))
  {
    /* csin(+inf + i inf) and csin(NaN + i inf) keep an infinite imaginary
       part, while the negative real limit has no dedicated result. */
    if (std::isinf(real))
    {
      return real > T() ? NuoComplex<T>(nan, std::copysign(infinity, imag))
                        : NuoComplex<T>(nan, nan);
    }
    if (std::isnan(real))
    {
      return NuoComplex<T>(nan, std::copysign(infinity, imag));
    }
    return NuoComplex<T>(NuoSaturatingProduct(std::sin(real),
                                              std::cosh(imag)),
                         NuoSaturatingProduct(std::cos(real),
                                              std::sinh(imag)));
  }
  return NuoComplex<T>(NuoSaturatingProduct(std::sin(real),
                                            std::cosh(imag)),
                       NuoSaturatingProduct(std::cos(real),
                                            std::sinh(imag)));
}

template <typename T>
NuoComplex<T> NuoCos(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (imag == T())
  {
    /* ccos(x + i0) is cos x - i0. */
    return NuoComplex<T>(std::cos(real), -T());
  }
  if (std::isnan(imag))
  {
    /* ccos(+-0 + iNaN) is NaN +- i0. */
    return real == T() ? NuoComplex<T>(nan, std::copysign(T(), real))
                       : NuoComplex<T>(nan, nan);
  }
  if (std::isinf(imag))
  {
    if (real == infinity || std::isnan(real))
    {
      return NuoComplex<T>(infinity, nan);
    }
    if (std::isinf(real))
    {
      return NuoComplex<T>(nan, nan);
    }
  }
  return NuoComplex<T>(NuoSaturatingProduct(std::cos(real),
                                            std::cosh(imag)),
                       -NuoSaturatingProduct(std::sin(real),
                                             std::sinh(imag)));
}

/* tan uses the doubled angle form, which avoids the cancellation that a plain
   sin over cos division suffers from. */
template <typename T>
NuoComplex<T> NuoTan(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  if (std::isinf(imag))
  {
    const T zero = std::isinf(real)
      ? std::copysign(T(), real)
      : std::copysign(T(), std::sin(T(2) * real));
    return NuoComplex<T>(zero, std::copysign(T(1), imag));
  }
  if (std::isnan(imag))
  {
    /* ctan(+-0 + iNaN) is +-0 + iNaN, every other NaN input is NaN. */
    return real == T()
      ? NuoComplex<T>(std::copysign(T(), real),
                      std::numeric_limits<T>::quiet_NaN())
      : NuoComplex<T>(std::numeric_limits<T>::quiet_NaN(),
                      std::numeric_limits<T>::quiet_NaN());
  }
  const T denominator = std::cos(T(2) * real) + std::cosh(T(2) * imag);
  return NuoComplex<T>(std::sin(T(2) * real) / denominator,
                       std::sinh(T(2) * imag) / denominator);
}

template <typename T>
NuoComplex<T> NuoSinh(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (real == T())
  {
    /* csinh(+-0 + i y) is +-0 + i sin y. */
    return NuoComplex<T>(std::copysign(T(), real), std::sin(imag));
  }
  if (std::isinf(imag))
  {
    if (std::isinf(real) && imag > T())
    {
      return NuoComplex<T>(std::copysign(infinity, real), nan);
    }
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(real) && std::isnan(imag))
  {
    return NuoComplex<T>(std::copysign(infinity, real), nan);
  }
  if (std::isnan(real))
  {
    /* csinh(NaN + i0) keeps the signed zero imaginary part. */
    return imag == T() ? NuoComplex<T>(nan, std::copysign(T(), imag))
                       : NuoComplex<T>(nan, nan);
  }
  return NuoComplex<T>(NuoSaturatingProduct(std::cos(imag),
                                            std::sinh(real)),
                       NuoSaturatingProduct(std::cosh(real),
                                            std::sin(imag)));
}

template <typename T>
NuoComplex<T> NuoCosh(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (std::isinf(imag))
  {
    if (std::isinf(real))
    {
      return imag > T() ? NuoComplex<T>(infinity, nan)
                        : NuoComplex<T>(nan, nan);
    }
    if (real == T())
    {
      return NuoComplex<T>(nan, std::copysign(T(), imag));
    }
    return NuoComplex<T>(nan, nan);
  }
  if (std::isnan(imag))
  {
    if (std::isinf(real))
    {
      return NuoComplex<T>(infinity, nan);
    }
    /* ccosh(+-0 + iNaN) is NaN +- i0. */
    return real == T() ? NuoComplex<T>(nan, std::copysign(T(), real))
                       : NuoComplex<T>(nan, nan);
  }
  if (std::isnan(real))
  {
    /* ccosh(NaN + i0) keeps the signed zero imaginary part. */
    return imag == T() ? NuoComplex<T>(nan, std::copysign(T(), imag))
                       : NuoComplex<T>(nan, nan);
  }
  return NuoComplex<T>(NuoSaturatingProduct(std::cos(imag),
                                            std::cosh(real)),
                       NuoSaturatingProduct(std::sin(imag),
                                            std::sinh(real)));
}

template <typename T>
NuoComplex<T> NuoTanh(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  if (std::isinf(real))
  {
    const T zero = std::isinf(imag)
      ? std::copysign(T(), imag)
      : std::copysign(T(), std::sin(T(2) * imag));
    return NuoComplex<T>(std::copysign(T(1), real), zero);
  }
  if (std::isnan(real) && imag == T())
  {
    /* ctanh(NaN + i0) keeps the signed zero imaginary part. */
    return NuoComplex<T>(std::numeric_limits<T>::quiet_NaN(),
                         std::copysign(T(), imag));
  }
  const T denominator = std::cosh(T(2) * real) + std::cos(T(2) * imag);
  return NuoComplex<T>(std::sinh(T(2) * real) / denominator,
                       std::sin(T(2) * imag) / denominator);
}

/* inverse trigonometric functions */

template <typename T>
NuoComplex<T> NuoAsin(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (std::isnan(imag))
  {
    if (real == T())
    {
      return NuoComplex<T>(std::copysign(T(), real), nan);
    }
    if (std::isinf(real))
    {
      return NuoComplex<T>(nan, infinity);
    }
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(imag))
  {
    if (std::isinf(real))
    {
      return NuoComplex<T>(std::copysign(kNuoQuarterPi<T>, real),
                           std::copysign(infinity, imag));
    }
    return NuoComplex<T>(std::isnan(real) ? nan : std::copysign(T(), real),
                         std::copysign(infinity, imag));
  }
  if (std::isnan(real))
  {
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(real))
  {
    return NuoComplex<T>(std::copysign(kNuoHalfPi<T>, real),
                         std::copysign(infinity, imag));
  }

  const NuoComplex<T> i(T(), T(1));
  const NuoComplex<T> one(T(1));
  /* Written as -(z * z - 1) so that the radicand keeps the negative zero
     imaginary part that selects the branch matching the C library. */
  const NuoComplex<T> radicand = -(value * value - one);
  return -i * NuoLog(i * value + NuoSqrt(radicand));
}

template <typename T>
NuoComplex<T> NuoAcos(const NuoComplex<T>& value)
{
  const NuoComplex<T> arc = NuoAsin(value);
  return NuoComplex<T>(kNuoHalfPi<T> - arc.real(), -arc.imag());
}

template <typename T>
NuoComplex<T> NuoAtan(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();
  const T half_pi = kNuoHalfPi<T>;

  if (std::isinf(imag))
  {
    if (std::isnan(real))
    {
      return NuoComplex<T>(-std::copysign(half_pi, imag), nan);
    }
    return NuoComplex<T>(std::copysign(half_pi, real),
                         std::copysign(T(), imag));
  }
  if (std::isnan(imag))
  {
    return std::isinf(real) ? NuoComplex<T>(std::copysign(half_pi, real), nan)
                            : NuoComplex<T>(nan, nan);
  }
  if (std::isnan(real))
  {
    return NuoComplex<T>(nan, nan);
  }
  if (std::isinf(real))
  {
    return NuoComplex<T>(std::copysign(half_pi, real),
                         std::copysign(T(), imag));
  }
  if (real == T())
  {
    /* catan(0 + iy) is i catanh(y) inside the unit interval and
       -+pi/2 +- i atanh(1/|y|) outside it. */
    const T magnitude = std::abs(imag);
    if (magnitude < T(1))
    {
      return NuoComplex<T>(std::copysign(T(), real), std::atanh(imag));
    }
    if (magnitude == T(1))
    {
      /* catan(+-i) is the infinite point of the imaginary axis. */
      return NuoComplex<T>(std::copysign(T(), real),
                           std::copysign(infinity, imag));
    }
    return NuoComplex<T>(half_pi,
                         std::copysign(std::atanh(T(1) / magnitude), imag));
  }

  const NuoComplex<T> i(T(), T(1));
  const NuoComplex<T> one(T(1));
  return -i / T(2) * NuoLog((one + i * value) / (one - i * value));
}

template <typename T>
NuoComplex<T> NuoAsinh(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (std::isinf(imag))
  {
    if (std::isinf(real))
    {
      return NuoComplex<T>(std::copysign(infinity, real),
                           std::copysign(kNuoQuarterPi<T>, imag));
    }
    if (std::isnan(real))
    {
      return NuoComplex<T>(infinity, nan);
    }
    return NuoComplex<T>(std::copysign(infinity, real),
                         std::copysign(kNuoHalfPi<T>, imag));
  }
  if (std::isnan(imag))
  {
    return std::isinf(real) ? NuoComplex<T>(std::copysign(infinity, real),
                                            nan)
                            : NuoComplex<T>(nan, nan);
  }
  if (std::isnan(real))
  {
    return imag == T() ? NuoComplex<T>(nan, std::copysign(T(), imag))
                       : NuoComplex<T>(nan, nan);
  }
  if (std::isinf(real))
  {
    return NuoComplex<T>(std::copysign(infinity, real),
                         std::copysign(T(), imag));
  }
  if (real == T() && std::abs(imag) >= T(1))
  {
    /* casinh(0 + iy) is acosh(|y|) + i sign(y) pi/2 outside the unit
       interval, and i asin(y) inside it. */
    return NuoComplex<T>(std::acosh(std::abs(imag)),
                         std::copysign(kNuoHalfPi<T>, imag));
  }

  return NuoLog(value + NuoSqrt(value * value + NuoComplex<T>(T(1))));
}

template <typename T>
NuoComplex<T> NuoAcosh(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();

  if (std::isinf(imag))
  {
    if (std::isinf(real))
    {
      const T angle = real > T() ? kNuoQuarterPi<T> : kNuoThreeQuarterPi<T>;
      return NuoComplex<T>(infinity, std::copysign(angle, imag));
    }
    if (std::isnan(real))
    {
      return NuoComplex<T>(infinity, nan);
    }
    return NuoComplex<T>(infinity, std::copysign(kNuoHalfPi<T>, imag));
  }
  if (std::isnan(imag))
  {
    return std::isinf(real) ? NuoComplex<T>(infinity, nan)
                            : NuoComplex<T>(nan, nan);
  }
  if (std::isinf(real))
  {
    return real > T()
      ? NuoComplex<T>(infinity, std::copysign(T(), imag))
      : NuoComplex<T>(infinity, std::copysign(kNuoPi<T>, imag));
  }

  return NuoLog(value + NuoSqrt(value - NuoComplex<T>(T(1))) *
                          NuoSqrt(value + NuoComplex<T>(T(1))));
}

template <typename T>
NuoComplex<T> NuoAtanh(const NuoComplex<T>& value)
{
  const T real = value.real();
  const T imag = value.imag();
  const T infinity = std::numeric_limits<T>::infinity();
  const T nan = std::numeric_limits<T>::quiet_NaN();
  const T half_pi = kNuoHalfPi<T>;

  if (std::isinf(imag))
  {
    if (std::isnan(real))
    {
      return NuoComplex<T>(nan, std::copysign(half_pi, imag));
    }
    return NuoComplex<T>(std::copysign(T(), real),
                         std::copysign(half_pi, imag));
  }
  if (std::isnan(imag))
  {
    return NuoComplex<T>(nan,
                         std::isinf(real) ? std::copysign(half_pi, real) : nan);
  }
  if (std::isinf(real))
  {
    return NuoComplex<T>(std::copysign(T(), real),
                         std::copysign(half_pi, imag));
  }
  if (imag == T())
  {
    if (std::isnan(real))
    {
      return NuoComplex<T>(nan, nan);
    }
    /* catanh(x + i0) leaves the unit interval through atanh(1/x). */
    const T magnitude = std::abs(real);
    if (magnitude < T(1))
    {
      return NuoComplex<T>(std::atanh(real), std::copysign(T(), imag));
    }
    if (magnitude == T(1))
    {
      return NuoComplex<T>(std::copysign(infinity, real),
                           std::copysign(T(), imag));
    }
    return NuoComplex<T>(std::atanh(T(1) / real),
                         std::copysign(half_pi, imag));
  }

  const NuoComplex<T> one(T(1));
  return NuoLog((one + value) / (one - value)) / T(2);
}

/* extensions beyond std::complex */

/* True when the distance between the values is at most
   tolerance * (1 + |rhs|). Any NaN component makes the result false. */
template <typename T>
bool NuoNearlyEqual(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs,
                    const T& tolerance)
{
  if (std::isnan(lhs.real()) || std::isnan(lhs.imag()) ||
      std::isnan(rhs.real()) || std::isnan(rhs.imag()))
  {
    return false;
  }
  const T distance = std::hypot(lhs.real() - rhs.real(),
                                lhs.imag() - rhs.imag());
  return distance <= tolerance * (T(1) + NuoAbs(rhs));
}

/* Exact integer powers through repeated squaring; negative exponents invert. */
template <typename T, typename Integer>
  requires std::is_integral_v<Integer>
NuoComplex<T> NuoIntegerPow(const NuoComplex<T>& base, Integer exponent)
{
  using Wide = long long;
  const Wide power = static_cast<Wide>(exponent);
  const Wide magnitude = power < 0 ? -power : power;

  NuoComplex<T> result(T(1));
  NuoComplex<T> factor(base);
  for (Wide step = magnitude; step > 0; step >>= 1)
  {
    if ((step & 1) != 0)
    {
      result *= factor;
    }
    if (step > 1)
    {
      factor *= factor;
    }
  }
  if (power < 0)
  {
    result = NuoComplex<T>(T(1)) / result;
  }
  return result;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const NuoComplex<T>& value)
{
  return stream << '(' << value.real() << ',' << value.imag() << ')';
}

} /* namespace nuostl */
