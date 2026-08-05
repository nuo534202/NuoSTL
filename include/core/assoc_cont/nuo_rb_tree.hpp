#pragma once

#include <initializer_list>

#include <utility>

#include "core/data_types/nuo_pair.hpp"
#include "utils/nuo_algorithm.hpp"
#include "utils/nuo_allocator.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_functional.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

/* rb tree node color type */
using NuoRbTreeColorType = bool;

constexpr NuoRbTreeColorType kNuoRbTreeRed = false;
constexpr NuoRbTreeColorType kNuoRbTreeBlack = true;

/* forward declaration */
template <typename T> class NuoRbTreeNodeBase;
template <typename T> class NuoRbTreeNode;
template <typename T> class NuoRbTreeIterator;
template <typename T> class NuoRbTreeConstIterator;
template <typename T, typename Compare> class NuoRbTree;

/****************************************************/
/* Rb Tree Value Traits */
/* Detect whether the value type has a member named first (map-style value). */
template <typename T>
class NuoRbTreeHasFirstMember
{
private:
  template <typename U>
  static auto Check(int) -> decltype(std::declval<U>().first, NuoTrueType());
  template <typename U>
  static NuoFalseType Check(...);

public:
  static const bool kValue = decltype(Check<T>(0))::kValue;
};

/* Non-pair version: the stored value is also the key. */
template <typename T, bool>
class NuoRbTreeValueTraitsImp
{
public:
  using key_type   = T;
  using value_type = T;

  template <typename Ty>
  static const key_type& GetKey(const Ty& value)
  {
    return value;
  }
};

/* Pair version: the first member is the key. */
template <typename T>
class NuoRbTreeValueTraitsImp<T, true>
{
public:
  using key_type = typename std::remove_reference<
    decltype(std::declval<T>().first)>::type;
  using value_type = T;

  template <typename Ty>
  static const key_type& GetKey(const Ty& value)
  {
    return value.first;
  }
};

template <typename T>
class NuoRbTreeValueTraits
{
public:
  static const bool kIsPair = NuoRbTreeHasFirstMember<T>::kValue;

  using value_traits_type = NuoRbTreeValueTraitsImp<T, kIsPair>;
  using key_type          = typename value_traits_type::key_type;
  using value_type        = typename value_traits_type::value_type;

  template <typename Ty>
  static const key_type& GetKey(const Ty& value)
  {
    return value_traits_type::GetKey(value);
  }
};

/****************************************************/
/* Rb Tree Node */
template <typename T>
class NuoRbTreeNodeBase
{
public:
  using base_ptr = NuoRbTreeNodeBase<T>*;
  using node_ptr = NuoRbTreeNode<T>*;

  node_ptr ToNode()
  {
    return static_cast<node_ptr>(this);
  }

  base_ptr Self()
  {
    return static_cast<base_ptr>(this);
  }

  base_ptr parent_;
  base_ptr left_;
  base_ptr right_;
  bool     color_;
};

template <typename T>
class NuoRbTreeNode : public NuoRbTreeNodeBase<T>
{
public:
  using base_ptr = typename NuoRbTreeNodeBase<T>::base_ptr;
  using node_ptr = NuoRbTreeNode<T>*;

  base_ptr ToBase()
  {
    return static_cast<base_ptr>(this);
  }

  T value_;
};

/****************************************************/
/* Rb Tree Algorithm */
/* Return the minimum node in the subtree rooted at x. */
template <typename NodePtr>
NodePtr NuoRbTreeMin(NodePtr x) noexcept
{
  while (x->left_ != nullptr)
    x = x->left_;
  return x;
}

/* Return the maximum node in the subtree rooted at x. */
template <typename NodePtr>
NodePtr NuoRbTreeMax(NodePtr x) noexcept
{
  while (x->right_ != nullptr)
    x = x->right_;
  return x;
}

/* Return whether node is the left child of its parent. */
template <typename NodePtr>
bool NuoRbTreeIsLChild(NodePtr node) noexcept
{
  return node == node->parent_->left_;
}

/* Return whether the node is red. */
template <typename NodePtr>
bool NuoRbTreeIsRed(NodePtr node) noexcept
{
  return node->color_ == kNuoRbTreeRed;
}

/* Set the node color to black. */
template <typename NodePtr>
void NuoRbTreeSetBlack(NodePtr node) noexcept
{
  node->color_ = kNuoRbTreeBlack;
}

/* Set the node color to red. */
template <typename NodePtr>
void NuoRbTreeSetRed(NodePtr node) noexcept
{
  node->color_ = kNuoRbTreeRed;
}

/* Return the in-order successor of node. */
template <typename NodePtr>
NodePtr NuoRbTreeNext(NodePtr node) noexcept
{
  if (node->right_ != nullptr)
    return NuoRbTreeMin(node->right_);
  while (!NuoRbTreeIsLChild(node))
    node = node->parent_;
  return node->parent_;
}

/*---------------------------------------*\
|       p                         p       |
|      / \                       / \      |
|     x   d    rotate left      y   d     |
|    / \       ===========>    / \        |
|   a   y                     x   c       |
|      / \                   / \          |
|     b   c                 a   b         |
\*---------------------------------------*/
/* Rotate left around the first argument; the second argument is the root. */
template <typename NodePtr>
void NuoRbTreeRotateLeft(NodePtr x, NodePtr& root) noexcept
{
  NodePtr y = x->right_;  /* y is the right child of x. */
  x->right_ = y->left_;
  if (y->left_ != nullptr)
    y->left_->parent_ = x;
  y->parent_ = x->parent_;

  if (x == root)
  {
    root = y;  /* x is the root, so y replaces x as the new root. */
  }
  else if (NuoRbTreeIsLChild(x))
  {
    x->parent_->left_ = y;
  }
  else
  {
    x->parent_->right_ = y;
  }
  y->left_ = x;
  x->parent_ = y;
}

