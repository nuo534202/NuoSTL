#pragma once

#include <initializer_list>
#include <type_traits>

#include "core/assoc_cont/nuo_rb_tree.hpp"

namespace nuostl
{

template <typename Key, typename T, typename Compare = NuoLess<Key>>
class NuoMap;

template <typename Key, typename T, typename Compare = NuoLess<Key>>
class NuoMultimap;

/*****************************************************************************/
/* NuoMap class declaration */
template <typename Key, typename T, typename Compare>
class NuoMap
{
public:
  using key_type               = Key;
  using mapped_type            = T;
  using value_type             = nuo_pair<const Key, T>;
  using key_compare            = Compare;
  using base_type              = NuoRbTree<value_type, key_compare>;
  using allocator_type         = typename base_type::allocator_type;
  using pointer                = typename base_type::pointer;
  using const_pointer          = typename base_type::const_pointer;
  using reference              = typename base_type::reference;
  using const_reference        = typename base_type::const_reference;
  using size_type              = typename base_type::size_type;
  using difference_type        = typename base_type::difference_type;
  using iterator               = typename base_type::iterator;
  using const_iterator         = typename base_type::const_iterator;
  using reverse_iterator       = typename base_type::reverse_iterator;
  using const_reverse_iterator = typename base_type::const_reverse_iterator;
  using node_type              = typename base_type::node_type;

  class ValueCompare
  {
  public:
    explicit ValueCompare(key_compare comp);
    bool operator()(const value_type& lhs, const value_type& rhs) const;

  private:
    key_compare comp_;
  };

  using value_compare = ValueCompare;

public:
  NuoMap();
  explicit NuoMap(const key_compare& comp);

  template <typename InputIterator>
  NuoMap(InputIterator first, InputIterator last);

  template <typename InputIterator>
  NuoMap(InputIterator first, InputIterator last, const key_compare& comp);

  NuoMap(std::initializer_list<value_type> ilist);
  NuoMap(std::initializer_list<value_type> ilist, const key_compare& comp);
  NuoMap(const NuoMap& rhs);
  NuoMap(NuoMap&& rhs) noexcept;
  ~NuoMap();

  NuoMap& operator=(const NuoMap& rhs);
  NuoMap& operator=(NuoMap&& rhs) noexcept;
  NuoMap& operator=(std::initializer_list<value_type> ilist);

  allocator_type GetAllocator() const;

  iterator Begin() noexcept;
  const_iterator Begin() const noexcept;
  iterator End() noexcept;
  const_iterator End() const noexcept;
  reverse_iterator RBegin() noexcept;
  const_reverse_iterator RBegin() const noexcept;
  reverse_iterator REnd() noexcept;
  const_reverse_iterator REnd() const noexcept;
  const_iterator CBegin() const noexcept;
  const_iterator CEnd() const noexcept;
  const_reverse_iterator CRBegin() const noexcept;
  const_reverse_iterator CREnd() const noexcept;

  bool Empty() const noexcept;
  size_type Size() const noexcept;
  size_type MaxSize() const noexcept;

  mapped_type& operator[](const key_type& key);
  mapped_type& operator[](key_type&& key);
  mapped_type& At(const key_type& key);
  const mapped_type& At(const key_type& key) const;

  template <typename... Args>
  nuo_pair<iterator, bool> Emplace(Args&&... args);

  template <typename... Args>
  iterator EmplaceHint(const_iterator hint, Args&&... args);

  nuo_pair<iterator, bool> Insert(const value_type& value);
  nuo_pair<iterator, bool> Insert(value_type&& value);
  iterator Insert(const_iterator hint, const value_type& value);
  iterator Insert(const_iterator hint, value_type&& value);

  template <typename InputIterator>
  void Insert(InputIterator first, InputIterator last);

  void Insert(std::initializer_list<value_type> ilist);

  template <typename... Args>
  nuo_pair<iterator, bool> TryEmplace(const key_type& key, Args&&... args);

  template <typename... Args>
  nuo_pair<iterator, bool> TryEmplace(key_type&& key, Args&&... args);

  template <typename... Args>
  iterator TryEmplace(const_iterator hint, const key_type& key, Args&&... args);

  template <typename... Args>
  iterator TryEmplace(const_iterator hint, key_type&& key, Args&&... args);

  template <typename M>
  nuo_pair<iterator, bool> InsertOrAssign(const key_type& key, M&& value);

