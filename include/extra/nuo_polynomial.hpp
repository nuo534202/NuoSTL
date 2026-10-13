#pragma once

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace nuostl
{

/* The standard library has no polynomial class template. This extension
   stores coefficients in ascending degree order, omitting trailing zeros. */
template <typename T>
class NuoPolynomial
{
public:
  using value_type = T;
  using size_type = std::size_t;

  NuoPolynomial() = default;
  NuoPolynomial(std::initializer_list<T> coefficients);

  size_type Size() const noexcept;
  size_type Degree() const noexcept;
  bool Empty() const noexcept;
  bool IsZero() const noexcept;

  T Coefficient(size_type degree) const;
  const T& At(size_type degree) const;
  void SetCoefficient(size_type degree, const T& value);

  T Evaluate(const T& value) const;
  T operator()(const T& value) const;
  NuoPolynomial Derivative() const;
  std::pair<NuoPolynomial, NuoPolynomial> Divmod(
    const NuoPolynomial& divisor) const;

  bool operator==(const NuoPolynomial& other) const;
  bool operator!=(const NuoPolynomial& other) const;

  NuoPolynomial& operator+=(const NuoPolynomial& other);
  NuoPolynomial& operator-=(const NuoPolynomial& other);
  NuoPolynomial& operator*=(const NuoPolynomial& other);
  NuoPolynomial& operator*=(const T& scalar);
  NuoPolynomial& operator/=(const T& scalar);

private:
  void Trim();
  static T DivideCoefficient(const T& numerator, const T& denominator,
                             bool require_exact);

  std::vector<T> coefficients_;
};

template <typename T>
NuoPolynomial<T>::NuoPolynomial(std::initializer_list<T> coefficients)
  : coefficients_(coefficients)
{
  Trim();
}

template <typename T>
typename NuoPolynomial<T>::size_type NuoPolynomial<T>::Size() const noexcept
{
  return coefficients_.size();
}

template <typename T>
typename NuoPolynomial<T>::size_type NuoPolynomial<T>::Degree() const noexcept
{
  return coefficients_.empty() ? 0 : coefficients_.size() - 1;
}

template <typename T>
bool NuoPolynomial<T>::Empty() const noexcept
{
  return coefficients_.empty();
}

template <typename T>
bool NuoPolynomial<T>::IsZero() const noexcept
{
  return coefficients_.empty();
}

template <typename T>
T NuoPolynomial<T>::Coefficient(size_type degree) const
{
  return degree < coefficients_.size() ? coefficients_[degree] : T();
}

template <typename T>
const T& NuoPolynomial<T>::At(size_type degree) const
{
  if (degree >= coefficients_.size())
  {
    throw std::out_of_range("NuoPolynomial::At: degree out of range");
  }
  return coefficients_[degree];
}

template <typename T>
void NuoPolynomial<T>::SetCoefficient(size_type degree, const T& value)
{
  if (degree >= coefficients_.size() && value == T())
  {
    return;
  }
  if (degree >= coefficients_.max_size())
  {
    throw std::length_error("NuoPolynomial: degree exceeds maximum size");
  }
  /* The value may refer to a coefficient invalidated by resize. */
  const T stored_value = value;
  if (degree >= coefficients_.size())
  {
    coefficients_.resize(degree + 1, T());
  }
  coefficients_[degree] = stored_value;
  Trim();
}

template <typename T>
T NuoPolynomial<T>::Evaluate(const T& value) const
{
  T result = T();
  for (size_type index = coefficients_.size(); index > 0; --index)
  {
    result = result * value + coefficients_[index - 1];
  }
  return result;
}

template <typename T>
T NuoPolynomial<T>::operator()(const T& value) const
{
  return Evaluate(value);
}

template <typename T>
NuoPolynomial<T> NuoPolynomial<T>::Derivative() const
{
  NuoPolynomial result;
  if (coefficients_.size() < 2)
  {
    return result;
  }
  result.coefficients_.resize(coefficients_.size() - 1);
  for (size_type degree = 1; degree < coefficients_.size(); ++degree)
  {
    result.coefficients_[degree - 1] = coefficients_[degree] * T(degree);
  }
  result.Trim();
  return result;
}

template <typename T>
std::pair<NuoPolynomial<T>, NuoPolynomial<T>> NuoPolynomial<T>::Divmod(
  const NuoPolynomial& divisor) const
{
  if (divisor.IsZero())
  {
    throw std::invalid_argument("NuoPolynomial::Divmod: zero divisor");
  }
  if constexpr (std::is_floating_point_v<T>)
  {
    for (const T& coefficient : coefficients_)
    {
      if (!std::isfinite(coefficient))
      {
        throw std::domain_error("NuoPolynomial::Divmod: nonfinite dividend");
      }
    }
    for (const T& coefficient : divisor.coefficients_)
    {
      if (!std::isfinite(coefficient))
      {
        throw std::domain_error("NuoPolynomial::Divmod: nonfinite divisor");
      }
    }
  }

  NuoPolynomial quotient;
  NuoPolynomial remainder(*this);
  if (remainder.IsZero() || remainder.Degree() < divisor.Degree())
  {
    return {quotient, remainder};
  }
  quotient.coefficients_.resize(remainder.Degree() - divisor.Degree() + 1, T());

  while (!remainder.IsZero() && remainder.Degree() >= divisor.Degree())
  {
    const size_type shift = remainder.Degree() - divisor.Degree();
    const T factor = DivideCoefficient(remainder.coefficients_.back(),
                                       divisor.coefficients_.back(), true);
    if (factor == T())
    {
      throw std::domain_error(
        "NuoPolynomial::Divmod: coefficient division cannot reduce degree");
    }
    quotient.coefficients_[shift] = factor;
    for (size_type index = 0; index + 1 < divisor.coefficients_.size(); ++index)
    {
      const size_type target = shift + index;
      remainder.coefficients_[target] = remainder.coefficients_[target] -
        factor * divisor.coefficients_[index];
      if constexpr (std::is_floating_point_v<T>)
      {
        if (!std::isfinite(remainder.coefficients_[target]))
        {
          throw std::overflow_error("NuoPolynomial::Divmod: nonfinite result");
        }
      }
    }

    if constexpr (!std::is_arithmetic_v<T>)
    {
      if (remainder.coefficients_.back() -
          factor * divisor.coefficients_.back() != T())
      {
        throw std::domain_error(
          "NuoPolynomial::Divmod: coefficients are not exactly divisible");
      }
    }
    /* The leading term cancels algebraically. Assign zero explicitly so
       floating point roundoff cannot prevent the degree from decreasing. */
    remainder.coefficients_.back() = T();
    remainder.Trim();
  }
  quotient.Trim();
  return {quotient, remainder};
}

template <typename T>
bool NuoPolynomial<T>::operator==(const NuoPolynomial& other) const
{
  return coefficients_ == other.coefficients_;
}

template <typename T>
bool NuoPolynomial<T>::operator!=(const NuoPolynomial& other) const
{
  return !(*this == other);
}

template <typename T>
NuoPolynomial<T>& NuoPolynomial<T>::operator+=(const NuoPolynomial& other)
{
  if (coefficients_.size() < other.coefficients_.size())
  {
    coefficients_.resize(other.coefficients_.size(), T());
  }
  for (size_type index = 0; index < other.coefficients_.size(); ++index)
  {
    coefficients_[index] = coefficients_[index] + other.coefficients_[index];
  }
  Trim();
  return *this;
}

template <typename T>
NuoPolynomial<T>& NuoPolynomial<T>::operator-=(const NuoPolynomial& other)
{
  if (coefficients_.size() < other.coefficients_.size())
  {
    coefficients_.resize(other.coefficients_.size(), T());
  }
  for (size_type index = 0; index < other.coefficients_.size(); ++index)
  {
    coefficients_[index] = coefficients_[index] - other.coefficients_[index];
  }
  Trim();
  return *this;
}

template <typename T>
NuoPolynomial<T>& NuoPolynomial<T>::operator*=(const NuoPolynomial& other)
{
  if (IsZero() || other.IsZero())
  {
    coefficients_.clear();
    return *this;
  }
  if (coefficients_.size() - 1 >
      coefficients_.max_size() - other.coefficients_.size())
  {
    throw std::length_error("NuoPolynomial: product exceeds maximum size");
  }
  std::vector<T> product(coefficients_.size() + other.coefficients_.size() - 1,
                         T());
  for (size_type lhs = 0; lhs < coefficients_.size(); ++lhs)
  {
    for (size_type rhs = 0; rhs < other.coefficients_.size(); ++rhs)
    {
      product[lhs + rhs] = product[lhs + rhs] +
        coefficients_[lhs] * other.coefficients_[rhs];
    }
  }
  coefficients_ = std::move(product);
  Trim();
  return *this;
}

template <typename T>
NuoPolynomial<T>& NuoPolynomial<T>::operator*=(const T& scalar)
{
  const T factor = scalar;
  for (T& coefficient : coefficients_)
  {
    coefficient = coefficient * factor;
  }
  Trim();
  return *this;
}

template <typename T>
NuoPolynomial<T>& NuoPolynomial<T>::operator/=(const T& scalar)
{
  if (scalar == T())
  {
    throw std::invalid_argument("NuoPolynomial: zero scalar divisor");
  }
  /* Compute separately to handle aliasing and leave the operand unchanged
     when a coefficient division throws. */
  NuoPolynomial result(*this);
  for (T& coefficient : result.coefficients_)
  {
    coefficient = DivideCoefficient(coefficient, scalar, false);
  }
  result.Trim();
  coefficients_.swap(result.coefficients_);
  return *this;
}

template <typename T>
T NuoPolynomial<T>::DivideCoefficient(const T& numerator, const T& denominator,
                                      bool require_exact)
{
  if constexpr (std::is_integral_v<T>)
  {
    if constexpr (std::is_signed_v<T>)
    {
      if (numerator == std::numeric_limits<T>::min() && denominator == T(-1))
      {
        throw std::overflow_error(
          "NuoPolynomial: coefficient division overflow");
      }
    }
    if (require_exact && numerator % denominator != T())
    {
      throw std::domain_error(
        "NuoPolynomial::Divmod: coefficients are not exactly divisible");
    }
  }
  const T result = numerator / denominator;
  if constexpr (std::is_floating_point_v<T>)
  {
    if (require_exact && !std::isfinite(result))
    {
      throw std::overflow_error("NuoPolynomial::Divmod: nonfinite quotient");
    }
  }
  return result;
}

template <typename T>
void NuoPolynomial<T>::Trim()
{
  while (!coefficients_.empty() && coefficients_.back() == T())
  {
    coefficients_.pop_back();
  }
}

template <typename T>
NuoPolynomial<T> operator+(NuoPolynomial<T> lhs,
                           const NuoPolynomial<T>& rhs)
{
  lhs += rhs;
  return lhs;
}

template <typename T>
NuoPolynomial<T> operator-(NuoPolynomial<T> lhs,
                           const NuoPolynomial<T>& rhs)
{
  lhs -= rhs;
  return lhs;
}

template <typename T>
NuoPolynomial<T> operator-(const NuoPolynomial<T>& polynomial)
{
  NuoPolynomial<T> result;
  for (typename NuoPolynomial<T>::size_type degree = 0;
       degree < polynomial.Size(); ++degree)
  {
    result.SetCoefficient(degree, T() - polynomial.Coefficient(degree));
  }
  return result;
}

template <typename T>
NuoPolynomial<T> operator*(NuoPolynomial<T> lhs,
                           const NuoPolynomial<T>& rhs)
{
  lhs *= rhs;
  return lhs;
}

template <typename T>
NuoPolynomial<T> operator*(NuoPolynomial<T> polynomial, const T& scalar)
{
  polynomial *= scalar;
  return polynomial;
}

template <typename T>
NuoPolynomial<T> operator*(const T& scalar, NuoPolynomial<T> polynomial)
{
  polynomial *= scalar;
  return polynomial;
}

template <typename T>
NuoPolynomial<T> operator/(NuoPolynomial<T> polynomial, const T& scalar)
{
  polynomial /= scalar;
  return polynomial;
}

template <typename T>
NuoPolynomial<T> operator/(const NuoPolynomial<T>& dividend,
                           const NuoPolynomial<T>& divisor)
{
  return dividend.Divmod(divisor).first;
}

template <typename T>
NuoPolynomial<T> operator%(const NuoPolynomial<T>& dividend,
                           const NuoPolynomial<T>& divisor)
{
  return dividend.Divmod(divisor).second;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream,
                         const NuoPolynomial<T>& polynomial)
{
  if (polynomial.IsZero())
  {
    return stream << '0';
  }
  bool first = true;
  for (typename NuoPolynomial<T>::size_type degree = 0;
       degree < polynomial.Size(); ++degree)
  {
    const T coefficient = polynomial.Coefficient(degree);
    if (coefficient == T())
    {
      continue;
    }
    const bool negative = coefficient < T();
    if (first)
    {
      stream << coefficient;
    }
    else
    {
      stream << (negative ? " - " : " + ");
      if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
      {
        /* Unsigned subtraction also represents the magnitude of T's minimum. */
        using Unsigned = std::make_unsigned_t<T>;
        const Unsigned magnitude = negative
          ? Unsigned(0) - static_cast<Unsigned>(coefficient)
          : static_cast<Unsigned>(coefficient);
        stream << magnitude;
      }
      else
      {
        stream << (negative ? T() - coefficient : coefficient);
      }
    }
    if (degree > 0)
    {
      stream << "*x";
      if (degree > 1)
      {
        stream << '^' << degree;
      }
    }
    first = false;
  }
  return stream;
}

} /* namespace nuostl */