/*----------------------------------------*\
|     p                         p          |
|    / \                       / \         |
|   d   x      rotate right   d   y        |
|      / \     ===========>      / \       |
|     y   a                     b   x      |
|    / \                           / \     |
|   b   c                         c   a    |
\*----------------------------------------*/
/* Rotate right around the first argument; the second argument is the root. */
template <typename NodePtr>
void NuoRbTreeRotateRight(NodePtr x, NodePtr& root) noexcept
{
  NodePtr y = x->left_;  /* y is the left child of x. */
  x->left_ = y->right_;
  if (y->right_ != nullptr)
    y->right_->parent_ = x;
  y->parent_ = x->parent_;

  if (x == root)
  {
    root = y;  /* x is the root, so y replaces x as the new root. */
  }
  else if (NuoRbTreeIsLChild(x))
  {
    x->parent_->left_ = y;
  }
  else
  {
    x->parent_->right_ = y;
  }
  y->right_ = x;
  x->parent_ = y;
}

/* Rebalance after insertion. The first argument is the new node and the second is the root. */
/* case 1: The new node is the root; color it black. */
/* case 2: The parent is black, so no invariant is violated. */
/* case 3: The parent and uncle are red; color both black, color the grandparent red, and continue from it. */
/* case 4: The parent is red, the uncle is black, and the current node is the inner child;
            rotate to convert the configuration into case 5. */
/* case 5: The parent is red, the uncle is black, and the current node is the outer child;
            color the parent black, color the grandparent red, then rotate around the grandparent. */
template <typename NodePtr>
void NuoRbTreeInsertRebalance(NodePtr x, NodePtr& root) noexcept
{
  NuoRbTreeSetRed(x);  /* New nodes are red. */
  while (x != root && NuoRbTreeIsRed(x->parent_))
  {
    if (NuoRbTreeIsLChild(x->parent_))
    {  /* The parent is a left child. */
      NodePtr uncle = x->parent_->parent_->right_;
      if (uncle != nullptr && NuoRbTreeIsRed(uncle))
      {  /* case 3 */
        NuoRbTreeSetBlack(x->parent_);
        NuoRbTreeSetBlack(uncle);
        x = x->parent_->parent_;
        NuoRbTreeSetRed(x);
      }
      else
      {  /* The uncle is absent or black. */
        if (!NuoRbTreeIsLChild(x))
        {  /* case 4: The current node is a right child. */
          x = x->parent_;
          NuoRbTreeRotateLeft(x, root);
        }
        /* Convert to case 5. */
        NuoRbTreeSetBlack(x->parent_);
        NuoRbTreeSetRed(x->parent_->parent_);
        NuoRbTreeRotateRight(x->parent_->parent_, root);
        break;
      }
    }
    else
    {  /* The parent is a right child; handle symmetrically. */
      NodePtr uncle = x->parent_->parent_->left_;
      if (uncle != nullptr && NuoRbTreeIsRed(uncle))
      {  /* case 3 */
        NuoRbTreeSetBlack(x->parent_);
        NuoRbTreeSetBlack(uncle);
        x = x->parent_->parent_;
        NuoRbTreeSetRed(x);
      }
      else
      {  /* The uncle is absent or black. */
        if (NuoRbTreeIsLChild(x))
        {  /* case 4: The current node is a left child. */
          x = x->parent_;
          NuoRbTreeRotateRight(x, root);
        }
        /* Convert to case 5. */
        NuoRbTreeSetBlack(x->parent_);
        NuoRbTreeSetRed(x->parent_->parent_);
        NuoRbTreeRotateLeft(x->parent_->parent_, root);
        break;
      }
    }
  }
  NuoRbTreeSetBlack(root);  /* The root is always black. */
}

/* Rebalance after deletion. Arguments are the node to erase, root, minimum node, and maximum node;
   return the node that is actually removed. */
/* case 1: The sibling is red; color the parent red and sibling black, rotate, and continue. */
/* case 2: The sibling and both of its children are black; color the sibling red and continue from the parent. */
/* case 3: The sibling is black and its inner child is red; rotate to convert the configuration into case 4. */
/* case 4: The sibling is black and its outer child is red; give the sibling the parent's color,
            color the parent and outer child black, rotate, and finish. */
