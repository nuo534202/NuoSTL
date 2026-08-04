#pragma once

/* This header contains two template classes: NuoSet and NuoMultiset.
 * NuoSet      : a set whose key is the value itself; elements are sorted and keys are unique.
 * NuoMultiset : a set whose key is the value itself; elements are sorted and keys may repeat.
 *
 * Notes:
 *
 * Exception guarantees:
 * nuostl::NuoSet<Key> / nuostl::NuoMultiset<Key> provide the basic exception guarantee,
 * and the strong exception safety guarantee for the following functions:
 *   * Emplace
 *   * EmplaceHint
 *   * Insert
 */

#include "core/assoc_cont/nuo_rb_tree.hpp"

namespace nuostl
{

/****************************************************/
/* Set */
/* Template class NuoSet, keys must be unique. */
/* Param 1 is the key type; Param 2 is the key comparison functor, defaulting to NuoLess. */
template <typename Key, typename Compare = NuoLess<Key>>
class NuoSet
{
public:
  /* Nested type definitions */
  using key_type       = Key;
  using value_type     = Key;
  using key_compare    = Compare;
  using value_compare  = Compare;
  using node_type      = typename NuoRbTree<value_type, key_compare>::node_type;
  using pointer        = typename NuoRbTree<value_type, key_compare>::const_pointer;
  using const_pointer  = typename NuoRbTree<value_type, key_compare>::const_pointer;
  using reference      = typename NuoRbTree<value_type, key_compare>::const_reference;
  using const_reference = typename NuoRbTree<value_type, key_compare>::const_reference;
  using iterator       = typename NuoRbTree<value_type, key_compare>::const_iterator;
  using const_iterator = typename NuoRbTree<value_type, key_compare>::const_iterator;
  using reverse_iterator = typename NuoRbTree<value_type, key_compare>::const_reverse_iterator;
  using const_reverse_iterator = typename NuoRbTree<value_type, key_compare>::const_reverse_iterator;
  using size_type      = typename NuoRbTree<value_type, key_compare>::size_type;
  using difference_type = typename NuoRbTree<value_type, key_compare>::difference_type;
  using allocator_type = typename NuoRbTree<value_type, key_compare>::allocator_type;

private:
  /* NuoRbTree is used as the underlying mechanism */
  using base_type = NuoRbTree<value_type, key_compare>;
  base_type tree_;

public:
  /* Construction, copy and move functions */
  NuoSet() = default;

  template <typename InputIterator>
  NuoSet(InputIterator first, InputIterator last)
    : tree_()
  {
    tree_.InsertUnique(first, last);
  }

  NuoSet(std::initializer_list<value_type> ilist)
    : tree_()
  {
    tree_.InsertUnique(ilist.begin(), ilist.end());
  }

  NuoSet(const NuoSet& rhs)
    : tree_(rhs.tree_)
  {
  }

  NuoSet(NuoSet&& rhs) noexcept
    : tree_(NuoMove(rhs.tree_))
  {
  }

  NuoSet& operator=(const NuoSet& rhs)
  {
    tree_ = rhs.tree_;
    return *this;
  }

  NuoSet& operator=(NuoSet&& rhs) noexcept
  {
    tree_ = NuoMove(rhs.tree_);
    return *this;
  }

  NuoSet& operator=(std::initializer_list<value_type> ilist)
  {
    tree_.Clear();
    tree_.InsertUnique(ilist.begin(), ilist.end());
    return *this;
  }

  /* Related interfaces */
  key_compare    KeyComp()      const { return tree_.KeyComp(); }
  value_compare  ValueComp()    const { return tree_.KeyComp(); }
  allocator_type GetAllocator() const { return tree_.GetAllocator(); }

  /* Iterator-related operations */
  iterator               Begin()        noexcept { return tree_.Begin(); }
  const_iterator         Begin()  const noexcept { return tree_.Begin(); }
  iterator               End()          noexcept { return tree_.End(); }
  const_iterator         End()    const noexcept { return tree_.End(); }

  reverse_iterator       RBegin()       noexcept { return reverse_iterator(End()); }
  const_reverse_iterator RBegin() const noexcept { return const_reverse_iterator(End()); }
  reverse_iterator       REnd()         noexcept { return reverse_iterator(Begin()); }
  const_reverse_iterator REnd()   const noexcept { return const_reverse_iterator(Begin()); }

  const_iterator         CBegin()  const noexcept { return Begin(); }
  const_iterator         CEnd()    const noexcept { return End(); }
  const_reverse_iterator CRBegin() const noexcept { return RBegin(); }
  const_reverse_iterator CREnd()   const noexcept { return REnd(); }