  template <typename M>
  nuo_pair<iterator, bool> InsertOrAssign(key_type&& key, M&& value);

  template <typename M>
  iterator InsertOrAssign(const_iterator hint, const key_type& key, M&& value);

  template <typename M>
  iterator InsertOrAssign(const_iterator hint, key_type&& key, M&& value);

  iterator Erase(const_iterator position);
  size_type Erase(const key_type& key);
  iterator Erase(const_iterator first, const_iterator last);
  void Swap(NuoMap& rhs) noexcept;
  void Clear() noexcept;

  template <typename C2>
  void Merge(NuoMap<Key, T, C2>& source);

  template <typename C2>
  void Merge(NuoMap<Key, T, C2>&& source);

  template <typename C2>
  void Merge(NuoMultimap<Key, T, C2>& source);

  template <typename C2>
  void Merge(NuoMultimap<Key, T, C2>&& source);

  key_compare KeyComp() const;
  value_compare ValueComp() const;

  iterator Find(const key_type& key);
  const_iterator Find(const key_type& key) const;
  size_type Count(const key_type& key) const;
  bool Contains(const key_type& key) const;
  iterator LowerBound(const key_type& key);
  const_iterator LowerBound(const key_type& key) const;
  iterator UpperBound(const key_type& key);
  const_iterator UpperBound(const key_type& key) const;
  nuo_pair<iterator, iterator> EqualRange(const key_type& key);
  nuo_pair<const_iterator, const_iterator> EqualRange(const key_type& key) const;

private:
  typename base_type::iterator ToTreeIter(const_iterator iterator) const noexcept;

private:
  base_type tree_;
};

/*****************************************************************************/
/* NuoMultimap class declaration */
template <typename Key, typename T, typename Compare>
class NuoMultimap
{
public:
  using key_type               = Key;
  using mapped_type            = T;
  using value_type             = nuo_pair<const Key, T>;
  using key_compare            = Compare;
  using base_type              = NuoRbTree<value_type, key_compare>;
  using allocator_type         = typename base_type::allocator_type;
  using pointer                = typename base_type::pointer;
  using const_pointer          = typename base_type::const_pointer;
  using reference              = typename base_type::reference;
  using const_reference        = typename base_type::const_reference;
  using size_type              = typename base_type::size_type;
  using difference_type        = typename base_type::difference_type;
  using iterator               = typename base_type::iterator;
  using const_iterator         = typename base_type::const_iterator;
  using reverse_iterator       = typename base_type::reverse_iterator;
  using const_reverse_iterator = typename base_type::const_reverse_iterator;
  using node_type              = typename base_type::node_type;

  class ValueCompare
  {
  public:
    explicit ValueCompare(key_compare comp);
    bool operator()(const value_type& lhs, const value_type& rhs) const;

  private:
    key_compare comp_;
  };

  using value_compare = ValueCompare;

public:
  NuoMultimap();
  explicit NuoMultimap(const key_compare& comp);

  template <typename InputIterator>
  NuoMultimap(InputIterator first, InputIterator last);

  template <typename InputIterator>
  NuoMultimap(InputIterator first, InputIterator last, const key_compare& comp);

  NuoMultimap(std::initializer_list<value_type> ilist);
  NuoMultimap(std::initializer_list<value_type> ilist, const key_compare& comp);
  NuoMultimap(const NuoMultimap& rhs);
  NuoMultimap(NuoMultimap&& rhs) noexcept;
  ~NuoMultimap();

  NuoMultimap& operator=(const NuoMultimap& rhs);
  NuoMultimap& operator=(NuoMultimap&& rhs) noexcept;
  NuoMultimap& operator=(std::initializer_list<value_type> ilist);

  allocator_type GetAllocator() const;

  iterator Begin() noexcept;
  const_iterator Begin() const noexcept;
  iterator End() noexcept;
  const_iterator End() const noexcept;
  reverse_iterator RBegin() noexcept;
  const_reverse_iterator RBegin() const noexcept;
  reverse_iterator REnd() noexcept;
  const_reverse_iterator REnd() const noexcept;
  const_iterator CBegin() const noexcept;
  const_iterator CEnd() const noexcept;
  const_reverse_iterator CRBegin() const noexcept;
  const_reverse_iterator CREnd() const noexcept;