template <typename NodePtr>
NodePtr NuoRbTreeEraseRebalance(NodePtr z, NodePtr& root, NodePtr& leftmost,
                                NodePtr& rightmost)
{
  /* y is the node ultimately removed, x is y's only child or NIL, and xp is x's parent. */
  NodePtr y = (z->left_ == nullptr || z->right_ == nullptr) ? z
                                                            : NuoRbTreeNext(z);
  NodePtr x = y->left_ != nullptr ? y->left_ : y->right_;
  NodePtr xp = nullptr;

  if (y != z)
  {  /* z has two non-null children; y is the leftmost node in z's right subtree. */
    z->left_->parent_ = y;
    y->left_ = z->left_;

    if (y != z->right_)
    {  /* y is not z's right child, so x replaces y. */
      xp = y->parent_;
      if (x != nullptr)
        x->parent_ = y->parent_;

      y->parent_->left_ = x;
      y->right_ = z->right_;
      z->right_->parent_ = y;
    }
    else
    {
      xp = y;
    }

    /* Replace z with y. */
    if (root == z)
      root = y;
    else if (NuoRbTreeIsLChild(z))
      z->parent_->left_ = y;
    else
      z->parent_->right_ = y;
    y->parent_ = z->parent_;
    NuoSwap(y->color_, z->color_);
    y = z;
  }
  else
  {  /* z has at most one child. */
    xp = y->parent_;
    if (x != nullptr)
      x->parent_ = y->parent_;

    if (root == z)
      root = x;
    else if (NuoRbTreeIsLChild(z))
      z->parent_->left_ = x;
    else
      z->parent_->right_ = x;

    /* z may be the leftmost or rightmost node; update the boundaries. */
    if (leftmost == z)
      leftmost = x == nullptr ? xp : NuoRbTreeMin(x);
    if (rightmost == z)
      rightmost = x == nullptr ? xp : NuoRbTreeMax(x);
  }

  /* Removing a red node preserves the invariants; otherwise rebalance from x. */
  if (!NuoRbTreeIsRed(y))
  {
    while (x != root && (x == nullptr || !NuoRbTreeIsRed(x)))
    {
      if (x == xp->left_)
      {  /* x is a left child. */
        NodePtr brother = xp->right_;
        if (NuoRbTreeIsRed(brother))
        {  /* case 1 */
          NuoRbTreeSetBlack(brother);
          NuoRbTreeSetRed(xp);
          NuoRbTreeRotateLeft(xp, root);
          brother = xp->right_;
        }
        if ((brother->left_ == nullptr || !NuoRbTreeIsRed(brother->left_)) &&
            (brother->right_ == nullptr || !NuoRbTreeIsRed(brother->right_)))
        {  /* case 2 */
          NuoRbTreeSetRed(brother);
          x = xp;
          xp = xp->parent_;
        }
        else
        {
          if (brother->right_ == nullptr || !NuoRbTreeIsRed(brother->right_))
          {  /* case 3 */
            if (brother->left_ != nullptr)
              NuoRbTreeSetBlack(brother->left_);
            NuoRbTreeSetRed(brother);
            NuoRbTreeRotateRight(brother, root);
            brother = xp->right_;
          }
          /* Convert to case 4. */
          brother->color_ = xp->color_;
          NuoRbTreeSetBlack(xp);
          if (brother->right_ != nullptr)
            NuoRbTreeSetBlack(brother->right_);
          NuoRbTreeRotateLeft(xp, root);
          break;
        }
      }
      else
      {  /* x is a right child; handle symmetrically. */
        NodePtr brother = xp->left_;
        if (NuoRbTreeIsRed(brother))
        {  /* case 1 */
          NuoRbTreeSetBlack(brother);
          NuoRbTreeSetRed(xp);
          NuoRbTreeRotateRight(xp, root);
          brother = xp->left_;
        }
        if ((brother->left_ == nullptr || !NuoRbTreeIsRed(brother->left_)) &&
            (brother->right_ == nullptr || !NuoRbTreeIsRed(brother->right_)))
        {  /* case 2 */
          NuoRbTreeSetRed(brother);
          x = xp;
          xp = xp->parent_;
        }
        else
        {
          if (brother->left_ == nullptr || !NuoRbTreeIsRed(brother->left_))
          {  /* case 3 */
            if (brother->right_ != nullptr)
              NuoRbTreeSetBlack(brother->right_);
            NuoRbTreeSetRed(brother);
            NuoRbTreeRotateLeft(brother, root);
            brother = xp->left_;
          }
          /* Convert to case 4. */
          brother->color_ = xp->color_;
          NuoRbTreeSetBlack(xp);
          if (brother->left_ != nullptr)
            NuoRbTreeSetBlack(brother->left_);
          NuoRbTreeRotateRight(xp, root);
          break;
        }
      }
    }
    if (x != nullptr)
      NuoRbTreeSetBlack(x);
  }
  return y;
}

/****************************************************/
/* Rb Tree Iterator */
template <typename T>
class NuoRbTreeIterator :
  public NuoIterator<NuoBidirectionalIteratorTag, T>
{
public:
  using value_type  = T;
  using pointer     = T*;
  using reference   = T&;
  using base_ptr    = NuoRbTreeNodeBase<T>*;
  using node_ptr    = NuoRbTreeNode<T>*;
  using self        = NuoRbTreeIterator<T>;

  NuoRbTreeIterator() = default;
  NuoRbTreeIterator(base_ptr node)
    : node_(node)
  {
  }
  NuoRbTreeIterator(node_ptr node)
    : node_(node->ToBase())
  {
  }
  NuoRbTreeIterator(const NuoRbTreeIterator& iterator)
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
    Inc();
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
    Dec();
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

private:
  /* Advance to the in-order successor. */
  void Inc()
  {
    if (node_->right_ != nullptr)
    {
      node_ = NuoRbTreeMin(node_->right_);
    }
    else
    {
      base_ptr y = node_->parent_;
      while (y->right_ == node_)
      {
        node_ = y;
        y = y->parent_;
      }
      /* Handle the case where the root has no right child and its successor is requested. */
      if (node_->right_ != y)
        node_ = y;
    }
  }

  /* Move back to the in-order predecessor. */
  void Dec()
  {
    if (node_->parent_->parent_ == node_ && NuoRbTreeIsRed(node_))
    {  /* node is the header; move to the maximum node in the tree. */
      node_ = node_->right_;
    }
    else if (node_->left_ != nullptr)
    {
      node_ = NuoRbTreeMax(node_->left_);
    }
    else
    {
      base_ptr y = node_->parent_;
      while (node_ == y->left_)
      {
        node_ = y;
        y = y->parent_;
      }
      node_ = y;
    }
  }
};

/****************************************************/
/* Rb Tree Const Iterator */
template <typename T>
class NuoRbTreeConstIterator :
  public NuoIterator<NuoBidirectionalIteratorTag, T>
{
public:
  using value_type  = T;
  using pointer     = const T*;
  using reference   = const T&;
  using base_ptr    = NuoRbTreeNodeBase<T>*;
  using node_ptr    = NuoRbTreeNode<T>*;
  using self        = NuoRbTreeConstIterator<T>;

  NuoRbTreeConstIterator() = default;
  NuoRbTreeConstIterator(base_ptr node)
    : node_(node)
  {
  }
  NuoRbTreeConstIterator(node_ptr node)
    : node_(node->ToBase())
  {
  }
  NuoRbTreeConstIterator(const NuoRbTreeIterator<T>& iterator)
    : node_(iterator.node_)
  {
  }
  NuoRbTreeConstIterator(const NuoRbTreeConstIterator& iterator)
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
    Inc();
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
    Dec();
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

private:
  /* Advance to the in-order successor. */
  void Inc()
  {
    if (node_->right_ != nullptr)
    {
      node_ = NuoRbTreeMin(node_->right_);
    }
    else
    {
      base_ptr y = node_->parent_;
      while (y->right_ == node_)
      {
        node_ = y;
        y = y->parent_;
      }
      /* Handle the case where the root has no right child and its successor is requested. */
      if (node_->right_ != y)
        node_ = y;
    }
  }

  /* Move back to the in-order predecessor. */
  void Dec()
  {
    if (node_->parent_->parent_ == node_ && NuoRbTreeIsRed(node_))
    {  /* node is the header; move to the maximum node in the tree. */
      node_ = node_->right_;
    }
    else if (node_->left_ != nullptr)
    {
      node_ = NuoRbTreeMax(node_->left_);
    }
    else
    {
      base_ptr y = node_->parent_;
      while (node_ == y->left_)
      {
        node_ = y;
        y = y->parent_;
      }
      node_ = y;
    }
  }
};

