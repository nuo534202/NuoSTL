#pragma once

#include <initializer_list>

#include "nuo_typedefs.hpp"
#include "utils/nuo_algorithm.hpp"
#include "utils/nuo_allocator.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_iterator.hpp"
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
  using self            = NuoDequeIterator<T, Ref, Ptr>;
  using iterator        = NuoDequeIterator<T, T&, T*>;
  using const_iterator  = NuoDequeIterator<T, const T&, const T*>;
  using iterator_category = NuoRandomAccessIteratorTag;

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
/* deque class definition */
template<typename T>
class NuoDeque
{
public:
  /* types */
  using allocator_type          = NuoAllocator<T>;
  using data_allocator          = NuoAllocator<T>;
  using map_allocator           = NuoAllocator<T*>;

  using value_type              = allocator_type::value_type;
  using pointer                 = allocator_type::pointer;
  using const_pointer           = allocator_type::const_pointer;
  using reference               = allocator_type::reference;
  using const_reference         = allocator_type::const_reference;
  using size_type               = allocator_type::size_type;
  using difference_type         = allocator_type::difference_type;
  using map_pointer             = pointer*;
  using const_map_pointer       = const_pointer*;

  using iterator                = NuoDequeIterator<T, T&, T*>;
  using const_iterator          = NuoDequeIterator<T, const T&, const T*>;
  using reverse_iterator        = NuoReverseIterator<iterator>;
  using const_reverse_iterator  = NuoReverseIterator<const_iterator>;

public:
  static constexpr size_type kBufferSize = NuoDequeBufSize<T>::value;

public:
  /* construct / copy / destroy */
  constexpr NuoDeque();
  constexpr explicit NuoDeque(size_type n);
  constexpr NuoDeque(size_type n, const value_type& value);
  template<typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
  constexpr NuoDeque(InputIter first, InputIter last);
  constexpr NuoDeque(const NuoDeque& rhs);
  constexpr NuoDeque(NuoDeque&& rhs);
  constexpr NuoDeque(std::initializer_list<value_type> ilist);
  constexpr ~NuoDeque();

  /* assignment */
  constexpr NuoDeque& operator=(const NuoDeque& rhs);
  constexpr NuoDeque& operator=(NuoDeque&& rhs);
  constexpr NuoDeque& operator=(std::initializer_list<value_type> ilist);

  constexpr void Assign(size_type n, const value_type& value);
  template<typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
  constexpr void Assign(InputIter first, InputIter last);
  constexpr void Assign(std::initializer_list<value_type> ilist);

  /* iterators */
  constexpr iterator Begin() noexcept;
  constexpr const_iterator Begin() const noexcept;
  constexpr iterator End() noexcept;
  constexpr const_iterator End() const noexcept;

  constexpr reverse_iterator Rbegin() noexcept;
  constexpr const_reverse_iterator Rbegin() const noexcept;
  constexpr reverse_iterator Rend() noexcept;
  constexpr const_reverse_iterator Rend() const noexcept;

  constexpr const_iterator Cbegin() const noexcept;
  constexpr const_iterator Cend() const noexcept;
  constexpr const_reverse_iterator Crbegin() const noexcept;
  constexpr const_reverse_iterator Crend() const noexcept;

  /* capacity */
  constexpr bool Empty() const noexcept;
  constexpr size_type Size() const noexcept;
  constexpr size_type MaxSize() const noexcept;
  constexpr void Resize(size_type size);
  constexpr void Resize(size_type size, const value_type& value);
  constexpr void ShrinkToFit();

  /* element access */
  constexpr reference operator[](size_type n);
  constexpr const_reference operator[](size_type n) const;
  constexpr reference At(size_type n);
  constexpr const_reference At(size_type n) const;
  constexpr reference Front();
  constexpr const_reference Front() const;
  constexpr reference Back();
  constexpr const_reference Back() const;

  /* modifiers */
  template<typename... Args>
  constexpr reference EmplaceFront(Args&&... args);
  template<typename... Args>
  constexpr reference EmplaceBack(Args&&... args);
  template<typename... Args>
  constexpr iterator Emplace(const_iterator position, Args&&... args);

  constexpr void push_front(const value_type& x);
  constexpr void push_front(value_type&& x);
  constexpr void push_back(const value_type& x);
  constexpr void push_back(value_type&& x);

  constexpr iterator Insert(const_iterator position, const value_type& value);
  constexpr iterator Insert(const_iterator position, value_type&& value);
  constexpr iterator Insert(const_iterator position,
                            size_type n,
                            const value_type& value);
  template<typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
  constexpr iterator Insert(const_iterator position,
                            InputIter first,
                            InputIter last);
  constexpr iterator Insert(const_iterator position,
                            std::initializer_list<value_type> ilist);

  constexpr void PopFront();
  constexpr void PopBack();

  constexpr iterator Erase(const_iterator position);
  constexpr iterator Erase(const_iterator first, const_iterator last);
  constexpr void Clear() noexcept;

  constexpr void Swap(NuoDeque& rhs);

