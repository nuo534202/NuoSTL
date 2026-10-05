#pragma once

#include <stddef.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "utils/nuo_iterator.hpp"

namespace nuostl
{

/* The character storage is owned by the caller and must outlive this view. */
template <class CharT, class Traits = std::char_traits<CharT>>
class NuoBasicStringView
{
public:
  using traits_type            = Traits;
  using value_type             = CharT;
  using pointer                = const CharT*;
  using const_pointer          = const CharT*;
  using reference              = const CharT&;
  using const_reference        = const CharT&;
  using const_iterator         = const CharT*;
  using iterator               = const_iterator;
  using const_reverse_iterator = NuoReverseIterator<const_iterator>;
  using reverse_iterator       = const_reverse_iterator;
  using size_type              = size_t;
  using difference_type        = ptrdiff_t;

  static constexpr size_type NPos = static_cast<size_type>(-1);
  static constexpr size_type npos = NPos;

  constexpr NuoBasicStringView() noexcept : data_(nullptr), size_(0)
  {
  }

  constexpr NuoBasicStringView(const NuoBasicStringView&) noexcept = default;
  constexpr NuoBasicStringView& operator=(
    const NuoBasicStringView&) noexcept = default;

  constexpr NuoBasicStringView(const CharT* data, size_type size) noexcept
    : data_(data), size_(size)
  {
  }

  NuoBasicStringView(const CharT* data)
    : data_(data), size_(Traits::length(data))
  {
  }

  template <class Allocator>
  NuoBasicStringView(
    const std::basic_string<CharT, Traits, Allocator>& string) noexcept
    : data_(string.data()), size_(string.size())
  {
  }

  template <size_type N>
  constexpr NuoBasicStringView(const CharT (&data)[N]) noexcept
    : data_(data), size_(N == 0 ? 0 : N - 1)
  {
  }

  constexpr const_iterator Begin() const noexcept
  {
    return data_;
  }

  constexpr const_iterator End() const noexcept
  {
    return size_ == 0 ? data_ : data_ + size_;
  }

  constexpr const_iterator CBegin() const noexcept
  {
    return Begin();
  }

  constexpr const_iterator CEnd() const noexcept
  {
    return End();
  }

  const_reverse_iterator RBegin() const noexcept
  {
    return const_reverse_iterator(End());
  }

  const_reverse_iterator REnd() const noexcept
  {
    return const_reverse_iterator(Begin());
  }

  const_reverse_iterator CRBegin() const noexcept
  {
    return RBegin();
  }

  const_reverse_iterator CREnd() const noexcept
  {
    return REnd();
  }

  constexpr size_type Size() const noexcept
  {
    return size_;
  }

  constexpr size_type Length() const noexcept
  {
    return size_;
  }

  constexpr size_type MaxSize() const noexcept
  {
    return (std::numeric_limits<size_type>::max)() / sizeof(CharT);
  }

  constexpr bool Empty() const noexcept
  {
    return size_ == 0;
  }

  constexpr const_reference operator[](size_type position) const noexcept
  {
    return data_[position];
  }

  const_reference At(size_type position) const
  {
    if (position >= size_)
    {
      throw std::out_of_range("NuoBasicStringView::At position out of range");
    }
    return data_[position];
  }

  constexpr const_reference Front() const
  {
    return data_[0];
  }

  constexpr const_reference Back() const
  {
    return data_[size_ - 1];
  }

  constexpr const_pointer Data() const noexcept
  {
    return data_;
  }

  constexpr void RemovePrefix(size_type count) noexcept
  {
    if (count != 0)
    {
      data_ += count;
    }
    size_ -= count;
  }

  constexpr void RemoveSuffix(size_type count) noexcept
  {
    size_ -= count;
  }

  constexpr void Swap(NuoBasicStringView& other) noexcept
  {
    const CharT* data = data_;
    size_type size = size_;
    data_ = other.data_;
    size_ = other.size_;
    other.data_ = data;
    other.size_ = size;
  }

