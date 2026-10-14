#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ios>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

namespace nuostl
{

/* The standard library class template is
   template <std::size_t N> class bitset;
   cppreference.com/w/cpp/utility/bitset.html
   This implementation stores the bits in 64 bit words and keeps the unused
   high bits of the last word clear, so observers such as Count never see
   padding bits. */
template <std::size_t N>
class NuoBitset
{
public:
  using size_type = std::size_t;

  static constexpr size_type NPos = static_cast<size_type>(-1);

  /* A proxy that allows bit assignment and flipping through operator[]. */
  class reference
  {
    friend class NuoBitset;

  public:
    constexpr reference(const reference&) noexcept = default;
    constexpr ~reference() = default;

    constexpr reference& operator=(bool value) noexcept
    {
      owner_->SetBit(position_, value);
      return *this;
    }

    constexpr reference& operator=(const reference& other) noexcept
    {
      owner_->SetBit(position_, other.owner_->GetBit(other.position_));
      return *this;
    }

    constexpr bool operator~() const noexcept
    {
      return !owner_->GetBit(position_);
    }

    constexpr operator bool() const noexcept
    {
      return owner_->GetBit(position_);
    }

    constexpr reference& Flip() noexcept
    {
      owner_->SetBit(position_, !owner_->GetBit(position_));
      return *this;
    }

  private:
    constexpr reference(NuoBitset& owner, size_type position) noexcept
      : owner_(&owner)
      , position_(position)
    {
    }

    NuoBitset* owner_;
    size_type position_;
  };

  constexpr NuoBitset() noexcept = default;

  constexpr NuoBitset(unsigned long long value) noexcept
  {
    if constexpr (kWordCount > 0)
    {
      words_[0] = static_cast<Word>(value);
      MaskTopWord();
    }
  }

  template <typename CharT, typename Traits, typename Allocator>
  explicit NuoBitset(const std::basic_string<CharT, Traits, Allocator>& text,
                     size_type position = 0,
                     size_type count = std::basic_string<CharT, Traits,
                       Allocator>::npos,
                     CharT zero = CharT('0'),
                     CharT one = CharT('1'))
  {
    const size_type length = text.size();
    if (position > length)
    {
      throw std::out_of_range("NuoBitset: position out of range");
    }
    const size_type available = length - position;
    size_type width = count < available ? count : available;
    if (width > N)
    {
      width = N;
    }
    for (size_type index = 0; index < width; ++index)
    {
      const CharT character = text[position + width - 1 - index];
      if (Traits::eq(character, one))
      {
        SetBit(index, true);
      }
      else if (Traits::eq(character, zero))
      {
        SetBit(index, false);
      }
      else
      {
        throw std::invalid_argument("NuoBitset: invalid character");
      }
    }
  }

  template <typename CharT>
  explicit NuoBitset(const CharT* text,
                     size_type count = NPos,
                     CharT zero = CharT('0'),
                     CharT one = CharT('1'))
  {
    using Traits = std::char_traits<CharT>;
    const size_type length = count == NPos ? Traits::length(text) : count;
    const size_type width = length < N ? length : N;
    for (size_type index = 0; index < width; ++index)
    {
      const CharT character = text[width - 1 - index];
      if (Traits::eq(character, one))
      {
        SetBit(index, true);
      }
      else if (Traits::eq(character, zero))
      {
        SetBit(index, false);
      }
      else
      {
        throw std::invalid_argument("NuoBitset: invalid character");
      }
    }
  }

  constexpr bool operator[](size_type position) const noexcept
  {
    return GetBit(position);
  }

  constexpr reference operator[](size_type position) noexcept
  {
    return reference(*this, position);
  }

  constexpr bool Test(size_type position) const
  {
    if (position >= N)
    {
      throw std::out_of_range("NuoBitset::Test: position out of range");
    }
    return GetBit(position);
  }

  constexpr bool All() const noexcept
  {
    return Count() == N;
  }

  constexpr bool Any() const noexcept
  {
    return Count() != 0;
  }

  constexpr bool None() const noexcept
  {
    return Count() == 0;
  }

  constexpr size_type Count() const noexcept
  {
    size_type total = 0;
    for (size_type index = 0; index < kWordCount; ++index)
    {
      total += static_cast<size_type>(std::popcount(words_[index]));
    }
    return total;
  }

  constexpr size_type Size() const noexcept
  {
    return N;
  }

