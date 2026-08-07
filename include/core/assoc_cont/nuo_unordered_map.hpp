#pragma once

#include <initializer_list>
#include <utility>

#include "core/data_types/nuo_pair.hpp"
#include "utils/nuo_allocator.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_functional.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

/****************************************************/
/* Forward declarations */
template <typename T> class NuoUnorderedMapNode;
template <typename T> class NuoUnorderedMapLocalIterator;
template <typename T> class NuoUnorderedMapConstLocalIterator;
template <typename T> class NuoUnorderedMapIterator;
template <typename T> class NuoUnorderedMapConstIterator;
template <typename Key, typename T, typename Hash, typename KeyEqual>
class NuoUnorderedMap;
template <typename Key, typename T, typename Hash, typename KeyEqual>
class NuoUnorderedMultimap;

/****************************************************/
/* Unordered Map Node */
/* A node stores one value and a pointer to the next node in the same bucket. */
template <typename T>
class NuoUnorderedMapNode
{
public:
  using node_ptr = NuoUnorderedMapNode<T>*;

  node_ptr next_;
  T        value_;

  NuoUnorderedMapNode()
    : next_(nullptr), value_()
  {
  }

  template <typename... Args>
  explicit NuoUnorderedMapNode(Args&&... args)
    : next_(nullptr), value_(NuoForward<Args>(args)...)
  {
  }
};

/****************************************************/
/* Prime number helpers */
/* The bucket count is always a prime number to reduce hash collisions. */
inline bool NuoUnorderedMapIsPrime(size_t n)
{
  if (n < 2)
    return false;
  if (n == 2)
    return true;
  if (n % 2 == 0)
    return false;
  for (size_t i = 3; i * i <= n; i += 2)
  {
    if (n % i == 0)
      return false;
  }
  return true;
}

/* Return the smallest prime number that is not smaller than n. */
inline size_t NuoUnorderedMapNextPrime(size_t n)
{
  if (n <= 2)
    return 2;
  if (n % 2 == 0)
    ++n;
  while (!NuoUnorderedMapIsPrime(n))
    n += 2;
  return n;
}

/****************************************************/
/* Unordered Map Local Iterator */
/* A local iterator walks over the nodes of a single bucket. */
template <typename T>
class NuoUnorderedMapLocalIterator :
  public NuoIterator<NuoForwardIteratorTag, T>
{
public:
  using value_type = T;
  using pointer    = T*;
  using reference  = T&;
  using node_ptr   = NuoUnorderedMapNode<T>*;
  using self       = NuoUnorderedMapLocalIterator<T>;

  NuoUnorderedMapLocalIterator() = default;
  NuoUnorderedMapLocalIterator(node_ptr node)
    : node_(node)
  {
  }
  NuoUnorderedMapLocalIterator(const self& iterator)
    : node_(iterator.node_)
  {
  }

  reference operator*() const
  {
    return node_->value_;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self& operator++()
  {
    node_ = node_->next_;
    return *this;
  }

  self operator++(int)
  {
    self tmp = *this;
    node_ = node_->next_;
    return tmp;
  }

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ != rhs.node_;
  }

  node_ptr node_;
};

/****************************************************/
/* Unordered Map Const Local Iterator */
template <typename T>
class NuoUnorderedMapConstLocalIterator :
  public NuoIterator<NuoForwardIteratorTag, T>
{
public:
  using value_type = T;
  using pointer    = const T*;
  using reference  = const T&;
  using node_ptr   = NuoUnorderedMapNode<T>*;
  using self       = NuoUnorderedMapConstLocalIterator<T>;

  NuoUnorderedMapConstLocalIterator() = default;
  NuoUnorderedMapConstLocalIterator(node_ptr node)
    : node_(node)
  {
  }
  NuoUnorderedMapConstLocalIterator(const self& iterator)
    : node_(iterator.node_)
  {
  }

  reference operator*() const
  {
    return node_->value_;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self& operator++()
  {
    node_ = node_->next_;
    return *this;
  }

  self operator++(int)
  {
    self tmp = *this;
    node_ = node_->next_;
    return tmp;
  }

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ != rhs.node_;
  }

  node_ptr node_;
};

/****************************************************/
/* Unordered Map Iterator */
/* A global iterator walks over the whole container, skipping empty buckets. */
template <typename T>
class NuoUnorderedMapIterator :
  public NuoIterator<NuoForwardIteratorTag, T>
{
public:
  using value_type = T;
  using pointer    = T*;
  using reference  = T&;
  using node_ptr   = NuoUnorderedMapNode<T>*;
  using bucket_ptr = NuoUnorderedMapNode<T>**;
  using self       = NuoUnorderedMapIterator<T>;

  NuoUnorderedMapIterator() = default;
  NuoUnorderedMapIterator(node_ptr node, bucket_ptr buckets,
                          size_t bucket_count, size_t index)
    : node_(node), buckets_(buckets),
      bucket_count_(bucket_count), index_(index)
  {
  }
  NuoUnorderedMapIterator(const self& iterator)
    : node_(iterator.node_), buckets_(iterator.buckets_),
      bucket_count_(iterator.bucket_count_), index_(iterator.index_)
  {
  }

  reference operator*() const
  {
    return node_->value_;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self& operator++()
  {
    Inc();
    return *this;
  }

  self operator++(int)
  {
    self tmp = *this;
    Inc();
    return tmp;
  }

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ != rhs.node_;
  }

  node_ptr node_;
  bucket_ptr buckets_;
  size_t bucket_count_;
  size_t index_;

private:
  /* Move to the next element, skipping empty buckets. */
  void Inc()
  {
    node_ = node_->next_;
    while (node_ == nullptr && index_ + 1 < bucket_count_)
    {
      ++index_;
      node_ = buckets_[index_];
    }
  }
};