/****************************************************/
/* Rb Tree */
/* Param 1 is the stored type; Param 2 is the key comparison type. */
template <typename T, typename Compare>
class NuoRbTree
{
public:
  /* Nested type definitions */
  using value_traits       = NuoRbTreeValueTraits<T>;
  using base_type          = NuoRbTreeNodeBase<T>;
  using base_ptr           = base_type*;
  using node_type          = NuoRbTreeNode<T>;
  using node_ptr           = node_type*;
  using key_type           = typename value_traits::key_type;
  using value_type         = typename value_traits::value_type;
  using key_compare        = Compare;

  using allocator_type     = NuoAllocator<T>;
  using data_allocator     = NuoAllocator<T>;
  using base_allocator     = NuoAllocator<base_type>;
  using node_allocator     = NuoAllocator<node_type>;

  using pointer            = typename allocator_type::pointer;
  using const_pointer      = typename allocator_type::const_pointer;
  using reference          = typename allocator_type::reference;
  using const_reference    = typename allocator_type::const_reference;
  using size_type          = typename allocator_type::size_type;
  using difference_type    = typename allocator_type::difference_type;

  using iterator           = NuoRbTreeIterator<T>;
  using const_iterator     = NuoRbTreeConstIterator<T>;
  using reverse_iterator   = NuoReverseIterator<iterator>;
  using const_reverse_iterator = NuoReverseIterator<const_iterator>;

  allocator_type GetAllocator() const { return allocator_type(); }
  key_compare    KeyComp() const { return key_comp_; }

private:
  /* The following three members represent the rb-tree. */
  base_ptr    header_;      /* Sentinel node; it and the root point to each other as parents. */
  size_type   node_count_;  /* Number of nodes. */
  key_compare key_comp_;    /* Key comparison rule. */

private:
  /* Access the root, minimum node, and maximum node. */
  base_ptr& Root() const { return header_->parent_; }
  base_ptr& Leftmost() const { return header_->left_; }
  base_ptr& Rightmost() const { return header_->right_; }

public:
  /* Construction, copy, move, and destruction */
  NuoRbTree() { RbTreeInit(); }
  explicit NuoRbTree(const key_compare& comp);

  NuoRbTree(const NuoRbTree& rhs);
  NuoRbTree(NuoRbTree&& rhs) noexcept;

  NuoRbTree& operator=(const NuoRbTree& rhs);
  NuoRbTree& operator=(NuoRbTree&& rhs);

  ~NuoRbTree()
  {
    Clear();
    base_allocator::Deallocate(header_);
  }

public:
  /* Iterator-related operations */
  iterator       Begin() noexcept { return Leftmost(); }
  const_iterator Begin() const noexcept { return Leftmost(); }
  iterator       End() noexcept { return header_; }
  const_iterator End() const noexcept { return header_; }

  reverse_iterator       RBegin() noexcept { return reverse_iterator(End()); }
  const_reverse_iterator RBegin() const noexcept
  {
    return const_reverse_iterator(End());
  }
  reverse_iterator       REnd() noexcept { return reverse_iterator(Begin()); }
  const_reverse_iterator REnd() const noexcept
  {
    return const_reverse_iterator(Begin());
  }

  const_iterator         CBegin() const noexcept { return Begin(); }
  const_iterator         CEnd() const noexcept { return End(); }
  const_reverse_iterator CRBegin() const noexcept { return RBegin(); }
  const_reverse_iterator CREnd() const noexcept { return REnd(); }

public:
  /* Capacity-related operations */
  bool      Empty() const noexcept { return node_count_ == 0; }
  size_type Size() const noexcept { return node_count_; }
  size_type MaxSize() const noexcept { return static_cast<size_type>(-1); }

public:
  /* Insertion operations */

  /* Construct an element in place; duplicate keys are allowed. */
  template <typename... Args>
  iterator EmplaceMulti(Args&&... args);

  /* Construct an element in place; duplicate keys are not allowed. */
  template <typename... Args>
  nuo_pair<iterator, bool> EmplaceUnique(Args&&... args);

  /* Construct an element in place with a hint; duplicate keys are allowed. */
  template <typename... Args>
  iterator EmplaceMultiUseHint(iterator hint, Args&&... args);

  /* Construct an element in place with a hint; duplicate keys are not allowed. */
  template <typename... Args>
  iterator EmplaceUniqueUseHint(iterator hint, Args&&... args);

  /* Insert an element; duplicate keys are allowed. */
  iterator InsertMulti(const value_type& value);
  iterator InsertMulti(value_type&& value)
  {
    return EmplaceMulti(NuoMove(value));
  }

  /* Insert an element with a hint; duplicate keys are allowed. */
  iterator InsertMulti(iterator hint, const value_type& value)
  {
    return EmplaceMultiUseHint(hint, value);
  }
  iterator InsertMulti(iterator hint, value_type&& value)
  {
    return EmplaceMultiUseHint(hint, NuoMove(value));
  }

  /* Insert a range of elements; duplicate keys are allowed. */
  template <typename InputIterator>
  void InsertMulti(InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      InsertMulti(End(), *first);
  }

  /* Insert an element; duplicate keys are not allowed. */
  nuo_pair<iterator, bool> InsertUnique(const value_type& value);
  nuo_pair<iterator, bool> InsertUnique(value_type&& value)
  {
    return EmplaceUnique(NuoMove(value));
  }

  /* Insert an element with a hint; duplicate keys are not allowed. */
  iterator InsertUnique(iterator hint, const value_type& value)
  {
    return EmplaceUniqueUseHint(hint, value);
  }
  iterator InsertUnique(iterator hint, value_type&& value)
  {
    return EmplaceUniqueUseHint(hint, NuoMove(value));
  }

  /* Insert a range of elements; duplicate keys are not allowed. */
  template <typename InputIterator>
  void InsertUnique(InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      InsertUnique(End(), *first);
  }

public:
  /* Erasure operations */
  iterator  Erase(iterator hint);

  /* Erase all elements whose key equals key and return the number erased. */
  size_type EraseMulti(const key_type& key);
  size_type EraseUnique(const key_type& key);

