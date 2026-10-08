#pragma once

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/seq_cont/nuo_string_view.hpp"

namespace nuostl
{

template <class CharT,
          class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
class NuoBasicString
{
public:
  using traits_type            = Traits;
  using value_type             = CharT;
  using allocator_type         = Allocator;
  using size_type              = typename std::basic_string<CharT, Traits, Allocator>::size_type;
  using difference_type        = typename std::basic_string<CharT, Traits, Allocator>::difference_type;
  using reference              = CharT&;
  using const_reference        = const CharT&;
  using pointer                = CharT*;
  using const_pointer          = const CharT*;
  using iterator               = typename std::basic_string<CharT, Traits, Allocator>::iterator;
  using const_iterator         = typename std::basic_string<CharT, Traits, Allocator>::const_iterator;
  using reverse_iterator       = typename std::basic_string<CharT, Traits, Allocator>::reverse_iterator;
  using const_reverse_iterator = typename std::basic_string<CharT, Traits, Allocator>::const_reverse_iterator;
  using view_type              = NuoBasicStringView<CharT, Traits>;

  static constexpr size_type NPos = std::basic_string<CharT, Traits, Allocator>::npos;
  static constexpr size_type npos = NPos;

  NuoBasicString() = default;

  explicit NuoBasicString(const Allocator& allocator)
    : storage_(allocator)
  {
  }

  NuoBasicString(const CharT* string, const Allocator& allocator = Allocator())
    : storage_(string, allocator)
  {
  }

  NuoBasicString(const CharT* string, size_type count,
                 const Allocator& allocator = Allocator())
    : storage_(string, count, allocator)
  {
  }

  NuoBasicString(size_type count, CharT value,
                 const Allocator& allocator = Allocator())
    : storage_(count, value, allocator)
  {
  }

  NuoBasicString(const std::basic_string<CharT, Traits, Allocator>& string)
    : storage_(string)
  {
  }

  NuoBasicString(std::basic_string<CharT, Traits, Allocator>&& string) noexcept
    : storage_(std::move(string))
  {
  }

  template <class OtherAllocator>
  NuoBasicString(const std::basic_string<CharT, Traits, OtherAllocator>& string)
    : storage_(string.data(), string.size())
  {
  }

  template <class OtherTraits, class OtherAllocator>
  NuoBasicString(const NuoBasicString<CharT, OtherTraits, OtherAllocator>& string)
    : storage_(string.Data(), string.Size())
  {
  }

  NuoBasicString(view_type view)
    : storage_(view.Data(), view.Size())
  {
  }

  NuoBasicString(const NuoBasicString&) = default;
  NuoBasicString(NuoBasicString&&) noexcept = default;
  NuoBasicString& operator=(const NuoBasicString&) = default;
  NuoBasicString& operator=(NuoBasicString&&) noexcept = default;

  NuoBasicString& operator=(const CharT* string)
  {
    storage_ = string;
    return *this;
  }

  NuoBasicString& operator=(CharT value)
  {
    storage_ = value;
    return *this;
  }

  NuoBasicString& operator=(view_type view)
  {
    Assign(view);
    return *this;
  }

  NuoBasicString& operator+=(const NuoBasicString& other)
  {
    Append(other.View());
    return *this;
  }

  NuoBasicString& operator+=(const CharT* string)
  {
    Append(string);
    return *this;
  }

  NuoBasicString& operator+=(CharT value)
  {
    PushBack(value);
    return *this;
  }

  NuoBasicString& operator+=(view_type view)
  {
    Append(view);
    return *this;
  }

  iterator Begin() noexcept { return storage_.begin(); }
  const_iterator Begin() const noexcept { return storage_.begin(); }
  iterator End() noexcept { return storage_.end(); }
  const_iterator End() const noexcept { return storage_.end(); }
  const_iterator CBegin() const noexcept { return storage_.cbegin(); }
  const_iterator CEnd() const noexcept { return storage_.cend(); }
  reverse_iterator RBegin() noexcept { return storage_.rbegin(); }
  const_reverse_iterator RBegin() const noexcept { return storage_.rbegin(); }
  reverse_iterator REnd() noexcept { return storage_.rend(); }
  const_reverse_iterator REnd() const noexcept { return storage_.rend(); }
  const_reverse_iterator CRBegin() const noexcept { return storage_.crbegin(); }
  const_reverse_iterator CREnd() const noexcept { return storage_.crend(); }