  /* Capacity-related operations */
  bool      Empty()    const noexcept { return tree_.Empty(); }
  size_type Size()     const noexcept { return tree_.Size(); }
  size_type MaxSize()  const noexcept { return tree_.MaxSize(); }

  /* Insertion and deletion operations */
  template <typename... Args>
  nuo_pair<iterator, bool> Emplace(Args&&... args)
  {
    auto res = tree_.EmplaceUnique(NuoForward<Args>(args)...);
    return nuo_pair<iterator, bool>(res.first, res.second);
  }

  template <typename... Args>
  iterator EmplaceHint(iterator hint, Args&&... args)
  {
    return tree_.EmplaceUniqueUseHint(ToTreeIter(hint), NuoForward<Args>(args)...);
  }

  nuo_pair<iterator, bool> Insert(const value_type& value)
  {
    auto res = tree_.InsertUnique(value);
    return nuo_pair<iterator, bool>(res.first, res.second);
  }

  nuo_pair<iterator, bool> Insert(value_type&& value)
  {
    auto res = tree_.InsertUnique(NuoMove(value));
    return nuo_pair<iterator, bool>(res.first, res.second);
  }

  iterator Insert(iterator hint, const value_type& value)
  {
    return tree_.InsertUnique(ToTreeIter(hint), value);
  }

  iterator Insert(iterator hint, value_type&& value)
  {
    return tree_.InsertUnique(ToTreeIter(hint), NuoMove(value));
  }

  template <typename InputIterator>
  void Insert(InputIterator first, InputIterator last)
  {
    tree_.InsertUnique(first, last);
  }

  iterator  Erase(iterator position) { return tree_.Erase(ToTreeIter(position)); }
  size_type Erase(const key_type& key) { return tree_.EraseUnique(key); }
  void      Erase(iterator first, iterator last)
  {
    tree_.Erase(ToTreeIter(first), ToTreeIter(last));
  }

  void Clear() { tree_.Clear(); }

  /* NuoSet-related operations */
  iterator       Find(const key_type& key)              { return tree_.Find(key); }
  const_iterator Find(const key_type& key)        const { return tree_.Find(key); }

  size_type Count(const key_type& key) const { return tree_.CountUnique(key); }

  iterator       LowerBound(const key_type& key)       { return tree_.LowerBound(key); }
  const_iterator LowerBound(const key_type& key) const { return tree_.LowerBound(key); }

  iterator       UpperBound(const key_type& key)       { return tree_.UpperBound(key); }
  const_iterator UpperBound(const key_type& key) const { return tree_.UpperBound(key); }

  nuo_pair<iterator, iterator> EqualRange(const key_type& key)
  {
    auto p = tree_.EqualRangeUnique(key);
    return nuo_pair<iterator, iterator>(p.first, p.second);
  }

  nuo_pair<const_iterator, const_iterator>
  EqualRange(const key_type& key) const
  {
    auto p = tree_.EqualRangeUnique(key);
    return nuo_pair<const_iterator, const_iterator>(p.first, p.second);
  }