  /* Erase elements in the range [first, last). */
  void Erase(iterator first, iterator last);

  /* Clear the rb-tree. */
  void Clear();

public:
  /* Rb-tree-related operations */
  iterator       Find(const key_type& key);
  const_iterator Find(const key_type& key) const;

  /* Count elements whose key equals key. */
  size_type CountMulti(const key_type& key) const
  {
    auto p = EqualRangeMulti(key);
    return static_cast<size_type>(NuoDistance(p.first, p.second));
  }
  size_type CountUnique(const key_type& key) const
  {
    return Find(key) != End() ? 1 : 0;
  }

  /* Return the first position whose key is not less than key. */
  iterator       LowerBound(const key_type& key);
  const_iterator LowerBound(const key_type& key) const;

  /* Return the first position whose key is greater than key. */
  iterator       UpperBound(const key_type& key);
  const_iterator UpperBound(const key_type& key) const;

  /* Return the range whose keys equal key. */
  nuo_pair<iterator, iterator> EqualRangeMulti(const key_type& key)
  {
    return nuo_make_pair(LowerBound(key), UpperBound(key));
  }
  nuo_pair<const_iterator, const_iterator>
  EqualRangeMulti(const key_type& key) const
  {
    return nuo_make_pair(LowerBound(key), UpperBound(key));
  }

  nuo_pair<iterator, iterator> EqualRangeUnique(const key_type& key)
  {
    iterator it = Find(key);
    if (it == End())
    {  /* Return the insertion point [lower_bound, lower_bound) when the key is absent. */
      iterator lb = LowerBound(key);
      return nuo_make_pair(lb, lb);
    }
    iterator next = it;
    return nuo_make_pair(it, ++next);
  }
  nuo_pair<const_iterator, const_iterator>
  EqualRangeUnique(const key_type& key) const
  {
    const_iterator it = Find(key);
    if (it == End())
    {  /* Return the insertion point [lower_bound, lower_bound) when the key is absent. */
      const_iterator lb = LowerBound(key);
      return nuo_make_pair(lb, lb);
    }
    const_iterator next = it;
    return nuo_make_pair(it, ++next);
  }

  /* Swap two rb-trees. */
  void Swap(NuoRbTree& rhs) noexcept;

private:
  /* Node-related helpers */
  template <typename... Args>
  node_ptr CreateNode(Args&&... args);
  node_ptr CloneNode(base_ptr x);
  void     DestroyNode(node_ptr p);

  /* Initialization and reset */
  void RbTreeInit();
  void Reset();

  /* Determine insertion positions. */
  nuo_pair<base_ptr, bool> GetInsertMultiPos(const key_type& key);
  nuo_pair<nuo_pair<base_ptr, bool>, bool>
  GetInsertUniquePos(const key_type& key);

  /* Insert a value or node. */
  iterator InsertValueAt(base_ptr x, const value_type& value, bool add_to_left);
  iterator InsertNodeAt(base_ptr x, node_ptr node, bool add_to_left);

  /* Insert using a hint. */
  iterator InsertMultiUseHint(iterator hint, key_type key, node_ptr node);
  iterator InsertUniqueUseHint(iterator hint, key_type key, node_ptr node);

  /* Copy a tree or erase a subtree. */
  base_ptr CopyFrom(base_ptr x, base_ptr p);
  void     EraseSince(base_ptr x);
};

/****************************************************/
/* Constructors and assignment operators */

template <typename T, typename Compare>
NuoRbTree<T, Compare>::NuoRbTree(const key_compare& comp)
  : key_comp_(comp)
{
  RbTreeInit();
}

/* Copy constructor */
template <typename T, typename Compare>
NuoRbTree<T, Compare>::NuoRbTree(const NuoRbTree& rhs)
{
  RbTreeInit();
  if (rhs.node_count_ != 0)
  {
    Root() = CopyFrom(rhs.Root(), header_);
    Leftmost() = NuoRbTreeMin(Root());
    Rightmost() = NuoRbTreeMax(Root());
  }
  node_count_ = rhs.node_count_;
  key_comp_ = rhs.key_comp_;
}

/* Move constructor */
template <typename T, typename Compare>
NuoRbTree<T, Compare>::NuoRbTree(NuoRbTree&& rhs) noexcept
  : header_(NuoMove(rhs.header_)),
    node_count_(rhs.node_count_),
    key_comp_(rhs.key_comp_)
{
  rhs.Reset();
}

/* Copy assignment operator */
template <typename T, typename Compare>
NuoRbTree<T, Compare>&
NuoRbTree<T, Compare>::operator=(const NuoRbTree& rhs)
{
  if (this != &rhs)
  {
    Clear();

    if (rhs.node_count_ != 0)
    {
      Root() = CopyFrom(rhs.Root(), header_);
      Leftmost() = NuoRbTreeMin(Root());
      Rightmost() = NuoRbTreeMax(Root());
    }

    node_count_ = rhs.node_count_;
    key_comp_ = rhs.key_comp_;
  }
  return *this;
}

/* Move assignment operator */
template <typename T, typename Compare>
NuoRbTree<T, Compare>&
NuoRbTree<T, Compare>::operator=(NuoRbTree&& rhs)
{
  if (this != &rhs)
  {
    Clear();
    base_allocator::Deallocate(header_);
    header_ = NuoMove(rhs.header_);
    node_count_ = rhs.node_count_;
    key_comp_ = rhs.key_comp_;
    rhs.Reset();
  }
  return *this;
}

/****************************************************/
/* Insertion operations */

/* Construct an element in place; duplicate keys are allowed. */
template <typename T, typename Compare>
template <typename... Args>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::EmplaceMulti(Args&&... args)
{
  NUO_THROW_LENGTH_ERROR_IF(node_count_ > MaxSize() - 1,
                            "NuoRbTree<T, Comp>'s size too big");
  node_ptr np = CreateNode(NuoForward<Args>(args)...);
  auto res = GetInsertMultiPos(value_traits::GetKey(np->value_));
  return InsertNodeAt(res.first, np, res.second);
}