  bool Empty() const noexcept { return storage_.empty(); }
  allocator_type GetAllocator() const noexcept { return storage_.get_allocator(); }
  size_type Size() const noexcept { return storage_.size(); }
  size_type Length() const noexcept { return storage_.length(); }
  size_type MaxSize() const noexcept { return storage_.max_size(); }
  size_type Capacity() const noexcept { return storage_.capacity(); }
  void Reserve(size_type count) { storage_.reserve(count); }
  void ShrinkToFit() { storage_.shrink_to_fit(); }
  void Resize(size_type count) { storage_.resize(count); }
  void Resize(size_type count, CharT value) { storage_.resize(count, value); }

  reference operator[](size_type position) noexcept { return storage_[position]; }
  const_reference operator[](size_type position) const noexcept { return storage_[position]; }
  reference At(size_type position) { return storage_.at(position); }
  const_reference At(size_type position) const { return storage_.at(position); }
  reference Front() { return storage_.front(); }
  const_reference Front() const { return storage_.front(); }
  reference Back() { return storage_.back(); }
  const_reference Back() const { return storage_.back(); }
  pointer Data() noexcept { return storage_.data(); }
  const_pointer Data() const noexcept { return storage_.data(); }
  const_pointer CStr() const noexcept { return storage_.c_str(); }

  void Clear() noexcept { storage_.clear(); }

  void Assign(const CharT* string) { storage_ = string; }
  void Assign(const CharT* string, size_type count) { storage_.assign(string, count); }
  void Assign(size_type count, CharT value) { storage_.assign(count, value); }
  void Assign(view_type view) { storage_.assign(view.Data(), view.Size()); }

  void Append(const CharT* string) { storage_.append(string); }
  void Append(const CharT* string, size_type count) { storage_.append(string, count); }
  void Append(size_type count, CharT value) { storage_.append(count, value); }
  void Append(view_type view) { storage_.append(view.Data(), view.Size()); }
  void PushBack(CharT value) { storage_.push_back(value); }
  void PopBack() { storage_.pop_back(); }

  void Insert(size_type position, const CharT* string) { storage_.insert(position, string); }
  void Insert(size_type position, size_type count, CharT value)
  {
    storage_.insert(position, count, value);
  }
  void Erase(size_type position = 0, size_type count = NPos)
  {
    storage_.erase(position, count);
  }
  void Replace(size_type position, size_type count, const CharT* string)
  {
    storage_.replace(position, count, string);
  }

  size_type Copy(CharT* destination, size_type count, size_type position = 0) const
  {
    return storage_.copy(destination, count, position);
  }

  NuoBasicString Substr(size_type position = 0, size_type count = NPos) const
  {
    return NuoBasicString(storage_.substr(position, count));
  }

  int Compare(const NuoBasicString& other) const noexcept
  {
    return storage_.compare(other.storage_);
  }
  int Compare(view_type other) const noexcept
  {
    return storage_.compare(0, storage_.size(), other.Data(), other.Size());
  }

  size_type Find(view_type value, size_type position = 0) const noexcept
  {
    return value.Empty() ? (position <= Size() ? position : NPos) :
      storage_.find(value.Data(), position, value.Size());
  }
  size_type Find(CharT value, size_type position = 0) const noexcept
  {
    return storage_.find(value, position);
  }
  size_type RFind(view_type value, size_type position = NPos) const noexcept
  {
    return value.Empty() ? (std::min)(position, Size()) :
      storage_.rfind(value.Data(), position, value.Size());
  }
  size_type RFind(CharT value, size_type position = NPos) const noexcept
  {
    return storage_.rfind(value, position);
  }
  size_type FindFirstOf(view_type value, size_type position = 0) const noexcept
  {
    return value.Empty() ? NPos :
      storage_.find_first_of(value.Data(), position, value.Size());
  }
  size_type FindLastOf(view_type value, size_type position = NPos) const noexcept
  {
    return value.Empty() ? NPos :
      storage_.find_last_of(value.Data(), position, value.Size());
  }
  size_type FindFirstNotOf(view_type value, size_type position = 0) const noexcept
  {
    return value.Empty() ? (position < Size() ? position : NPos) :
      storage_.find_first_not_of(value.Data(), position, value.Size());
  }
  size_type FindLastNotOf(view_type value, size_type position = NPos) const noexcept
  {
    return value.Empty() ? (Empty() ? NPos : (std::min)(position, Size() - 1)) :
      storage_.find_last_not_of(value.Data(), position, value.Size());
  }