  void Swap(NuoSet& rhs) noexcept
  {
    tree_.Swap(rhs.tree_);
  }

private:
  /* Convert the set's const_iterator to the underlying rb-tree iterator */
  typename base_type::iterator ToTreeIter(iterator it) const noexcept
  {
    return typename base_type::iterator(it.node_);
  }

public:
  friend bool operator==(const NuoSet& lhs, const NuoSet& rhs)
  {
    return lhs.tree_ == rhs.tree_;
  }
  friend bool operator<(const NuoSet& lhs, const NuoSet& rhs)
  {
    return lhs.tree_ < rhs.tree_;
  }
};

/* Overloaded comparison operators */
template <typename Key, typename Compare>
bool operator==(const NuoSet<Key, Compare>& lhs, const NuoSet<Key, Compare>& rhs)
{
  return lhs == rhs;
}

template <typename Key, typename Compare>
bool operator<(const NuoSet<Key, Compare>& lhs, const NuoSet<Key, Compare>& rhs)
{
  return lhs < rhs;
}

template <typename Key, typename Compare>
bool operator!=(const NuoSet<Key, Compare>& lhs, const NuoSet<Key, Compare>& rhs)
{
  return !(lhs == rhs);
}

template <typename Key, typename Compare>
bool operator>(const NuoSet<Key, Compare>& lhs, const NuoSet<Key, Compare>& rhs)
{
  return rhs < lhs;
}

template <typename Key, typename Compare>
bool operator<=(const NuoSet<Key, Compare>& lhs, const NuoSet<Key, Compare>& rhs)
{
  return !(rhs < lhs);
}

template <typename Key, typename Compare>
bool operator>=(const NuoSet<Key, Compare>& lhs, const NuoSet<Key, Compare>& rhs)
{
  return !(lhs < rhs);
}

/* Overload nuostl swap */
template <typename Key, typename Compare>
void swap(NuoSet<Key, Compare>& lhs, NuoSet<Key, Compare>& rhs) noexcept
{
  lhs.Swap(rhs);
}

/*****************************************************************************************/

/****************************************************/
/* Multiset */
/* Template class NuoMultiset, duplicate keys are allowed. */
/* Param 1 is the key type; Param 2 is the key comparison functor, defaulting to NuoLess. */
template <typename Key, typename Compare = NuoLess<Key>>
class NuoMultiset
{
public:
  /* Nested type definitions */
  using key_type       = Key;
  using value_type     = Key;
  using key_compare    = Compare;
  using value_compare  = Compare;
  using node_type      = typename NuoRbTree<value_type, key_compare>::node_type;
  using pointer        = typename NuoRbTree<value_type, key_compare>::const_pointer;
  using const_pointer  = typename NuoRbTree<value_type, key_compare>::const_pointer;
  using reference      = typename NuoRbTree<value_type, key_compare>::const_reference;
  using const_reference = typename NuoRbTree<value_type, key_compare>::const_reference;
  using iterator       = typename NuoRbTree<value_type, key_compare>::const_iterator;
  using const_iterator = typename NuoRbTree<value_type, key_compare>::const_iterator;
  using reverse_iterator = typename NuoRbTree<value_type, key_compare>::const_reverse_iterator;
  using const_reverse_iterator = typename NuoRbTree<value_type, key_compare>::const_reverse_iterator;
  using size_type      = typename NuoRbTree<value_type, key_compare>::size_type;
  using difference_type = typename NuoRbTree<value_type, key_compare>::difference_type;
  using allocator_type = typename NuoRbTree<value_type, key_compare>::allocator_type;

private:
  /* NuoRbTree is used as the underlying mechanism */
  using base_type = NuoRbTree<value_type, key_compare>;
  base_type tree_;

public:
  /* Construction, copy and move functions */
  NuoMultiset() = default;

  template <typename InputIterator>
  NuoMultiset(InputIterator first, InputIterator last)
    : tree_()
  {
    tree_.InsertMulti(first, last);
  }

  NuoMultiset(std::initializer_list<value_type> ilist)
    : tree_()
  {
    tree_.InsertMulti(ilist.begin(), ilist.end());
  }

  NuoMultiset(const NuoMultiset& rhs)
    : tree_(rhs.tree_)
  {
  }

  NuoMultiset(NuoMultiset&& rhs) noexcept
    : tree_(NuoMove(rhs.tree_))
  {
  }

  NuoMultiset& operator=(const NuoMultiset& rhs)
  {
    tree_ = rhs.tree_;
    return *this;
  }

  NuoMultiset& operator=(NuoMultiset&& rhs) noexcept
  {
    tree_ = NuoMove(rhs.tree_);
    return *this;
  }

  NuoMultiset& operator=(std::initializer_list<value_type> ilist)
  {
    tree_.Clear();
    tree_.InsertMulti(ilist.begin(), ilist.end());
    return *this;
  }

  /* Related interfaces */
  key_compare    KeyComp()      const { return tree_.KeyComp(); }
  value_compare  ValueComp()    const { return tree_.KeyComp(); }
  allocator_type GetAllocator() const { return tree_.GetAllocator(); }

  /* Iterator-related operations */
  iterator               Begin()        noexcept { return tree_.Begin(); }
  const_iterator         Begin()  const noexcept { return tree_.Begin(); }
  iterator               End()          noexcept { return tree_.End(); }
  const_iterator         End()    const noexcept { return tree_.End(); }

  reverse_iterator       RBegin()       noexcept { return reverse_iterator(End()); }
  const_reverse_iterator RBegin() const noexcept { return const_reverse_iterator(End()); }
  reverse_iterator       REnd()         noexcept { return reverse_iterator(Begin()); }
  const_reverse_iterator REnd()   const noexcept { return const_reverse_iterator(Begin()); }

  const_iterator         CBegin()  const noexcept { return Begin(); }
  const_iterator         CEnd()    const noexcept { return End(); }
  const_reverse_iterator CRBegin() const noexcept { return RBegin(); }
  const_reverse_iterator CREnd()   const noexcept { return REnd(); }

  /* Capacity-related operations */
  bool      Empty()    const noexcept { return tree_.Empty(); }
  size_type Size()     const noexcept { return tree_.Size(); }
  size_type MaxSize()  const noexcept { return tree_.MaxSize(); }

