#pragma once

#include <initializer_list>

#include "utils/nuo_allocator.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_functional.hpp"
#include "utils/nuo_iterator.hpp"

namespace nuostl
{

/****************************************************/
/* Forward List Node */
template <typename T> class NuoForwardListNodeBase;
template <typename T> class NuoForwardListNode;

template <typename T>
struct NuoNodeTraits
{
public:
  using base_ptr = NuoForwardListNodeBase<T>*;
  using node_ptr = NuoForwardListNode<T>*;
};

template <typename T>
class NuoForwardListNodeBase
{
public:
  using base_ptr = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr = typename NuoNodeTraits<T>::node_ptr;

  NuoForwardListNodeBase() = default;

  node_ptr ToNode()
  {
    return static_cast<node_ptr>(Self());
  }

  void Unlink()
  {
    next_ = Self();
  }

  base_ptr Self()
  {
    return static_cast<base_ptr>(&(*this));
  }

  base_ptr next_;
};

template <typename T>
class NuoForwardListNode : public NuoForwardListNodeBase<T>
{
public:
  using base_ptr = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr = typename NuoNodeTraits<T>::node_ptr;

  NuoForwardListNode() = default;
  NuoForwardListNode(const T& value)
    : value_(value)
  {
  }
  NuoForwardListNode(T&& value)
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
/* Forward List Iterator */
template <typename T>
class NuoForwardListIterator :
  public NuoIterator<NuoForwardIteratorTag, T>
{
public:
  using value_type  = T;
  using pointer     = T*;
  using reference   = T&;
  using base_ptr    = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr    = typename NuoNodeTraits<T>::node_ptr;
  using self        = NuoForwardListIterator<T>;

  NuoForwardListIterator() = default;
  explicit NuoForwardListIterator(base_ptr node)
    : node_(node)
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

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ != rhs.node_;
  }

  base_ptr node_;
};

/****************************************************/
/* Forward List Const Iterator */
template <typename T>
class NuoForwardListConstIterator :
  public NuoIterator<NuoForwardIteratorTag, T>
{
public:
  using value_type  = T;
  using pointer     = const T*;
  using reference   = const T&;
  using base_ptr    = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr    = typename NuoNodeTraits<T>::node_ptr;
  using self        = NuoForwardListConstIterator<T>;

  NuoForwardListConstIterator() = default;
  explicit NuoForwardListConstIterator(base_ptr node)
    : node_(node)
  {
  }
  /* converting constructor from mutable iterator */
  NuoForwardListConstIterator(const NuoForwardListIterator<T>& iterator)
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

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ != rhs.node_;
  }

  base_ptr node_;
};

/****************************************************/
/* Forward List */
template <typename T, typename Allocator = NuoAllocator<T>>
class NuoForwardList
{
public:
  /* types */
  using allocator_type          = Allocator;
  using data_allocator          = NuoAllocator<T>;
  using base_allocator          = NuoAllocator<NuoForwardListNodeBase<T>>;
  using node_allocator          = NuoAllocator<NuoForwardListNode<T>>;

  using value_type              = typename allocator_type::value_type;
  using pointer                 = typename allocator_type::pointer;
  using const_pointer           = typename allocator_type::const_pointer;
  using reference               = typename allocator_type::reference;
  using const_reference         = typename allocator_type::const_reference;
  using size_type               = typename allocator_type::size_type;
  using difference_type         = typename allocator_type::difference_type;

  using iterator                = NuoForwardListIterator<T>;
  using const_iterator          = NuoForwardListConstIterator<T>;
  using reverse_iterator        = NuoReverseIterator<iterator>;
  using const_reverse_iterator  = NuoReverseIterator<const_iterator>;

  using base_ptr                = typename NuoNodeTraits<T>::base_ptr;
  using node_ptr                = typename NuoNodeTraits<T>::node_ptr;

public:
  /* construct / copy / destroy */
  NuoForwardList() : NuoForwardList(0) {};
  explicit NuoForwardList(size_type n) : NuoForwardList(n, value_type()) {};
  NuoForwardList(size_type n, const value_type& value);
  NuoForwardList(size_type n, const value_type& value, const Allocator& alloc);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  NuoForwardList(InputIter first, InputIter last);
  NuoForwardList(const NuoForwardList& nflist);
  NuoForwardList(NuoForwardList&& nflist);
  NuoForwardList(std::initializer_list<T> ilist);
  ~NuoForwardList();