  /* get methods */
  constexpr allocator_type GetAllocator() const noexcept;

private:
  /* helper functions */
  /* create / destroy map and buffers */
  map_pointer CreateMap(size_type size);
  void CreateBuffer(map_pointer nstart, map_pointer nfinish);
  void DestroyBuffer(map_pointer nstart, map_pointer nfinish);

  /* initialize */
  void MapInit(size_type nelem);
  void FillInit(size_type n, const value_type& value);
  template <typename InputIter>
  void CopyInit(InputIter first, InputIter last, NuoInputIteratorTag);
  template <typename ForwardIter>
  void CopyInit(ForwardIter first, ForwardIter last, NuoForwardIteratorTag);

  /* assign */
  void FillAssign(size_type n, const value_type& value);
  template <typename InputIter>
  void CopyAssign(InputIter first, InputIter last, NuoInputIteratorTag);
  template <typename ForwardIter>
  void CopyAssign(ForwardIter first, ForwardIter last, NuoForwardIteratorTag);

  /* insert */
  template <typename... Args>
  iterator InsertAux(iterator position, Args&&... args);
  iterator FillInsert(iterator position, size_type n, const value_type& value);
  template <typename ForwardIter>
  iterator CopyInsert(iterator position,
                      ForwardIter first,
                      ForwardIter last,
                      size_type n);

  template <typename InputIter>
  void InsertDispatch(iterator position,
                      InputIter first,
                      InputIter last,
                      NuoInputIteratorTag);
  template <typename ForwardIter>
  void InsertDispatch(iterator position,
                      ForwardIter first,
                      ForwardIter last,
                      NuoForwardIteratorTag);

  /* reallocate */
  void RequireCapacity(size_type n, bool front);
  void ReallocateMapAtFront(size_type need_buffer);
  void ReallocateMapAtBack(size_type need_buffer);

private:
  iterator        begin_;
  iterator        end_;
  map_pointer     map_;
  size_type       map_size_;
};

template <typename T>
void NuoSwap(NuoDeque<T>& lhs, NuoDeque<T>& rhs) noexcept
{
  lhs.Swap(rhs);
}

/* overload comparison operators */
template <typename T>
bool operator==(const NuoDeque<T>& lhs, const NuoDeque<T>& rhs)
{
  return lhs.Size() == rhs.Size() &&
    NuoEqual(lhs.Begin(), lhs.End(), rhs.Begin());
}

template <typename T>
bool operator!=(const NuoDeque<T>& lhs, const NuoDeque<T>& rhs)
{
  return !(lhs == rhs);
}