  constexpr bool operator==(const NuoBitset& other) const noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      if (words_[index] != other.words_[index])
      {
        return false;
      }
    }
    return true;
  }

  constexpr NuoBitset& operator&=(const NuoBitset& other) noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      words_[index] &= other.words_[index];
    }
    return *this;
  }

  constexpr NuoBitset& operator|=(const NuoBitset& other) noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      words_[index] |= other.words_[index];
    }
    return *this;
  }

  constexpr NuoBitset& operator^=(const NuoBitset& other) noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      words_[index] ^= other.words_[index];
    }
    return *this;
  }

  constexpr NuoBitset& operator<<=(size_type position) noexcept
  {
    if constexpr (N == 0)
    {
      return *this;
    }
    if (position == 0)
    {
      return *this;
    }
    if (position >= N)
    {
      Reset();
      return *this;
    }

    const size_type word_shift = position / kBitsPerWord;
    const size_type bit_shift = position % kBitsPerWord;
    for (size_type index = kWordCount; index > 0; --index)
    {
      const size_type target = index - 1;
      Word value = 0;
      if (target >= word_shift)
      {
        const size_type source = target - word_shift;
        value = static_cast<Word>(words_[source] << bit_shift);
        if (bit_shift != 0 && source > 0)
        {
          value |= static_cast<Word>(words_[source - 1] >>
                                     (kBitsPerWord - bit_shift));
        }
      }
      words_[target] = value;
    }
    MaskTopWord();
    return *this;
  }

  constexpr NuoBitset& operator>>=(size_type position) noexcept
  {
    if constexpr (N == 0)
    {
      return *this;
    }
    if (position == 0)
    {
      return *this;
    }
    if (position >= N)
    {
      Reset();
      return *this;
    }

    const size_type word_shift = position / kBitsPerWord;
    const size_type bit_shift = position % kBitsPerWord;
    for (size_type target = 0; target < kWordCount; ++target)
    {
      Word value = 0;
      const size_type source = target + word_shift;
      if (source < kWordCount)
      {
        value = static_cast<Word>(words_[source] >> bit_shift);
        if (bit_shift != 0 && source + 1 < kWordCount)
        {
          value |= static_cast<Word>(words_[source + 1] <<
                                     (kBitsPerWord - bit_shift));
        }
      }
      words_[target] = value;
    }
    MaskTopWord();
    return *this;
  }

  constexpr NuoBitset& Set() noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      words_[index] = ~Word(0);
    }
    MaskTopWord();
    return *this;
  }

  constexpr NuoBitset& Set(size_type position, bool value = true)
  {
    if (position >= N)
    {
      throw std::out_of_range("NuoBitset::Set: position out of range");
    }
    SetBit(position, value);
    return *this;
  }

  constexpr NuoBitset& Reset() noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      words_[index] = 0;
    }
    return *this;
  }

  constexpr NuoBitset& Reset(size_type position)
  {
    if (position >= N)
    {
      throw std::out_of_range("NuoBitset::Reset: position out of range");
    }
    SetBit(position, false);
    return *this;
  }

  constexpr NuoBitset& Flip() noexcept
  {
    for (size_type index = 0; index < kWordCount; ++index)
    {
      words_[index] = static_cast<Word>(~words_[index]);
    }
    MaskTopWord();
    return *this;
  }

  constexpr NuoBitset& Flip(size_type position)
  {
    if (position >= N)
    {
      throw std::out_of_range("NuoBitset::Flip: position out of range");
    }
    SetBit(position, !GetBit(position));
    return *this;
  }

  template <typename CharT = char,
            typename Traits = std::char_traits<CharT>,
            typename Allocator = std::allocator<CharT>>
  std::basic_string<CharT, Traits, Allocator> ToString(
    CharT zero = CharT('0'), CharT one = CharT('1')) const
  {
    std::basic_string<CharT, Traits, Allocator> result(N, zero);
    for (size_type index = 0; index < N; ++index)
    {
      result[N - 1 - index] = GetBit(index) ? one : zero;
    }
    return result;
  }

  constexpr unsigned long ToUlong() const
  {
    constexpr size_type kUlongBits =
      static_cast<size_type>(std::numeric_limits<unsigned long>::digits);
    if constexpr (N > kUlongBits)
    {
      for (size_type index = kUlongBits; index < N; ++index)
      {
        if (GetBit(index))
        {
          throw std::overflow_error(
            "NuoBitset::ToUlong: value does not fit");
        }
      }
    }
    unsigned long result = 0;
    const size_type width = N < kUlongBits ? N : kUlongBits;
    for (size_type index = 0; index < width; ++index)
    {
      if (GetBit(index))
      {
        result |= static_cast<unsigned long>(1ul << index);
      }
    }
    return result;
  }

  constexpr unsigned long long ToUllong() const
  {
    constexpr size_type kUllongBits =
      static_cast<size_type>(std::numeric_limits<unsigned long long>::digits);
    if constexpr (N > kUllongBits)
    {
      for (size_type index = kUllongBits; index < N; ++index)
      {
        if (GetBit(index))
        {
          throw std::overflow_error(
            "NuoBitset::ToUllong: value does not fit");
        }
      }
    }
    unsigned long long result = 0;
    const size_type width = N < kUllongBits ? N : kUllongBits;
    for (size_type index = 0; index < width; ++index)
    {
      if (GetBit(index))
      {
        result |= static_cast<unsigned long long>(1ull << index);
      }
    }
    return result;
  }

  /* A word based hash used by the std::hash specialization below. */
  constexpr size_type Hash() const noexcept
  {
    size_type result = static_cast<size_type>(14695981039346656037ull);
    for (size_type index = 0; index < kWordCount; ++index)
    {
      result ^= static_cast<size_type>(words_[index]);
      result *= static_cast<size_type>(1099511628211ull);
    }
    return result;
  }

