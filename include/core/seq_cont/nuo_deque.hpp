#pragma once

#include "nuo_typedefs.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

#ifndef DEQUE_MAP_INIT_SIZE
#define DEQUE_MAP_INIT_SIZE 8
#endif

template <typename T>
struct NuoDequeBufSize
{
  static constexpr size_t value = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;
};

/*****************************************************************************/
/* deque iterator definition */
template <typename T, typename Ref, typename Ptr>
class NuoDequeIterator
{
public:
  using self            = NuoDequeIterator<T, T&, T*>;
  using iterator        = NuoDequeIterator<T, T&, T*>;
  using const_iterator  = NuoDequeIterator<T, const T&, const T*>;

  using value_type      = T;
  using reference       = Ref;
  using pointer         = Ptr;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;
  using value_pointer   = T*;
  using map_pointer     = T**;

public:
  static constexpr size_type kBufferSize = NuoDequeBufSize<T>::value;

public:
  /* construct / copy / destroy */
  NuoDequeIterator() noexcept;
  NuoDequeIterator(value_pointer vp, map_pointer mp);
  NuoDequeIterator(const iterator& rhs);
  NuoDequeIterator(iterator&& rhs) noexcept;
  NuoDequeIterator(const const_iterator& rhs);
  ~NuoDequeIterator() = default;

  /* operator */
  self& operator=(const iterator& rhs);
  reference operator*() const;
  pointer operator->() const;

  difference_type operator-(const self& x) const;

  self& operator++();
  self operator++(int);
  self& operator--();
  self operator--(int);
  self operator+(difference_type n) const;
  self& operator+=(difference_type n);
  self operator-(difference_type n) const;
  self& operator-=(difference_type n);
  
  reference operator[](difference_type n) const;

  bool operator==(const self& rhs) const;
  bool operator!=(const self& rhs) const;
  bool operator>(const self& rhs) const;
  bool operator>=(const self& rhs) const;
  bool operator<(const self& rhs) const;
  bool operator<=(const self& rhs) const;

  /* move to next buffer */
  void SetNode(map_pointer new_node);

public:
  value_pointer first_; /* head of buffer */
  value_pointer cur_;   /* current position in buffer */
  value_pointer last_;  /* tail of buffer */
  map_pointer node_;    /* buffer node */
};

/*****************************************************************************/
/* deque iterator implementation */
template <typename T, typename Ref, typename Ptr>
NuoDequeIterator<T, Ref, Ptr>::NuoDequeIterator() noexcept
  : first_ (nullptr), cur_(nullptr), last_(nullptr), node_(nullptr)
{
}

template <typename T, typename Ref, typename Ptr>
NuoDequeIterator<T, Ref, Ptr>::NuoDequeIterator(value_pointer vp,
                                                map_pointer mp)
  : first_(*mp), cur_(*vp), last_(*mp + kBufferSize), node_(mp)
{
}

template <typename T, typename Ref, typename Ptr>
NuoDequeIterator<T, Ref, Ptr>::NuoDequeIterator(const iterator& rhs)
  : first_(rhs.first_), cur_(rhs.cur_), last_(rhs.last_), node_(rhs.node_)
{
}

template <typename T, typename Ref, typename Ptr>
NuoDequeIterator<T, Ref, Ptr>::NuoDequeIterator(iterator&& rhs) noexcept
  : first_(NuoMove(rhs.first_))
  , cur_(NuoMove(rhs.cur_))
  , last_(NuoMove(rhs.last_))
  , node_(NuoMove(rhs.node_))
{
}