  /* assign */
  NuoForwardList& operator=(const NuoForwardList& nflist);
  NuoForwardList& operator=(NuoForwardList&& nflist) noexcept;
  NuoForwardList& operator=(std::initializer_list<T> ilist);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  void Assign(InputIter first, InputIter last);
  void Assign(size_type n, const value_type& value);
  void Assign(std::initializer_list<T> ilist);
  allocator_type GetAllocator() const noexcept;

  /* iterators */
  iterator BeforeBegin() noexcept;
  const_iterator BeforeBegin() const noexcept;
  iterator Begin() noexcept;
  const_iterator Begin() const noexcept;
  iterator End() noexcept;
  const_iterator End() const noexcept;

  const_iterator CBeforeBegin() const noexcept;
  const_iterator Cbegin() const noexcept;
  const_iterator Cend() const noexcept;

  /* capacity */
  bool Empty() const noexcept;
  size_type Size() const noexcept;
  size_type MaxSize() const noexcept;
  void Resize(size_type size);
  void Resize(size_type size, const value_type& value);

  /* element access */
  reference Front();
  const_reference Front() const;

  /* modifiers */
  template <typename... Args>
  reference EmplaceFront(Args&& ...args);
  void PushFront(const value_type& value);
  void PushFront(value_type&& value);
  void PopFront();

  template <typename... Args>
  iterator EmplaceAfter(const_iterator position, Args&& ...args);
  iterator InsertAfter(const_iterator position, const value_type& value);
  iterator InsertAfter(const_iterator position, value_type&& value);
  iterator InsertAfter(const_iterator position,
                       size_type n,
                       const value_type& value);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  iterator InsertAfter(const_iterator position, InputIter first, InputIter last);
  iterator InsertAfter(const_iterator position, std::initializer_list<T> ilist);

  iterator EraseAfter(const_iterator position);
  iterator EraseAfter(const_iterator first, const_iterator last);
  void Swap(NuoForwardList& nflist) noexcept;
  void Clear() noexcept;

  /* forward_list operations */
  void SpliceAfter(const_iterator position, NuoForwardList& nflist);
  void SpliceAfter(const_iterator position, NuoForwardList&& nflist);
  void SpliceAfter(const_iterator position,
                   NuoForwardList& nflist,
                   const_iterator i);
  void SpliceAfter(const_iterator position,
                   NuoForwardList&& nflist,
                   const_iterator i);
  void SpliceAfter(const_iterator position,
                   NuoForwardList& nflist,
                   const_iterator first,
                   const_iterator last);
  void SpliceAfter(const_iterator position,
                   NuoForwardList&& nflist,
                   const_iterator first,
                   const_iterator last);

  size_type Remove(const value_type& value);
  template <typename Predicate>
  size_type RemoveIf(Predicate pred);

  size_type Unique();
  template <typename BinaryPredicate>
  size_type Unique(BinaryPredicate binary_pred);

  void Merge(NuoForwardList& nflist);
  void Merge(NuoForwardList&& nflist);
  template <typename Compare>
  void Merge(NuoForwardList& nflist, Compare comp);
  template <typename Compare>
  void Merge(NuoForwardList&& nflist, Compare comp);

  void Sort();
  template <typename Compare>
  void Sort(Compare comp);

  void Reverse() noexcept;

private:
  /* helper functions */
  node_ptr CreateNode(const value_type& value);
  template <typename... Args>
  node_ptr CreateNode(Args&& ...args);
  void DestroyNode(node_ptr node);
  void AddNodeToFront(base_ptr node);
  void AddNodeAfter(base_ptr pos_node, base_ptr node);
  base_ptr PositionNode(const_iterator position);

