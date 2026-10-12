#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "core/seq_cont/nuo_vector.hpp"

namespace nuostl
{

/* cppreference declaration:
   template< class Allocator >
   class vector<bool, Allocator>;
   https://en.cppreference.com/w/cpp/container/vector_bool.html
   The specialization packs one bit per element, so element access returns a
   proxy object that converts to and assigns from bool. */
template <typename Allocator>
class nuo_vector<bool, Allocator>
{
public:
  using value_type      = bool;
  using allocator_type  = Allocator;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;

  /* Proxy returned by non-const access; a bool& cannot address a single bit. */
  class reference
  {
  public:
    reference() noexcept = default;

    operator bool() const noexcept
    {
      return owner_->GetBit(position_);
    }

    reference& operator=(bool value) noexcept
    {
      owner_->SetBit(position_, value);
      return *this;
    }

    reference& operator=(const reference& other) noexcept
    {
      return *this = static_cast<bool>(other);
    }

    reference& flip() noexcept
    {
      owner_->SetBit(position_, !owner_->GetBit(position_));
      return *this;
    }

    bool operator~() const noexcept
    {
      return !owner_->GetBit(position_);
    }

    /* Hidden friend so algorithms exchanging proxies find it through ADL. */
    friend void swap(reference lhs, reference rhs) noexcept
    {
      const bool value = static_cast<bool>(lhs);
      lhs = static_cast<bool>(rhs);
      rhs = value;
    }

  private:
    friend class nuo_vector<bool, Allocator>;

    reference(nuo_vector* owner, size_type position) noexcept
      : owner_(owner)
      , position_(position)
    {
    }

    nuo_vector* owner_ = nullptr;
    size_type position_ = 0;
  };

  using const_reference = bool;

  template <bool IsConst>
  class IteratorBase;

  using iterator = IteratorBase<false>;
  using const_iterator = IteratorBase<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /* construct / copy / destroy */
  nuo_vector() noexcept(noexcept(Allocator()));
  explicit nuo_vector(const Allocator& alloc) noexcept;
  explicit nuo_vector(size_type n, const Allocator& alloc = Allocator());
  nuo_vector(size_type n, bool value, const Allocator& alloc = Allocator());

  template <typename InputIterator>
  nuo_vector(InputIterator first, InputIterator last,
             const Allocator& alloc = Allocator());

  nuo_vector(std::initializer_list<bool> values,
             const Allocator& alloc = Allocator());
  nuo_vector(const nuo_vector& other);
  nuo_vector(nuo_vector&& other) noexcept;
  nuo_vector(const nuo_vector& other, const Allocator& alloc);
  ~nuo_vector();

  /* assign */
  nuo_vector& operator=(const nuo_vector& other);
  nuo_vector& operator=(nuo_vector&& other) noexcept;
  nuo_vector& operator=(std::initializer_list<bool> values);

  void assign(size_type n, bool value);

  template <typename InputIterator>
  void assign(InputIterator first, InputIterator last);

  void assign(std::initializer_list<bool> values);
  allocator_type get_allocator() const noexcept;

  /* iterators */
  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;
  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  reverse_iterator rend() noexcept;
  const_reverse_iterator rend() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

  /* capacity */
  bool empty() const noexcept;
  size_type size() const noexcept;
  size_type max_size() const noexcept;
  size_type capacity() const noexcept;
  void reserve(size_type n);
  void shrink_to_fit();

  /* element access */
  reference operator[](size_type n);
  const_reference operator[](size_type n) const;
  reference at(size_type n);
  const_reference at(size_type n) const;
  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;

  /* modifiers */
  void clear() noexcept;
  void push_back(bool value);
  template <typename... Args>
  reference emplace_back(Args&&... args);
  void pop_back();
  void resize(size_type n);
  void resize(size_type n, bool value);
  void flip() noexcept;

  iterator insert(const_iterator position, bool value);
  iterator insert(const_iterator position, size_type n, bool value);