  /* Insertion and deletion operations */
  template <typename... Args>
  iterator Emplace(Args&&... args)
  {
    return tree_.EmplaceMulti(NuoForward<Args>(args)...);
  }

  template <typename... Args>
  iterator EmplaceHint(iterator hint, Args&&... args)
  {
    return tree_.EmplaceMultiUseHint(ToTreeIter(hint), NuoForward<Args>(args)...);
  }

  iterator Insert(const value_type& value)
  {
    return tree_.InsertMulti(value);
  }

  iterator Insert(value_type&& value)
  {
    return tree_.InsertMulti(NuoMove(value));
  }

  iterator Insert(iterator hint, const value_type& value)
  {
    return tree_.InsertMulti(ToTreeIter(hint), value);
  }

  iterator Insert(iterator hint, value_type&& value)
  {
    return tree_.InsertMulti(ToTreeIter(hint), NuoMove(value));
  }

  template <typename InputIterator>
  void Insert(InputIterator first, InputIterator last)
  {
    tree_.InsertMulti(first, last);
  }

  iterator  Erase(iterator position) { return tree_.Erase(ToTreeIter(position)); }
  size_type Erase(const key_type& key) { return tree_.EraseMulti(key); }
  void      Erase(iterator first, iterator last)
  {
    tree_.Erase(ToTreeIter(first), ToTreeIter(last));
  }

  void Clear() { tree_.Clear(); }

  /* NuoMultiset-related operations */
  iterator       Find(const key_type& key)              { return tree_.Find(key); }
  const_iterator Find(const key_type& key)        const { return tree_.Find(key); }

  size_type Count(const key_type& key) const { return tree_.CountMulti(key); }

  iterator       LowerBound(const key_type& key)       { return tree_.LowerBound(key); }
  const_iterator LowerBound(const key_type& key) const { return tree_.LowerBound(key); }

  iterator       UpperBound(const key_type& key)       { return tree_.UpperBound(key); }
  const_iterator UpperBound(const key_type& key) const { return tree_.UpperBound(key); }

  nuo_pair<iterator, iterator> EqualRange(const key_type& key)
  {
    auto p = tree_.EqualRangeMulti(key);
    return nuo_pair<iterator, iterator>(p.first, p.second);
  }

  nuo_pair<const_iterator, const_iterator>
  EqualRange(const key_type& key) const
  {
    auto p = tree_.EqualRangeMulti(key);
    return nuo_pair<const_iterator, const_iterator>(p.first, p.second);
  }

  void Swap(NuoMultiset& rhs) noexcept
  {
    tree_.Swap(rhs.tree_);
  }

private:
  /* Convert the multiset's const_iterator to the underlying rb-tree iterator */
  typename base_type::iterator ToTreeIter(iterator it) const noexcept
  {
    return typename base_type::iterator(it.node_);
  }

public:
  friend bool operator==(const NuoMultiset& lhs, const NuoMultiset& rhs)
  {
    return lhs.tree_ == rhs.tree_;
  }
  friend bool operator<(const NuoMultiset& lhs, const NuoMultiset& rhs)
  {
    return lhs.tree_ < rhs.tree_;
  }
};

/* Overloaded comparison operators */
template <typename Key, typename Compare>
bool operator==(const NuoMultiset<Key, Compare>& lhs, const NuoMultiset<Key, Compare>& rhs)
{
  return lhs == rhs;
}

template <typename Key, typename Compare>
bool operator<(const NuoMultiset<Key, Compare>& lhs, const NuoMultiset<Key, Compare>& rhs)
{
  return lhs < rhs;
}

template <typename Key, typename Compare>
bool operator!=(const NuoMultiset<Key, Compare>& lhs, const NuoMultiset<Key, Compare>& rhs)
{
  return !(lhs == rhs);
}

template <typename Key, typename Compare>
bool operator>(const NuoMultiset<Key, Compare>& lhs, const NuoMultiset<Key, Compare>& rhs)
{
  return rhs < lhs;
}

template <typename Key, typename Compare>
bool operator<=(const NuoMultiset<Key, Compare>& lhs, const NuoMultiset<Key, Compare>& rhs)
{
  return !(rhs < lhs);
}

template <typename Key, typename Compare>
bool operator>=(const NuoMultiset<Key, Compare>& lhs, const NuoMultiset<Key, Compare>& rhs)
{
  return !(lhs < rhs);
}

/* Overload nuostl swap */
template <typename Key, typename Compare>
void swap(NuoMultiset<Key, Compare>& lhs, NuoMultiset<Key, Compare>& rhs) noexcept
{
  lhs.Swap(rhs);
}

} /* namespace nuostl */
