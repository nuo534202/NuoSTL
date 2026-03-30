#pragma once

#include "utils/nuo_iterator.hpp"

namespace nuostl
{

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

  node_ptr to_node()
  {
    return static_cast<node_ptr>(self());
  }

  void unlink()
  {
    prev_ = next_ = self();
  }

  base_ptr self()
  {
    return static_case<base_ptr>(&(*this));
  }

  base_ptr prev_;
  base_ptr next_;
};

template <typename T>
class NuoListNode : public NuoListNodeBase<T>
{
public:
  NuoListNode() = default;
  NuoListNode(const T& value)
    : value_(value)
  {
  }
  NuoListNode(T&& value)
    : value_(std::NuoMove(value))
  {
  }

  base_ptr to_base()
  {
    return static_cast<base_ptr>(&(*this));
  }

  node_ptr self()
  {
    return static_case<node_ptr>(&(*this));
  }

  T value_;
};

/* List Iterator */
template <typename T>
class NuoListIterator : public NuoIterator<T>
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
    : node_(iterator.GetNode())
  {
  }

  reference operator* const
  {
    return node->to_node()->value;
  }

  pointer operator-> const
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
    return node_ == rhs_.node_;
  }

  bool operator!=(const self& rhs) const
  {
    return node_ == rhs.node_;
  }

  const base_ptr GetNode() const
  {
    return node_;
  }

  void SetNode(base_ptr node)
  {
    node_ = node;
  }

private:
  base_ptr node_;
};

} /* namespace nuostl */