  bool StartsWith(view_type prefix) const noexcept
  {
    return View().StartsWith(prefix);
  }
  bool EndsWith(view_type suffix) const noexcept
  {
    return View().EndsWith(suffix);
  }
  view_type View() const noexcept
  {
    return view_type(storage_.data(), storage_.size());
  }

  void Swap(NuoBasicString& other) noexcept(noexcept(storage_.swap(other.storage_)))
  {
    storage_.swap(other.storage_);
  }

private:
  std::basic_string<CharT, Traits, Allocator> storage_;
};

using NuoString = NuoBasicString<char>;
using NuoWString = NuoBasicString<wchar_t>;
using NuoU8String = NuoBasicString<char8_t>;
using NuoU16String = NuoBasicString<char16_t>;
using NuoU32String = NuoBasicString<char32_t>;

template <class CharT, class Traits, class Allocator>
bool operator==(const NuoBasicString<CharT, Traits, Allocator>& left,
                const NuoBasicString<CharT, Traits, Allocator>& right) noexcept
{
  return left.Compare(right) == 0;
}

template <class CharT, class Traits, class Allocator>
bool operator!=(const NuoBasicString<CharT, Traits, Allocator>& left,
                const NuoBasicString<CharT, Traits, Allocator>& right) noexcept
{
  return !(left == right);
}

template <class CharT, class Traits, class Allocator>
bool operator<(const NuoBasicString<CharT, Traits, Allocator>& left,
               const NuoBasicString<CharT, Traits, Allocator>& right) noexcept
{
  return left.Compare(right) < 0;
}

template <class CharT, class Traits, class Allocator>
bool operator>(const NuoBasicString<CharT, Traits, Allocator>& left,
               const NuoBasicString<CharT, Traits, Allocator>& right) noexcept
{
  return right < left;
}

template <class CharT, class Traits, class Allocator>
bool operator<=(const NuoBasicString<CharT, Traits, Allocator>& left,
                const NuoBasicString<CharT, Traits, Allocator>& right) noexcept
{
  return !(right < left);
}

template <class CharT, class Traits, class Allocator>
bool operator>=(const NuoBasicString<CharT, Traits, Allocator>& left,
                const NuoBasicString<CharT, Traits, Allocator>& right) noexcept
{
  return !(left < right);
}

template <class CharT, class Traits, class Allocator>
bool operator==(const NuoBasicString<CharT, Traits, Allocator>& left,
                const CharT* right)
{
  return left.Compare(NuoBasicStringView<CharT, Traits>(right)) == 0;
}

template <class CharT, class Traits, class Allocator>
bool operator!=(const NuoBasicString<CharT, Traits, Allocator>& left,
                const CharT* right)
{
  return !(left == right);
}

template <class CharT, class Traits, class Allocator>
bool operator==(const CharT* left,
                const NuoBasicString<CharT, Traits, Allocator>& right)
{
  return right == left;
}

template <class CharT, class Traits, class Allocator>
bool operator!=(const CharT* left,
                const NuoBasicString<CharT, Traits, Allocator>& right)
{
  return !(left == right);
}

template <class CharT, class Traits, class Allocator>
NuoBasicString<CharT, Traits, Allocator> operator+(
  const NuoBasicString<CharT, Traits, Allocator>& left,
  const NuoBasicString<CharT, Traits, Allocator>& right)
{
  NuoBasicString<CharT, Traits, Allocator> result(left);
  result += right;
  return result;
}

template <class CharT, class Traits, class Allocator>
NuoBasicString<CharT, Traits, Allocator> operator+(
  const NuoBasicString<CharT, Traits, Allocator>& left,
  const CharT* right)
{
  NuoBasicString<CharT, Traits, Allocator> result(left);
  result += right;
  return result;
}

template <class CharT, class Traits, class Allocator>
NuoBasicString<CharT, Traits, Allocator> operator+(
  const CharT* left,
  const NuoBasicString<CharT, Traits, Allocator>& right)
{
  NuoBasicString<CharT, Traits, Allocator> result(left);
  result += right;
  return result;
}

template <class CharT, class Traits, class Allocator>
void NuoSwap(NuoBasicString<CharT, Traits, Allocator>& left,
             NuoBasicString<CharT, Traits, Allocator>& right) noexcept(
  noexcept(left.Swap(right)))
{
  left.Swap(right);
}

} /* namespace nuostl */