  template <typename InputIterator>
  iterator insert(const_iterator position, InputIterator first,
                  InputIterator last);

  iterator insert(const_iterator position, std::initializer_list<bool> values);

  template <typename... Args>
  iterator emplace(const_iterator position, Args&&... args);

  iterator erase(const_iterator position);
  iterator erase(const_iterator first, const_iterator last);

  void swap(nuo_vector& other) noexcept;

  friend void swap(nuo_vector& lhs, nuo_vector& rhs) noexcept
  {
    lhs.swap(rhs);
  }

private:
  using word_type = std::uint64_t;
  using word_allocator =
    typename std::allocator_traits<Allocator>::template rebind_alloc<word_type>;
  using word_allocator_traits = std::allocator_traits<word_allocator>;

  static constexpr size_type kBitsPerWord = sizeof(word_type) * 8;

  static bool BitAt(const word_type* words, size_type position) noexcept;
  static void SetBitAt(word_type* words, size_type position,
                       bool value) noexcept;
  static size_type WordCount(size_type bits) noexcept;

  bool GetBit(size_type position) const noexcept;
  void SetBit(size_type position, bool value) noexcept;
  void ClearBits(size_type first, size_type last) noexcept;
  void MoveBits(size_type destination, size_type source,
                size_type count) noexcept;
  void ReallocateBits(size_type bits);
  void RequireBits(size_type bits);
  size_type GrownCapacity(size_type required) const noexcept;
  void DeallocateWords() noexcept;
  iterator InsertRange(const_iterator position, const nuo_vector& source);
  size_type Offset(const_iterator position) const noexcept;

  word_type* words_;
  size_type size_;
  size_type capacity_;  /* measured in bits */
  allocator_type alloc_;
  word_allocator word_alloc_;
};

/* Random access iterator over packed bits. */
template <typename Allocator>
template <bool IsConst>
class nuo_vector<bool, Allocator>::IteratorBase
{
  template <bool>
  friend class IteratorBase;

public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type        = bool;
  using difference_type   = ptrdiff_t;
  using pointer           = std::conditional_t<IsConst, const bool*, bool*>;
  using proxy_reference = typename nuo_vector<bool, Allocator>::reference;
  using reference = std::conditional_t<IsConst, bool, proxy_reference>;

  IteratorBase() noexcept = default;

  IteratorBase(nuo_vector* owner, size_type position) noexcept
    : owner_(owner)
    , position_(position)
  {
  }

  /* Mutable iterators convert to const ones, never the other way around. */
  template <bool Other>
    requires (IsConst || !Other)
  IteratorBase(const IteratorBase<Other>& other) noexcept
    : owner_(other.owner_)
    , position_(other.position_)
  {
  }

  reference operator*() const noexcept
  {
    if constexpr (IsConst)
    {
      return owner_->GetBit(position_);
    }
    else
    {
      return reference(owner_, position_);
    }
  }

  reference operator[](difference_type n) const noexcept
  {
    return *(*this + n);
  }

  IteratorBase& operator++() noexcept
  {
    ++position_;
    return *this;
  }

  IteratorBase operator++(int) noexcept
  {
    IteratorBase copy = *this;
    ++position_;
    return copy;
  }

  IteratorBase& operator--() noexcept
  {
    --position_;
    return *this;
  }

  IteratorBase operator--(int) noexcept
  {
    IteratorBase copy = *this;
    --position_;
    return copy;
  }

  IteratorBase& operator+=(difference_type n) noexcept
  {
    position_ = static_cast<size_type>(
      static_cast<difference_type>(position_) + n);
    return *this;
  }

  IteratorBase& operator-=(difference_type n) noexcept
  {
    return *this += -n;
  }

  IteratorBase operator+(difference_type n) const noexcept
  {
    IteratorBase copy = *this;
    copy += n;
    return copy;
  }

  IteratorBase operator-(difference_type n) const noexcept
  {
    IteratorBase copy = *this;
    copy -= n;
    return copy;
  }