private:
  using Word = std::uint64_t;

  static constexpr size_type kBitsPerWord = 64;
  static constexpr size_type kWordCount = N / kBitsPerWord +
    (N % kBitsPerWord != 0 ? 1 : 0);

  constexpr bool GetBit(size_type position) const noexcept
  {
    return ((words_[position / kBitsPerWord] >>
             (position % kBitsPerWord)) & Word(1)) != 0;
  }

  constexpr void SetBit(size_type position, bool value) noexcept
  {
    const Word mask = Word(1) << (position % kBitsPerWord);
    if (value)
    {
      words_[position / kBitsPerWord] |= mask;
    }
    else
    {
      words_[position / kBitsPerWord] &= static_cast<Word>(~mask);
    }
  }

  constexpr void MaskTopWord() noexcept
  {
    if constexpr ((N % kBitsPerWord) != 0)
    {
      constexpr size_type kUsedBits = N % kBitsPerWord;
      constexpr Word kMask =
        static_cast<Word>((Word(1) << kUsedBits) - Word(1));
      words_[kWordCount - 1] &= kMask;
    }
  }

  std::array<Word, kWordCount> words_{};
};

template <std::size_t N>
constexpr NuoBitset<N> operator&(NuoBitset<N> lhs,
                                 const NuoBitset<N>& rhs) noexcept
{
  lhs &= rhs;
  return lhs;
}

template <std::size_t N>
constexpr NuoBitset<N> operator|(NuoBitset<N> lhs,
                                 const NuoBitset<N>& rhs) noexcept
{
  lhs |= rhs;
  return lhs;
}

template <std::size_t N>
constexpr NuoBitset<N> operator^(NuoBitset<N> lhs,
                                 const NuoBitset<N>& rhs) noexcept
{
  lhs ^= rhs;
  return lhs;
}

template <std::size_t N>
constexpr NuoBitset<N> operator~(NuoBitset<N> value) noexcept
{
  value.Flip();
  return value;
}

template <std::size_t N>
constexpr NuoBitset<N> operator<<(NuoBitset<N> value,
                                  std::size_t position) noexcept
{
  value <<= position;
  return value;
}

template <std::size_t N>
constexpr NuoBitset<N> operator>>(NuoBitset<N> value,
                                  std::size_t position) noexcept
{
  value >>= position;
  return value;
}

template <typename CharT, typename Traits, std::size_t N>
std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& stream,
  const NuoBitset<N>& value)
{
  return stream << value.template ToString<CharT, Traits>(
    stream.widen('0'), stream.widen('1'));
}

template <typename CharT, typename Traits, std::size_t N>
std::basic_istream<CharT, Traits>& operator>>(
  std::basic_istream<CharT, Traits>& stream,
  NuoBitset<N>& value)
{
  typename std::basic_istream<CharT, Traits>::sentry sentry(stream);
  if (!sentry)
  {
    return stream;
  }
  if constexpr (N == 0)
  {
    return stream;
  }

  const CharT zero = stream.widen('0');
  const CharT one = stream.widen('1');
  std::basic_string<CharT, Traits> text;
  for (std::size_t index = 0; index < N; ++index)
  {
    const typename Traits::int_type next = stream.rdbuf()->sgetc();
    if (Traits::eq_int_type(next, Traits::eof()))
    {
      stream.setstate(std::ios_base::eofbit);
      break;
    }
    const CharT character = Traits::to_char_type(next);
    if (!Traits::eq(character, zero) && !Traits::eq(character, one))
    {
      break;
    }
    text.push_back(character);
    stream.rdbuf()->sbumpc();
  }

  if (text.empty())
  {
    stream.setstate(std::ios_base::failbit);
    return stream;
  }
  value = NuoBitset<N>(text);
  return stream;
}

} /* namespace nuostl */

namespace std
{

template <std::size_t N>
struct hash<nuostl::NuoBitset<N>>
{
  size_t operator()(const nuostl::NuoBitset<N>& value) const noexcept
  {
    return value.Hash();
  }
};

} /* namespace std */
