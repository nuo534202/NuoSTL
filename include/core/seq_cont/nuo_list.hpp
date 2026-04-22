#pragma once

#include <initializer_list>

#include "utils/nuo_allocator.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_functional.hpp"
#include "utils/nuo_iterator.hpp"

namespace nuostl
{

/****************************************************/
/* List Node */
template <typename T> class NuoListNodeBase;
template <typename T> class NuoListNode;

template <typename T>
struct NuoNodeTraits
{
public:
  using base_ptr = NuoListNodeBase<T>*;
  using node_ptr = NuoListNode<T>*;
};

template <typename T>
class NuoListNodeBase
{
public:
  using base_ptr = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr = typename NuoNodeTraits<T>::node_ptr;

  NuoListNodeBase() = default;

  node_ptr ToNode()
  {
    return static_cast<node_ptr>(Self());
  }

  void Unlink()
  {
    prev_ = next_ = Self();
  }

  base_ptr Self()
  {
    return static_cast<base_ptr>(&(*this));
  }

  base_ptr prev_;
  base_ptr next_;
};

template <typename T>
class NuoListNode : public NuoListNodeBase<T>
{
public:
  using base_ptr = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr = typename NuoNodeTraits<T>::node_ptr;

  NuoListNode() = default;
  NuoListNode(const T& value)
    : value_(value)
  {
  }
  NuoListNode(T&& value)
    : value_(NuoMove(value))
  {
  }

  base_ptr ToBase()
  {
    return static_cast<base_ptr>(&(*this));
  }

  node_ptr Self()
  {
    return static_cast<node_ptr>(&(*this));
  }