  size_type Copy(CharT* destination, size_type count,
                 size_type position = 0) const
  {
    if (position > size_)
    {
      throw std::out_of_range("NuoBasicStringView::Copy position out of range");
    }
    const size_type copied = (std::min)(count, size_ - position);
    if (copied != 0)
    {
      Traits::copy(destination, data_ + position, copied);
    }
    return copied;
  }

  constexpr NuoBasicStringView Substr(size_type position = 0,
                                      size_type count = NPos) const
  {
    if (position > size_)
    {
      throw std::out_of_range("NuoBasicStringView::Substr position out of range");
    }
    const size_type length = (std::min)(count, size_ - position);
    const CharT* first = position == 0 ? data_ : data_ + position;
    return NuoBasicStringView(first, length);
  }

  constexpr int Compare(NuoBasicStringView other) const noexcept
  {
    const size_type compared = (std::min)(size_, other.size_);
    const int result = compared == 0 ? 0 : Traits::compare(data_, other.data_, compared);
    if (result != 0)
    {
      return result;
    }
    return size_ < other.size_ ? -1 : (size_ > other.size_ ? 1 : 0);
  }

  constexpr int Compare(size_type position, size_type count,
                        NuoBasicStringView other) const
  {
    return Substr(position, count).Compare(other);
  }

  constexpr int Compare(size_type position, size_type count,
                        const CharT* other) const
  {
    return Compare(position, count, NuoBasicStringView(other));
  }

  constexpr int Compare(size_type position, size_type count,
                        const CharT* other, size_type other_count) const
  {
    return Compare(position, count, NuoBasicStringView(other, other_count));
  }

  constexpr int Compare(size_type position, size_type count,
                        NuoBasicStringView other, size_type other_position,
                        size_type other_count = NPos) const
  {
    return Substr(position, count).Compare(other.Substr(other_position, other_count));
  }

  constexpr int Compare(const CharT* other) const
  {
    return Compare(NuoBasicStringView(other));
  }

  constexpr bool StartsWith(NuoBasicStringView prefix) const noexcept
  {
    return size_ >= prefix.size_ &&
      (prefix.size_ == 0 || Traits::compare(data_, prefix.data_, prefix.size_) == 0);
  }

  constexpr bool StartsWith(CharT value) const noexcept
  {
    return !Empty() && Traits::eq(Front(), value);
  }

  constexpr bool StartsWith(const CharT* prefix) const
  {
    return StartsWith(NuoBasicStringView(prefix));
  }

  constexpr bool EndsWith(NuoBasicStringView suffix) const noexcept
  {
    return size_ >= suffix.size_ &&
      (suffix.size_ == 0 ||
       Traits::compare(data_ + size_ - suffix.size_, suffix.data_, suffix.size_) == 0);
  }

  constexpr bool EndsWith(CharT value) const noexcept
  {
    return !Empty() && Traits::eq(Back(), value);
  }

  constexpr bool EndsWith(const CharT* suffix) const
  {
    return EndsWith(NuoBasicStringView(suffix));
  }

  constexpr size_type Find(NuoBasicStringView value,
                           size_type position = 0) const noexcept
  {
    if (position > size_)
    {
      return NPos;
    }
    if (value.size_ == 0)
    {
      return position;
    }
    if (value.size_ > size_ - position)
    {
      return NPos;
    }
    for (size_type index = position; index <= size_ - value.size_; ++index)
    {
      if (Traits::compare(data_ + index, value.data_, value.size_) == 0)
      {
        return index;
      }
    }
    return NPos;
  }

  constexpr size_type Find(CharT value, size_type position = 0) const noexcept
  {
    for (size_type index = position; index < size_; ++index)
    {
      if (Traits::eq(data_[index], value))
      {
        return index;
      }
    }
    return NPos;
  }

  constexpr size_type Find(const CharT* value, size_type position,
                           size_type count) const noexcept
  {
    return Find(NuoBasicStringView(value, count), position);
  }

  constexpr size_type Find(const CharT* value,
                           size_type position = 0) const
  {
    return Find(NuoBasicStringView(value), position);
  }