  bool Empty() const noexcept;
  size_type Size() const noexcept;
  size_type MaxSize() const noexcept;

  template <typename... Args>
  iterator Emplace(Args&&... args);

  template <typename... Args>
  iterator EmplaceHint(const_iterator hint, Args&&... args);

  iterator Insert(const value_type& value);
  iterator Insert(value_type&& value);
  iterator Insert(const_iterator hint, const value_type& value);
  iterator Insert(const_iterator hint, value_type&& value);

  template <typename InputIterator>
  void Insert(InputIterator first, InputIterator last);

  void Insert(std::initializer_list<value_type> ilist);

  iterator Erase(const_iterator position);
  size_type Erase(const key_type& key);
  iterator Erase(const_iterator first, const_iterator last);
  void Swap(NuoMultimap& rhs) noexcept;
  void Clear() noexcept;

  template <typename C2>
  void Merge(NuoMultimap<Key, T, C2>& source);

  template <typename C2>
  void Merge(NuoMultimap<Key, T, C2>&& source);

  template <typename C2>
  void Merge(NuoMap<Key, T, C2>& source);

  template <typename C2>
  void Merge(NuoMap<Key, T, C2>&& source);

  key_compare KeyComp() const;
  value_compare ValueComp() const;

  iterator Find(const key_type& key);
  const_iterator Find(const key_type& key) const;
  size_type Count(const key_type& key) const;
  bool Contains(const key_type& key) const;
  iterator LowerBound(const key_type& key);
  const_iterator LowerBound(const key_type& key) const;
  iterator UpperBound(const key_type& key);
  const_iterator UpperBound(const key_type& key) const;
  nuo_pair<iterator, iterator> EqualRange(const key_type& key);
  nuo_pair<const_iterator, const_iterator> EqualRange(const key_type& key) const;

private:
  typename base_type::iterator ToTreeIter(const_iterator iterator) const noexcept;

private:
  base_type tree_;
};

/*****************************************************************************/
/* NuoMap class implementation */

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::ValueCompare::ValueCompare(key_compare comp)
  : comp_(comp)
{
}

template <typename Key, typename T, typename Compare>
bool NuoMap<Key, T, Compare>::ValueCompare::operator()(
  const value_type& lhs, const value_type& rhs) const
{
  return comp_(lhs.first, rhs.first);
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::NuoMap()
  : tree_()
{
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::NuoMap(const key_compare& comp)
  : tree_(comp)
{
}

template <typename Key, typename T, typename Compare>
template <typename InputIterator>
NuoMap<Key, T, Compare>::NuoMap(InputIterator first, InputIterator last)
  : tree_()
{
  tree_.InsertUnique(first, last);
}

template <typename Key, typename T, typename Compare>
template <typename InputIterator>
NuoMap<Key, T, Compare>::NuoMap(InputIterator first, InputIterator last,
                                const key_compare& comp)
  : tree_(comp)
{
  tree_.InsertUnique(first, last);
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::NuoMap(std::initializer_list<value_type> ilist)
  : tree_()
{
  tree_.InsertUnique(ilist.begin(), ilist.end());
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::NuoMap(std::initializer_list<value_type> ilist,
                                const key_compare& comp)
  : tree_(comp)
{
  tree_.InsertUnique(ilist.begin(), ilist.end());
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::NuoMap(const NuoMap& rhs)
  : tree_(rhs.tree_)
{
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::NuoMap(NuoMap&& rhs) noexcept
  : tree_(NuoMove(rhs.tree_))
{
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>::~NuoMap() = default;

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>& NuoMap<Key, T, Compare>::operator=(const NuoMap& rhs)
{
  tree_ = rhs.tree_;
  return *this;
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>& NuoMap<Key, T, Compare>::operator=(NuoMap&& rhs) noexcept
{
  tree_ = NuoMove(rhs.tree_);
  return *this;
}

template <typename Key, typename T, typename Compare>
NuoMap<Key, T, Compare>& NuoMap<Key, T, Compare>::operator=(
  std::initializer_list<value_type> ilist)
{
  Clear();
  Insert(ilist);
  return *this;
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::allocator_type
NuoMap<Key, T, Compare>::GetAllocator() const
{
  return tree_.GetAllocator();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::Begin() noexcept
{
  return tree_.Begin();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::Begin() const noexcept
{
  return tree_.Begin();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::End() noexcept
{
  return tree_.End();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::End() const noexcept
{
  return tree_.End();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::reverse_iterator
NuoMap<Key, T, Compare>::RBegin() noexcept
{
  return tree_.RBegin();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_reverse_iterator
NuoMap<Key, T, Compare>::RBegin() const noexcept
{
  return tree_.RBegin();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::reverse_iterator
NuoMap<Key, T, Compare>::REnd() noexcept
{
  return tree_.REnd();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_reverse_iterator
NuoMap<Key, T, Compare>::REnd() const noexcept
{
  return tree_.REnd();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::CBegin() const noexcept
{
  return Begin();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::CEnd() const noexcept
{
  return End();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_reverse_iterator
NuoMap<Key, T, Compare>::CRBegin() const noexcept
{
  return RBegin();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_reverse_iterator
NuoMap<Key, T, Compare>::CREnd() const noexcept
{
  return REnd();
}

template <typename Key, typename T, typename Compare>
bool NuoMap<Key, T, Compare>::Empty() const noexcept
{
  return tree_.Empty();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::size_type NuoMap<Key, T, Compare>::Size() const noexcept
{
  return tree_.Size();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::size_type
NuoMap<Key, T, Compare>::MaxSize() const noexcept
{
  return tree_.MaxSize();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::mapped_type&
NuoMap<Key, T, Compare>::operator[](const key_type& key)
{
  return TryEmplace(key).first->second;
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::mapped_type&
NuoMap<Key, T, Compare>::operator[](key_type&& key)
{
  return TryEmplace(NuoMove(key)).first->second;
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::mapped_type&
NuoMap<Key, T, Compare>::At(const key_type& key)
{
  iterator iterator = Find(key);
  NUO_THROW_OUT_OF_RANGE_IF(iterator == End(), "NuoMap::At key not found");
  return iterator->second;
}

template <typename Key, typename T, typename Compare>
const typename NuoMap<Key, T, Compare>::mapped_type&
NuoMap<Key, T, Compare>::At(const key_type& key) const
{
  const_iterator iterator = Find(key);
  NUO_THROW_OUT_OF_RANGE_IF(iterator == End(), "NuoMap::At key not found");
  return iterator->second;
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::Emplace(Args&&... args)
{
  return tree_.EmplaceUnique(NuoForward<Args>(args)...);
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::EmplaceHint(const_iterator hint, Args&&... args)
{
  return tree_.EmplaceUniqueUseHint(ToTreeIter(hint), NuoForward<Args>(args)...);
}

template <typename Key, typename T, typename Compare>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::Insert(const value_type& value)
{
  return tree_.InsertUnique(value);
}

template <typename Key, typename T, typename Compare>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::Insert(value_type&& value)
{
  return tree_.InsertUnique(NuoMove(value));
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::Insert(const_iterator hint, const value_type& value)
{
  return tree_.InsertUnique(ToTreeIter(hint), value);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::Insert(const_iterator hint, value_type&& value)
{
  return tree_.InsertUnique(ToTreeIter(hint), NuoMove(value));
}

template <typename Key, typename T, typename Compare>
template <typename InputIterator>
void NuoMap<Key, T, Compare>::Insert(InputIterator first, InputIterator last)
{
  tree_.InsertUnique(first, last);
}

template <typename Key, typename T, typename Compare>
void NuoMap<Key, T, Compare>::Insert(std::initializer_list<value_type> ilist)
{
  Insert(ilist.begin(), ilist.end());
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::TryEmplace(const key_type& key, Args&&... args)
{
  iterator existing = Find(key);
  if (existing != End())
    return nuo_make_pair(existing, false);
  mapped_type mapped(NuoForward<Args>(args)...);
  return Insert(value_type(key, NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::TryEmplace(key_type&& key, Args&&... args)
{
  iterator existing = Find(key);
  if (existing != End())
    return nuo_make_pair(existing, false);
  mapped_type mapped(NuoForward<Args>(args)...);
  return Insert(value_type(NuoMove(key), NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::TryEmplace(
  const_iterator hint, const key_type& key, Args&&... args)
{
  iterator existing = Find(key);
  if (existing != End())
    return existing;
  mapped_type mapped(NuoForward<Args>(args)...);
  return Insert(hint, value_type(key, NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::TryEmplace(
  const_iterator hint, key_type&& key, Args&&... args)
{
  iterator existing = Find(key);
  if (existing != End())
    return existing;
  mapped_type mapped(NuoForward<Args>(args)...);
  return Insert(hint, value_type(NuoMove(key), NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename M>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::InsertOrAssign(const key_type& key, M&& value)
{
  iterator existing = Find(key);
  if (existing != End())
  {
    existing->second = NuoForward<M>(value);
    return nuo_make_pair(existing, false);
  }
  mapped_type mapped(NuoForward<M>(value));
  return Insert(value_type(key, NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename M>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator, bool>
NuoMap<Key, T, Compare>::InsertOrAssign(key_type&& key, M&& value)
{
  iterator existing = Find(key);
  if (existing != End())
  {
    existing->second = NuoForward<M>(value);
    return nuo_make_pair(existing, false);
  }
  mapped_type mapped(NuoForward<M>(value));
  return Insert(value_type(NuoMove(key), NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename M>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::InsertOrAssign(
  const_iterator hint, const key_type& key, M&& value)
{
  iterator existing = Find(key);
  if (existing != End())
  {
    existing->second = NuoForward<M>(value);
    return existing;
  }
  mapped_type mapped(NuoForward<M>(value));
  return Insert(hint, value_type(key, NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
template <typename M>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::InsertOrAssign(
  const_iterator hint, key_type&& key, M&& value)
{
  iterator existing = Find(key);
  if (existing != End())
  {
    existing->second = NuoForward<M>(value);
    return existing;
  }
  mapped_type mapped(NuoForward<M>(value));
  return Insert(hint, value_type(NuoMove(key), NuoMove(mapped)));
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::Erase(const_iterator position)
{
  return tree_.Erase(ToTreeIter(position));
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::size_type
NuoMap<Key, T, Compare>::Erase(const key_type& key)
{
  return tree_.EraseUnique(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator NuoMap<Key, T, Compare>::Erase(
  const_iterator first, const_iterator last)
{
  iterator result = ToTreeIter(last);
  tree_.Erase(ToTreeIter(first), result);
  return result;
}

template <typename Key, typename T, typename Compare>
void NuoMap<Key, T, Compare>::Swap(NuoMap& rhs) noexcept
{
  tree_.Swap(rhs.tree_);
}

template <typename Key, typename T, typename Compare>
void NuoMap<Key, T, Compare>::Clear() noexcept
{
  tree_.Clear();
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMap<Key, T, Compare>::Merge(NuoMap<Key, T, C2>& source)
{
  if constexpr (std::is_same_v<C2, Compare>)
  {
    if (this == &source)
      return;
  }
  auto current = source.Begin();
  while (current != source.End())
  {
    auto candidate = current++;
    if (Insert(*candidate).second)
      source.Erase(candidate);
  }
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMap<Key, T, Compare>::Merge(NuoMap<Key, T, C2>&& source)
{
  Merge(source);
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMap<Key, T, Compare>::Merge(NuoMultimap<Key, T, C2>& source)
{
  auto current = source.Begin();
  while (current != source.End())
  {
    auto candidate = current++;
    if (Insert(*candidate).second)
      source.Erase(candidate);
  }
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMap<Key, T, Compare>::Merge(NuoMultimap<Key, T, C2>&& source)
{
  Merge(source);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::key_compare NuoMap<Key, T, Compare>::KeyComp() const
{
  return tree_.KeyComp();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::value_compare
NuoMap<Key, T, Compare>::ValueComp() const
{
  return value_compare(KeyComp());
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::Find(const key_type& key)
{
  return tree_.Find(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::Find(const key_type& key) const
{
  return tree_.Find(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::size_type
NuoMap<Key, T, Compare>::Count(const key_type& key) const
{
  return tree_.CountUnique(key);
}

template <typename Key, typename T, typename Compare>
bool NuoMap<Key, T, Compare>::Contains(const key_type& key) const
{
  return Find(key) != End();
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::LowerBound(const key_type& key)
{
  return tree_.LowerBound(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::LowerBound(const key_type& key) const
{
  return tree_.LowerBound(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::iterator
NuoMap<Key, T, Compare>::UpperBound(const key_type& key)
{
  return tree_.UpperBound(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::const_iterator
NuoMap<Key, T, Compare>::UpperBound(const key_type& key) const
{
  return tree_.UpperBound(key);
}

template <typename Key, typename T, typename Compare>
nuo_pair<typename NuoMap<Key, T, Compare>::iterator,
         typename NuoMap<Key, T, Compare>::iterator>
NuoMap<Key, T, Compare>::EqualRange(const key_type& key)
{
  return tree_.EqualRangeUnique(key);
}

template <typename Key, typename T, typename Compare>
nuo_pair<typename NuoMap<Key, T, Compare>::const_iterator,
         typename NuoMap<Key, T, Compare>::const_iterator>
NuoMap<Key, T, Compare>::EqualRange(const key_type& key) const
{
  return tree_.EqualRangeUnique(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMap<Key, T, Compare>::base_type::iterator
NuoMap<Key, T, Compare>::ToTreeIter(const_iterator iterator) const noexcept
{
  return typename base_type::iterator(iterator.node_);
}

/*****************************************************************************/
/* NuoMultimap class implementation */

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::ValueCompare::ValueCompare(key_compare comp)
  : comp_(comp)
{
}

template <typename Key, typename T, typename Compare>
bool NuoMultimap<Key, T, Compare>::ValueCompare::operator()(
  const value_type& lhs, const value_type& rhs) const
{
  return comp_(lhs.first, rhs.first);
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::NuoMultimap()
  : tree_()
{
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::NuoMultimap(const key_compare& comp)
  : tree_(comp)
{
}

template <typename Key, typename T, typename Compare>
template <typename InputIterator>
NuoMultimap<Key, T, Compare>::NuoMultimap(InputIterator first, InputIterator last)
  : tree_()
{
  tree_.InsertMulti(first, last);
}

template <typename Key, typename T, typename Compare>
template <typename InputIterator>
NuoMultimap<Key, T, Compare>::NuoMultimap(InputIterator first, InputIterator last,
                                          const key_compare& comp)
  : tree_(comp)
{
  tree_.InsertMulti(first, last);
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::NuoMultimap(std::initializer_list<value_type> ilist)
  : tree_()
{
  tree_.InsertMulti(ilist.begin(), ilist.end());
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::NuoMultimap(
  std::initializer_list<value_type> ilist, const key_compare& comp)
  : tree_(comp)
{
  tree_.InsertMulti(ilist.begin(), ilist.end());
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::NuoMultimap(const NuoMultimap& rhs)
  : tree_(rhs.tree_)
{
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::NuoMultimap(NuoMultimap&& rhs) noexcept
  : tree_(NuoMove(rhs.tree_))
{
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>::~NuoMultimap() = default;

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>& NuoMultimap<Key, T, Compare>::operator=(
  const NuoMultimap& rhs)
{
  tree_ = rhs.tree_;
  return *this;
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>& NuoMultimap<Key, T, Compare>::operator=(
  NuoMultimap&& rhs) noexcept
{
  tree_ = NuoMove(rhs.tree_);
  return *this;
}

template <typename Key, typename T, typename Compare>
NuoMultimap<Key, T, Compare>& NuoMultimap<Key, T, Compare>::operator=(
  std::initializer_list<value_type> ilist)
{
  Clear();
  Insert(ilist);
  return *this;
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::allocator_type
NuoMultimap<Key, T, Compare>::GetAllocator() const
{
  return tree_.GetAllocator();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Begin() noexcept
{
  return tree_.Begin();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::Begin() const noexcept
{
  return tree_.Begin();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::End() noexcept
{
  return tree_.End();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::End() const noexcept
{
  return tree_.End();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::reverse_iterator
NuoMultimap<Key, T, Compare>::RBegin() noexcept
{
  return tree_.RBegin();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_reverse_iterator
NuoMultimap<Key, T, Compare>::RBegin() const noexcept
{
  return tree_.RBegin();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::reverse_iterator
NuoMultimap<Key, T, Compare>::REnd() noexcept
{
  return tree_.REnd();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_reverse_iterator
NuoMultimap<Key, T, Compare>::REnd() const noexcept
{
  return tree_.REnd();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::CBegin() const noexcept
{
  return Begin();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::CEnd() const noexcept
{
  return End();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_reverse_iterator
NuoMultimap<Key, T, Compare>::CRBegin() const noexcept
{
  return RBegin();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_reverse_iterator
NuoMultimap<Key, T, Compare>::CREnd() const noexcept
{
  return REnd();
}

template <typename Key, typename T, typename Compare>
bool NuoMultimap<Key, T, Compare>::Empty() const noexcept
{
  return tree_.Empty();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::size_type
NuoMultimap<Key, T, Compare>::Size() const noexcept
{
  return tree_.Size();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::size_type
NuoMultimap<Key, T, Compare>::MaxSize() const noexcept
{
  return tree_.MaxSize();
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Emplace(Args&&... args)
{
  return tree_.EmplaceMulti(NuoForward<Args>(args)...);
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::EmplaceHint(const_iterator hint, Args&&... args)
{
  return tree_.EmplaceMultiUseHint(ToTreeIter(hint), NuoForward<Args>(args)...);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Insert(const value_type& value)
{
  return tree_.InsertMulti(value);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Insert(value_type&& value)
{
  return tree_.InsertMulti(NuoMove(value));
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Insert(const_iterator hint, const value_type& value)
{
  return tree_.InsertMulti(ToTreeIter(hint), value);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Insert(const_iterator hint, value_type&& value)
{
  return tree_.InsertMulti(ToTreeIter(hint), NuoMove(value));
}

template <typename Key, typename T, typename Compare>
template <typename InputIterator>
void NuoMultimap<Key, T, Compare>::Insert(InputIterator first, InputIterator last)
{
  tree_.InsertMulti(first, last);
}

template <typename Key, typename T, typename Compare>
void NuoMultimap<Key, T, Compare>::Insert(std::initializer_list<value_type> ilist)
{
  Insert(ilist.begin(), ilist.end());
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Erase(const_iterator position)
{
  return tree_.Erase(ToTreeIter(position));
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::size_type
NuoMultimap<Key, T, Compare>::Erase(const key_type& key)
{
  return tree_.EraseMulti(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Erase(const_iterator first, const_iterator last)
{
  iterator result = ToTreeIter(last);
  tree_.Erase(ToTreeIter(first), result);
  return result;
}

template <typename Key, typename T, typename Compare>
void NuoMultimap<Key, T, Compare>::Swap(NuoMultimap& rhs) noexcept
{
  tree_.Swap(rhs.tree_);
}

template <typename Key, typename T, typename Compare>
void NuoMultimap<Key, T, Compare>::Clear() noexcept
{
  tree_.Clear();
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMultimap<Key, T, Compare>::Merge(NuoMultimap<Key, T, C2>& source)
{
  if constexpr (std::is_same_v<C2, Compare>)
  {
    if (this == &source)
      return;
  }
  auto current = source.Begin();
  while (current != source.End())
  {
    auto candidate = current++;
    Insert(*candidate);
    source.Erase(candidate);
  }
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMultimap<Key, T, Compare>::Merge(NuoMultimap<Key, T, C2>&& source)
{
  Merge(source);
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMultimap<Key, T, Compare>::Merge(NuoMap<Key, T, C2>& source)
{
  auto current = source.Begin();
  while (current != source.End())
  {
    auto candidate = current++;
    Insert(*candidate);
    source.Erase(candidate);
  }
}

template <typename Key, typename T, typename Compare>
template <typename C2>
void NuoMultimap<Key, T, Compare>::Merge(NuoMap<Key, T, C2>&& source)
{
  Merge(source);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::key_compare
NuoMultimap<Key, T, Compare>::KeyComp() const
{
  return tree_.KeyComp();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::value_compare
NuoMultimap<Key, T, Compare>::ValueComp() const
{
  return value_compare(KeyComp());
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::Find(const key_type& key)
{
  return tree_.Find(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::Find(const key_type& key) const
{
  return tree_.Find(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::size_type
NuoMultimap<Key, T, Compare>::Count(const key_type& key) const
{
  return tree_.CountMulti(key);
}

template <typename Key, typename T, typename Compare>
bool NuoMultimap<Key, T, Compare>::Contains(const key_type& key) const
{
  return Find(key) != End();
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::LowerBound(const key_type& key)
{
  return tree_.LowerBound(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::LowerBound(const key_type& key) const
{
  return tree_.LowerBound(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::iterator
NuoMultimap<Key, T, Compare>::UpperBound(const key_type& key)
{
  return tree_.UpperBound(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::const_iterator
NuoMultimap<Key, T, Compare>::UpperBound(const key_type& key) const
{
  return tree_.UpperBound(key);
}

template <typename Key, typename T, typename Compare>
nuo_pair<typename NuoMultimap<Key, T, Compare>::iterator,
         typename NuoMultimap<Key, T, Compare>::iterator>
NuoMultimap<Key, T, Compare>::EqualRange(const key_type& key)
{
  return tree_.EqualRangeMulti(key);
}

template <typename Key, typename T, typename Compare>
nuo_pair<typename NuoMultimap<Key, T, Compare>::const_iterator,
         typename NuoMultimap<Key, T, Compare>::const_iterator>
NuoMultimap<Key, T, Compare>::EqualRange(const key_type& key) const
{
  return tree_.EqualRangeMulti(key);
}

template <typename Key, typename T, typename Compare>
typename NuoMultimap<Key, T, Compare>::base_type::iterator
NuoMultimap<Key, T, Compare>::ToTreeIter(const_iterator iterator) const noexcept
{
  return typename base_type::iterator(iterator.node_);
}

/*****************************************************************************/
/* NuoMap non-member operations */

template <typename Key, typename T, typename Compare>
bool operator==(const NuoMap<Key, T, Compare>& lhs,
                const NuoMap<Key, T, Compare>& rhs)
{
  return lhs.Size() == rhs.Size() && NuoEqual(lhs.Begin(), lhs.End(), rhs.Begin());
}

template <typename Key, typename T, typename Compare>
bool operator!=(const NuoMap<Key, T, Compare>& lhs,
                const NuoMap<Key, T, Compare>& rhs)
{
  return !(lhs == rhs);
}

template <typename Key, typename T, typename Compare>
bool operator<(const NuoMap<Key, T, Compare>& lhs,
               const NuoMap<Key, T, Compare>& rhs)
{
  return NuoLexicographicalCompare(lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template <typename Key, typename T, typename Compare>
bool operator>(const NuoMap<Key, T, Compare>& lhs,
               const NuoMap<Key, T, Compare>& rhs)
{
  return rhs < lhs;
}

template <typename Key, typename T, typename Compare>
bool operator<=(const NuoMap<Key, T, Compare>& lhs,
                const NuoMap<Key, T, Compare>& rhs)
{
  return !(rhs < lhs);
}

template <typename Key, typename T, typename Compare>
bool operator>=(const NuoMap<Key, T, Compare>& lhs,
                const NuoMap<Key, T, Compare>& rhs)
{
  return !(lhs < rhs);
}

template <typename Key, typename T, typename Compare>
void swap(NuoMap<Key, T, Compare>& lhs, NuoMap<Key, T, Compare>& rhs) noexcept
{
  lhs.Swap(rhs);
}

/*****************************************************************************/
/* NuoMultimap non-member operations */

template <typename Key, typename T, typename Compare>
bool operator==(const NuoMultimap<Key, T, Compare>& lhs,
                const NuoMultimap<Key, T, Compare>& rhs)
{
  return lhs.Size() == rhs.Size() && NuoEqual(lhs.Begin(), lhs.End(), rhs.Begin());
}

template <typename Key, typename T, typename Compare>
bool operator!=(const NuoMultimap<Key, T, Compare>& lhs,
                const NuoMultimap<Key, T, Compare>& rhs)
{
  return !(lhs == rhs);
}

template <typename Key, typename T, typename Compare>
bool operator<(const NuoMultimap<Key, T, Compare>& lhs,
               const NuoMultimap<Key, T, Compare>& rhs)
{
  return NuoLexicographicalCompare(lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template <typename Key, typename T, typename Compare>
bool operator>(const NuoMultimap<Key, T, Compare>& lhs,
               const NuoMultimap<Key, T, Compare>& rhs)
{
  return rhs < lhs;
}

template <typename Key, typename T, typename Compare>
bool operator<=(const NuoMultimap<Key, T, Compare>& lhs,
                const NuoMultimap<Key, T, Compare>& rhs)
{
  return !(rhs < lhs);
}

template <typename Key, typename T, typename Compare>
bool operator>=(const NuoMultimap<Key, T, Compare>& lhs,
                const NuoMultimap<Key, T, Compare>& rhs)
{
  return !(lhs < rhs);
}

template <typename Key, typename T, typename Compare>
void swap(NuoMultimap<Key, T, Compare>& lhs,
          NuoMultimap<Key, T, Compare>& rhs) noexcept
{
  lhs.Swap(rhs);
}

} /* namespace nuostl */