template <typename T>
bool operator<(const NuoDeque<T>& lhs, const NuoDeque<T>& rhs)
{
  return NuoLexicographicalCompare(
    lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template <typename T>
bool operator>(const NuoDeque<T>& lhs, const NuoDeque<T>& rhs)
{
  return rhs < lhs;
}

template <typename T>
bool operator<=(const NuoDeque<T>& lhs, const NuoDeque<T>& rhs)
{
  return !(rhs < lhs);
}

template <typename T>
bool operator>=(const NuoDeque<T>& lhs, const NuoDeque<T>& rhs)
{
  return !(lhs < rhs);
}

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
template<class InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
NuoDeque(InputIter, InputIter)
  -> NuoDeque<typename NuoIteratorTraits<InputIter>::value_type>;
#endif

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
  : first_(*mp), cur_(vp), last_(*mp + kBufferSize), node_(mp)
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
  if (node_ == nullptr && x.node_ == nullptr)
    return 0;

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

  if (offset >= 0 && offset < static_cast<difference_type>(kBufferSize))
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

/*****************************************************************************/
/* deque class implementation */

/* construct / copy / destroy */

template <typename T>
constexpr NuoDeque<T>::NuoDeque()
{
  MapInit(0);
}

template <typename T>
constexpr NuoDeque<T>::NuoDeque(size_type n)
{
  FillInit(n, value_type());
}

template <typename T>
constexpr NuoDeque<T>::NuoDeque(size_type n, const value_type& value)
{
  FillInit(n, value);
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type>
constexpr NuoDeque<T>::NuoDeque(InputIter first, InputIter last)
{
  CopyInit(first, last, NuoIteratorCategory(first));
}

template <typename T>
constexpr NuoDeque<T>::NuoDeque(const NuoDeque& rhs)
{
  CopyInit(rhs.Begin(), rhs.End(), NuoForwardIteratorTag());
}

template <typename T>
constexpr NuoDeque<T>::NuoDeque(NuoDeque&& rhs)
  : begin_(NuoMove(rhs.begin_)),
    end_(NuoMove(rhs.end_)),
    map_(rhs.map_),
    map_size_(rhs.map_size_)
{
  rhs.begin_ = iterator();
  rhs.end_ = iterator();
  rhs.map_ = nullptr;
  rhs.map_size_ = 0;
}

template <typename T>
constexpr NuoDeque<T>::NuoDeque(std::initializer_list<value_type> ilist)
{
  CopyInit(ilist.begin(), ilist.end(), NuoForwardIteratorTag());
}

template <typename T>
constexpr NuoDeque<T>::~NuoDeque()
{
  if (map_ != nullptr)
  {
    Clear();
    data_allocator::Deallocate(*begin_.node_, kBufferSize);
    *begin_.node_ = nullptr;
    map_allocator::Deallocate(map_, map_size_);
    map_ = nullptr;
  }
}

/* assignment */

template <typename T>
constexpr NuoDeque<T>& NuoDeque<T>::operator=(const NuoDeque& rhs)
{
  if (this != &rhs)
  {
    const size_type len = Size();
    if (len >= rhs.Size())
    {
      Erase(NuoCopy(rhs.Begin(), rhs.End(), Begin()), End());
    }
    else
    {
      iterator mid = rhs.begin_ + static_cast<difference_type>(len);
      NuoCopy(rhs.begin_, mid, Begin());
      Insert(end_, mid, rhs.end_);
    }
  }
  return *this;
}

template <typename T>
constexpr NuoDeque<T>& NuoDeque<T>::operator=(NuoDeque&& rhs)
{
  if (this != &rhs)
  {
    Clear();
    begin_ = NuoMove(rhs.begin_);
    end_ = NuoMove(rhs.end_);
    map_ = rhs.map_;
    map_size_ = rhs.map_size_;
    rhs.begin_ = iterator();
    rhs.end_ = iterator();
    rhs.map_ = nullptr;
    rhs.map_size_ = 0;
  }
  return *this;
}

template <typename T>
constexpr NuoDeque<T>& NuoDeque<T>::operator=(
  std::initializer_list<value_type> ilist)
{
  NuoDeque tmp(ilist);
  Swap(tmp);
  return *this;
}

template <typename T>
constexpr void NuoDeque<T>::Assign(size_type n, const value_type& value)
{
  FillAssign(n, value);
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type>
constexpr void NuoDeque<T>::Assign(InputIter first, InputIter last)
{
  CopyAssign(first, last, NuoIteratorCategory(first));
}

template <typename T>
constexpr void NuoDeque<T>::Assign(std::initializer_list<value_type> ilist)
{
  CopyAssign(ilist.begin(), ilist.end(), NuoForwardIteratorTag());
}

/* iterators */
template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Begin() noexcept
{
  return begin_;
}

template <typename T>
constexpr typename NuoDeque<T>::const_iterator
NuoDeque<T>::Begin() const noexcept
{
  return begin_;
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::End() noexcept
{
  return end_;
}

template <typename T>
constexpr typename NuoDeque<T>::const_iterator
NuoDeque<T>::End() const noexcept
{
  return end_;
}

template <typename T>
constexpr typename NuoDeque<T>::reverse_iterator
NuoDeque<T>::Rbegin() noexcept
{
  return reverse_iterator(end_);
}

template <typename T>
constexpr typename NuoDeque<T>::const_reverse_iterator
NuoDeque<T>::Rbegin() const noexcept
{
  return const_reverse_iterator(end_);
}

template <typename T>
constexpr typename NuoDeque<T>::reverse_iterator
NuoDeque<T>::Rend() noexcept
{
  return reverse_iterator(begin_);
}

template <typename T>
constexpr typename NuoDeque<T>::const_reverse_iterator
NuoDeque<T>::Rend() const noexcept
{
  return const_reverse_iterator(begin_);
}

template <typename T>
constexpr typename NuoDeque<T>::const_iterator
NuoDeque<T>::Cbegin() const noexcept
{
  return begin_;
}

template <typename T>
constexpr typename NuoDeque<T>::const_iterator
NuoDeque<T>::Cend() const noexcept
{
  return end_;
}

template <typename T>
constexpr typename NuoDeque<T>::const_reverse_iterator
NuoDeque<T>::Crbegin() const noexcept
{
  return const_reverse_iterator(end_);
}

template <typename T>
constexpr typename NuoDeque<T>::const_reverse_iterator
NuoDeque<T>::Crend() const noexcept
{
  return const_reverse_iterator(begin_);
}

/* capacity */
template <typename T>
constexpr bool NuoDeque<T>::Empty() const noexcept
{
  return begin_ == end_;
}

template <typename T>
constexpr typename NuoDeque<T>::size_type
NuoDeque<T>::Size() const noexcept
{
  return end_ - begin_;
}

template <typename T>
constexpr typename NuoDeque<T>::size_type
NuoDeque<T>::MaxSize() const noexcept
{
  return static_cast<size_type>(-1);
}

template <typename T>
constexpr void NuoDeque<T>::Resize(size_type size)
{
  Resize(size, value_type());
}

template <typename T>
constexpr void NuoDeque<T>::Resize(size_type size, const value_type& value)
{
  const size_type len = Size();
  if (size < len)
  {
    Erase(begin_ + size, end_);
  }
  else
  {
    Insert(end_, size - len, value);
  }
}

template <typename T>
constexpr void NuoDeque<T>::ShrinkToFit()
{
  for (map_pointer cur = map_; cur < begin_.node_; cur++)
  {
    data_allocator::Deallocate(*cur, kBufferSize);
    *cur = nullptr;
  }

  for (map_pointer cur = end_.node_ + 1; cur < map_ + map_size_; cur++)
  {
    data_allocator::Deallocate(*cur, kBufferSize);
    *cur = nullptr;
  }
}

/* element access */
template <typename T>
constexpr typename NuoDeque<T>::reference
NuoDeque<T>::operator[](size_type n)
{
  return begin_[n];
}

template <typename T>
constexpr typename NuoDeque<T>::const_reference
NuoDeque<T>::operator[](size_type n) const
{
  return begin_[n];
}

template <typename T>
constexpr typename NuoDeque<T>::reference
NuoDeque<T>::At(size_type n)
{
  NUO_THROW_OUT_OF_RANGE_IF(!(n < Size()),
    "NuoDeque<T>::At() subscript out of range!");

  return (*this)[n];
}

template <typename T>
constexpr typename NuoDeque<T>::const_reference
NuoDeque<T>::At(size_type n) const
{
  NUO_THROW_OUT_OF_RANGE_IF(!(n < Size()),
    "NuoDeque<T>::At() subscript out of range!");

  return (*this)[n];
}

template <typename T>
constexpr typename NuoDeque<T>::reference
NuoDeque<T>::Front()
{
  return *Begin();
}

template <typename T>
constexpr typename NuoDeque<T>::const_reference
NuoDeque<T>::Front() const
{
  return *Begin();
}

template <typename T>
constexpr typename NuoDeque<T>::reference
NuoDeque<T>::Back()
{
  return *(End() - 1);
}

template <typename T>
constexpr typename NuoDeque<T>::const_reference
NuoDeque<T>::Back() const
{
  return *(End() - 1);
}

/* modifiers */

template <typename T>
template <typename... Args>
constexpr typename NuoDeque<T>::reference
NuoDeque<T>::EmplaceFront(Args&&... args)
{
  if (map_ == nullptr)
    MapInit(0);

  if (begin_.cur_ != begin_.first_)
  {
    data_allocator::Construct(begin_.cur_ - 1, NuoForward<Args>(args)...);
    --begin_.cur_;
  }
  else
  {
    RequireCapacity(1, true);
    try
    {
      --begin_;
      data_allocator::Construct(begin_.cur_, NuoForward<Args>(args)...);
    }
    catch (...)
    {
      ++begin_;
      throw;
    }
  }
  return *Begin();
}

template <typename T>
template <typename... Args>
constexpr typename NuoDeque<T>::reference
NuoDeque<T>::EmplaceBack(Args&&... args)
{
  if (map_ == nullptr)
    MapInit(0);

  if (end_.cur_ != end_.last_ - 1)
  {
    data_allocator::Construct(end_.cur_, NuoForward<Args>(args)...);
    ++end_.cur_;
  }
  else
  {
    RequireCapacity(1, false);
    data_allocator::Construct(end_.cur_, NuoForward<Args>(args)...);
    ++end_;
  }
  return *(End() - 1);
}

template <typename T>
template <typename... Args>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Emplace(const_iterator position, Args&&... args)
{
  if (position.cur_ == begin_.cur_)
  {
    EmplaceFront(NuoForward<Args>(args)...);
    return Begin();
  }
  else if (position.cur_ == end_.cur_)
  {
    EmplaceBack(NuoForward<Args>(args)...);
    return End() - 1;
  }
  return InsertAux(iterator(position), NuoForward<Args>(args)...);
}

template <typename T>
constexpr void NuoDeque<T>::push_front(const value_type& x)
{
  EmplaceFront(x);
}

template <typename T>
constexpr void NuoDeque<T>::push_front(value_type&& x)
{
  EmplaceFront(NuoMove(x));
}

template <typename T>
constexpr void NuoDeque<T>::push_back(const value_type& x)
{
  EmplaceBack(x);
}

template <typename T>
constexpr void NuoDeque<T>::push_back(value_type&& x)
{
  EmplaceBack(NuoMove(x));
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Insert(const_iterator position, const value_type& value)
{
  if (position.cur_ == begin_.cur_)
  {
    push_front(value);
    return Begin();
  }
  else if (position.cur_ == end_.cur_)
  {
    push_back(value);
    return End() - 1;
  }
  return InsertAux(iterator(position), value);
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Insert(const_iterator position, value_type&& value)
{
  if (position.cur_ == begin_.cur_)
  {
    EmplaceFront(NuoMove(value));
    return Begin();
  }
  else if (position.cur_ == end_.cur_)
  {
    EmplaceBack(NuoMove(value));
    return End() - 1;
  }
  return InsertAux(iterator(position), NuoMove(value));
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Insert(const_iterator position,
                    size_type n,
                    const value_type& value)
{
  const size_type elems_before = static_cast<size_type>(position - begin_);

  if (position.cur_ == begin_.cur_)
  {
    RequireCapacity(n, true);
    begin_ = begin_ - static_cast<difference_type>(n);
    NuoUninitializedFillN(begin_, n, value);
  }
  else if (position.cur_ == end_.cur_)
  {
    RequireCapacity(n, false);
    end_ = NuoUninitializedFillN(end_, n, value);
  }
  else
  {
    FillInsert(iterator(position), n, value);
  }
  return begin_ + static_cast<difference_type>(elems_before);
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Insert(const_iterator position, InputIter first, InputIter last)
{
  const size_type elems_before = static_cast<size_type>(position - begin_);

  InsertDispatch(iterator(position), first, last, NuoIteratorCategory(first));
  return begin_ + static_cast<difference_type>(elems_before);
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Insert(const_iterator position,
                    std::initializer_list<value_type> ilist)
{
  return Insert(position, ilist.begin(), ilist.end());
}

template <typename T>
constexpr void NuoDeque<T>::PopFront()
{
  if (begin_.cur_ != begin_.last_ - 1)
  {
    data_allocator::Destroy(begin_.cur_);
    ++begin_.cur_;
  }
  else
  {
    data_allocator::Destroy(begin_.cur_);
    ++begin_;
    DestroyBuffer(begin_.node_ - 1, begin_.node_ - 1);
  }
}

template <typename T>
constexpr void NuoDeque<T>::PopBack()
{
  if (end_.cur_ != end_.first_)
  {
    --end_.cur_;
    data_allocator::Destroy(end_.cur_);
  }
  else
  {
    --end_;
    data_allocator::Destroy(end_.cur_);
    DestroyBuffer(end_.node_ + 1, end_.node_ + 1);
  }
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Erase(const_iterator position)
{
  iterator pos = iterator(position);
  iterator next = pos;
  ++next;
  const size_type elems_before = static_cast<size_type>(pos - begin_);

  if (elems_before < (Size() / 2))
  {
    NuoCopyBackward(begin_, pos, next);
    PopFront();
  }
  else
  {
    NuoCopy(next, End(), pos);
    PopBack();
  }
  return begin_ + static_cast<difference_type>(elems_before);
}

template <typename T>
constexpr typename NuoDeque<T>::iterator
NuoDeque<T>::Erase(const_iterator first, const_iterator last)
{
  iterator first1 = iterator(first);
  iterator last1 = iterator(last);

  if (first1 == begin_ && last1 == end_)
  {
    Clear();
    return End();
  }
  else
  {
    const size_type len = static_cast<size_type>(last1 - first1);
    const size_type elems_before = static_cast<size_type>(first1 - begin_);

    if (elems_before < ((Size() - len) / 2))
    {
      NuoCopyBackward(begin_, first1, last1);
      iterator new_begin = begin_ + static_cast<difference_type>(len);
      data_allocator::Destroy(begin_.cur_, new_begin.cur_);
      begin_ = new_begin;
    }
    else
    {
      NuoCopy(last1, end_, first1);
      iterator new_end = end_ - static_cast<difference_type>(len);
      data_allocator::Destroy(new_end.cur_, end_.cur_);
      end_ = new_end;
    }
    return begin_ + static_cast<difference_type>(elems_before);
  }
}

template <typename T>
constexpr void NuoDeque<T>::Clear() noexcept
{
  for (map_pointer cur = begin_.node_ + 1; cur < end_.node_; cur++)
  {
    data_allocator::Destroy(*cur, *cur + kBufferSize);
  }

  if (begin_.node_ != end_.node_)
  {
    data_allocator::Destroy(begin_.cur_, begin_.last_);
    data_allocator::Destroy(end_.first_, end_.cur_);
  }
  else
  {
    data_allocator::Destroy(begin_.cur_, end_.cur_);
  }
  ShrinkToFit();
  end_ = begin_;
}

template <typename T>
constexpr void NuoDeque<T>::Swap(NuoDeque& rhs)
{
  if (this != &rhs)
  {
    NuoSwap(begin_, rhs.begin_);
    NuoSwap(end_, rhs.end_);
    NuoSwap(map_, rhs.map_);
    NuoSwap(map_size_, rhs.map_size_);
  }
}

/* get methods */
template <typename T>
constexpr typename NuoDeque<T>::allocator_type
NuoDeque<T>::GetAllocator() const noexcept
{
  return allocator_type();
}

/*****************************************************************************/
/* helper function implementation */

/* create / destroy map and buffers */

template <typename T>
typename NuoDeque<T>::map_pointer
NuoDeque<T>::CreateMap(size_type size)
{
  map_pointer mp = map_allocator::Allocate(size);
  for (size_type i = 0; i < size; i++)
    *(mp + i) = nullptr;
  return mp;
}

template <typename T>
void NuoDeque<T>::CreateBuffer(map_pointer nstart, map_pointer nfinish)
{
  map_pointer cur;
  try
  {
    for (cur = nstart; cur <= nfinish; cur++)
      *cur = data_allocator::Allocate(kBufferSize);
  }
  catch (...)
  {
    while (cur != nstart)
    {
      --cur;
      data_allocator::Deallocate(*cur, kBufferSize);
      *cur = nullptr;
    }
    throw;
  }
}

template <typename T>
void NuoDeque<T>::DestroyBuffer(map_pointer nstart, map_pointer nfinish)
{
  for (map_pointer cur = nstart; cur <= nfinish; cur++)
  {
    data_allocator::Deallocate(*cur, kBufferSize);
    *cur = nullptr;
  }
}

/* initialize */

template <typename T>
void NuoDeque<T>::MapInit(size_type nelem)
{
  const size_type n_node = nelem / kBufferSize + 1;
  map_size_ = (n_node + 2) > DEQUE_MAP_INIT_SIZE
    ? (n_node + 2) : DEQUE_MAP_INIT_SIZE;

  try
  {
    map_ = CreateMap(map_size_);
  }
  catch (...)
  {
    map_ = nullptr;
    map_size_ = 0;
    throw;
  }

  /* let nstart and nfinish point to the middle of the map,
     so the deque can expand in both directions */
  map_pointer nstart = map_ + (map_size_ - n_node) / 2;
  map_pointer nfinish = nstart + n_node - 1;
  try
  {
    CreateBuffer(nstart, nfinish);
  }
  catch (...)
  {
    map_allocator::Deallocate(map_, map_size_);
    map_ = nullptr;
    map_size_ = 0;
    throw;
  }

  begin_.SetNode(nstart);
  end_.SetNode(nfinish);
  begin_.cur_ = begin_.first_;
  end_.cur_ = end_.first_ + (nelem % kBufferSize);
}

template <typename T>
void NuoDeque<T>::FillInit(size_type n, const value_type& value)
{
  MapInit(n);
  if (n != 0)
  {
    for (map_pointer cur = begin_.node_; cur < end_.node_; cur++)
      NuoUninitializedFill(*cur, *cur + kBufferSize, value);
    NuoUninitializedFill(end_.first_, end_.cur_, value);
  }
}

template <typename T>
template <typename InputIter>
void NuoDeque<T>::CopyInit(InputIter first,
                           InputIter last,
                           NuoInputIteratorTag)
{
  const size_type n = static_cast<size_type>(NuoDistance(first, last));
  MapInit(n);
  pointer cur = begin_.cur_;
  for (; first != last; ++first, ++cur)
    data_allocator::Construct(cur, *first);
}

template <typename T>
template <typename ForwardIter>
void NuoDeque<T>::CopyInit(ForwardIter first,
                           ForwardIter last,
                           NuoForwardIteratorTag)
{
  const size_type n = static_cast<size_type>(NuoDistance(first, last));
  MapInit(n);
  for (map_pointer cur = begin_.node_; cur < end_.node_; cur++)
  {
    ForwardIter next = first;
    NuoAdvance(next, static_cast<difference_type>(kBufferSize));
    NuoUninitializedCopy(first, next, *cur);
    first = next;
  }
  NuoUninitializedCopy(first, last, end_.first_);
}

/* assign */

template <typename T>
void NuoDeque<T>::FillAssign(size_type n, const value_type& value)
{
  if (n > Size())
  {
    NuoFill(Begin(), End(), value);
    Insert(End(), n - Size(), value);
  }
  else
  {
    Erase(Begin() + static_cast<difference_type>(n), End());
    NuoFill(Begin(), End(), value);
  }
}

template <typename T>
template <typename InputIter>
void NuoDeque<T>::CopyAssign(InputIter first,
                             InputIter last,
                             NuoInputIteratorTag)
{
  iterator first1 = begin_;
  iterator last1 = end_;
  for (; first != last && first1 != last1; ++first, ++first1)
    *first1 = *first;

  if (first1 != last1)
    Erase(first1, last1);
  else
    InsertDispatch(end_, first, last, NuoInputIteratorTag{});
}

template <typename T>
template <typename ForwardIter>
void NuoDeque<T>::CopyAssign(ForwardIter first,
                             ForwardIter last,
                             NuoForwardIteratorTag)
{
  const size_type len1 = Size();
  const size_type len2 = static_cast<size_type>(NuoDistance(first, last));

  if (len1 < len2)
  {
    ForwardIter next = first;
    NuoAdvance(next, static_cast<difference_type>(len1));
    NuoCopy(first, next, begin_);
    InsertDispatch(end_, next, last, NuoForwardIteratorTag{});
  }
  else
  {
    Erase(NuoCopy(first, last, begin_), end_);
  }
}

/* insert */

template <typename T>
template <typename... Args>
typename NuoDeque<T>::iterator
NuoDeque<T>::InsertAux(iterator position, Args&&... args)
{
  const size_type elems_before = static_cast<size_type>(position - begin_);
  value_type value_copy = value_type(NuoForward<Args>(args)...);

  if (elems_before < (Size() / 2))
  { /* insert in the front half */
    EmplaceFront(Front());
    iterator front1 = begin_;
    ++front1;
    iterator front2 = front1;
    ++front2;
    position = begin_ + static_cast<difference_type>(elems_before);
    iterator pos = position;
    ++pos;
    NuoCopy(front2, pos, front1);
  }
  else
  { /* insert in the back half */
    EmplaceBack(Back());
    iterator back1 = end_;
    --back1;
    iterator back2 = back1;
    --back2;
    position = begin_ + static_cast<difference_type>(elems_before);
    NuoCopyBackward(position, back2, back1);
  }
  *position = NuoMove(value_copy);
  return position;
}

template <typename T>
typename NuoDeque<T>::iterator
NuoDeque<T>::FillInsert(iterator position,
                        size_type n,
                        const value_type& value)
{
  const size_type elems_before = static_cast<size_type>(position - begin_);
  const size_type len = Size();
  value_type value_copy = value;

  if (elems_before < (len / 2))
  {
    RequireCapacity(n, true);
    /* the original iterators may be invalidated */
    iterator old_begin = begin_;
    iterator new_begin = begin_ - static_cast<difference_type>(n);
    position = begin_ + static_cast<difference_type>(elems_before);
    try
    {
      if (elems_before >= n)
      {
        iterator begin_n = begin_ + static_cast<difference_type>(n);
        NuoUninitializedCopy(begin_, begin_n, new_begin);
        begin_ = new_begin;
        NuoCopy(begin_n, position, old_begin);
        NuoFill(position - static_cast<difference_type>(n), position,
                value_copy);
      }
      else
      {
        NuoUninitializedFill(
          NuoUninitializedCopy(begin_, position, new_begin),
          begin_, value_copy);
        begin_ = new_begin;
        NuoFill(old_begin, position, value_copy);
      }
    }
    catch (...)
    {
      if (new_begin.node_ != begin_.node_)
        DestroyBuffer(new_begin.node_, begin_.node_ - 1);
      throw;
    }
  }
  else
  {
    RequireCapacity(n, false);
    /* the original iterators may be invalidated */
    iterator old_end = end_;
    iterator new_end = end_ + static_cast<difference_type>(n);
    const size_type elems_after = len - elems_before;
    position = end_ - static_cast<difference_type>(elems_after);
    try
    {
      if (elems_after > n)
      {
        iterator end_n = end_ - static_cast<difference_type>(n);
        NuoUninitializedCopy(end_n, end_, end_);
        end_ = new_end;
        NuoCopyBackward(position, end_n, old_end);
        NuoFill(position, position + static_cast<difference_type>(n),
                value_copy);
      }
      else
      {
        NuoUninitializedFill(
          end_, position + static_cast<difference_type>(n), value_copy);
        NuoUninitializedCopy(position, end_, position
                             + static_cast<difference_type>(n));
        end_ = new_end;
        NuoFill(position, old_end, value_copy);
      }
    }
    catch (...)
    {
      if (new_end.node_ != end_.node_)
        DestroyBuffer(end_.node_ + 1, new_end.node_);
      throw;
    }
  }
  return begin_ + static_cast<difference_type>(elems_before);
}

template <typename T>
template <typename ForwardIter>
typename NuoDeque<T>::iterator
NuoDeque<T>::CopyInsert(iterator position,
                        ForwardIter first,
                        ForwardIter last,
                        size_type n)
{
  const size_type elems_before = static_cast<size_type>(position - begin_);
  const size_type len = Size();

  if (elems_before < (len / 2))
  {
    RequireCapacity(n, true);
    /* the original iterators may be invalidated */
    iterator old_begin = begin_;
    iterator new_begin = begin_ - static_cast<difference_type>(n);
    position = begin_ + static_cast<difference_type>(elems_before);
    try
    {
      if (elems_before >= n)
      {
        iterator begin_n = begin_ + static_cast<difference_type>(n);
        NuoUninitializedCopy(begin_, begin_n, new_begin);
        begin_ = new_begin;
        NuoCopy(begin_n, position, old_begin);
        NuoCopy(first, last, position - static_cast<difference_type>(n));
      }
      else
      {
        ForwardIter mid = first;
        NuoAdvance(mid, static_cast<difference_type>(n - elems_before));
        NuoUninitializedCopy(
          first, mid,
          NuoUninitializedCopy(begin_, position, new_begin));
        begin_ = new_begin;
        NuoCopy(mid, last, old_begin);
      }
    }
    catch (...)
    {
      if (new_begin.node_ != begin_.node_)
        DestroyBuffer(new_begin.node_, begin_.node_ - 1);
      throw;
    }
  }
  else
  {
    RequireCapacity(n, false);
    /* the original iterators may be invalidated */
    iterator old_end = end_;
    iterator new_end = end_ + static_cast<difference_type>(n);
    const size_type elems_after = len - elems_before;
    position = end_ - static_cast<difference_type>(elems_after);
    try
    {
      if (elems_after > n)
      {
        iterator end_n = end_ - static_cast<difference_type>(n);
        NuoUninitializedCopy(end_n, end_, end_);
        end_ = new_end;
        NuoCopyBackward(position, end_n, old_end);
        NuoCopy(first, last, position);
      }
      else
      {
        ForwardIter mid = first;
        NuoAdvance(mid, static_cast<difference_type>(elems_after));
        NuoUninitializedCopy(
          position, end_,
          NuoUninitializedCopy(mid, last, end_));
        end_ = new_end;
        NuoCopy(first, mid, position);
      }
    }
    catch (...)
    {
      if (new_end.node_ != end_.node_)
        DestroyBuffer(end_.node_ + 1, new_end.node_);
      throw;
    }
  }
  return begin_ + static_cast<difference_type>(elems_before);
}

template <typename T>
template <typename InputIter>
void NuoDeque<T>::InsertDispatch(iterator position,
                                 InputIter first,
                                 InputIter last,
                                 NuoInputIteratorTag)
{
  if (last <= first)
    return;

  const size_type n = static_cast<size_type>(NuoDistance(first, last));
  const size_type elems_before = static_cast<size_type>(position - begin_);
  if (elems_before < (Size() / 2))
    RequireCapacity(n, true);
  else
    RequireCapacity(n, false);

  InputIter cur = --last;
  for (size_type i = 0; i < n; ++i, --cur)
  {
    /* begin_ may move during insertion, so recompute position */
    position = begin_ + static_cast<difference_type>(elems_before);
    Insert(position, *cur);
  }
}

template <typename T>
template <typename ForwardIter>
void NuoDeque<T>::InsertDispatch(iterator position,
                                 ForwardIter first,
                                 ForwardIter last,
                                 NuoForwardIteratorTag)
{
  if (last <= first)
    return;

  const size_type n = static_cast<size_type>(NuoDistance(first, last));

  if (position.cur_ == begin_.cur_)
  {
    RequireCapacity(n, true);
    iterator new_begin = begin_ - static_cast<difference_type>(n);
    try
    {
      NuoUninitializedCopy(first, last, new_begin);
      begin_ = new_begin;
    }
    catch (...)
    {
      if (new_begin.node_ != begin_.node_)
        DestroyBuffer(new_begin.node_, begin_.node_ - 1);
      throw;
    }
  }
  else if (position.cur_ == end_.cur_)
  {
    RequireCapacity(n, false);
    iterator new_end = end_ + static_cast<difference_type>(n);
    try
    {
      NuoUninitializedCopy(first, last, end_);
      end_ = new_end;
    }
    catch (...)
    {
      if (new_end.node_ != end_.node_)
        DestroyBuffer(end_.node_ + 1, new_end.node_);
      throw;
    }
  }
  else
  {
    CopyInsert(position, first, last, n);
  }
}

/* reallocate */

template <typename T>
void NuoDeque<T>::RequireCapacity(size_type n, bool front)
{
  if (front && (static_cast<size_type>(begin_.cur_ - begin_.first_) < n))
  {
    const size_type need_buffer =
      (n - static_cast<size_type>(begin_.cur_ - begin_.first_))
        / kBufferSize + 1;
    if (need_buffer > static_cast<size_type>(begin_.node_ - map_))
    {
      ReallocateMapAtFront(need_buffer);
      return;
    }
    CreateBuffer(begin_.node_ - need_buffer, begin_.node_ - 1);
  }
  else if (!front && (static_cast<size_type>(end_.last_ - end_.cur_ - 1) < n))
  {
    const size_type need_buffer =
      (n - static_cast<size_type>(end_.last_ - end_.cur_ - 1))
        / kBufferSize + 1;
    if (need_buffer > static_cast<size_type>((map_ + map_size_)
                                             - end_.node_ - 1))
    {
      ReallocateMapAtBack(need_buffer);
      return;
    }
    CreateBuffer(end_.node_ + 1, end_.node_ + need_buffer);
  }
}

template <typename T>
void NuoDeque<T>::ReallocateMapAtFront(size_type need_buffer)
{
  const size_type new_map_size =
    (map_size_ << 1) > (map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE)
      ? (map_size_ << 1)
      : (map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
  map_pointer new_map = CreateMap(new_map_size);
  const size_type old_buffer =
    static_cast<size_type>(end_.node_ - begin_.node_) + 1;
  const size_type new_buffer = old_buffer + need_buffer;

  /* the new map points to the original buffers, and new buffers are created */
  map_pointer begin = new_map + (new_map_size - new_buffer) / 2;
  map_pointer mid = begin + need_buffer;
  map_pointer end = mid + old_buffer;
  CreateBuffer(begin, mid - 1);
  for (map_pointer p1 = mid, p2 = begin_.node_; p1 != end; ++p1, ++p2)
    *p1 = *p2;

  /* update data */
  map_allocator::Deallocate(map_, map_size_);
  map_ = new_map;
  map_size_ = new_map_size;
  begin_ = iterator(*mid + (begin_.cur_ - begin_.first_), mid);
  end_ = iterator(*(end - 1) + (end_.cur_ - end_.first_), end - 1);
}

template <typename T>
void NuoDeque<T>::ReallocateMapAtBack(size_type need_buffer)
{
  const size_type new_map_size =
    (map_size_ << 1) > (map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE)
      ? (map_size_ << 1)
      : (map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
  map_pointer new_map = CreateMap(new_map_size);
  const size_type old_buffer =
    static_cast<size_type>(end_.node_ - begin_.node_) + 1;
  const size_type new_buffer = old_buffer + need_buffer;

  /* the new map points to the original buffers, and new buffers are created */
  map_pointer begin = new_map + (new_map_size - new_buffer) / 2;
  map_pointer mid = begin + old_buffer;
  map_pointer end = mid + need_buffer;
  for (map_pointer p1 = begin, p2 = begin_.node_; p1 != mid; ++p1, ++p2)
    *p1 = *p2;
  CreateBuffer(mid, end - 1);

  /* update data */
  map_allocator::Deallocate(map_, map_size_);
  map_ = new_map;
  map_size_ = new_map_size;
  begin_ = iterator(*begin + (begin_.cur_ - begin_.first_), begin);
  end_ = iterator(*(mid - 1) + (end_.cur_ - end_.first_), mid - 1);
}

} /* namespace nuostl */