  constexpr size_type FindFirstOf(const CharT* values, size_type position,
                                  size_type count) const noexcept
  {
    return FindFirstOf(NuoBasicStringView(values, count), position);
  }

  constexpr size_type FindFirstOf(NuoBasicStringView values,
                                  size_type position = 0) const noexcept
  {
    for (size_type index = position; index < size_; ++index)
    {
      if (values.Find(data_[index]) != NPos)
      {
        return index;
      }
    }
    return NPos;
  }

  constexpr size_type FindFirstOf(CharT value,
                                  size_type position = 0) const noexcept
  {
    return Find(value, position);
  }

  constexpr size_type FindFirstOf(const CharT* values,
                                  size_type position = 0) const
  {
    return FindFirstOf(NuoBasicStringView(values), position);
  }

  constexpr size_type FindLastOf(NuoBasicStringView values,
                                 size_type position = NPos) const noexcept
  {
    if (size_ == 0)
    {
      return NPos;
    }
    size_type index = (std::min)(position, size_ - 1);
    for (;;)
    {
      if (values.Find(data_[index]) != NPos)
      {
        return index;
      }
      if (index == 0)
      {
        return NPos;
      }
      --index;
    }
  }

  constexpr size_type FindLastOf(CharT value,
                                 size_type position = NPos) const noexcept
  {
    return RFind(value, position);
  }

  constexpr size_type FindLastOf(const CharT* values,
                                 size_type position = NPos) const
  {
    return FindLastOf(NuoBasicStringView(values), position);
  }

  constexpr size_type FindLastOf(const CharT* values, size_type position,
                                 size_type count) const noexcept
  {
    return FindLastOf(NuoBasicStringView(values, count), position);
  }

  constexpr size_type FindFirstNotOf(NuoBasicStringView values,
                                     size_type position = 0) const noexcept
  {
    for (size_type index = position; index < size_; ++index)
    {
      if (values.Find(data_[index]) == NPos)
      {
        return index;
      }
    }
    return NPos;
  }

  constexpr size_type FindFirstNotOf(CharT value,
                                     size_type position = 0) const noexcept
  {
    for (size_type index = position; index < size_; ++index)
    {
      if (!Traits::eq(data_[index], value))
      {
        return index;
      }
    }
    return NPos;
  }

  constexpr size_type FindFirstNotOf(const CharT* values,
                                     size_type position = 0) const
  {
    return FindFirstNotOf(NuoBasicStringView(values), position);
  }

  constexpr size_type FindFirstNotOf(const CharT* values, size_type position,
                                     size_type count) const noexcept
  {
    return FindFirstNotOf(NuoBasicStringView(values, count), position);
  }

  constexpr size_type FindLastNotOf(NuoBasicStringView values,
                                    size_type position = NPos) const noexcept
  {
    if (size_ == 0)
    {
      return NPos;
    }
    size_type index = (std::min)(position, size_ - 1);
    for (;;)
    {
      if (values.Find(data_[index]) == NPos)
      {
        return index;
      }
      if (index == 0)
      {
        return NPos;
      }
      --index;
    }
  }

  constexpr size_type FindLastNotOf(CharT value,
                                    size_type position = NPos) const noexcept
  {
    if (size_ == 0)
    {
      return NPos;
    }
    size_type index = (std::min)(position, size_ - 1);
    for (;;)
    {
      if (!Traits::eq(data_[index], value))
      {
        return index;
      }
      if (index == 0)
      {
        return NPos;
      }
      --index;
    }
  }

  constexpr size_type FindLastNotOf(const CharT* values,
                                    size_type position = NPos) const
  {
    return FindLastNotOf(NuoBasicStringView(values), position);
  }

  constexpr size_type FindLastNotOf(const CharT* values, size_type position,
                                    size_type count) const noexcept
  {
    return FindLastNotOf(NuoBasicStringView(values, count), position);
  }