/****************************************************/
/* Unordered Map Const Iterator */
template <typename T>
class NuoUnorderedMapConstIterator :
  public NuoIterator<NuoForwardIteratorTag, T>
{
public:
  using value_type = T;
  using pointer    = const T*;
  using reference  = const T&;
  using node_ptr   = NuoUnorderedMapNode<T>*;
  using bucket_ptr = NuoUnorderedMapNode<T>**;
  using self       = NuoUnorderedMapConstIterator<T>;

  NuoUnorderedMapConstIterator() = default;
  NuoUnorderedMapConstIterator(node_ptr node, bucket_ptr buckets,
                               size_t bucket_count, size_t index)
    : node_(node), buckets_(buckets),
      bucket_count_(bucket_count), index_(index)
  {
  }
  NuoUnorderedMapConstIterator(const NuoUnorderedMapIterator<T>& iterator)
    : node_(iterator.node_), buckets_(iterator.buckets_),
      bucket_count_(iterator.bucket_count_), index_(iterator.index_)
  {
  }
  NuoUnorderedMapConstIterator(const self& iterator) = default;

  reference operator*() const
  {
    return node_->value_;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self& operator++()
  {
    Inc();
    return *this;
  }

  self operator++(int)
  {
    self tmp = *this;
    Inc();
    return tmp;
  }

  bool operator==(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ != rhs.node_;
  }

  node_ptr node_;
  bucket_ptr buckets_;
  size_t bucket_count_;
  size_t index_;

private:
  /* Move to the next element, skipping empty buckets. */
  void Inc()
  {
    node_ = node_->next_;
    while (node_ == nullptr && index_ + 1 < bucket_count_)
    {
      ++index_;
      node_ = buckets_[index_];
    }
  }
};

/****************************************************/
/* NuoUnorderedMap class declaration */
template <typename Key, typename T,
          typename Hash = NuoHash<Key>,
          typename KeyEqual = NuoEqualTo<Key>>
class NuoUnorderedMap
{
public:
  /* types */
  using key_type        = Key;
  using mapped_type     = T;
  using value_type      = nuo_pair<const Key, T>;
  using hasher          = Hash;
  using key_equal       = KeyEqual;
  using allocator_type  = NuoAllocator<value_type>;

  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;

  using node_type   = NuoUnorderedMapNode<value_type>;
  using node_ptr    = node_type*;
  using bucket_type = node_ptr;

  using iterator             = NuoUnorderedMapIterator<value_type>;
  using const_iterator       = NuoUnorderedMapConstIterator<value_type>;
  using local_iterator       = NuoUnorderedMapLocalIterator<value_type>;
  using const_local_iterator = NuoUnorderedMapConstLocalIterator<value_type>;
  using self = NuoUnorderedMap<Key, T, Hash, KeyEqual>;

  /* construction / destruction */
  NuoUnorderedMap();
  explicit NuoUnorderedMap(size_type bucket_count,
                           const hasher& hash = hasher(),
                           const key_equal& equal = key_equal());
  template <typename InputIterator>
  NuoUnorderedMap(InputIterator first, InputIterator last,
                  size_type bucket_count = kDefaultBucketCount,
                  const hasher& hash = hasher(),
                  const key_equal& equal = key_equal());
  NuoUnorderedMap(std::initializer_list<value_type> init_list,
                  size_type bucket_count = kDefaultBucketCount,
                  const hasher& hash = hasher(),
                  const key_equal& equal = key_equal());
  NuoUnorderedMap(const self& other);
  NuoUnorderedMap(self&& other) noexcept;
  NuoUnorderedMap& operator=(const self& other);
  NuoUnorderedMap& operator=(self&& other) noexcept;
  NuoUnorderedMap& operator=(std::initializer_list<value_type> init_list);
  ~NuoUnorderedMap();

  /* iterators */
  iterator Begin() noexcept;
  const_iterator Begin() const noexcept;
  iterator End() noexcept;
  const_iterator End() const noexcept;
  const_iterator CBegin() const noexcept;
  const_iterator CEnd() const noexcept;

  /* capacity */
  bool Empty() const noexcept;
  size_type Size() const noexcept;
  size_type MaxSize() const noexcept;

  /* element access */
  T& operator[](const key_type& key);
  T& operator[](key_type&& key);
  T& At(const key_type& key);
  const T& At(const key_type& key) const;

  /* modifiers */
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
  void Insert(std::initializer_list<value_type> init_list);
  template <typename... Args>
  nuo_pair<iterator, bool> TryEmplace(const key_type& key, Args&&... args);
  template <typename... Args>
  nuo_pair<iterator, bool> TryEmplace(key_type&& key, Args&&... args);
  template <typename M>
  nuo_pair<iterator, bool> InsertOrAssign(const key_type& key, M&& obj);
  template <typename M>
  nuo_pair<iterator, bool> InsertOrAssign(key_type&& key, M&& obj);
  size_type Erase(const key_type& key);
  iterator Erase(const_iterator pos);
  iterator Erase(const_iterator first, const_iterator last);
  void Clear() noexcept;
  void Swap(self& other) noexcept;
  template <typename H2, typename E2>
  void Merge(NuoUnorderedMap<Key, T, H2, E2>& source);
  template <typename H2, typename E2>
  void Merge(NuoUnorderedMultimap<Key, T, H2, E2>& source);

  /* lookup */
  iterator Find(const key_type& key);
  const_iterator Find(const key_type& key) const;
  size_type Count(const key_type& key) const;
  bool Contains(const key_type& key) const;
  nuo_pair<iterator, iterator> EqualRange(const key_type& key);
  nuo_pair<const_iterator, const_iterator>
    EqualRange(const key_type& key) const;

  /* bucket interface */
  local_iterator Begin(size_type n);
  const_local_iterator Begin(size_type n) const;
  local_iterator End(size_type n);
  const_local_iterator End(size_type n) const;
  const_local_iterator CBegin(size_type n) const;
  const_local_iterator CEnd(size_type n) const;
  size_type BucketCount() const noexcept;
  size_type MaxBucketCount() const noexcept;
  size_type BucketSize(size_type n) const;
  size_type Bucket(const key_type& key) const;

  /* hash policy */
  float LoadFactor() const noexcept;
  float MaxLoadFactor() const noexcept;
  void MaxLoadFactor(float z);
  void Rehash(size_type n);
  void Reserve(size_type n);

  /* observers */
  hasher HashFunction() const;
  key_equal KeyEq() const;
  allocator_type GetAllocator() const;

  /* non-member swap */
  friend void Swap(self& lhs, self& rhs) noexcept
  {
    lhs.Swap(rhs);
  }

private:
  /* helpers */
  size_type GetBucketIndex(const key_type& key) const;
  template <typename... Args>
  node_ptr CreateNode(Args&&... args);
  void DestroyNode(node_ptr node);
  void InsertNode(node_ptr node);
  void EnsureNonEmpty();
  void RehashInternal(size_type new_count);

  /* data members */
  bucket_type* buckets_;
  size_type    bucket_count_;
  size_type    size_;
  float        max_load_factor_;
  hasher       hasher_;
  key_equal    key_equal_;

  static constexpr size_type kDefaultBucketCount = 101;
};

/****************************************************/
/* NuoUnorderedMultimap class declaration */
template <typename Key, typename T,
          typename Hash = NuoHash<Key>,
          typename KeyEqual = NuoEqualTo<Key>>
class NuoUnorderedMultimap
{
public:
  /* types */
  using key_type        = Key;
  using mapped_type     = T;
  using value_type      = nuo_pair<const Key, T>;
  using hasher          = Hash;
  using key_equal       = KeyEqual;
  using allocator_type  = NuoAllocator<value_type>;

  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;

  using node_type   = NuoUnorderedMapNode<value_type>;
  using node_ptr    = node_type*;
  using bucket_type = node_ptr;

  using iterator             = NuoUnorderedMapIterator<value_type>;
  using const_iterator       = NuoUnorderedMapConstIterator<value_type>;
  using local_iterator       = NuoUnorderedMapLocalIterator<value_type>;
  using const_local_iterator = NuoUnorderedMapConstLocalIterator<value_type>;
  using self = NuoUnorderedMultimap<Key, T, Hash, KeyEqual>;

  /* construction / destruction */
  NuoUnorderedMultimap();
  explicit NuoUnorderedMultimap(size_type bucket_count,
                                const hasher& hash = hasher(),
                                const key_equal& equal = key_equal());
  template <typename InputIterator>
  NuoUnorderedMultimap(InputIterator first, InputIterator last,
                       size_type bucket_count = kDefaultBucketCount,
                       const hasher& hash = hasher(),
                       const key_equal& equal = key_equal());
  NuoUnorderedMultimap(std::initializer_list<value_type> init_list,
                       size_type bucket_count = kDefaultBucketCount,
                       const hasher& hash = hasher(),
                       const key_equal& equal = key_equal());
  NuoUnorderedMultimap(const self& other);
  NuoUnorderedMultimap(self&& other) noexcept;
  NuoUnorderedMultimap& operator=(const self& other);
  NuoUnorderedMultimap& operator=(self&& other) noexcept;
  NuoUnorderedMultimap& operator=(std::initializer_list<value_type> init_list);
  ~NuoUnorderedMultimap();

  /* iterators */
  iterator Begin() noexcept;
  const_iterator Begin() const noexcept;
  iterator End() noexcept;
  const_iterator End() const noexcept;
  const_iterator CBegin() const noexcept;
  const_iterator CEnd() const noexcept;

  /* capacity */
  bool Empty() const noexcept;
  size_type Size() const noexcept;
  size_type MaxSize() const noexcept;

  /* modifiers */
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
  void Insert(std::initializer_list<value_type> init_list);
  size_type Erase(const key_type& key);
  iterator Erase(const_iterator pos);
  iterator Erase(const_iterator first, const_iterator last);
  void Clear() noexcept;
  void Swap(self& other) noexcept;
  template <typename H2, typename E2>
  void Merge(NuoUnorderedMap<Key, T, H2, E2>& source);
  template <typename H2, typename E2>
  void Merge(NuoUnorderedMultimap<Key, T, H2, E2>& source);

  /* lookup */
  iterator Find(const key_type& key);
  const_iterator Find(const key_type& key) const;
  size_type Count(const key_type& key) const;
  bool Contains(const key_type& key) const;
  nuo_pair<iterator, iterator> EqualRange(const key_type& key);
  nuo_pair<const_iterator, const_iterator>
    EqualRange(const key_type& key) const;

  /* bucket interface */
  local_iterator Begin(size_type n);
  const_local_iterator Begin(size_type n) const;
  local_iterator End(size_type n);
  const_local_iterator End(size_type n) const;
  const_local_iterator CBegin(size_type n) const;
  const_local_iterator CEnd(size_type n) const;
  size_type BucketCount() const noexcept;
  size_type MaxBucketCount() const noexcept;
  size_type BucketSize(size_type n) const;
  size_type Bucket(const key_type& key) const;

  /* hash policy */
  float LoadFactor() const noexcept;
  float MaxLoadFactor() const noexcept;
  void MaxLoadFactor(float z);
  void Rehash(size_type n);
  void Reserve(size_type n);

  /* observers */
  hasher HashFunction() const;
  key_equal KeyEq() const;
  allocator_type GetAllocator() const;

  /* non-member swap */
  friend void Swap(self& lhs, self& rhs) noexcept
  {
    lhs.Swap(rhs);
  }

private:
  /* helpers */
  size_type GetBucketIndex(const key_type& key) const;
  template <typename... Args>
  node_ptr CreateNode(Args&&... args);
  void DestroyNode(node_ptr node);
  void MultimapInsertNode(node_ptr node);
  void EnsureNonEmpty();
  void RehashInternal(size_type new_count);

  /* data members */
  bucket_type* buckets_;
  size_type    bucket_count_;
  size_type    size_;
  float        max_load_factor_;
  hasher       hasher_;
  key_equal    key_equal_;

  static constexpr size_type kDefaultBucketCount = 101;
};

/****************************************************/
/* NuoUnorderedMap member definitions */

/* construction / destruction */
template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::NuoUnorderedMap()
  : NuoUnorderedMap(kDefaultBucketCount)
{
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::NuoUnorderedMap(
    size_type bucket_count,
    const hasher& hash,
    const key_equal& equal)
  : buckets_(nullptr),
    bucket_count_(NuoUnorderedMapNextPrime(bucket_count)),
    size_(0),
    max_load_factor_(1.0f),
    hasher_(hash),
    key_equal_(equal)
{
  buckets_ = NuoAllocator<bucket_type>::Allocate(bucket_count_);
  for (size_type i = 0; i < bucket_count_; ++i)
    buckets_[i] = nullptr;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename InputIterator>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::NuoUnorderedMap(
    InputIterator first,
    InputIterator last,
    size_type bucket_count,
    const hasher& hash,
    const key_equal& equal)
  : NuoUnorderedMap(bucket_count, hash, equal)
{
  while (first != last)
  {
    Emplace(*first);
    ++first;
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::NuoUnorderedMap(
    std::initializer_list<value_type> init_list,
    size_type bucket_count,
    const hasher& hash,
    const key_equal& equal)
  : NuoUnorderedMap(bucket_count, hash, equal)
{
  Insert(init_list.begin(), init_list.end());
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::NuoUnorderedMap(const self& other)
  : buckets_(nullptr),
    bucket_count_(other.bucket_count_),
    size_(0),
    max_load_factor_(other.max_load_factor_),
    hasher_(other.hasher_),
    key_equal_(other.key_equal_)
{
  if (bucket_count_ == 0)
    return;
  buckets_ = NuoAllocator<bucket_type>::Allocate(bucket_count_);
  for (size_type i = 0; i < bucket_count_; ++i)
    buckets_[i] = nullptr;
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    node_ptr cur = other.buckets_[i];
    node_ptr tail = nullptr;
    while (cur != nullptr)
    {
      node_ptr new_node = CreateNode(cur->value_);
      new_node->next_ = nullptr;
      if (tail == nullptr)
        buckets_[i] = new_node;
      else
        tail->next_ = new_node;
      tail = new_node;
      ++size_;
      cur = cur->next_;
    }
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::NuoUnorderedMap(self&& other) noexcept
  : buckets_(other.buckets_),
    bucket_count_(other.bucket_count_),
    size_(other.size_),
    max_load_factor_(other.max_load_factor_),
    hasher_(NuoMove(other.hasher_)),
    key_equal_(NuoMove(other.key_equal_))
{
  other.buckets_ = nullptr;
  other.bucket_count_ = 0;
  other.size_ = 0;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>&
NuoUnorderedMap<Key, T, Hash, KeyEqual>::operator=(const self& other)
{
  if (this != &other)
  {
    self tmp(other);
    Swap(tmp);
  }
  return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>&
NuoUnorderedMap<Key, T, Hash, KeyEqual>::operator=(self&& other) noexcept
{
  if (this != &other)
  {
    self tmp(NuoMove(other));
    Swap(tmp);
  }
  return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>&
NuoUnorderedMap<Key, T, Hash, KeyEqual>::operator=(
    std::initializer_list<value_type> init_list)
{
  self tmp(init_list);
  Swap(tmp);
  return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::~NuoUnorderedMap()
{
  Clear();
  NuoAllocator<bucket_type>::Deallocate(buckets_, bucket_count_);
}

/* iterators */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Begin() noexcept
{
  if (size_ == 0)
    return End();
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    if (buckets_[i] != nullptr)
      return iterator(buckets_[i], buckets_, bucket_count_, i);
  }
  return End();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Begin() const noexcept
{
  if (size_ == 0)
    return End();
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    if (buckets_[i] != nullptr)
      return const_iterator(buckets_[i], buckets_, bucket_count_, i);
  }
  return End();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::End() noexcept
{
  return iterator(nullptr, buckets_, bucket_count_, bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::End() const noexcept
{
  return const_iterator(nullptr, buckets_, bucket_count_, bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::CBegin() const noexcept
{
  return Begin();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::CEnd() const noexcept
{
  return End();
}

/* capacity */
template <typename Key, typename T, typename Hash, typename KeyEqual>
bool NuoUnorderedMap<Key, T, Hash, KeyEqual>::Empty() const noexcept
{
  return size_ == 0;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Size() const noexcept
{
  return size_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::MaxSize() const noexcept
{
  return static_cast<size_type>(-1) / sizeof(node_type);
}

/* element access */
template <typename Key, typename T, typename Hash, typename KeyEqual>
T& NuoUnorderedMap<Key, T, Hash, KeyEqual>::operator[](const key_type& key)
{
  return Emplace(key, T()).first->second;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
T& NuoUnorderedMap<Key, T, Hash, KeyEqual>::operator[](key_type&& key)
{
  return Emplace(NuoMove(key), T()).first->second;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
T& NuoUnorderedMap<Key, T, Hash, KeyEqual>::At(const key_type& key)
{
  iterator it = Find(key);
  NUO_THROW_OUT_OF_RANGE_IF(it == End(), "nuostl::NuoUnorderedMap::At");
  return it->second;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
const T& NuoUnorderedMap<Key, T, Hash, KeyEqual>::At(const key_type& key) const
{
  const_iterator it = Find(key);
  NUO_THROW_OUT_OF_RANGE_IF(it == End(), "nuostl::NuoUnorderedMap::At");
  return it->second;
}

/* modifiers */
template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Emplace(Args&&... args)
{
  EnsureNonEmpty();
  node_ptr node = CreateNode(NuoForward<Args>(args)...);
  size_type index = GetBucketIndex(node->value_.first);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, node->value_.first))
    cur = cur->next_;
  if (cur != nullptr)
  {
    DestroyNode(node);
    return nuo_pair<iterator, bool>(
        iterator(cur, buckets_, bucket_count_, index), false);
  }
  InsertNode(node);
  return nuo_pair<iterator, bool>(
      iterator(node, buckets_, bucket_count_,
               GetBucketIndex(node->value_.first)),
      true);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::EmplaceHint(
    const_iterator hint, Args&&... args)
{
  return Emplace(NuoForward<Args>(args)...).first;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Insert(const value_type& value)
{
  return Emplace(value);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Insert(value_type&& value)
{
  return Emplace(NuoMove(value));
}

/* The hint is ignored for unordered containers. */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Insert(
    const_iterator hint, const value_type& value)
{
  (void)hint;
  return Emplace(value).first;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Insert(
    const_iterator hint, value_type&& value)
{
  (void)hint;
  return Emplace(NuoMove(value)).first;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename InputIterator>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Insert(
    InputIterator first, InputIterator last)
{
  while (first != last)
  {
    Emplace(*first);
    ++first;
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Insert(
    std::initializer_list<value_type> init_list)
{
  Insert(init_list.begin(), init_list.end());
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::TryEmplace(
    const key_type& key, Args&&... args)
{
  EnsureNonEmpty();
  node_ptr node = CreateNode(key, NuoForward<Args>(args)...);
  size_type index = GetBucketIndex(node->value_.first);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, node->value_.first))
    cur = cur->next_;
  if (cur != nullptr)
  {
    DestroyNode(node);
    return nuo_pair<iterator, bool>(
        iterator(cur, buckets_, bucket_count_, index), false);
  }
  InsertNode(node);
  return nuo_pair<iterator, bool>(
      iterator(node, buckets_, bucket_count_,
               GetBucketIndex(node->value_.first)),
      true);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::TryEmplace(
    key_type&& key, Args&&... args)
{
  EnsureNonEmpty();
  node_ptr node =
      CreateNode(NuoForward<key_type>(key), NuoForward<Args>(args)...);
  size_type index = GetBucketIndex(node->value_.first);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, node->value_.first))
    cur = cur->next_;
  if (cur != nullptr)
  {
    DestroyNode(node);
    return nuo_pair<iterator, bool>(
        iterator(cur, buckets_, bucket_count_, index), false);
  }
  InsertNode(node);
  return nuo_pair<iterator, bool>(
      iterator(node, buckets_, bucket_count_,
               GetBucketIndex(node->value_.first)),
      true);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename M>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::InsertOrAssign(
    const key_type& key, M&& obj)
{
  iterator it = Find(key);
  if (it != End())
  {
    it->second = NuoForward<M>(obj);
    return nuo_pair<iterator, bool>(it, false);
  }
  return Emplace(key, NuoForward<M>(obj));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename M>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator, bool>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::InsertOrAssign(
    key_type&& key, M&& obj)
{
  iterator it = Find(key);
  if (it != End())
  {
    it->second = NuoForward<M>(obj);
    return nuo_pair<iterator, bool>(it, false);
  }
  return Emplace(NuoMove(key), NuoForward<M>(obj));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Erase(const key_type& key)
{
  if (size_ == 0)
    return 0;
  size_type index = GetBucketIndex(key);
  size_type count = 0;
  node_ptr cur = buckets_[index];
  node_ptr prev = nullptr;
  while (cur != nullptr)
  {
    if (key_equal_(cur->value_.first, key))
    {
      node_ptr to_erase = cur;
      node_ptr next = cur->next_;
      if (prev == nullptr)
        buckets_[index] = next;
      else
        prev->next_ = next;
      DestroyNode(to_erase);
      --size_;
      ++count;
      cur = next;
    }
    else
    {
      prev = cur;
      cur = cur->next_;
    }
  }
  return count;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Erase(const_iterator pos)
{
  iterator next(pos.node_, pos.buckets_, pos.bucket_count_, pos.index_);
  ++next;
  size_type index = pos.index_;
  node_ptr cur = buckets_[index];
  if (cur == pos.node_)
  {
    buckets_[index] = pos.node_->next_;
  }
  else
  {
    while (cur != nullptr && cur->next_ != pos.node_)
      cur = cur->next_;
    if (cur != nullptr)
      cur->next_ = pos.node_->next_;
  }
  DestroyNode(pos.node_);
  --size_;
  return next;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Erase(
    const_iterator first, const_iterator last)
{
  iterator result;
  while (first != last)
  {
    result = Erase(first);
    first = result;
  }
  return result;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Clear() noexcept
{
  if (buckets_ == nullptr)
    return;
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    node_ptr cur = buckets_[i];
    while (cur != nullptr)
    {
      node_ptr next = cur->next_;
      DestroyNode(cur);
      cur = next;
    }
    buckets_[i] = nullptr;
  }
  size_ = 0;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Swap(self& other) noexcept
{
  NuoSwap(buckets_, other.buckets_);
  NuoSwap(bucket_count_, other.bucket_count_);
  NuoSwap(size_, other.size_);
  NuoSwap(max_load_factor_, other.max_load_factor_);
  NuoSwap(hasher_, other.hasher_);
  NuoSwap(key_equal_, other.key_equal_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename H2, typename E2>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Merge(
    NuoUnorderedMap<Key, T, H2, E2>& source)
{
  if (static_cast<void*>(this) == static_cast<void*>(&source))
    return;
  for (typename NuoUnorderedMap<Key, T, H2, E2>::iterator it = source.Begin();
       it != source.End();)
  {
    if (!Contains(it->first))
    {
      node_ptr node = CreateNode(it->first, NuoMove(it->second));
      InsertNode(node);
      it = source.Erase(it);
    }
    else
    {
      ++it;
    }
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename H2, typename E2>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Merge(
    NuoUnorderedMultimap<Key, T, H2, E2>& source)
{
  if (static_cast<void*>(this) == static_cast<void*>(&source))
    return;
  for (typename NuoUnorderedMultimap<Key, T, H2, E2>::iterator it =
           source.Begin();
       it != source.End();)
  {
    if (!Contains(it->first))
    {
      node_ptr node = CreateNode(it->first, NuoMove(it->second));
      InsertNode(node);
      it = source.Erase(it);
    }
    else
    {
      ++it;
    }
  }
}

/* lookup */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Find(const key_type& key)
{
  if (size_ == 0)
    return End();
  size_type index = GetBucketIndex(key);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, key))
    cur = cur->next_;
  if (cur == nullptr)
    return End();
  return iterator(cur, buckets_, bucket_count_, index);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Find(const key_type& key) const
{
  if (size_ == 0)
    return End();
  size_type index = GetBucketIndex(key);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, key))
    cur = cur->next_;
  if (cur == nullptr)
    return End();
  return const_iterator(cur, buckets_, bucket_count_, index);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Count(const key_type& key) const
{
  if (size_ == 0)
    return 0;
  size_type index = GetBucketIndex(key);
  size_type count = 0;
  node_ptr cur = buckets_[index];
  while (cur != nullptr)
  {
    if (key_equal_(cur->value_.first, key))
      ++count;
    cur = cur->next_;
  }
  return count;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
bool NuoUnorderedMap<Key, T, Hash, KeyEqual>::Contains(
    const key_type& key) const
{
  return Find(key) != End();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator,
         typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::iterator>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::EqualRange(const key_type& key)
{
  iterator first = Find(key);
  if (first == End())
    return nuo_pair<iterator, iterator>(End(), End());
  iterator last = first;
  while (last != End() && key_equal_(last->first, key))
    ++last;
  return nuo_pair<iterator, iterator>(first, last);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
nuo_pair<typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator,
         typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_iterator>
NuoUnorderedMap<Key, T, Hash, KeyEqual>::EqualRange(const key_type& key) const
{
  const_iterator first = Find(key);
  if (first == End())
    return nuo_pair<const_iterator, const_iterator>(End(), End());
  const_iterator last = first;
  while (last != End() && key_equal_(last->first, key))
    ++last;
  return nuo_pair<const_iterator, const_iterator>(first, last);
}

/* bucket interface */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::local_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Begin(size_type n)
{
  return local_iterator(buckets_[n]);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Begin(size_type n) const
{
  return const_local_iterator(buckets_[n]);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::local_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::End(size_type n)
{
  return local_iterator(nullptr);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::End(size_type n) const
{
  return const_local_iterator(nullptr);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::CBegin(size_type n) const
{
  return Begin(n);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMap<Key, T, Hash, KeyEqual>::CEnd(size_type n) const
{
  return End(n);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::BucketCount() const noexcept
{
  return bucket_count_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::MaxBucketCount() const noexcept
{
  return static_cast<size_type>(-1) / sizeof(bucket_type);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::BucketSize(size_type n) const
{
  if (n >= bucket_count_)
    return 0;
  size_type count = 0;
  node_ptr cur = buckets_[n];
  while (cur != nullptr)
  {
    ++count;
    cur = cur->next_;
  }
  return count;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::Bucket(const key_type& key) const
{
  return GetBucketIndex(key);
}

/* hash policy */
template <typename Key, typename T, typename Hash, typename KeyEqual>
float NuoUnorderedMap<Key, T, Hash, KeyEqual>::LoadFactor() const noexcept
{
  if (bucket_count_ == 0)
    return 0.0f;
  return static_cast<float>(size_) / static_cast<float>(bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
float NuoUnorderedMap<Key, T, Hash, KeyEqual>::MaxLoadFactor() const noexcept
{
  return max_load_factor_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::MaxLoadFactor(float z)
{
  max_load_factor_ = z;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Rehash(size_type n)
{
  if (n <= bucket_count_)
    return;
  RehashInternal(NuoUnorderedMapNextPrime(n));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::Reserve(size_type n)
{
  float ratio = static_cast<float>(n) / max_load_factor_;
  size_type needed = static_cast<size_type>(ratio);
  if (static_cast<float>(needed) < ratio)
    ++needed;
  if (needed > bucket_count_)
    RehashInternal(NuoUnorderedMapNextPrime(needed));
}

/* observers */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::hasher
NuoUnorderedMap<Key, T, Hash, KeyEqual>::HashFunction() const
{
  return hasher_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::key_equal
NuoUnorderedMap<Key, T, Hash, KeyEqual>::KeyEq() const
{
  return key_equal_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::allocator_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::GetAllocator() const
{
  return allocator_type();
}

/* private helpers */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMap<Key, T, Hash, KeyEqual>::GetBucketIndex(
    const key_type& key) const
{
  if (bucket_count_ == 0)
    return 0;
  hasher h = hasher_;
  return static_cast<size_type>(h(key) % bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
typename NuoUnorderedMap<Key, T, Hash, KeyEqual>::node_ptr
NuoUnorderedMap<Key, T, Hash, KeyEqual>::CreateNode(Args&&... args)
{
  node_ptr node = NuoAllocator<node_type>::Allocate();
  NuoAllocator<node_type>::Construct(node, NuoForward<Args>(args)...);
  return node;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::DestroyNode(node_ptr node)
{
  NuoAllocator<node_type>::Destroy(node);
  NuoAllocator<node_type>::Deallocate(node);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::InsertNode(node_ptr node)
{
  size_type index = GetBucketIndex(node->value_.first);
  node->next_ = buckets_[index];
  buckets_[index] = node;
  ++size_;
  if (size_ > static_cast<size_type>(bucket_count_ * max_load_factor_))
    RehashInternal(NuoUnorderedMapNextPrime(bucket_count_ * 2 + 1));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::EnsureNonEmpty()
{
  if (buckets_ == nullptr)
  {
    buckets_ = NuoAllocator<bucket_type>::Allocate(kDefaultBucketCount);
    for (size_type i = 0; i < kDefaultBucketCount; ++i)
      buckets_[i] = nullptr;
    bucket_count_ = kDefaultBucketCount;
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMap<Key, T, Hash, KeyEqual>::RehashInternal(
    size_type new_count)
{
  bucket_type* new_buckets =
      NuoAllocator<bucket_type>::Allocate(new_count);
  for (size_type i = 0; i < new_count; ++i)
    new_buckets[i] = nullptr;
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    node_ptr cur = buckets_[i];
    while (cur != nullptr)
    {
      node_ptr next = cur->next_;
      hasher h = hasher_;
      size_type new_index =
          static_cast<size_type>(h(cur->value_.first) % new_count);
      cur->next_ = new_buckets[new_index];
      new_buckets[new_index] = cur;
      cur = next;
    }
  }
  NuoAllocator<bucket_type>::Deallocate(buckets_, bucket_count_);
  buckets_ = new_buckets;
  bucket_count_ = new_count;
}

/****************************************************/
/* NuoUnorderedMultimap member definitions */

/* construction / destruction */
template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::NuoUnorderedMultimap()
  : NuoUnorderedMultimap(kDefaultBucketCount)
{
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::NuoUnorderedMultimap(
    size_type bucket_count,
    const hasher& hash,
    const key_equal& equal)
  : buckets_(nullptr),
    bucket_count_(NuoUnorderedMapNextPrime(bucket_count)),
    size_(0),
    max_load_factor_(1.0f),
    hasher_(hash),
    key_equal_(equal)
{
  buckets_ = NuoAllocator<bucket_type>::Allocate(bucket_count_);
  for (size_type i = 0; i < bucket_count_; ++i)
    buckets_[i] = nullptr;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename InputIterator>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::NuoUnorderedMultimap(
    InputIterator first,
    InputIterator last,
    size_type bucket_count,
    const hasher& hash,
    const key_equal& equal)
  : NuoUnorderedMultimap(bucket_count, hash, equal)
{
  while (first != last)
  {
    Emplace(*first);
    ++first;
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::NuoUnorderedMultimap(
    std::initializer_list<value_type> init_list,
    size_type bucket_count,
    const hasher& hash,
    const key_equal& equal)
  : NuoUnorderedMultimap(bucket_count, hash, equal)
{
  Insert(init_list.begin(), init_list.end());
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::NuoUnorderedMultimap(
    const self& other)
  : buckets_(nullptr),
    bucket_count_(other.bucket_count_),
    size_(0),
    max_load_factor_(other.max_load_factor_),
    hasher_(other.hasher_),
    key_equal_(other.key_equal_)
{
  if (bucket_count_ == 0)
    return;
  buckets_ = NuoAllocator<bucket_type>::Allocate(bucket_count_);
  for (size_type i = 0; i < bucket_count_; ++i)
    buckets_[i] = nullptr;
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    node_ptr cur = other.buckets_[i];
    node_ptr tail = nullptr;
    while (cur != nullptr)
    {
      node_ptr new_node = CreateNode(cur->value_);
      new_node->next_ = nullptr;
      if (tail == nullptr)
        buckets_[i] = new_node;
      else
        tail->next_ = new_node;
      tail = new_node;
      ++size_;
      cur = cur->next_;
    }
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::NuoUnorderedMultimap(
    self&& other) noexcept
  : buckets_(other.buckets_),
    bucket_count_(other.bucket_count_),
    size_(other.size_),
    max_load_factor_(other.max_load_factor_),
    hasher_(NuoMove(other.hasher_)),
    key_equal_(NuoMove(other.key_equal_))
{
  other.buckets_ = nullptr;
  other.bucket_count_ = 0;
  other.size_ = 0;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>&
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::operator=(const self& other)
{
  if (this != &other)
  {
    self tmp(other);
    Swap(tmp);
  }
  return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>&
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::operator=(self&& other) noexcept
{
  if (this != &other)
  {
    self tmp(NuoMove(other));
    Swap(tmp);
  }
  return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>&
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::operator=(
    std::initializer_list<value_type> init_list)
{
  self tmp(init_list);
  Swap(tmp);
  return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::~NuoUnorderedMultimap()
{
  Clear();
  NuoAllocator<bucket_type>::Deallocate(buckets_, bucket_count_);
}

/* iterators */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Begin() noexcept
{
  if (size_ == 0)
    return End();
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    if (buckets_[i] != nullptr)
      return iterator(buckets_[i], buckets_, bucket_count_, i);
  }
  return End();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Begin() const noexcept
{
  if (size_ == 0)
    return End();
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    if (buckets_[i] != nullptr)
      return const_iterator(buckets_[i], buckets_, bucket_count_, i);
  }
  return End();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::End() noexcept
{
  return iterator(nullptr, buckets_, bucket_count_, bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::End() const noexcept
{
  return const_iterator(nullptr, buckets_, bucket_count_, bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::CBegin() const noexcept
{
  return Begin();
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::CEnd() const noexcept
{
  return End();
}

/* capacity */
template <typename Key, typename T, typename Hash, typename KeyEqual>
bool NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Empty() const noexcept
{
  return size_ == 0;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Size() const noexcept
{
  return size_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::MaxSize() const noexcept
{
  return static_cast<size_type>(-1) / sizeof(node_type);
}

/* modifiers */
template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Emplace(Args&&... args)
{
  EnsureNonEmpty();
  node_ptr node = CreateNode(NuoForward<Args>(args)...);
  MultimapInsertNode(node);
  return iterator(node, buckets_, bucket_count_,
                  GetBucketIndex(node->value_.first));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::EmplaceHint(
    const_iterator hint, Args&&... args)
{
  return Emplace(NuoForward<Args>(args)...);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Insert(const value_type& value)
{
  return Emplace(value);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Insert(value_type&& value)
{
  return Emplace(NuoMove(value));
}

/* The hint is ignored for unordered containers. */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Insert(
    const_iterator hint, const value_type& value)
{
  (void)hint;
  return Emplace(value);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Insert(
    const_iterator hint, value_type&& value)
{
  (void)hint;
  return Emplace(NuoMove(value));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename InputIterator>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Insert(
    InputIterator first, InputIterator last)
{
  while (first != last)
  {
    Emplace(*first);
    ++first;
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Insert(
    std::initializer_list<value_type> init_list)
{
  Insert(init_list.begin(), init_list.end());
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Erase(const key_type& key)
{
  if (size_ == 0)
    return 0;
  size_type index = GetBucketIndex(key);
  size_type count = 0;
  node_ptr cur = buckets_[index];
  node_ptr prev = nullptr;
  while (cur != nullptr)
  {
    if (key_equal_(cur->value_.first, key))
    {
      node_ptr to_erase = cur;
      node_ptr next = cur->next_;
      if (prev == nullptr)
        buckets_[index] = next;
      else
        prev->next_ = next;
      DestroyNode(to_erase);
      --size_;
      ++count;
      cur = next;
    }
    else
    {
      prev = cur;
      cur = cur->next_;
    }
  }
  return count;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Erase(const_iterator pos)
{
  iterator next(pos.node_, pos.buckets_, pos.bucket_count_, pos.index_);
  ++next;
  size_type index = pos.index_;
  node_ptr cur = buckets_[index];
  if (cur == pos.node_)
  {
    buckets_[index] = pos.node_->next_;
  }
  else
  {
    while (cur != nullptr && cur->next_ != pos.node_)
      cur = cur->next_;
    if (cur != nullptr)
      cur->next_ = pos.node_->next_;
  }
  DestroyNode(pos.node_);
  --size_;
  return next;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Erase(
    const_iterator first, const_iterator last)
{
  iterator result;
  while (first != last)
  {
    result = Erase(first);
    first = result;
  }
  return result;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Clear() noexcept
{
  if (buckets_ == nullptr)
    return;
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    node_ptr cur = buckets_[i];
    while (cur != nullptr)
    {
      node_ptr next = cur->next_;
      DestroyNode(cur);
      cur = next;
    }
    buckets_[i] = nullptr;
  }
  size_ = 0;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Swap(self& other) noexcept
{
  NuoSwap(buckets_, other.buckets_);
  NuoSwap(bucket_count_, other.bucket_count_);
  NuoSwap(size_, other.size_);
  NuoSwap(max_load_factor_, other.max_load_factor_);
  NuoSwap(hasher_, other.hasher_);
  NuoSwap(key_equal_, other.key_equal_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename H2, typename E2>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Merge(
    NuoUnorderedMap<Key, T, H2, E2>& source)
{
  if (static_cast<void*>(this) == static_cast<void*>(&source))
    return;
  for (typename NuoUnorderedMap<Key, T, H2, E2>::iterator it = source.Begin();
       it != source.End();)
  {
    node_ptr node = CreateNode(it->first, NuoMove(it->second));
    MultimapInsertNode(node);
    it = source.Erase(it);
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename H2, typename E2>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Merge(
    NuoUnorderedMultimap<Key, T, H2, E2>& source)
{
  if (static_cast<void*>(this) == static_cast<void*>(&source))
    return;
  for (typename NuoUnorderedMultimap<Key, T, H2, E2>::iterator it =
           source.Begin();
       it != source.End();)
  {
    node_ptr node = CreateNode(it->first, NuoMove(it->second));
    MultimapInsertNode(node);
    it = source.Erase(it);
  }
}

/* lookup */
/* Find returns the last matching node in the bucket chain. */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Find(const key_type& key)
{
  if (size_ == 0)
    return End();
  size_type index = GetBucketIndex(key);
  node_ptr found = nullptr;
  node_ptr cur = buckets_[index];
  while (cur != nullptr)
  {
    if (key_equal_(cur->value_.first, key))
      found = cur;
    cur = cur->next_;
  }
  if (found == nullptr)
    return End();
  return iterator(found, buckets_, bucket_count_, index);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Find(const key_type& key) const
{
  if (size_ == 0)
    return End();
  size_type index = GetBucketIndex(key);
  node_ptr found = nullptr;
  node_ptr cur = buckets_[index];
  while (cur != nullptr)
  {
    if (key_equal_(cur->value_.first, key))
      found = cur;
    cur = cur->next_;
  }
  if (found == nullptr)
    return End();
  return const_iterator(found, buckets_, bucket_count_, index);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Count(const key_type& key) const
{
  if (size_ == 0)
    return 0;
  size_type index = GetBucketIndex(key);
  size_type count = 0;
  node_ptr cur = buckets_[index];
  while (cur != nullptr)
  {
    if (key_equal_(cur->value_.first, key))
      ++count;
    cur = cur->next_;
  }
  return count;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
bool NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Contains(
    const key_type& key) const
{
  return Find(key) != End();
}

/* EqualRange scans the bucket from the head to cover every matching element. */
template <typename Key, typename T, typename Hash, typename KeyEqual>
nuo_pair<typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator,
         typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::iterator>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::EqualRange(const key_type& key)
{
  if (size_ == 0)
    return nuo_pair<iterator, iterator>(End(), End());
  size_type index = GetBucketIndex(key);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, key))
    cur = cur->next_;
  if (cur == nullptr)
    return nuo_pair<iterator, iterator>(End(), End());
  iterator first(cur, buckets_, bucket_count_, index);
  iterator last = first;
  while (last != End() && key_equal_(last->first, key))
    ++last;
  return nuo_pair<iterator, iterator>(first, last);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
nuo_pair<typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator,
         typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_iterator>
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::EqualRange(
    const key_type& key) const
{
  if (size_ == 0)
    return nuo_pair<const_iterator, const_iterator>(End(), End());
  size_type index = GetBucketIndex(key);
  node_ptr cur = buckets_[index];
  while (cur != nullptr && !key_equal_(cur->value_.first, key))
    cur = cur->next_;
  if (cur == nullptr)
    return nuo_pair<const_iterator, const_iterator>(End(), End());
  const_iterator first(cur, buckets_, bucket_count_, index);
  const_iterator last = first;
  while (last != End() && key_equal_(last->first, key))
    ++last;
  return nuo_pair<const_iterator, const_iterator>(first, last);
}

/* bucket interface */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::local_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Begin(size_type n)
{
  return local_iterator(buckets_[n]);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Begin(size_type n) const
{
  return const_local_iterator(buckets_[n]);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::local_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::End(size_type n)
{
  return local_iterator(nullptr);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::End(size_type n) const
{
  return const_local_iterator(nullptr);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::CBegin(size_type n) const
{
  return Begin(n);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::const_local_iterator
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::CEnd(size_type n) const
{
  return End(n);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::BucketCount() const noexcept
{
  return bucket_count_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::MaxBucketCount() const noexcept
{
  return static_cast<size_type>(-1) / sizeof(bucket_type);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::BucketSize(size_type n) const
{
  if (n >= bucket_count_)
    return 0;
  size_type count = 0;
  node_ptr cur = buckets_[n];
  while (cur != nullptr)
  {
    ++count;
    cur = cur->next_;
  }
  return count;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Bucket(const key_type& key) const
{
  return GetBucketIndex(key);
}

/* hash policy */
template <typename Key, typename T, typename Hash, typename KeyEqual>
float NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::LoadFactor() const noexcept
{
  if (bucket_count_ == 0)
    return 0.0f;
  return static_cast<float>(size_) / static_cast<float>(bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
float NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::MaxLoadFactor()
    const noexcept
{
  return max_load_factor_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::MaxLoadFactor(float z)
{
  max_load_factor_ = z;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Rehash(size_type n)
{
  if (n <= bucket_count_)
    return;
  RehashInternal(NuoUnorderedMapNextPrime(n));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::Reserve(size_type n)
{
  float ratio = static_cast<float>(n) / max_load_factor_;
  size_type needed = static_cast<size_type>(ratio);
  if (static_cast<float>(needed) < ratio)
    ++needed;
  if (needed > bucket_count_)
    RehashInternal(NuoUnorderedMapNextPrime(needed));
}

/* observers */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::hasher
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::HashFunction() const
{
  return hasher_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::key_equal
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::KeyEq() const
{
  return key_equal_;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::allocator_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::GetAllocator() const
{
  return allocator_type();
}

/* private helpers */
template <typename Key, typename T, typename Hash, typename KeyEqual>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::size_type
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::GetBucketIndex(
    const key_type& key) const
{
  if (bucket_count_ == 0)
    return 0;
  hasher h = hasher_;
  return static_cast<size_type>(h(key) % bucket_count_);
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
template <typename... Args>
typename NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::node_ptr
NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::CreateNode(Args&&... args)
{
  node_ptr node = NuoAllocator<node_type>::Allocate();
  NuoAllocator<node_type>::Construct(node, NuoForward<Args>(args)...);
  return node;
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::DestroyNode(node_ptr node)
{
  NuoAllocator<node_type>::Destroy(node);
  NuoAllocator<node_type>::Deallocate(node);
}

/* Keep nodes with the same key contiguous so that EqualRange is linear. */
template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::MultimapInsertNode(
    node_ptr node)
{
  size_type index = GetBucketIndex(node->value_.first);
  node_ptr head = buckets_[index];
  if (head == nullptr)
  {
    buckets_[index] = node;
  }
  else
  {
    node_ptr prev = nullptr;
    node_ptr cur = head;
    while (cur != nullptr &&
           key_equal_(cur->value_.first, node->value_.first))
    {
      prev = cur;
      cur = cur->next_;
    }
    if (prev == nullptr)
    {
      node->next_ = head;
      buckets_[index] = node;
    }
    else
    {
      node->next_ = cur;
      prev->next_ = node;
    }
  }
  ++size_;
  if (size_ > static_cast<size_type>(bucket_count_ * max_load_factor_))
    RehashInternal(NuoUnorderedMapNextPrime(bucket_count_ * 2 + 1));
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::EnsureNonEmpty()
{
  if (buckets_ == nullptr)
  {
    buckets_ = NuoAllocator<bucket_type>::Allocate(kDefaultBucketCount);
    for (size_type i = 0; i < kDefaultBucketCount; ++i)
      buckets_[i] = nullptr;
    bucket_count_ = kDefaultBucketCount;
  }
}

template <typename Key, typename T, typename Hash, typename KeyEqual>
void NuoUnorderedMultimap<Key, T, Hash, KeyEqual>::RehashInternal(
    size_type new_count)
{
  bucket_type* new_buckets =
      NuoAllocator<bucket_type>::Allocate(new_count);
  for (size_type i = 0; i < new_count; ++i)
    new_buckets[i] = nullptr;
  for (size_type i = 0; i < bucket_count_; ++i)
  {
    node_ptr cur = buckets_[i];
    while (cur != nullptr)
    {
      node_ptr next = cur->next_;
      hasher h = hasher_;
      size_type new_index =
          static_cast<size_type>(h(cur->value_.first) % new_count);
      cur->next_ = new_buckets[new_index];
      new_buckets[new_index] = cur;
      cur = next;
    }
  }
  NuoAllocator<bucket_type>::Deallocate(buckets_, bucket_count_);
  buckets_ = new_buckets;
  bucket_count_ = new_count;
}

} /* namespace nuostl */