  T value_;
};

/****************************************************/
/* List Iterator */
template <typename T>
class NuoListIterator :
  public NuoIterator<NuoBidirectionalIteratorTag, T>
{
public:
  using value_type  = T;
  using pointer     = T*;
  using reference   = T&;
  using base_ptr    = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr    = typename NuoNodeTraits<T>::node_ptr;
  using self        = NuoListIterator<T>;

  NuoListIterator() = default;
  NuoListIterator(base_ptr node)
    : node_(node)
  {
  }
  NuoListIterator(node_ptr node)
    : node_(node->to_base())
  {
  }
  NuoListIterator(const NuoListIterator& iterator)
    : node_(iterator.node_)
  {
  }

  reference operator*() const
  {
    return node_->ToNode()->value_;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self operator++(int)
  {
    self tmp = *this;
    ++*this;
    return tmp;
  }

  self& operator++()
  {
    node_ = node_->next_;
    return *this;
  }

  self operator--(int)
  {
    self tmp = *this;
    --*this;
    return tmp;
  }

  self& operator--()
  {
    node_ = node_->prev_;
    return *this;
  }

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  base_ptr node_;
};

/****************************************************/
/* List Const Iterator*/
template <typename T>
class NuoListConstIterator :
  public NuoIterator<NuoBidirectionalIteratorTag, T>
{
public:
  using value_type  = T;
  using pointer     = const T*;
  using reference   = const T&;
  using base_ptr    = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr    = typename NuoNodeTraits<T>::node_ptr;
  using self        = NuoListConstIterator<T>;

  NuoListConstIterator() = default;
  NuoListConstIterator(base_ptr node)
    : node_(node)
  {
  }
  NuoListConstIterator(node_ptr node)
    : node_(node->to_base())
  {
  }
  NuoListConstIterator(const NuoListConstIterator& iterator)
    : node_(iterator.node_)
  {
  }

  reference operator*() const
  {
    return node_->ToNode()->value_;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self operator++(int)
  {
    self tmp = *this;
    ++*this;
    return tmp;
  }

  self& operator++()
  {
    node_ = node_->next_;
    return *this;
  }

  self operator--(int)
  {
    self tmp = *this;
    --*this;
    return tmp;
  }

  self& operator--()
  {
    node_ = node_->prev_;
    return *this;
  }

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  base_ptr node_;
};

/****************************************************/
/* List */
template <typename T>
class NuoList
{
public:
  /* types */
  using allocator_type          = NuoAllocator<T>;
  using data_allocator          = NuoAllocator<T>;
  using base_allocator          = NuoAllocator<NuoListNodeBase<T>>;
  using node_allocator          = NuoAllocator<NuoListNode<T>>;

  using value_type              = allocator_type::value_type;
  using pointer                 = allocator_type::pointer;
  using const_pointer           = allocator_type::const_pointer;
  using reference               = allocator_type::reference;
  using const_reference         = allocator_type::const_reference;
  using size_type               = allocator_type::size_type;
  using difference_type         = allocator_type::difference_type;

  using iterator                = NuoListIterator<T>;
  using const_iterator          = NuoListConstIterator<T>;
  using reverse_iterator        = NuoReverseIterator<iterator>;
  using const_reverse_iterator  = NuoReverseIterator<const_iterator>;

  using base_ptr                = NuoNodeTraits<T>::base_ptr;
  using node_ptr                = NuoNodeTraits<T>::node_ptr;

public:
  /* construct / copy / destroy */
  NuoList() : NuoList(0) {};
  explicit NuoList(size_type n) : NuoList(n, value_type()) {};
  NuoList(size_type n, const T& value);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::value, bool>::type = false>
  NuoList(InputIter first, InputIter last);
  NuoList(const NuoList& nlist);
  NuoList(NuoList&& nlist);
  NuoList(std::initializer_list<T> ilist);
  ~NuoList();

  /* assign */
  NuoList& operator=(const NuoList& nlist);
  NuoList& operator=(NuoList&& nlist) noexcept;
  NuoList& operator=(std::initializer_list<T> ilist);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::value, bool>::type = false>
  void Assign(InputIter first, InputIter last);
  void Assign(size_type n, const value_type& value);
  void Assign(std::initializer_list<T> ilist);

  /* iterators */
  iterator Begin() noexcept;
  const_iterator Begin() const noexcept;
  iterator End() noexcept;
  const_iterator End() const noexcept;
  reverse_iterator Rbegin() noexcept;
  const_reverse_iterator Rbegin() const noexcept;
  reverse_iterator Rend() noexcept;
  const_reverse_iterator Rend() const noexcept;

  const_iterator Cbegin() const noexcept;
  const_iterator Cend() const noexcept;
  const_reverse_iterator Crbegin() const noexcept;
  const_reverse_iterator Crend() const noexcept;

  /* capacity */
  bool Empty() const noexcept;
  size_type Size() const noexcept;
  size_type MaxSize() const noexcept;
  void Resize(size_type size);
  void Resize(size_type size, const value_type& value);

  /* element access */
  reference Front();
  const_reference Front() const;
  reference Back();
  const_reference Back() const;

  /* modifiers */
  template <typename... Args>
  reference EmplaceFront(Args&& ...args);
  template <typename... Args>
  reference EmplaceBack(Args&& ...args);
  void PushFront(const value_type& value);
  void PushFront(value_type&& value);
  void PopFront();
  void PushBack(const value_type& value);
  void PushBack(value_type&& value);
  void PopBack();

  template <typename... Args>
  iterator Emplace(const_iterator position, Args&& ...args);
  iterator Insert(const_iterator position, const value_type& value);
  iterator Insert(const_iterator position, value_type&& value);
  iterator Insert(const_iterator position,
                  size_type n,
                  const value_type& value);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::value, bool>::type = false>
  iterator Insert(const_iterator position, InputIter first, InputIter last);
  iterator Insert(const_iterator position, std::initializer_list<T> ilist);

  iterator Erase(const_iterator position);
  iterator Erase(const_iterator first, const_iterator last);
  void Swap(NuoList& nlist) noexcept;
  void Clear() noexcept;

  void Splice(const_iterator position, NuoList& nlist);
  void Splice(const_iterator position, NuoList&& nlist);
  void Splice(const_iterator position, NuoList& nlist, const_iterator i);
  void Splice(const_iterator position, NuoList&& nlist, const_iterator i);
  void Splice(const_iterator position,
              NuoList& nlist,
              const_iterator first,
              const_iterator last);
  void Splice(const_iterator position,
              NuoList&& nlist,
              const_iterator first,
              const_iterator last);

  size_type Remove(const value_type& value);
  template <typename Predicate>
  size_type RemoveIf(Predicate pred);

  size_type Unique();
  template <typename BinaryPredicate>
  size_type Unique(BinaryPredicate binary_pred);

  void Merge(NuoList& nlist);
  void Merge(NuoList&& nlist);
  template <typename Compare>
  void Merge(NuoList& nlist, Compare comp);
  template <typename Compare>
  void Merge(NuoList&& nlist, Compare comp);

  void Sort();
  template <typename Compare>
  void Sort(Compare comp);

  void Reverse() noexcept;

private:
  /* helper functions */
  node_ptr CreateNode(const value_type& value);
  void DestroyNode(node_ptr node);
  void AddNodeToFront(base_ptr node);
  void AddNodeToBack(base_ptr node);
  void AddLinkNodeToBack(base_ptr first, base_ptr last);
  void RemoveLinkedNode(base_ptr first, base_ptr last);
  void UnlinkNodes(base_ptr first, base_ptr last);

  void FillInit(size_type n, const value_type& value);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  void CopyInit(InputIter first, InputIter last);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  void CopyAssign(InputIter first, InputIter last);
  void CopyAssign(size_type n, const value_type& value);

  template <typename Compare>
  iterator MergeSort(iterator first,
                     iterator last,
                     size_type size,
                     Compare comp);

private:
  base_ptr  tail_;  /* circular linked list, no need head ptr */
  size_type size_;
};

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
template <class InputIter,
          typename std::enable_if<NuoIsInputIterator<InputIter>::value,
                                  int>::type = 0>
NuoList(InputIter, InputIter)
  -> NuoList<typename NuoIteratorTraits<InputIter>::value_type>;
#endif

/* implementation */

/* construct / copy / destroy */

template <typename T>
NuoList<T>::NuoList(size_type n, const T& value)
{
  FillInit(n, value);
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::value, bool>::type>
NuoList<T>::NuoList(InputIter first, InputIter last)
{
  CopyInit(first, last);
}

template <typename T>
NuoList<T>::NuoList(const NuoList& nlist)
{
  CopyInit(nlist.Begin(), nlist.End());
}

template <typename T>
NuoList<T>::NuoList(NuoList&& nlist)
  : tail_(NuoMove(nlist.tail_))
  , size_(NuoMove(nlist.size_))
{
  nlist.tail_ = nullptr;
  nlist.tail_ = 0;
}

template <typename T>
NuoList<T>::NuoList(std::initializer_list<T> ilist)
{
  CopyInit(ilist.begin(), ilist.end());
}

template <typename T>
NuoList<T>::~NuoList()
{
  if (tail_ != nullptr)
  {
    Clear();
    base_allocator::Deallocate(tail_);
    tail_ = nullptr;
    size_ = 0;
  }
}

/* assign */

template <typename T>
typename NuoList<T>::NuoList&
NuoList<T>::operator=(const NuoList& nlist)
{
  if (this != &nlist)
    CopyAssign(nlist.Begin(), nlist.End());

  return *this;
}

template <typename T>
typename NuoList<T>::NuoList&
NuoList<T>::operator=(NuoList&& nlist) noexcept
{
  Clear();
  Splice(End(), NuoMove(nlist));
  return *this;
}

template <typename T>
typename NuoList<T>::NuoList&
NuoList<T>::operator=(std::initializer_list<T> ilist)
{
  Clear();
  CopyAssign(ilist.begin(), ilist.end());
  return *this;
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::value, bool>::type>
void
NuoList<T>::Assign(InputIter first, InputIter last)
{
  Clear();
  CopyAssign(first, last);
}

template <typename T>
void
NuoList<T>::Assign(size_type n, const value_type& value)
{
  Clear();
  CopyAssign(n, value);
}

template <typename T>
void
NuoList<T>::Assign(std::initializer_list<T> ilist)
{
  Clear();
  CopyAssign(ilist.begin(), ilist.end());
}

/* iterators */

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Begin() noexcept
{
  return iterator(tail_->next_);
}

template <typename T>
typename NuoList<T>::const_iterator
NuoList<T>::Begin() const noexcept
{
  return const_iterator(tail_->next_);
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::End() noexcept
{
  return iterator(tail_);
}

template <typename T>
typename NuoList<T>::const_iterator
NuoList<T>::End() const noexcept
{
  return const_iterator(tail_);
}

template <typename T>
typename NuoList<T>::reverse_iterator
NuoList<T>::Rbegin() noexcept
{
  return reverse_iterator(tail_);
}

template <typename T>
typename NuoList<T>::const_reverse_iterator
NuoList<T>::Rbegin() const noexcept
{
  return const_reverse_iterator(tail_);
}

template <typename T>
typename NuoList<T>::reverse_iterator
NuoList<T>::Rend() noexcept
{
  return reverse_iterator(tail_->next_);
}

template <typename T>
typename NuoList<T>::const_reverse_iterator
NuoList<T>::Rend() const noexcept
{
  return const_reverse_iterator(tail_->next_);
}

template <typename T>
typename NuoList<T>::const_iterator
NuoList<T>::Cbegin() const noexcept
{
  return const_iterator(tail_->next_);
}

template <typename T>
typename NuoList<T>::const_iterator
NuoList<T>::Cend() const noexcept
{
  return const_iterator(tail_);
}

template <typename T>
typename NuoList<T>::const_reverse_iterator
NuoList<T>::Crbegin() const noexcept
{
  return const_reverse_iterator(tail_);
}

template <typename T>
typename NuoList<T>::const_reverse_iterator
NuoList<T>::Crend() const noexcept
{
  return const_reverse_iterator(tail_->next_);
}

/* capacity */

template <typename T>
bool NuoList<T>::Empty() const noexcept
{
  return size_ == 0;
}

template <typename T>
typename NuoList<T>::size_type
NuoList<T>::Size() const noexcept
{
  return size_;
}

template <typename T>
typename NuoList<T>::size_type
NuoList<T>::MaxSize() const noexcept
{
  return static_cast<size_type>(-1);
}

template <typename T>
void NuoList<T>::Resize(size_type size)
{
  Resize(size, value_type());
}

template <typename T>
void NuoList<T>::Resize(size_type size, const value_type& value)
{
  base_ptr cur = Begin();
  size_type i = 0;
  while (cur != tail_ && i < size)
    cur++, size++;

  if (i < size_)
  {
    Erase(cur, tail_);
  }
  else if (i > size_)
  {
    Insert(cur, value);
  }
}

/* element access */

template <typename T>
typename NuoList<T>::reference
NuoList<T>::Front()
{
  return *Begin();
}

template <typename T>
typename NuoList<T>::const_reference
NuoList<T>::Front() const
{
  return *Cbegin();
}

template <typename T>
typename NuoList<T>::reference
NuoList<T>::Back()
{
  return *Rbegin();
}

template <typename T>
typename NuoList<T>::const_reference
NuoList<T>::Back() const
{
  return *Crbegin();
}

/* modifiers */
template <typename T>
template <typename... Args>
typename NuoList<T>::reference
NuoList<T>::EmplaceFront(Args&& ...args)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoList<T> size is too large!");

  node_ptr node = CreateNode(NuoForward<Args>(args)...);
  AddNodeToFront(node->ToBase());

  return Front();
}

template <typename T>
template <typename... Args>
typename NuoList<T>::reference
NuoList<T>::EmplaceBack(Args&& ...args)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoList<T> size is too large!");

  node_ptr node = CreateNode(NuoForward<Args>(args)...);
  AddNodeToFront(node->ToBase());

  return Back();
}

template <typename T>
void
NuoList<T>::PushFront(const value_type& value)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoList<T> size is too large!");

  node_ptr node = CreateNode(value);
  AddNodeToFront(node->ToBase());
}

template <typename T>
void
NuoList<T>::PushFront(value_type&& value)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoList<T> size is too large!");

  node_ptr node = CreateNode(NuoMove(value));
  AddNodeToFront(node->ToBase());
}

template <typename T>
void
NuoList<T>::PopFront()
{
  base_ptr front_node = tail_->next_;
  RemoveLinkedNode(front_node, front_node);
  DestroyNode(front_node->ToNode());
  size_--;
}

template <typename T>
void
NuoList<T>::PushBack(const value_type& value)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoList<T> size is too large!");

  node_ptr node = CreateNode(value);
  AddNodeToBack(node->ToBase());
}

template <typename T>
void
NuoList<T>::PushBack(value_type&& value)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoList<T> size is too large!");