/* Construct an element in place; duplicate keys are not allowed. */
template <typename T, typename Compare>
template <typename... Args>
nuo_pair<typename NuoRbTree<T, Compare>::iterator, bool>
NuoRbTree<T, Compare>::EmplaceUnique(Args&&... args)
{
  NUO_THROW_LENGTH_ERROR_IF(node_count_ > MaxSize() - 1,
                            "NuoRbTree<T, Comp>'s size too big");
  node_ptr np = CreateNode(NuoForward<Args>(args)...);
  auto res = GetInsertUniquePos(value_traits::GetKey(np->value_));
  if (res.second)
  {  /* Insertion succeeded. */
    return nuo_make_pair(InsertNodeAt(res.first.first, np, res.first.second), true);
  }
  DestroyNode(np);
  return nuo_make_pair(iterator(res.first.first), false);
}

/* Construct an element in place with a hint; duplicate keys are allowed. */
template <typename T, typename Compare>
template <typename... Args>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::EmplaceMultiUseHint(iterator hint, Args&&... args)
{
  NUO_THROW_LENGTH_ERROR_IF(node_count_ > MaxSize() - 1,
                            "NuoRbTree<T, Comp>'s size too big");
  node_ptr np = CreateNode(NuoForward<Args>(args)...);
  if (node_count_ == 0)
  {
    return InsertNodeAt(header_, np, true);
  }
  key_type key = value_traits::GetKey(np->value_);
  if (hint == Begin())
  {  /* hint points to begin. */
    if (key_comp_(key, value_traits::GetKey(*hint)))
    {
      return InsertNodeAt(hint.node_, np, true);
    }
    else
    {
      auto pos = GetInsertMultiPos(key);
      return InsertNodeAt(pos.first, np, pos.second);
    }
  }
  else if (hint == End())
  {  /* hint points to end. */
    if (!key_comp_(key, value_traits::GetKey(Rightmost()->ToNode()->value_)))
    {
      return InsertNodeAt(Rightmost(), np, false);
    }
    else
    {
      auto pos = GetInsertMultiPos(key);
      return InsertNodeAt(pos.first, np, pos.second);
    }
  }
  return InsertMultiUseHint(hint, key, np);
}

/* Construct an element in place with a hint; duplicate keys are not allowed. */
template <typename T, typename Compare>
template <typename... Args>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::EmplaceUniqueUseHint(iterator hint, Args&&... args)
{
  NUO_THROW_LENGTH_ERROR_IF(node_count_ > MaxSize() - 1,
                            "NuoRbTree<T, Comp>'s size too big");
  node_ptr np = CreateNode(NuoForward<Args>(args)...);
  if (node_count_ == 0)
  {
    return InsertNodeAt(header_, np, true);
  }
  key_type key = value_traits::GetKey(np->value_);
  if (hint == Begin())
  {  /* hint points to begin. */
    if (key_comp_(key, value_traits::GetKey(*hint)))
    {
      return InsertNodeAt(hint.node_, np, true);
    }
    else
    {
      auto pos = GetInsertUniquePos(key);
      if (!pos.second)
      {
        DestroyNode(np);
        return pos.first.first;
      }
      return InsertNodeAt(pos.first.first, np, pos.first.second);
    }
  }
  else if (hint == End())
  {  /* hint points to end. */
    if (key_comp_(value_traits::GetKey(Rightmost()->ToNode()->value_), key))
    {
      return InsertNodeAt(Rightmost(), np, false);
    }
    else
    {
      auto pos = GetInsertUniquePos(key);
      if (!pos.second)
      {
        DestroyNode(np);
        return pos.first.first;
      }
      return InsertNodeAt(pos.first.first, np, pos.first.second);
    }
  }
  return InsertUniqueUseHint(hint, key, np);
}

/* Insert an element; duplicate keys are allowed. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::InsertMulti(const value_type& value)
{
  NUO_THROW_LENGTH_ERROR_IF(node_count_ > MaxSize() - 1,
                            "NuoRbTree<T, Comp>'s size too big");
  auto res = GetInsertMultiPos(value_traits::GetKey(value));
  return InsertValueAt(res.first, value, res.second);
}

/* Insert an element; duplicate keys are not allowed. */
template <typename T, typename Compare>
nuo_pair<typename NuoRbTree<T, Compare>::iterator, bool>
NuoRbTree<T, Compare>::InsertUnique(const value_type& value)
{
  NUO_THROW_LENGTH_ERROR_IF(node_count_ > MaxSize() - 1,
                            "NuoRbTree<T, Comp>'s size too big");
  auto res = GetInsertUniquePos(value_traits::GetKey(value));
  if (res.second)
  {  /* Insertion succeeded. */
    return nuo_make_pair(InsertValueAt(res.first.first, value, res.first.second), true);
  }
  /* Insertion failed; return an iterator to the existing node. */
  return nuo_make_pair(iterator(res.first.first), false);
}

/****************************************************/
/* Erasure operations */

/* Erase the node at hint. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::Erase(iterator hint)
{
  node_ptr node = hint.node_->ToNode();
  iterator next(node);
  ++next;

  NuoRbTreeEraseRebalance(hint.node_, Root(), Leftmost(), Rightmost());
  DestroyNode(node);
  --node_count_;
  return next;
}

/* Erase all elements whose key equals key and return the number erased. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::size_type
NuoRbTree<T, Compare>::EraseMulti(const key_type& key)
{
  auto p = EqualRangeMulti(key);
  size_type n = static_cast<size_type>(NuoDistance(p.first, p.second));
  Erase(p.first, p.second);
  return n;
}

/* Erase one element whose key equals key and return the number erased. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::size_type
NuoRbTree<T, Compare>::EraseUnique(const key_type& key)
{
  auto it = Find(key);
  if (it != End())
  {
    Erase(it);
    return 1;
  }
  return 0;
}

/* Erase elements in the range [first, last). */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::Erase(iterator first, iterator last)
{
  if (first == Begin() && last == End())
  {
    Clear();
  }
  else
  {
    while (first != last)
      Erase(first++);
  }
}

/* Clear the rb-tree. */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::Clear()
{
  if (node_count_ != 0)
  {
    EraseSince(Root());
    Leftmost() = header_;
    Root() = nullptr;
    Rightmost() = header_;
    node_count_ = 0;
  }
}

/****************************************************/
/* Lookup operations */