  constexpr size_type RFind(NuoBasicStringView value,
                            size_type position = NPos) const noexcept
  {
    if (value.size_ > size_)
    {
      return NPos;
    }
    size_type index = (std::min)(position, size_ - value.size_);
    for (;;)
    {
      if (value.size_ == 0 ||
          Traits::compare(data_ + index, value.data_, value.size_) == 0)
      {
        return index;
      }
      if (index == 0)
      {
        return NPos;
      }
      --index;
    }
  }

  constexpr size_type RFind(CharT value, size_type position = NPos) const noexcept
  {
    if (size_ == 0)
    {
      return NPos;
    }
    size_type index = (std::min)(position, size_ - 1);
    for (;;)
    {
      if (Traits::eq(data_[index], value))
      {
        return index;
      }
      if (index == 0)
      {
        return NPos;
      }
      --index;
    }
  }

  constexpr size_type RFind(const CharT* value,
                            size_type position = NPos) const
  {
    return RFind(NuoBasicStringView(value), position);
  }

  constexpr size_type RFind(const CharT* value, size_type position,
                            size_type count) const noexcept
  {
    return RFind(NuoBasicStringView(value, count), position);
  }

private:
  const CharT* data_;
  size_type size_;
};

using NuoStringView = NuoBasicStringView<char>;
using NuoWStringView = NuoBasicStringView<wchar_t>;
using NuoU8StringView = NuoBasicStringView<char8_t>;
using NuoU16StringView = NuoBasicStringView<char16_t>;
using NuoU32StringView = NuoBasicStringView<char32_t>;

template <class CharT, class Traits>
constexpr bool operator==(NuoBasicStringView<CharT, Traits> lhs,
                          NuoBasicStringView<CharT, Traits> rhs) noexcept
{
  return lhs.Compare(rhs) == 0;
}

template <class CharT, class Traits>
constexpr bool operator==(NuoBasicStringView<CharT, Traits> lhs,
                          const CharT* rhs)
{
  return lhs == NuoBasicStringView<CharT, Traits>(rhs);
}

template <class CharT, class Traits>
constexpr bool operator==(const CharT* lhs,
                          NuoBasicStringView<CharT, Traits> rhs)
{
  return NuoBasicStringView<CharT, Traits>(lhs) == rhs;
}

template <class CharT, class Traits>
constexpr bool operator!=(NuoBasicStringView<CharT, Traits> lhs,
                          NuoBasicStringView<CharT, Traits> rhs) noexcept
{
  return !(lhs == rhs);
}

template <class CharT, class Traits>
constexpr bool operator!=(NuoBasicStringView<CharT, Traits> lhs,
                          const CharT* rhs)
{
  return !(lhs == rhs);
}

template <class CharT, class Traits>
constexpr bool operator!=(const CharT* lhs,
                          NuoBasicStringView<CharT, Traits> rhs)
{
  return !(lhs == rhs);
}

template <class CharT, class Traits>
constexpr bool operator<(NuoBasicStringView<CharT, Traits> lhs,
                         NuoBasicStringView<CharT, Traits> rhs) noexcept
{
  return lhs.Compare(rhs) < 0;
}

template <class CharT, class Traits>
constexpr bool operator>(NuoBasicStringView<CharT, Traits> lhs,
                         NuoBasicStringView<CharT, Traits> rhs) noexcept
{
  return rhs < lhs;
}

template <class CharT, class Traits>
constexpr bool operator<=(NuoBasicStringView<CharT, Traits> lhs,
                          NuoBasicStringView<CharT, Traits> rhs) noexcept
{
  return !(rhs < lhs);
}

template <class CharT, class Traits>
constexpr bool operator>=(NuoBasicStringView<CharT, Traits> lhs,
                          NuoBasicStringView<CharT, Traits> rhs) noexcept
{
  return !(lhs < rhs);
}

template <class CharT, class Traits>
constexpr void NuoSwap(NuoBasicStringView<CharT, Traits>& lhs,
                       NuoBasicStringView<CharT, Traits>& rhs) noexcept
{
  lhs.Swap(rhs);
}

} /* namespace nuostl */