  node_ptr node = CreateNode(NuoMove(value));
  AddNodeToBack(node->ToBase());
}

template <typename T>
void
NuoList<T>::PopBack()
{
  base_ptr back_node = tail_->prev_;
  RemoveLinkedNode(back_node, back_node);
  DestroyNode(back_node->ToNode());
  size_--;
}

template <typename T>
template <typename... Args>
typename NuoList<T>::iterator
NuoList<T>::Emplace(const_iterator position, Args&& ...args)
{
  base_ptr pos_node = position.node_;
  base_ptr node = CreateNode(NuoForward<Args>(args)...);

  node->prev_ = pos_node;
  node->next_ = pos_node->next_;
  pos_node->next_->prev_ = node;
  pos_node->next_ = node;

  return iterator(node);
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Insert(const_iterator position, const value_type& value)
{
  return Emplace(position, value);
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Insert(const_iterator position, value_type&& value)
{
  return Emplace(position, NuoMove(value));
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Insert(const_iterator position,
                   size_type n,
                   const value_type& value)
{
  base_ptr pos_node = position.node_;
  while (n--)
  {
    base_ptr node = CreateNode(value);
    node->prev_ = pos_node;
    node->next_ = pos_node->next_;
    pos_node->next_->prev_ = node;
    pos_node->next_ = node;
  }
  
  return (position + 1);
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::value, bool>::type>
typename NuoList<T>::iterator
NuoList<T>::Insert(const_iterator position, InputIter first, InputIter last)
{
  base_ptr prev_node = position.node_;
  base_ptr next_node = prev_node->next_;

  for (InputIter it = first; it != last; it++)
  {
    base_ptr node = CreateNode(*it);

    node->prev_ = prev_node;
    node->next_ = next_node;
    prev_node->next_ = next_node->prev_ = node;

    prev_node = node;
  }

  return position.node_->next_;
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Insert(const_iterator position, std::initializer_list<T> ilist)
{
  return Insert(ilist.begin(), ilist.end());
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Erase(const_iterator position)
{
  return Erase(position, position + 1);
}

template <typename T>
typename NuoList<T>::iterator
NuoList<T>::Erase(const_iterator first, const_iterator last)
{
  base_ptr prev_node = first.node_->prev_;
  base_ptr next_node = last.node_->next_;

  prev_node->next_ = next_node;
  next_node->prev_ = prev_node;

  for (const_iterator it = first; it != last; it++)
    DestroyNode(it.node_->ToNode());
}

template <typename T>
void
NuoList<T>::Swap(NuoList& nlist) noexcept
{
  NuoSwap(tail_, nlist.tail_);
  NuoSwap(size_, nlist.size_);
}

template <typename T>
void
NuoList<T>::Clear() noexcept
{
  for (base_ptr cur = tail_->next_; cur != tail_;)
  {
    base_ptr next = cur->next_;
    DestroyNode(cur->ToNode());
    cur = next;
  }

  UnlinkNodes(tail_, tail_);
  size_ = 0;
}

/* list operations */

template <typename T>
void
NuoList<T>::Splice(const_iterator position, NuoList& nlist)
{
  Splice(position, nlist, Begin(), End());
}

template <typename T>
void
NuoList<T>::Splice(const_iterator position, NuoList&& nlist)
{
  Splice(position, NuoMove(nlist), Begin(), End());
}

template <typename T>
void
NuoList<T>::Splice(const_iterator position, NuoList& nlist, const_iterator i)
{
  Splice(position, nlist, i, i + 1);
}

template <typename T>
void
NuoList<T>::Splice(const_iterator position, NuoList&& nlist, const_iterator i)
{
  Splice(position, NuoMove(nlist), i, i + 1);
}

template <typename T>
void
NuoList<T>::Splice(const_iterator position,
                   NuoList& nlist,
                   const_iterator first,
                   const_iterator last)
{
  if (first == last || this == &nlist)
    return;

  NUO_THROW_LENGTH_ERROR_IF(size_ + NuoDistance(first, last) > MaxSize(),
    "NuoList<T> size is too large!");

  iterator prev_pos = position->node_->prev;
  iterator next_pos = position->node_;

  iterator first_pos = first->node_;
  iterator last_pos = last->node_;

  first_pos->prev_->next_ = last->next_;
  last_pos->next_->prev_ = first->prev_;

  first_pos->prev_ = prev_pos;
  last_pos->next = next_pos;

  prev_pos->next_ = first_pos;
  next_pos->prev_ = last_pos;
}

template <typename T>
void
NuoList<T>::Splice(const_iterator position,
                   NuoList&& nlist,
                   const_iterator first,
                   const_iterator last)
{
  Splice(position, NuoMove(nlist), first, last);
}

template <typename T>
typename NuoList<T>::size_type
NuoList<T>::Remove(const value_type& value)
{
  RemoveIf([&](const value_type& v){ return v == value; });
}

template <typename T>
template <typename Predicate>
typename NuoList<T>::size_type
NuoList<T>::RemoveIf(Predicate pred)
{
  const_iterator first = Begin(), last = End();
  for (iterator cur = first; cur != last; cur++)
  {
    if (pred(*cur))
      Erase(cur);
  }
}

template <typename T>
typename NuoList<T>::size_type
NuoList<T>::Unique()
{
  return Unique(NuoEqualTo<T>());
}

template <typename T>
template <typename BinaryPredicate>
typename NuoList<T>::size_type
NuoList<T>::Unique(BinaryPredicate binary_pred)
{
  iterator first = Begin(), last = End();
  size_type del_count = 0;

  while (first != last)
  {
    if (pred(*first, *(first + 1)))
    {
      Erase(first + 1);
      del_count++;
    }

    first++;
  }

  return del_count;
}

template <typename T>
void
NuoList<T>::Sort()
{
  MergeSort(Begin(), End(), Size(), NuoLess<T>());
}

template <typename T>
template <typename Compare>
void
NuoList<T>::Sort(Compare comp)
{
  MergeSort(Begin(), End(), Size(), comp);
}

template <typename T>
void
NuoList<T>::Merge(NuoList& nlist)
{
  Merge(nlist, NuoLess<T>());
}

template <typename T>
void
NuoList<T>::Merge(NuoList&& nlist)
{
  Merge(NuoMove(nlist), NuoLess<T>());
}

template <typename T>
template <typename Compare>
void
NuoList<T>::Merge(NuoList& nlist, Compare comp)
{
  if (this == &nlist)
    return;

  NUO_THROW_LENGTH_ERROR_IF(size_ + nlist.size_ > MaxSize(),
    "NuoList<T> size is too large!");

  iterator first1 = Begin(), last1 = End();
  iterator first2 = nlist.Begin(), last2 = nlist.End();

  while (first1 != last1 && first2 != last2)
  {
    if (comp(*first2, *first1))
    {
      iterator next = first2;
      next++;

      base_ptr next_node = first1->node_;
      base_ptr insert_node = first2->node_;

      UnlinkNodes(insert_node, insert_node);
      insert_node->prev_ = next_node->prev_;
      insert_node->next_ = next_node;
    
      next_node->prev_->next_ = insert_node;
      next_node->prev_ = insert_node;
    }
    else
    {
      first1++;
    }
  }

  if (first2 != last2)
  {
    UnlinkNodes(first2->node_, last2->node_->prev_);
    AddLinkNodeToBack(first2->node_, last2->node_->prev_);
  }

  size_ += nlist.size_, nlist.size_ = 0;
}

template <typename T>
template <typename Compare>
void
NuoList<T>::Merge(NuoList&& nlist, Compare comp)
{
  Merge(NuoMove(nlist), comp);
}

template <typename T>
void NuoList<T>::Reverse() noexcept
{
  if (size_ <= 1)
    return;

  iterator first = Begin(), last = End();
  while (first != last)
  {
    NuoSwap(first->node_->prev_, first->node_->next_);
    first = first->node_->prev_;
  }
  NuoSwap(last->node_->prev_, last->node_->next_);
}

/* helper functions */

/* Add node to the back of the list */

template <typename T>
typename NuoList<T>::node_ptr
NuoList<T>::CreateNode(const value_type& value)
{
  node_ptr node = node_allocator::Allocate();
  node_allocator::Construct(node, value);
  return node;
}

template <typename T>
void
NuoList<T>::DestroyNode(node_ptr node)
{
  data_allocator::Destroy(&node->value_);
  node_allocator::Destroy(node);
}

template <typename T>
void
NuoList<T>::AddNodeToFront(base_ptr node)
{
  node->prev_ = tail_, node->next_ = tail_->next_;
  tail_->next_->prev_ = node;
  tail_->next_ = node;
  size_++;
}

template <typename T>
void
NuoList<T>::AddNodeToBack(base_ptr node)
{
  node->prev_ = tail_->prev_;
  node->next_ = tail_;
  tail_->prev_->next_ = node;
  tail_->prev_ = node;
  size_++;
}

/* Add link node [first, last] to the back of the list */
template <typename T>
void
NuoList<T>::AddLinkNodeToBack(base_ptr first, base_ptr last)
{
  first.prev_ = tail_.prev_;
  last.next_ = tail_;
  tail_.prev_.next_ = first;
  tail_.prev_ = last;
  size_ += NuoDistance(first, last);
}

template <typename T>
void
NuoList<T>::RemoveLinkedNode(base_ptr first, base_ptr last)
{
  size_ -= NuoDistance(first, last);

  first->prev_->next_ = last->next_;
  last->next_->prev_ = first->prev_;

  UnlinkNodes(first, last);
  for (base_ptr cur = first; cur != last;)
  {
    base_ptr next = cur->next_;
    DestroyNode(cur->ToNode());
    cur = next;
  }
}

template <typename T>
void
NuoList<T>::UnlinkNodes(base_ptr first, base_ptr last)
{
  first->prev_->next_ = last->next_;
  last->next_->prev_ = first->prev_;
  first->prev_ = last;
  last->next_ = first;
}

template <typename T>
void
NuoList<T>::FillInit(size_type n, const value_type& value)
{
  NUO_THROW_OUT_OF_RANGE_IF(n < 0, "out of range: n must be a natural number!");

  tail_ = base_allocator::Allocate();
  tail_->Unlink();
  size_ = 0;

  try
  {
    while (n--)
    {
      node_ptr node = CreateNode(value);
      AddNodeToBack(node->ToBase());
    }

  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(tail_);
    tail_ = nullptr, size_ = 0;
    throw;
  }
}

template <typename T>
template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type>
void
NuoList<T>::CopyInit(InputIter first, InputIter last)
{
  tail_ = base_allocator::Allocate();
  tail_->Unlink();
  size_ = 0;

  try
  {
    for (InputIter it = first; it != last; it++)
    {
      node_ptr node = CreateNode(*it);
      AddNodeToBack(node->ToBase());
    }
  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(tail_);
    tail_ = nullptr, size_ = 0;
    throw;
  } 
}

template <typename T>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, bool>::type>
void
NuoList<T>::CopyAssign(InputIter first, InputIter last)
{
  tail_->Unlink();
  size_ = 0;

  try
  {
    for (InputIter it = first; it != last; it++)
    {
      node_ptr node = CreateNode(*it);
      AddNodeToBack(node->ToBase());
    }
  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(tail_);
    tail_ = nullptr, size_ = 0;
    throw;
  }
}

template <typename T>
void
NuoList<T>::CopyAssign(size_type n, const value_type& value)
{
  tail_->Unlink();
  size_ = 0;

  try
  {
    while (n--)
    {
      node_ptr node = CreateNode(value);
      AddNodeToBack(node);
    }
  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(tail_);
    tail_ = nullptr, size_ = 0;
    throw;
  }
}

template <typename T>
template <typename Compare>
typename NuoList<T>::iterator
NuoList<T>::MergeSort(iterator first,
                      iterator last,
                      size_type size,
                      Compare comp)
{
  if (size <= 1)
    return first;

  iterator mid_iter = first + NuoAdvance(first, size / 2);
  iterator first1 = MergeSort(first, mid_iter, size / 2, comp);
  iterator first2 = MergeSort(mid_iter, last, size - size / 2, comp);

  while (first != mid_iter && first2 != last)
  {
    if (comp(*first2, *first1))
    {
      UnlinkNodes(first2->node_, first2->node_);
      first2->node_->prev_ = first1->node_->prev_;
      first2->node_->next_ = first1->node_;
      first1->node_->prev_->next_ = first1->node_->prev_ = first2->node_;
    }
    else
    {
      first1++;
    }
  }

  return first;
}

/* overload NuoSwap */
template <typename T>
void
NuoSwap(NuoList<T>& lhs, NuoList<T>& rhs) noexcept
{
  lhs.Swap(rhs);
}

} /* namespace nuostl */