#include "extra/nuo_big_integer.hpp"

#include <climits>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace nuostl
{

namespace
{

/* 2^63 as an unsigned value, the magnitude of LLONG_MIN. */
constexpr unsigned long long kLongLongMagnitudeLimit = 9223372036854775808ull;

} /* namespace */

NuoBigInteger::NuoBigInteger()
  : negative_(false)
  , blocks_()
{
}

NuoBigInteger::NuoBigInteger(int value)
  : NuoBigInteger(static_cast<long long>(value))
{
}

NuoBigInteger::NuoBigInteger(unsigned int value)
  : NuoBigInteger(static_cast<unsigned long long>(value))
{
}

NuoBigInteger::NuoBigInteger(long value)
  : NuoBigInteger(static_cast<long long>(value))
{
}

NuoBigInteger::NuoBigInteger(unsigned long value)
  : NuoBigInteger(static_cast<unsigned long long>(value))
{
}

NuoBigInteger::NuoBigInteger(long long value)
  : negative_(false)
  , blocks_()
{
  /* Negating in unsigned arithmetic keeps LLONG_MIN well defined. */
  const unsigned long long magnitude = value < 0
    ? ~static_cast<unsigned long long>(value) + 1ull
    : static_cast<unsigned long long>(value);
  SetFromUnsigned(magnitude);
  negative_ = value < 0;
}

NuoBigInteger::NuoBigInteger(unsigned long long value)
  : negative_(false)
  , blocks_()
{
  SetFromUnsigned(value);
}

NuoBigInteger::NuoBigInteger(std::string_view text)
  : negative_(false)
  , blocks_()
{
  if (text.empty())
  {
    throw std::invalid_argument("NuoBigInteger: empty literal");
  }

  std::size_t position = 0;
  bool negative = false;
  if (text[0] == '+' || text[0] == '-')
  {
    negative = text[0] == '-';
    position = 1;
  }
  if (position == text.size())
  {
    throw std::invalid_argument("NuoBigInteger: literal without digits");
  }
  for (std::size_t index = position; index < text.size(); ++index)
  {
    if (text[index] < '0' || text[index] > '9')
    {
      throw std::invalid_argument("NuoBigInteger: literal is not decimal");
    }
  }

  /* Group the digits from the right into base 1e9 blocks. */
  for (std::size_t end = text.size(); end > position;)
  {
    const std::size_t begin = end - position > kDigitsPerBlock
      ? end - kDigitsPerBlock
      : position;
    Digit block = 0;
    for (std::size_t index = begin; index < end; ++index)
    {
      block = static_cast<Digit>(block * 10 +
                                static_cast<Digit>(text[index] - '0'));
    }
    blocks_.push_back(block);
    end = begin;
  }

  TrimMagnitude(blocks_);
  negative_ = negative && !blocks_.empty();
}

/* observers */

bool NuoBigInteger::IsZero() const
{
  return blocks_.empty();
}

bool NuoBigInteger::IsNegative() const
{
  return negative_;
}

std::size_t NuoBigInteger::DigitCount() const
{
  if (blocks_.empty())
  {
    return 1;
  }
  std::size_t count = (blocks_.size() - 1) * kDigitsPerBlock;
  Digit leading = blocks_.back();
  while (leading > 0)
  {
    ++count;
    leading /= 10;
  }
  return count;
}

std::string NuoBigInteger::ToString() const
{
  if (blocks_.empty())
  {
    return "0";
  }

  std::string text;
  if (negative_)
  {
    text.push_back('-');
  }
  text += std::to_string(blocks_.back());
  for (std::size_t index = blocks_.size() - 1; index-- > 0;)
  {
    const std::string block = std::to_string(blocks_[index]);
    text.append(kDigitsPerBlock - block.size(), '0');
    text += block;
  }
  return text;
}

long long NuoBigInteger::ToLongLong() const
{
  const unsigned long long limit =
    negative_ ? kLongLongMagnitudeLimit : kLongLongMagnitudeLimit - 1ull;
  unsigned long long magnitude = 0;
  for (std::size_t index = blocks_.size(); index-- > 0;)
  {
    if (magnitude > (limit - blocks_[index]) / kBase)
    {
      throw std::overflow_error(
        "NuoBigInteger: value does not fit in long long");
    }
    magnitude = magnitude * kBase + blocks_[index];
  }
  if (negative_)
  {
    return magnitude == kLongLongMagnitudeLimit
      ? LLONG_MIN
      : -static_cast<long long>(magnitude);
  }
  return static_cast<long long>(magnitude);
}

NuoBigInteger NuoBigInteger::Abs() const
{
  NuoBigInteger result(*this);
  result.negative_ = false;
  return result;
}

/* comparison */

bool NuoBigInteger::operator==(const NuoBigInteger& other) const
{
  return negative_ == other.negative_ && blocks_ == other.blocks_;
}

std::strong_ordering NuoBigInteger::operator<=>(
  const NuoBigInteger& other) const
{
  if (negative_ != other.negative_)
  {
    return negative_ ? std::strong_ordering::less
                     : std::strong_ordering::greater;
  }
  const int comparison = CompareMagnitude(blocks_, other.blocks_);
  if (comparison == 0)
  {
    return std::strong_ordering::equal;
  }
  if (negative_)
  {
    return comparison > 0 ? std::strong_ordering::less
                          : std::strong_ordering::greater;
  }
  return comparison < 0 ? std::strong_ordering::less
                        : std::strong_ordering::greater;
}

/* arithmetic */

NuoBigInteger NuoBigInteger::operator-() const
{
  NuoBigInteger result(*this);
  if (!result.blocks_.empty())
  {
    result.negative_ = !result.negative_;
  }
  return result;
}

NuoBigInteger& NuoBigInteger::operator+=(const NuoBigInteger& other)
{
  if (negative_ == other.negative_)
  {
    blocks_ = AddMagnitude(blocks_, other.blocks_);
    if (blocks_.empty())
    {
      negative_ = false;
    }
    return *this;
  }

  const int comparison = CompareMagnitude(blocks_, other.blocks_);
  if (comparison == 0)
  {
    blocks_.clear();
    negative_ = false;
    return *this;
  }
  if (comparison > 0)
  {
    blocks_ = SubtractMagnitude(blocks_, other.blocks_);
  }
  else
  {
    blocks_ = SubtractMagnitude(other.blocks_, blocks_);
    negative_ = other.negative_;
  }
  return *this;
}

NuoBigInteger& NuoBigInteger::operator-=(const NuoBigInteger& other)
{
  return *this += -other;
}

NuoBigInteger& NuoBigInteger::operator*=(const NuoBigInteger& other)
{
  const bool negative = (negative_ != other.negative_) &&
    !blocks_.empty() && !other.blocks_.empty();
  blocks_ = MultiplyMagnitude(blocks_, other.blocks_);
  negative_ = negative && !blocks_.empty();
  return *this;
}

NuoBigInteger& NuoBigInteger::operator/=(const NuoBigInteger& other)
{
  if (other.blocks_.empty())
  {
    throw std::domain_error("NuoBigInteger: division by zero");
  }
  Digits quotient;
  Digits remainder;
  DivideMagnitude(blocks_, other.blocks_, &quotient, &remainder);
  const bool negative = (negative_ != other.negative_) && !quotient.empty();
  blocks_ = std::move(quotient);
  negative_ = negative;
  return *this;
}

NuoBigInteger& NuoBigInteger::operator%=(const NuoBigInteger& other)
{
  if (other.blocks_.empty())
  {
    throw std::domain_error("NuoBigInteger: division by zero");
  }
  Digits quotient;
  Digits remainder;
  DivideMagnitude(blocks_, other.blocks_, &quotient, &remainder);
  blocks_ = std::move(remainder);
  /* The remainder keeps the sign of the dividend, like built in integers. */
  negative_ = negative_ && !blocks_.empty();
  return *this;
}

NuoBigInteger& NuoBigInteger::operator++()
{
  return *this += NuoBigInteger(1);
}

NuoBigInteger NuoBigInteger::operator++(int)
{
  NuoBigInteger result(*this);
  ++*this;
  return result;
}

NuoBigInteger& NuoBigInteger::operator--()
{
  return *this -= NuoBigInteger(1);
}

NuoBigInteger NuoBigInteger::operator--(int)
{
  NuoBigInteger result(*this);
  --*this;
  return result;
}

NuoBigInteger NuoBigInteger::Pow(const NuoBigInteger& base,
                                 unsigned long long exponent)
{
  NuoBigInteger result(1);
  NuoBigInteger factor(base);
  while (exponent > 0)
  {
    if ((exponent & 1ull) != 0ull)
    {
      result *= factor;
    }
    exponent >>= 1;
    if (exponent > 0)
    {
      factor *= factor;
    }
  }
  return result;
}

NuoBigInteger NuoBigInteger::Gcd(const NuoBigInteger& lhs,
                                 const NuoBigInteger& rhs)
{
  NuoBigInteger left = lhs.Abs();
  NuoBigInteger right = rhs.Abs();
  while (!right.IsZero())
  {
    NuoBigInteger remainder = left % right;
    left = right;
    right = remainder;
  }
  return left;
}

/* helpers */

void NuoBigInteger::TrimMagnitude(Digits& digits)
{
  while (!digits.empty() && digits.back() == 0)
  {
    digits.pop_back();
  }
}

void NuoBigInteger::SetFromUnsigned(unsigned long long magnitude)
{
  blocks_.clear();
  while (magnitude > 0)
  {
    blocks_.push_back(static_cast<Digit>(magnitude % kBase));
    magnitude /= kBase;
  }
}

int NuoBigInteger::CompareMagnitude(const Digits& lhs, const Digits& rhs)
{
  if (lhs.size() != rhs.size())
  {
    return lhs.size() < rhs.size() ? -1 : 1;
  }
  for (std::size_t index = lhs.size(); index-- > 0;)
  {
    if (lhs[index] != rhs[index])
    {
      return lhs[index] < rhs[index] ? -1 : 1;
    }
  }
  return 0;
}

NuoBigInteger::Digits NuoBigInteger::AddMagnitude(const Digits& lhs,
                                                 const Digits& rhs)
{
  const Digits& longer = lhs.size() >= rhs.size() ? lhs : rhs;
  const Digits& shorter = lhs.size() >= rhs.size() ? rhs : lhs;
  Digits result;
  result.reserve(longer.size() + 1);

  std::uint64_t carry = 0;
  for (std::size_t index = 0; index < longer.size(); ++index)
  {
    const std::uint64_t addend = index < shorter.size() ? shorter[index] : 0u;
    const std::uint64_t sum =
      static_cast<std::uint64_t>(longer[index]) + carry + addend;
    result.push_back(static_cast<Digit>(sum % kBase));
    carry = sum / kBase;
  }
  if (carry != 0)
  {
    result.push_back(static_cast<Digit>(carry));
  }
  return result;
}

NuoBigInteger::Digits NuoBigInteger::SubtractMagnitude(const Digits& lhs,
                                                       const Digits& rhs)
{
  Digits result;
  result.reserve(lhs.size());

  std::int64_t borrow = 0;
  for (std::size_t index = 0; index < lhs.size(); ++index)
  {
    std::int64_t difference = static_cast<std::int64_t>(lhs[index]) - borrow -
      (index < rhs.size() ? static_cast<std::int64_t>(rhs[index]) : 0);
    if (difference < 0)
    {
      difference += kBase;
      borrow = 1;
    }
    else
    {
      borrow = 0;
    }
    result.push_back(static_cast<Digit>(difference));
  }
  TrimMagnitude(result);
  return result;
}

NuoBigInteger::Digits NuoBigInteger::MultiplyMagnitude(const Digits& lhs,
                                                       const Digits& rhs)
{
  Digits result(lhs.size() + rhs.size(), 0);
  for (std::size_t left = 0; left < lhs.size(); ++left)
  {
    std::uint64_t carry = 0;
    for (std::size_t right = 0; right < rhs.size(); ++right)
    {
      /* lhs and rhs are below 1e9, so the product stays far below 2^64. */
      const std::uint64_t current = static_cast<std::uint64_t>(lhs[left]) *
        rhs[right] + result[left + right] + carry;
      result[left + right] = static_cast<Digit>(current % kBase);
      carry = current / kBase;
    }
    result[left + rhs.size()] += static_cast<Digit>(carry);
  }
  TrimMagnitude(result);
  return result;
}

void NuoBigInteger::DivideMagnitude(const Digits& dividend,
                                    const Digits& divisor,
                                    Digits* quotient, Digits* remainder)
{
  quotient->assign(dividend.size(), 0);
  Digits running;

  for (std::size_t index = dividend.size(); index-- > 0;)
  {
    /* running = running * kBase + dividend[index] */
    running.insert(running.begin(), dividend[index]);
    TrimMagnitude(running);

    /* Largest q below kBase with divisor * q <= running; the estimate is
       refined by bisection so no digit correction step is needed. */
    Digit low = 0;
    Digit high = kBase - 1;
    while (low < high)
    {
      const Digit middle = static_cast<Digit>(
        (static_cast<std::uint64_t>(low) + high + 1) / 2);
      if (CompareMagnitude(MultiplyMagnitude(divisor, Digits{middle}),
                           running) <= 0)
      {
        low = middle;
      }
      else
      {
        high = static_cast<Digit>(middle - 1);
      }
    }
    if (low != 0)
    {
      running = SubtractMagnitude(running, MultiplyMagnitude(divisor,
                                                             Digits{low}));
    }
    (*quotient)[index] = low;
  }

  TrimMagnitude(running);
  TrimMagnitude(*quotient);
  *remainder = std::move(running);
}

/* free operators */

NuoBigInteger operator+(NuoBigInteger lhs, const NuoBigInteger& rhs)
{
  return lhs += rhs;
}

NuoBigInteger operator-(NuoBigInteger lhs, const NuoBigInteger& rhs)
{
  return lhs -= rhs;
}

NuoBigInteger operator*(NuoBigInteger lhs, const NuoBigInteger& rhs)
{
  return lhs *= rhs;
}

NuoBigInteger operator/(NuoBigInteger lhs, const NuoBigInteger& rhs)
{
  return lhs /= rhs;
}

NuoBigInteger operator%(NuoBigInteger lhs, const NuoBigInteger& rhs)
{
  return lhs %= rhs;
}

std::ostream& operator<<(std::ostream& stream, const NuoBigInteger& value)
{
  return stream << value.ToString();
}

} /* namespace nuostl */