/* Find the node whose key equals key and return an iterator to it. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::Find(const key_type& key)
{
  auto y = header_;  /* Last node whose key is not less than key. */
  auto x = Root();
  while (x != nullptr)
  {
    if (!key_comp_(value_traits::GetKey(x->ToNode()->value_), key))
    {  /* key is less than or equal to x's key; move left. */
      y = x, x = x->left_;
    }
    else
    {  /* key is greater than x's key; move right. */
      x = x->right_;
    }
  }
  iterator j = iterator(y);
  return (j == End() || key_comp_(key, value_traits::GetKey(*j))) ? End() : j;
}

template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::const_iterator
NuoRbTree<T, Compare>::Find(const key_type& key) const
{
  auto y = header_;
  auto x = Root();
  while (x != nullptr)
  {
    if (!key_comp_(value_traits::GetKey(x->ToNode()->value_), key))
    {
      y = x, x = x->left_;
    }
    else
    {
      x = x->right_;
    }
  }
  const_iterator j = const_iterator(y);
  return (j == End() || key_comp_(key, value_traits::GetKey(*j))) ? End() : j;
}

/* Return the first position whose key is not less than key. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::LowerBound(const key_type& key)
{
  auto y = header_;
  auto x = Root();
  while (x != nullptr)
  {
    if (!key_comp_(value_traits::GetKey(x->ToNode()->value_), key))
    {  /* key <= x */
      y = x, x = x->left_;
    }
    else
    {
      x = x->right_;
    }
  }
  return iterator(y);
}

template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::const_iterator
NuoRbTree<T, Compare>::LowerBound(const key_type& key) const
{
  auto y = header_;
  auto x = Root();
  while (x != nullptr)
  {
    if (!key_comp_(value_traits::GetKey(x->ToNode()->value_), key))
    {
      y = x, x = x->left_;
    }
    else
    {
      x = x->right_;
    }
  }
  return const_iterator(y);
}

/* Return the first position whose key is greater than key. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::UpperBound(const key_type& key)
{
  auto y = header_;
  auto x = Root();
  while (x != nullptr)
  {
    if (key_comp_(key, value_traits::GetKey(x->ToNode()->value_)))
    {  /* key < x */
      y = x, x = x->left_;
    }
    else
    {
      x = x->right_;
    }
  }
  return iterator(y);
}

template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::const_iterator
NuoRbTree<T, Compare>::UpperBound(const key_type& key) const
{
  auto y = header_;
  auto x = Root();
  while (x != nullptr)
  {
    if (key_comp_(key, value_traits::GetKey(x->ToNode()->value_)))
    {
      y = x, x = x->left_;
    }
    else
    {
      x = x->right_;
    }
  }
  return const_iterator(y);
}

/* Swap two rb-trees. */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::Swap(NuoRbTree& rhs) noexcept
{
  if (this != &rhs)
  {
    NuoSwap(header_, rhs.header_);
    NuoSwap(node_count_, rhs.node_count_);
    NuoSwap(key_comp_, rhs.key_comp_);
  }
}

/****************************************************/
/* Helper functions */

/* Create a node. */
template <typename T, typename Compare>
template <typename... Args>
typename NuoRbTree<T, Compare>::node_ptr
NuoRbTree<T, Compare>::CreateNode(Args&&... args)
{
  node_ptr tmp = node_allocator::Allocate();
  try
  {
    data_allocator::Construct(&tmp->value_, NuoForward<Args>(args)...);
    tmp->left_ = nullptr;
    tmp->right_ = nullptr;
    tmp->parent_ = nullptr;
  }
  catch (...)
  {
    node_allocator::Deallocate(tmp);
    throw;
  }
  return tmp;
}

/* Clone a node. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::node_ptr
NuoRbTree<T, Compare>::CloneNode(base_ptr x)
{
  node_ptr tmp = CreateNode(x->ToNode()->value_);
  tmp->color_ = x->color_;
  tmp->left_ = nullptr;
  tmp->right_ = nullptr;
  return tmp;
}

/* Destroy a node. */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::DestroyNode(node_ptr p)
{
  data_allocator::Destroy(&p->value_);
  node_allocator::Deallocate(p);
}

/* Initialize the container. */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::RbTreeInit()
{
  header_ = base_allocator::Allocate();
  header_->color_ = kNuoRbTreeRed;  /* The header is red to distinguish it from the root. */
  Root() = nullptr;
  Leftmost() = header_;
  Rightmost() = header_;
  node_count_ = 0;
}

/* Reset function */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::Reset()
{
  header_ = nullptr;
  node_count_ = 0;
}

/* Find the insertion position when duplicate keys are allowed; return the parent and insertion side. */
template <typename T, typename Compare>
nuo_pair<typename NuoRbTree<T, Compare>::base_ptr, bool>
NuoRbTree<T, Compare>::GetInsertMultiPos(const key_type& key)
{
  auto x = Root();
  auto y = header_;
  bool add_to_left = true;
  while (x != nullptr)
  {
    y = x;
    add_to_left = key_comp_(key, value_traits::GetKey(x->ToNode()->value_));
    x = add_to_left ? x->left_ : x->right_;
  }
  return nuo_make_pair(y, add_to_left);
}

/* Find the insertion position when duplicate keys are not allowed. */
template <typename T, typename Compare>
nuo_pair<nuo_pair<typename NuoRbTree<T, Compare>::base_ptr, bool>, bool>
NuoRbTree<T, Compare>::GetInsertUniquePos(const key_type& key)
{
  /* The first pair contains the insertion parent and side.
     The second value indicates whether insertion is allowed. */
  auto x = Root();
  auto y = header_;
  bool add_to_left = true;  /* Insert to the left of header when the tree is empty. */
  while (x != nullptr)
  {
    y = x;
    add_to_left = key_comp_(key, value_traits::GetKey(x->ToNode()->value_));
    x = add_to_left ? x->left_ : x->right_;
  }
  iterator j = iterator(y);  /* y is now the insertion parent. */
  if (add_to_left)
  {
    if (y == header_ || j == Begin())
    {  /* Insertion is valid for an empty tree or before the leftmost node. */
      return nuo_make_pair(nuo_make_pair(y, true), true);
    }
    else
    {  /* Otherwise, if a duplicate exists, --j refers to it. */
      --j;
    }
  }
  if (key_comp_(value_traits::GetKey(*j), key))
  {  /* The new node is not a duplicate. */
    return nuo_make_pair(nuo_make_pair(y, add_to_left), true);
  }
  /* Reaching this point means the new key duplicates an existing key. */
  return nuo_make_pair(nuo_make_pair(y, add_to_left), false);
}