  void FillInit(size_type n, const value_type& value);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  void CopyInit(InputIter first, InputIter last);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type = false>
  void CopyAssign(InputIter first, InputIter last);

private:
  base_ptr  head_;  /* head sentinel node, points to first element or itself */
  size_type size_;
};

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
template <class InputIter,
          typename std::enable_if<NuoIsInputIterator<InputIter>::kValue,
                                  int>::type = 0>
NuoForwardList(InputIter, InputIter)
  -> NuoForwardList<typename NuoIteratorTraits<InputIter>::value_type>;
#endif

/* implementation */

/* construct / copy / destroy */

template <typename T, class Allocator>
NuoForwardList<T, Allocator>::NuoForwardList(size_type n,
                                             const value_type& value)
{
  FillInit(n, value);
}

template <typename T, class Allocator>
NuoForwardList<T, Allocator>::NuoForwardList(size_type n,
                                             const value_type& value,
                                             const Allocator& alloc)
  : NuoForwardList(n, value)
{
  (void)alloc;
}

template <typename T, class Allocator>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, bool>::type>
NuoForwardList<T, Allocator>::NuoForwardList(InputIter first, InputIter last)
{
  CopyInit(first, last);
}

template <typename T, class Allocator>
NuoForwardList<T, Allocator>::NuoForwardList(const NuoForwardList& nflist)
{
  CopyInit(nflist.Begin(), nflist.End());
}

template <typename T, class Allocator>
NuoForwardList<T, Allocator>::NuoForwardList(NuoForwardList&& nflist)
  : head_(nflist.head_)
  , size_(nflist.size_)
{
  /* leave the source as a valid empty list */
  nflist.head_ = base_allocator::Allocate();
  nflist.head_->next_ = nullptr;
  nflist.size_ = 0;
}

template <typename T, class Allocator>
NuoForwardList<T, Allocator>::NuoForwardList(std::initializer_list<T> ilist)
{
  CopyInit(ilist.begin(), ilist.end());
}

template <typename T, class Allocator>
NuoForwardList<T, Allocator>::~NuoForwardList()
{
  if (head_ != nullptr)
  {
    Clear();
    base_allocator::Deallocate(head_);
    head_ = nullptr;
    size_ = 0;
  }
}