template <typename T, typename Ref, typename Ptr>
NuoDequeIterator<T, Ref, Ptr>::NuoDequeIterator(const const_iterator& rhs)
  : first_(rhs.first_), cur_(rhs.cur_), last_(rhs.last_), node_(rhs.node_)
{
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self&
NuoDequeIterator<T, Ref, Ptr>::operator=(const iterator& rhs)
{
  if (this != & rhs)
  {
    first_ = rhs.first_;
    cur_ = rhs.cur_;
    last_ = rhs.last_;
    node_ = rhs.node_;
  }

  return *this;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::reference
NuoDequeIterator<T, Ref, Ptr>::operator*() const
{
  return *cur_;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::pointer
NuoDequeIterator<T, Ref, Ptr>::operator->() const
{
  return cur_;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::difference_type
NuoDequeIterator<T, Ref, Ptr>::operator-(const self& x) const
{
  return static_cast<difference_type>(kBufferSize) * (node_ - x.node_)
    + (cur_ - first_) - (x.cur_ - x.first_);
}

/* TODO */
template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self&
NuoDequeIterator<T, Ref, Ptr>::operator++()
{
  cur_++;

  if (cur_ == last_)
  {
    SetNode(node_ + 1);
    cur_ = first_;
  }

  return *this;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self
NuoDequeIterator<T, Ref, Ptr>::operator++(int)
{
  self tmp = *this;
  ++(*this);
  return tmp;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self&
NuoDequeIterator<T, Ref, Ptr>::operator--()
{
  if (cur_ == first_)
  {
    SetNode(node_ - 1);
    cur_ = last_;
  }

  cur_--;

  return *this;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self
NuoDequeIterator<T, Ref, Ptr>::operator--(int)
{
  self tmp = *this;
  --(*this);
  return tmp;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self
NuoDequeIterator<T, Ref, Ptr>::operator+(difference_type n) const
{
  self tmp = *this;
  return tmp += n;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self&
NuoDequeIterator<T, Ref, Ptr>::operator+=(difference_type n)
{
  const difference_type offset = n + (cur_ - first_);

  if (offset >= 0 && offset <= static_cast<difference_type>(kBufferSize))
  {
    /* still in current buffer */
    cur_ += n;
  }
  else
  {
    /* move to other buffers */
    const difference_type node_offset = offset > 0
      ? offset / static_cast<difference_type>(kBufferSize)
      : -static_cast<difference_type>((-offset - 1) / kBufferSize) - 1;

    SetNode(node_ + node_offset);
    cur_ = first_ +
           (offset - node_offset * static_cast<difference_type>(kBufferSize));
  }

  return *this;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self
NuoDequeIterator<T, Ref, Ptr>::operator-(difference_type n) const
{
  self tmp = *this;
  return tmp -= n;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::self&
NuoDequeIterator<T, Ref, Ptr>::operator-=(difference_type n)
{
  return *this += -n;
}

template <typename T, typename Ref, typename Ptr>
typename NuoDequeIterator<T, Ref, Ptr>::reference
NuoDequeIterator<T, Ref, Ptr>::operator[](difference_type n) const
{
  return *(*this + n);
}

template <typename T, typename Ref, typename Ptr>
bool NuoDequeIterator<T, Ref, Ptr>::operator==(const self& rhs) const
{
  return cur_ == rhs.cur_;
}

template <typename T, typename Ref, typename Ptr>
bool NuoDequeIterator<T, Ref, Ptr>::operator!=(const self& rhs) const
{
  return cur_ != rhs.cur_;
}

template <typename T, typename Ref, typename Ptr>
bool NuoDequeIterator<T, Ref, Ptr>::operator>(const self& rhs) const
{
  return node_ == rhs.node_ ? cur_ > rhs.cur_ : node_ > rhs.node_;
}

template <typename T, typename Ref, typename Ptr>
bool NuoDequeIterator<T, Ref, Ptr>::operator>=(const self& rhs) const
{
  return node_ == rhs.node_ ? cur_ >= rhs.cur_ : node_ >= rhs.node_;
}

template <typename T, typename Ref, typename Ptr>
bool NuoDequeIterator<T, Ref, Ptr>::operator<(const self& rhs) const
{
  return node_ == rhs.node_ ? cur_ < rhs.cur_ : node_ < rhs.node_;
}

template <typename T, typename Ref, typename Ptr>
bool NuoDequeIterator<T, Ref, Ptr>::operator<=(const self& rhs) const
{
  return node_ == rhs.node_ ? cur_ <= rhs.cur_ : node_ <= rhs.node_;
}

template <typename T, typename Ref, typename Ptr>
void NuoDequeIterator<T, Ref, Ptr>::SetNode(map_pointer new_node)
{
  first_ = *new_node;
  last_ = *new_node + kBufferSize;
  node_ = new_node;
}

} /* namespace nuostl */