/* Insert value at x; add_to_left selects the left or right side. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::InsertValueAt(base_ptr x, const value_type& value,
                                     bool add_to_left)
{
  node_ptr node = CreateNode(value);
  node->parent_ = x;
  base_ptr base_node = node->ToBase();
  if (x == header_)
  {
    Root() = base_node;
    Leftmost() = base_node;
    Rightmost() = base_node;
  }
  else if (add_to_left)
  {
    x->left_ = base_node;
    if (Leftmost() == x)
      Leftmost() = base_node;
  }
  else
  {
    x->right_ = base_node;
    if (Rightmost() == x)
      Rightmost() = base_node;
  }
  NuoRbTreeInsertRebalance(base_node, Root());
  ++node_count_;
  return iterator(node);
}

/* Insert node at x; add_to_left selects the left or right side. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::InsertNodeAt(base_ptr x, node_ptr node, bool add_to_left)
{
  node->parent_ = x;
  base_ptr base_node = node->ToBase();
  if (x == header_)
  {
    Root() = base_node;
    Leftmost() = base_node;
    Rightmost() = base_node;
  }
  else if (add_to_left)
  {
    x->left_ = base_node;
    if (Leftmost() == x)
      Leftmost() = base_node;
  }
  else
  {
    x->right_ = base_node;
    if (Rightmost() == x)
      Rightmost() = base_node;
  }
  NuoRbTreeInsertRebalance(base_node, Root());
  ++node_count_;
  return iterator(node);
}

/* Insert an element with a hint; duplicate keys are allowed. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::InsertMultiUseHint(iterator hint, key_type key,
                                          node_ptr node)
{
  /* Search near hint for a valid insertion position. */
  auto np = hint.node_;
  auto before = hint;
  --before;
  auto bnp = before.node_;
  if (!key_comp_(key, value_traits::GetKey(*before)) &&
      !key_comp_(value_traits::GetKey(*hint), key))
  {  /* before <= node <= hint */
    if (bnp->right_ == nullptr)
    {
      return InsertNodeAt(bnp, node, false);
    }
    else if (np->left_ == nullptr)
    {
      return InsertNodeAt(np, node, true);
    }
  }
  auto pos = GetInsertMultiPos(key);
  return InsertNodeAt(pos.first, node, pos.second);
}

/* Insert an element with a hint; duplicate keys are not allowed. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::iterator
NuoRbTree<T, Compare>::InsertUniqueUseHint(iterator hint, key_type key,
                                           node_ptr node)
{
  /* Search near hint for a valid insertion position. */
  auto np = hint.node_;
  auto before = hint;
  --before;
  auto bnp = before.node_;
  if (key_comp_(value_traits::GetKey(*before), key) &&
      key_comp_(key, value_traits::GetKey(*hint)))
  {  /* before < node < hint */
    if (bnp->right_ == nullptr)
    {
      return InsertNodeAt(bnp, node, false);
    }
    else if (np->left_ == nullptr)
    {
      return InsertNodeAt(np, node, true);
    }
  }
  auto pos = GetInsertUniquePos(key);
  if (!pos.second)
  {
    DestroyNode(node);
    return pos.first.first;
  }
  return InsertNodeAt(pos.first.first, node, pos.first.second);
}

/* Recursively copy a tree starting at x, whose parent is p. */
template <typename T, typename Compare>
typename NuoRbTree<T, Compare>::base_ptr
NuoRbTree<T, Compare>::CopyFrom(base_ptr x, base_ptr p)
{
  base_ptr top = CloneNode(x);
  top->parent_ = p;
  try
  {
    if (x->right_)
      top->right_ = CopyFrom(x->right_, top);
    p = top;
    x = x->left_;
    while (x != nullptr)
    {
      base_ptr y = CloneNode(x);
      p->left_ = y;
      y->parent_ = p;
      if (x->right_)
        y->right_ = CopyFrom(x->right_, y);
      p = y;
      x = x->left_;
    }
  }
  catch (...)
  {
    EraseSince(top);
    throw;
  }
  return top;
}

/* Erase x and its subtree. */
template <typename T, typename Compare>
void NuoRbTree<T, Compare>::EraseSince(base_ptr x)
{
  while (x != nullptr)
  {
    EraseSince(x->right_);
    base_ptr y = x->left_;
    DestroyNode(x->ToNode());
    x = y;
  }
}

/****************************************************/
/* Overloaded comparison operators */
template <typename T, typename Compare>
bool operator==(const NuoRbTree<T, Compare>& lhs,
                const NuoRbTree<T, Compare>& rhs)
{
  return lhs.Size() == rhs.Size() && NuoEqual(lhs.Begin(), lhs.End(), rhs.Begin());
}

template <typename T, typename Compare>
bool operator<(const NuoRbTree<T, Compare>& lhs,
               const NuoRbTree<T, Compare>& rhs)
{
  return NuoLexicographicalCompare(lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template <typename T, typename Compare>
bool operator!=(const NuoRbTree<T, Compare>& lhs,
                const NuoRbTree<T, Compare>& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Compare>
bool operator>(const NuoRbTree<T, Compare>& lhs,
               const NuoRbTree<T, Compare>& rhs)
{
  return rhs < lhs;
}

template <typename T, typename Compare>
bool operator<=(const NuoRbTree<T, Compare>& lhs,
                const NuoRbTree<T, Compare>& rhs)
{
  return !(rhs < lhs);
}

template <typename T, typename Compare>
bool operator>=(const NuoRbTree<T, Compare>& lhs,
                const NuoRbTree<T, Compare>& rhs)
{
  return !(lhs < rhs);
}

/* Overload nuostl swap */
template <typename T, typename Compare>
void swap(NuoRbTree<T, Compare>& lhs, NuoRbTree<T, Compare>& rhs) noexcept
{
  lhs.Swap(rhs);
}

} /* namespace nuostl */