/* assign */

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::NuoForwardList&
NuoForwardList<T, Allocator>::operator=(const NuoForwardList& nflist)
{
  if (this != &nflist)
  {
    Clear();
    CopyAssign(nflist.Begin(), nflist.End());
  }

  return *this;
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::NuoForwardList&
NuoForwardList<T, Allocator>::operator=(NuoForwardList&& nflist) noexcept
{
  Clear();
  SpliceAfter(End(), NuoMove(nflist));
  return *this;
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::NuoForwardList&
NuoForwardList<T, Allocator>::operator=(std::initializer_list<T> ilist)
{
  Clear();
  CopyAssign(ilist.begin(), ilist.end());
  return *this;
}

template <typename T, class Allocator>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, bool>::type>
void
NuoForwardList<T, Allocator>::Assign(InputIter first, InputIter last)
{
  Clear();
  CopyAssign(first, last);
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::Assign(size_type n, const value_type& value)
{
  Clear();
  while (n--)
  {
    node_ptr node = CreateNode(value);
    AddNodeAfter(head_, node->ToBase());
  }
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::Assign(std::initializer_list<T> ilist)
{
  Clear();
  CopyAssign(ilist.begin(), ilist.end());
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::allocator_type
NuoForwardList<T, Allocator>::GetAllocator() const noexcept
{
  return allocator_type();
}

/* iterators */

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::BeforeBegin() noexcept
{
  return iterator(head_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_iterator
NuoForwardList<T, Allocator>::BeforeBegin() const noexcept
{
  return const_iterator(head_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::Begin() noexcept
{
  return iterator(head_->next_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_iterator
NuoForwardList<T, Allocator>::Begin() const noexcept
{
  return const_iterator(head_->next_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::End() noexcept
{
  return iterator(nullptr);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_iterator
NuoForwardList<T, Allocator>::End() const noexcept
{
  return const_iterator(nullptr);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_iterator
NuoForwardList<T, Allocator>::CBeforeBegin() const noexcept
{
  return const_iterator(head_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_iterator
NuoForwardList<T, Allocator>::Cbegin() const noexcept
{
  return const_iterator(head_->next_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_iterator
NuoForwardList<T, Allocator>::Cend() const noexcept
{
  return const_iterator(nullptr);
}

/* capacity */

template <typename T, class Allocator>
bool NuoForwardList<T, Allocator>::Empty() const noexcept
{
  return size_ == 0;
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::size_type
NuoForwardList<T, Allocator>::Size() const noexcept
{
  return size_;
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::size_type
NuoForwardList<T, Allocator>::MaxSize() const noexcept
{
  return static_cast<size_type>(-1);
}

template <typename T, class Allocator>
void NuoForwardList<T, Allocator>::Resize(size_type size)
{
  Resize(size, value_type());
}

template <typename T, class Allocator>
void NuoForwardList<T, Allocator>::Resize(size_type size,
                                          const value_type& value)
{
  base_ptr cur = head_;
  size_type i = 0;
  while (cur->next_ != nullptr && i < size)
  {
    cur = cur->next_;
    i++;
  }

  if (cur->next_ != nullptr)
  {
    /* shrink: erase all nodes after cur */
    node_ptr to_erase = cur->next_->ToNode();
    cur->next_ = nullptr;
    while (to_erase != nullptr)
    {
      node_ptr next = to_erase->next_ == nullptr ?
        nullptr : to_erase->next_->ToNode();
      DestroyNode(to_erase);
      to_erase = next;
    }
    size_ = size;
  }
  else
  {
    /* grow: append nodes until size */
    while (i < size)
    {
      node_ptr node = CreateNode(value);
      AddNodeAfter(cur, node->ToBase());
      cur = cur->next_;
      i++;
    }
  }
}

/* element access */

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::reference
NuoForwardList<T, Allocator>::Front()
{
  return *Begin();
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::const_reference
NuoForwardList<T, Allocator>::Front() const
{
  return *Cbegin();
}

/* modifiers */

template <typename T, class Allocator>
template <typename... Args>
typename NuoForwardList<T, Allocator>::reference
NuoForwardList<T, Allocator>::EmplaceFront(Args&& ...args)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoForwardList<T> size is too large!");

  node_ptr node = CreateNode(NuoForward<Args>(args)...);
  AddNodeToFront(node->ToBase());

  return Front();
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::PushFront(const value_type& value)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoForwardList<T> size is too large!");

  node_ptr node = CreateNode(value);
  AddNodeToFront(node->ToBase());
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::PushFront(value_type&& value)
{
  NUO_THROW_LENGTH_ERROR_IF(size_ > MaxSize() - 1,
    "NuoForwardList<T> size is too large!");

  node_ptr node = CreateNode(NuoMove(value));
  AddNodeToFront(node->ToBase());
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::PopFront()
{
  node_ptr front_node = head_->next_->ToNode();
  head_->next_ = head_->next_->next_;
  DestroyNode(front_node);
  size_--;
}

template <typename T, class Allocator>
template <typename... Args>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::EmplaceAfter(const_iterator position,
                                           Args&& ...args)
{
  base_ptr pos_node = PositionNode(position);
  node_ptr node = CreateNode(NuoForward<Args>(args)...);
  AddNodeAfter(pos_node, node->ToBase());
  return iterator(node);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::InsertAfter(const_iterator position,
                                          const value_type& value)
{
  return EmplaceAfter(position, value);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::InsertAfter(const_iterator position,
                                          value_type&& value)
{
  return EmplaceAfter(position, NuoMove(value));
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::InsertAfter(const_iterator position,
                                          size_type n,
                                          const value_type& value)
{
  base_ptr pos_node = PositionNode(position);
  while (n--)
  {
    node_ptr node = CreateNode(value);
    AddNodeAfter(pos_node, node->ToBase());
    pos_node = node->ToBase();
  }
  return iterator(pos_node);
}

template <typename T, class Allocator>
template <typename InputIt, typename std::enable_if<
  NuoIsInputIterator<InputIt>::kValue, bool>::type>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::InsertAfter(const_iterator position,
                                          InputIt first,
                                          InputIt last)
{
  base_ptr pos_node = PositionNode(position);
  for (InputIt it = first; it != last; it++)
  {
    node_ptr node = CreateNode(*it);
    AddNodeAfter(pos_node, node->ToBase());
    pos_node = node->ToBase();
  }
  return iterator(pos_node);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::InsertAfter(const_iterator position,
                                          std::initializer_list<T> ilist)
{
  return InsertAfter(position, ilist.begin(), ilist.end());
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::EraseAfter(const_iterator position)
{
  if (position.node_ == nullptr || position.node_->next_ == nullptr)
    return End();

  node_ptr to_erase = position.node_->next_->ToNode();
  position.node_->next_ = position.node_->next_->next_;
  DestroyNode(to_erase);
  size_--;

  return iterator(position.node_->next_);
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::iterator
NuoForwardList<T, Allocator>::EraseAfter(const_iterator first,
                                         const_iterator last)
{
  if (first.node_ == nullptr || first.node_->next_ == nullptr)
    return End();

  /* empty range (first, last): nothing to erase */
  if (first.node_ == last.node_ || first.node_->next_ == last.node_)
    return iterator(last.node_);

  node_ptr to_erase = first.node_->next_->ToNode();
  while (to_erase != nullptr && to_erase->ToBase() != last.node_)
  {
    node_ptr next = to_erase->next_ == nullptr ?
      nullptr : to_erase->next_->ToNode();
    DestroyNode(to_erase);
    to_erase = next;
    size_--;
  }
  first.node_->next_ = last.node_;

  return iterator(last.node_);
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::Swap(NuoForwardList& nflist) noexcept
{
  NuoSwap(head_, nflist.head_);
  NuoSwap(size_, nflist.size_);
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::Clear() noexcept
{
  for (base_ptr cur = head_->next_; cur != nullptr;)
  {
    base_ptr next = cur->next_;
    DestroyNode(cur->ToNode());
    cur = next;
  }
  head_->next_ = nullptr;
  size_ = 0;
}

/* forward_list operations */

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::SpliceAfter(const_iterator position,
                                          NuoForwardList& nflist)
{
  if (this == &nflist)
    return;

  SpliceAfter(position, nflist, nflist.BeforeBegin(), nflist.End());
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::SpliceAfter(const_iterator position,
                                          NuoForwardList&& nflist)
{
  SpliceAfter(position, nflist);
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::SpliceAfter(const_iterator position,
                                          NuoForwardList& nflist,
                                          const_iterator i)
{
  if (nflist.Empty())
    return;

  base_ptr i_node = i.node_;
  if (i_node == nullptr || i_node->next_ == nullptr)
    return;

  node_ptr moved = i_node->next_->ToNode();
  i_node->next_ = moved->next_;
  nflist.size_--;

  base_ptr pos_node = PositionNode(position);
  moved->next_ = pos_node->next_;
  pos_node->next_ = moved->ToBase();
  size_++;
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::SpliceAfter(const_iterator position,
                                          NuoForwardList&& nflist,
                                          const_iterator i)
{
  SpliceAfter(position, nflist, i);
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::SpliceAfter(const_iterator position,
                                          NuoForwardList&& nflist,
                                          const_iterator first,
                                          const_iterator last)
{
  SpliceAfter(position, nflist, first, last);
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::SpliceAfter(const_iterator position,
                                          NuoForwardList& nflist,
                                          const_iterator first,
                                          const_iterator last)
{
  if (this == &nflist)
    return;

  if (first.node_ == nullptr || first.node_->next_ == nullptr)
    return;
  if (first.node_->next_ == last.node_)
    return;

  base_ptr first_moved = first.node_->next_;
  base_ptr last_prev = first_moved;

  /* count the moved nodes (range is [first_moved, last_prev]) */
  size_type count = 0;
  while (last_prev->next_ != last.node_ && last_prev->next_ != nullptr)
  {
    count++;
    last_prev = last_prev->next_;
  }
  count++; /* include last_prev itself */

  /* splice out of nflist (last_prev->next_ may be nullptr if truncated) */
  first.node_->next_ = last_prev->next_;

  /* insert after position */
  base_ptr pos_node = PositionNode(position);
  last_prev->next_ = pos_node->next_;
  pos_node->next_ = first_moved;

  size_ += count;
  nflist.size_ -= count;
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::size_type
NuoForwardList<T, Allocator>::Remove(const value_type& value)
{
  return RemoveIf([&](const value_type& v){ return v == value; });
}

template <typename T, class Allocator>
template <typename Predicate>
typename NuoForwardList<T, Allocator>::size_type
NuoForwardList<T, Allocator>::RemoveIf(Predicate pred)
{
  size_type removed = 0;
  base_ptr cur = head_;
  while (cur->next_ != nullptr)
  {
    if (pred(cur->next_->ToNode()->value_))
    {
      node_ptr target = cur->next_->ToNode();
      cur->next_ = target->next_;
      DestroyNode(target);
      size_--;
      removed++;
    }
    else
    {
      cur = cur->next_;
    }
  }
  return removed;
}

template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::size_type
NuoForwardList<T, Allocator>::Unique()
{
  return Unique(NuoEqualTo<T>());
}

template <typename T, class Allocator>
template <typename BinaryPredicate>
typename NuoForwardList<T, Allocator>::size_type
NuoForwardList<T, Allocator>::Unique(BinaryPredicate binary_pred)
{
  size_type removed = 0;
  if (size_ == 0)
    return 0;

  base_ptr cur = head_->next_;
  while (cur->next_ != nullptr)
  {
    if (binary_pred(cur->ToNode()->value_, cur->next_->ToNode()->value_))
    {
      node_ptr target = cur->next_->ToNode();
      cur->next_ = target->next_;
      DestroyNode(target);
      size_--;
      removed++;
    }
    else
    {
      cur = cur->next_;
    }
  }
  return removed;
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::Merge(NuoForwardList& nflist)
{
  Merge(nflist, NuoLess<T>());
}

template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::Merge(NuoForwardList&& nflist)
{
  Merge(nflist, NuoLess<T>());
}

template <typename T, class Allocator>
template <typename Compare>
void
NuoForwardList<T, Allocator>::Merge(NuoForwardList& nflist, Compare comp)
{
  if (this == &nflist)
    return;

  if (nflist.Empty())
    return;

  base_ptr cur = head_;
  while (nflist.head_->next_ != nullptr && cur->next_ != nullptr)
  {
    if (comp(nflist.head_->next_->ToNode()->value_, cur->next_->ToNode()->value_))
    {
      node_ptr moved = nflist.head_->next_->ToNode();
      nflist.head_->next_ = moved->next_;
      moved->next_ = cur->next_;
      cur->next_ = moved->ToBase();
      nflist.size_--;
      size_++;
    }
    cur = cur->next_;
  }
  if (nflist.head_->next_ != nullptr)
  {
    cur->next_ = nflist.head_->next_;
    size_ += nflist.size_;
    nflist.size_ = 0;
    nflist.head_->next_ = nullptr;
  }
}

template <typename T, class Allocator>
template <typename Compare>
void
NuoForwardList<T, Allocator>::Merge(NuoForwardList&& nflist, Compare comp)
{
  Merge(nflist, comp);
}

template <typename T, class Allocator>
void NuoForwardList<T, Allocator>::Sort()
{
  Sort(NuoLess<T>());
}

template <typename T, class Allocator>
template <typename Compare>
void NuoForwardList<T, Allocator>::Sort(Compare comp)
{
  /* insertion sort on a singly linked list */
  if (size_ <= 1)
    return;

  base_ptr sorted = head_->next_;
  base_ptr cur = sorted->next_;
  sorted->next_ = nullptr;

  while (cur != nullptr)
  {
    base_ptr next = cur->next_;

    base_ptr pos = head_;
    while (pos->next_ != nullptr &&
           comp(pos->next_->ToNode()->value_, cur->ToNode()->value_))
      pos = pos->next_;

    cur->next_ = pos->next_;
    pos->next_ = cur;
    cur = next;
  }
}

template <typename T, class Allocator>
void NuoForwardList<T, Allocator>::Reverse() noexcept
{
  if (size_ <= 1)
    return;

  base_ptr prev = nullptr;
  base_ptr cur = head_->next_;
  while (cur != nullptr)
  {
    base_ptr next = cur->next_;
    cur->next_ = prev;
    prev = cur;
    cur = next;
  }
  head_->next_ = prev;
}

/* helper functions */

/* Create a node with the given value */
template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::node_ptr
NuoForwardList<T, Allocator>::CreateNode(const value_type& value)
{
  node_ptr node = node_allocator::Allocate();
  node_allocator::Construct(node, value);
  return node;
}

/* Create node (forward args) */
template <typename T, class Allocator>
template <typename... Args>
typename NuoForwardList<T, Allocator>::node_ptr
NuoForwardList<T, Allocator>::CreateNode(Args&& ...args)
{
  node_ptr node = node_allocator::Allocate();
  node_allocator::Construct(node, NuoForward<Args>(args)...);
  return node;
}

/* Destroy a node */
template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::DestroyNode(node_ptr node)
{
  data_allocator::Destroy(&node->value_);
  node_allocator::Destroy(node);
}

/* Add node to the front of the list */
template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::AddNodeToFront(base_ptr node)
{
  node->next_ = head_->next_;
  head_->next_ = node;
  size_++;
}

/* Add node after the given position */
template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::AddNodeAfter(base_ptr pos_node, base_ptr node)
{
  node->next_ = pos_node->next_;
  pos_node->next_ = node;
  size_++;
}

/* Resolve a position iterator to a concrete node: End() (nullptr)
   maps to the tail node so that insert/splice-at-end works */
template <typename T, class Allocator>
typename NuoForwardList<T, Allocator>::base_ptr
NuoForwardList<T, Allocator>::PositionNode(const_iterator position)
{
  if (position.node_ != nullptr)
    return position.node_;

  base_ptr cur = head_;
  while (cur->next_ != nullptr)
    cur = cur->next_;
  return cur;
}

/* FillInit: initialize with n copies of value */
template <typename T, class Allocator>
void
NuoForwardList<T, Allocator>::FillInit(size_type n, const value_type& value)
{
  head_ = base_allocator::Allocate();
  head_->next_ = nullptr;
  size_ = 0;

  try
  {
    while (n--)
    {
      node_ptr node = CreateNode(value);
      AddNodeToFront(node->ToBase());
    }
  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(head_);
    head_ = nullptr, size_ = 0;
    throw;
  }
}

/* CopyInit from input iterator range */
template <typename T, class Allocator>
template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, bool>::type>
void
NuoForwardList<T, Allocator>::CopyInit(InputIter first, InputIter last)
{
  head_ = base_allocator::Allocate();
  head_->next_ = nullptr;
  size_ = 0;

  try
  {
    base_ptr cur = head_;
    for (InputIter it = first; it != last; it++)
    {
      node_ptr node = CreateNode(*it);
      AddNodeAfter(cur, node->ToBase());
      cur = node->ToBase();
    }
  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(head_);
    head_ = nullptr, size_ = 0;
    throw;
  }
}

/* CopyAssign from input iterator range */
template <typename T, class Allocator>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, bool>::type>
void
NuoForwardList<T, Allocator>::CopyAssign(InputIter first, InputIter last)
{
  head_->next_ = nullptr;
  size_ = 0;

  try
  {
    base_ptr cur = head_;
    for (InputIter it = first; it != last; it++)
    {
      node_ptr node = CreateNode(*it);
      AddNodeAfter(cur, node->ToBase());
      cur = cur->next_;
    }
  }
  catch(...)
  {
    Clear();
    base_allocator::Deallocate(head_);
    head_ = nullptr, size_ = 0;
    throw;
  }
}

/* overload NuoSwap */
template <typename T, class Allocator>
void
NuoSwap(NuoForwardList<T, Allocator>& lhs,
        NuoForwardList<T, Allocator>& rhs) noexcept
{
  lhs.Swap(rhs);
}

} /* namespace nuostl */