  template <bool Other>
  difference_type operator-(const IteratorBase<Other>& other) const noexcept
  {
    return static_cast<difference_type>(position_) -
      static_cast<difference_type>(other.position_);
  }

  template <bool Other>
  bool operator==(const IteratorBase<Other>& other) const noexcept
  {
    return owner_ == other.owner_ && position_ == other.position_;
  }

  template <bool Other>
  bool operator!=(const IteratorBase<Other>& other) const noexcept
  {
    return !(*this == other);
  }

  template <bool Other>
  bool operator<(const IteratorBase<Other>& other) const noexcept
  {
    return position_ < other.position_;
  }

  template <bool Other>
  bool operator<=(const IteratorBase<Other>& other) const noexcept
  {
    return position_ <= other.position_;
  }

  template <bool Other>
  bool operator>(const IteratorBase<Other>& other) const noexcept
  {
    return position_ > other.position_;
  }

  template <bool Other>
  bool operator>=(const IteratorBase<Other>& other) const noexcept
  {
    return position_ >= other.position_;
  }

  friend IteratorBase operator+(difference_type n,
                                IteratorBase iterator) noexcept
  {
    iterator += n;
    return iterator;
  }

private:
  nuo_vector* owner_ = nullptr;
  size_type position_ = 0;
};

/* implementation */

template <typename Allocator>
bool nuo_vector<bool, Allocator>::BitAt(const word_type* words,
                                        size_type position) noexcept
{
  return ((words[position / kBitsPerWord] >>
           (position % kBitsPerWord)) & word_type(1)) != word_type(0);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::SetBitAt(word_type* words, size_type position,
                                           bool value) noexcept
{
  const word_type mask = word_type(1) << (position % kBitsPerWord);
  if (value)
  {
    words[position / kBitsPerWord] |= mask;
  }
  else
  {
    words[position / kBitsPerWord] &= static_cast<word_type>(~mask);
  }
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::size_type
nuo_vector<bool, Allocator>::WordCount(size_type bits) noexcept
{
  return bits == 0 ? 0 : (bits - 1) / kBitsPerWord + 1;
}

template <typename Allocator>
bool nuo_vector<bool, Allocator>::GetBit(size_type position) const noexcept
{
  return BitAt(words_, position);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::SetBit(size_type position,
                                         bool value) noexcept
{
  SetBitAt(words_, position, value);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::ClearBits(size_type first,
                                            size_type last) noexcept
{
  for (size_type position = first; position < last; ++position)
  {
    SetBit(position, false);
  }
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::MoveBits(size_type destination,
                                           size_type source,
                                           size_type count) noexcept
{
  if (destination == source || count == 0)
  {
    return;
  }
  if (destination < source)
  {
    for (size_type index = 0; index < count; ++index)
    {
      SetBit(destination + index, GetBit(source + index));
    }
  }
  else
  {
    for (size_type index = count; index > 0; --index)
    {
      SetBit(destination + index - 1, GetBit(source + index - 1));
    }
  }
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::ReallocateBits(size_type bits)
{
  const size_type new_word_count = WordCount(bits);
  word_type* new_words =
    word_allocator_traits::allocate(word_alloc_, new_word_count);
  for (size_type index = 0; index < new_word_count; ++index)
  {
    new_words[index] = word_type(0);
  }
  for (size_type index = 0; index < size_; ++index)
  {
    if (BitAt(words_, index))
    {
      SetBitAt(new_words, index, true);
    }
  }
  if (words_ != nullptr)
  {
    word_allocator_traits::deallocate(word_alloc_, words_,
                                      WordCount(capacity_));
  }
  words_ = new_words;
  capacity_ = new_word_count * kBitsPerWord;
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::RequireBits(size_type bits)
{
  if (bits > capacity_)
  {
    ReallocateBits(GrownCapacity(bits));
  }
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::size_type
nuo_vector<bool, Allocator>::GrownCapacity(size_type required) const noexcept
{
  const size_type doubled = capacity_ == 0 ? kBitsPerWord : capacity_ * 2;
  return doubled > required ? doubled : required;
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::DeallocateWords() noexcept
{
  if (words_ != nullptr)
  {
    word_allocator_traits::deallocate(word_alloc_, words_,
                                      WordCount(capacity_));
    words_ = nullptr;
  }
  capacity_ = 0;
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::size_type
nuo_vector<bool, Allocator>::Offset(const_iterator position) const noexcept
{
  return static_cast<size_type>(position - cbegin());
}

/* construct / copy / destroy */

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector() noexcept(noexcept(Allocator()))
  : words_(nullptr)
  , size_(0)
  , capacity_(0)
  , alloc_()
  , word_alloc_(alloc_)
{
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(const Allocator& alloc) noexcept
  : words_(nullptr)
  , size_(0)
  , capacity_(0)
  , alloc_(alloc)
  , word_alloc_(alloc_)
{
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(size_type n, const Allocator& alloc)
  : words_(nullptr)
  , size_(0)
  , capacity_(0)
  , alloc_(alloc)
  , word_alloc_(alloc_)
{
  if (n != 0)
  {
    ReallocateBits(n);
    size_ = n;
  }
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(size_type n, bool value,
                                        const Allocator& alloc)
  : nuo_vector(n, alloc)
{
  if (value)
  {
    for (size_type index = 0; index < size_; ++index)
    {
      SetBit(index, true);
    }
  }
}

template <typename Allocator>
template <typename InputIterator>
nuo_vector<bool, Allocator>::nuo_vector(InputIterator first,
                                        InputIterator last,
                                        const Allocator& alloc)
  : nuo_vector(alloc)
{
  for (; first != last; ++first)
  {
    push_back(static_cast<bool>(*first));
  }
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(std::initializer_list<bool> values,
                                        const Allocator& alloc)
  : nuo_vector(alloc)
{
  for (bool value : values)
  {
    push_back(value);
  }
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(const nuo_vector& other)
  : words_(nullptr)
  , size_(0)
  , capacity_(0)
  , alloc_(other.alloc_)
  , word_alloc_(alloc_)
{
  if (other.size_ != 0)
  {
    ReallocateBits(other.size_);
    for (size_type index = 0; index < other.size_; ++index)
    {
      if (other.BitAt(other.words_, index))
      {
        SetBitAt(words_, index, true);
      }
    }
    size_ = other.size_;
  }
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(const nuo_vector& other,
                                        const Allocator& alloc)
  : nuo_vector(other)
{
  (void)alloc;
}

template <typename Allocator>
nuo_vector<bool, Allocator>::nuo_vector(nuo_vector&& other) noexcept
  : words_(other.words_)
  , size_(other.size_)
  , capacity_(other.capacity_)
  , alloc_(std::move(other.alloc_))
  , word_alloc_(std::move(other.word_alloc_))
{
  other.words_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

template <typename Allocator>
nuo_vector<bool, Allocator>::~nuo_vector()
{
  DeallocateWords();
}

/* assign */

template <typename Allocator>
nuo_vector<bool, Allocator>&
nuo_vector<bool, Allocator>::operator=(const nuo_vector& other)
{
  if (this != &other)
  {
    nuo_vector copy(other);
    swap(copy);
  }
  return *this;
}

template <typename Allocator>
nuo_vector<bool, Allocator>&
nuo_vector<bool, Allocator>::operator=(nuo_vector&& other) noexcept
{
  if (this != &other)
  {
    DeallocateWords();
    words_ = other.words_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.words_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }
  return *this;
}

template <typename Allocator>
nuo_vector<bool, Allocator>&
nuo_vector<bool, Allocator>::operator=(std::initializer_list<bool> values)
{
  assign(values);
  return *this;
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::assign(size_type n, bool value)
{
  nuo_vector replacement(n, value, alloc_);
  swap(replacement);
}

template <typename Allocator>
template <typename InputIterator>
void nuo_vector<bool, Allocator>::assign(InputIterator first,
                                         InputIterator last)
{
  nuo_vector replacement(first, last, alloc_);
  swap(replacement);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::assign(std::initializer_list<bool> values)
{
  nuo_vector replacement(values, alloc_);
  swap(replacement);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::allocator_type
nuo_vector<bool, Allocator>::get_allocator() const noexcept
{
  return alloc_;
}

/* iterators */

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::begin() noexcept
{
  return iterator(this, 0);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_iterator
nuo_vector<bool, Allocator>::begin() const noexcept
{
  return cbegin();
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::end() noexcept
{
  return iterator(this, size_);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_iterator
nuo_vector<bool, Allocator>::end() const noexcept
{
  return cend();
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_iterator
nuo_vector<bool, Allocator>::cbegin() const noexcept
{
  return const_iterator(const_cast<nuo_vector*>(this), 0);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_iterator
nuo_vector<bool, Allocator>::cend() const noexcept
{
  return const_iterator(const_cast<nuo_vector*>(this), size_);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::reverse_iterator
nuo_vector<bool, Allocator>::rbegin() noexcept
{
  return reverse_iterator(end());
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reverse_iterator
nuo_vector<bool, Allocator>::rbegin() const noexcept
{
  return crbegin();
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::reverse_iterator
nuo_vector<bool, Allocator>::rend() noexcept
{
  return reverse_iterator(begin());
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reverse_iterator
nuo_vector<bool, Allocator>::rend() const noexcept
{
  return crend();
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reverse_iterator
nuo_vector<bool, Allocator>::crbegin() const noexcept
{
  return const_reverse_iterator(cend());
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reverse_iterator
nuo_vector<bool, Allocator>::crend() const noexcept
{
  return const_reverse_iterator(cbegin());
}

/* capacity */

template <typename Allocator>
bool nuo_vector<bool, Allocator>::empty() const noexcept
{
  return size_ == 0;
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::size_type
nuo_vector<bool, Allocator>::size() const noexcept
{
  return size_;
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::size_type
nuo_vector<bool, Allocator>::max_size() const noexcept
{
  return word_allocator_traits::max_size(word_alloc_) * kBitsPerWord;
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::size_type
nuo_vector<bool, Allocator>::capacity() const noexcept
{
  return capacity_;
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::reserve(size_type n)
{
  if (n > capacity_)
  {
    ReallocateBits(n);
  }
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::shrink_to_fit()
{
  if (size_ == 0)
  {
    DeallocateWords();
    return;
  }
  const size_type target = WordCount(size_) * kBitsPerWord;
  if (target < capacity_)
  {
    ReallocateBits(size_);
  }
}

/* element access */

template <typename Allocator>
typename nuo_vector<bool, Allocator>::reference
nuo_vector<bool, Allocator>::operator[](size_type n)
{
  return reference(this, n);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reference
nuo_vector<bool, Allocator>::operator[](size_type n) const
{
  return GetBit(n);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::reference
nuo_vector<bool, Allocator>::at(size_type n)
{
  if (n >= size_)
  {
    throw std::out_of_range("nuo_vector<bool>::at");
  }
  return reference(this, n);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reference
nuo_vector<bool, Allocator>::at(size_type n) const
{
  if (n >= size_)
  {
    throw std::out_of_range("nuo_vector<bool>::at");
  }
  return GetBit(n);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::reference
nuo_vector<bool, Allocator>::front()
{
  return reference(this, 0);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reference
nuo_vector<bool, Allocator>::front() const
{
  return GetBit(0);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::reference
nuo_vector<bool, Allocator>::back()
{
  return reference(this, size_ - 1);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::const_reference
nuo_vector<bool, Allocator>::back() const
{
  return GetBit(size_ - 1);
}

/* modifiers */

template <typename Allocator>
void nuo_vector<bool, Allocator>::clear() noexcept
{
  ClearBits(0, size_);
  size_ = 0;
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::push_back(bool value)
{
  RequireBits(size_ + 1);
  SetBit(size_, value);
  ++size_;
}

template <typename Allocator>
template <typename... Args>
typename nuo_vector<bool, Allocator>::reference
nuo_vector<bool, Allocator>::emplace_back(Args&&... args)
{
  push_back(bool(std::forward<Args>(args)...));
  return reference(this, size_ - 1);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::pop_back()
{
  --size_;
  SetBit(size_, false);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::resize(size_type n)
{
  resize(n, false);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::resize(size_type n, bool value)
{
  if (n < size_)
  {
    ClearBits(n, size_);
  }
  else if (n > size_)
  {
    RequireBits(n);
    for (size_type index = size_; index < n; ++index)
    {
      SetBit(index, value);
    }
  }
  size_ = n;
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::flip() noexcept
{
  for (size_type index = 0; index < size_; ++index)
  {
    SetBit(index, !GetBit(index));
  }
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::insert(const_iterator position, bool value)
{
  const size_type index = Offset(position);
  RequireBits(size_ + 1);
  MoveBits(index + 1, index, size_ - index);
  SetBit(index, value);
  ++size_;
  return iterator(this, index);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::insert(const_iterator position, size_type n,
                                    bool value)
{
  const size_type index = Offset(position);
  if (n == 0)
  {
    return iterator(this, index);
  }
  RequireBits(size_ + n);
  MoveBits(index + n, index, size_ - index);
  for (size_type step = 0; step < n; ++step)
  {
    SetBit(index + step, value);
  }
  size_ += n;
  return iterator(this, index);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::insert(const_iterator position,
                                    std::initializer_list<bool> values)
{
  const nuo_vector buffer(values, alloc_);
  return InsertRange(position, buffer);
}

template <typename Allocator>
template <typename InputIterator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::insert(const_iterator position,
                                    InputIterator first,
                                    InputIterator last)
{
  nuo_vector buffer(alloc_);
  for (; first != last; ++first)
  {
    buffer.push_back(static_cast<bool>(*first));
  }
  return InsertRange(position, buffer);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::InsertRange(const_iterator position,
                                         const nuo_vector& source)
{
  const size_type index = Offset(position);
  const size_type count = source.size();
  if (count == 0)
  {
    return iterator(this, index);
  }
  RequireBits(size_ + count);
  MoveBits(index + count, index, size_ - index);
  for (size_type step = 0; step < count; ++step)
  {
    SetBit(index + step, source.GetBit(step));
  }
  size_ += count;
  return iterator(this, index);
}

template <typename Allocator>
template <typename... Args>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::emplace(const_iterator position, Args&&... args)
{
  return insert(position, bool(std::forward<Args>(args)...));
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::erase(const_iterator position)
{
  return erase(position, position + 1);
}

template <typename Allocator>
typename nuo_vector<bool, Allocator>::iterator
nuo_vector<bool, Allocator>::erase(const_iterator first, const_iterator last)
{
  const size_type index = Offset(first);
  const size_type count = static_cast<size_type>(last - first);
  if (count == 0)
  {
    return iterator(this, index);
  }
  MoveBits(index, index + count, size_ - index - count);
  ClearBits(size_ - count, size_);
  size_ -= count;
  return iterator(this, index);
}

template <typename Allocator>
void nuo_vector<bool, Allocator>::swap(nuo_vector& other) noexcept
{
  word_type* words = words_;
  words_ = other.words_;
  other.words_ = words;
  const size_type size = size_;
  size_ = other.size_;
  other.size_ = size;
  const size_type capacity = capacity_;
  capacity_ = other.capacity_;
  other.capacity_ = capacity;
  using std::swap;
  swap(alloc_, other.alloc_);
  swap(word_alloc_, other.word_alloc_);
}

/* Convenience alias matching the roadmap name. */
template <typename Allocator = std::allocator<bool>>
using NuoVectorBool = nuo_vector<bool, Allocator>;

} /* namespace nuostl */
