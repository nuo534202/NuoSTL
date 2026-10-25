#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace nuostl
{

/* An arbitrary precision signed integer.
   The magnitude is stored as little endian blocks in base 1e9, so decimal
   formatting and parsing need no long division, and every product of two
   blocks still fits into a 64 bit unsigned integer. */
class NuoBigInteger
{
public:
  NuoBigInteger();
  NuoBigInteger(int value);
  NuoBigInteger(unsigned int value);
  NuoBigInteger(long value);
  NuoBigInteger(unsigned long value);
  NuoBigInteger(long long value);
  NuoBigInteger(unsigned long long value);

  /* Text is a decimal literal with an optional leading sign; anything else
     throws std::invalid_argument. */
  explicit NuoBigInteger(std::string_view text);

  /* observers */
  bool IsZero() const;
  bool IsNegative() const;
  std::size_t DigitCount() const;
  std::string ToString() const;
  long long ToLongLong() const;
  NuoBigInteger Abs() const;

  /* comparison */
  bool operator==(const NuoBigInteger& other) const;
  std::strong_ordering operator<=>(const NuoBigInteger& other) const;

  /* arithmetic */
  NuoBigInteger operator-() const;
  NuoBigInteger& operator+=(const NuoBigInteger& other);
  NuoBigInteger& operator-=(const NuoBigInteger& other);
  NuoBigInteger& operator*=(const NuoBigInteger& other);
  NuoBigInteger& operator/=(const NuoBigInteger& other);
  NuoBigInteger& operator%=(const NuoBigInteger& other);
  NuoBigInteger& operator++();
  NuoBigInteger operator++(int);
  NuoBigInteger& operator--();
  NuoBigInteger operator--(int);

  static NuoBigInteger Pow(const NuoBigInteger& base,
                           unsigned long long exponent);
  static NuoBigInteger Gcd(const NuoBigInteger& lhs,
                           const NuoBigInteger& rhs);

private:
  using Digit = std::uint32_t;
  using Digits = std::vector<Digit>;

  static constexpr Digit kBase = 1000000000;
  static constexpr std::size_t kDigitsPerBlock = 9;

  static void TrimMagnitude(Digits& digits);
  void SetFromUnsigned(unsigned long long magnitude);
  static int CompareMagnitude(const Digits& lhs, const Digits& rhs);
  static Digits AddMagnitude(const Digits& lhs, const Digits& rhs);
  static Digits SubtractMagnitude(const Digits& lhs, const Digits& rhs);
  static Digits MultiplyMagnitude(const Digits& lhs, const Digits& rhs);
  static void DivideMagnitude(const Digits& dividend, const Digits& divisor,
                              Digits* quotient, Digits* remainder);

  bool negative_;
  Digits blocks_;
};

NuoBigInteger operator+(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator-(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator*(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator/(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator%(NuoBigInteger lhs, const NuoBigInteger& rhs);

std::ostream& operator<<(std::ostream& stream, const NuoBigInteger& value);

} /* namespace nuostl */
