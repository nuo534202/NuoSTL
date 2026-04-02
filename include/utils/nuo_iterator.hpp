#pragma once

#include <stddef.h>

#include "utils/nuo_type_traits.hpp"

namespace nuostl
{

/* 5 iterator types */
class NuoInputIteratorTag {};
class NuoOutputIteratorTag {};
class NuoForwardIteratorTag : public NuoInputIteratorTag {};
class NuoBidirectionalIteratorTag : public NuoForwardIteratorTag {};
class NuoRandomAccessIteratorTag : public NuoBidirectionalIteratorTag {};

/* iterator template */
template <typename Category, typename T>
class NuoIterator
{
public:
  using iterator_category = Category;
  using value_type        = T;
  using pointer           = T*;
  using reference         = T&;
  using difference_type   = ptrdiff_t;
};

/* iterator traits */
template <typename T>
class NuoHasIteratorCat
{
private:
  struct FalseStruct
  {
    char a, b;
  };

  using true_type = char;
  using false_type = FalseStruct;

public:
  static const bool kValue = sizeof(HasIteratorCat<T>(0)) == sizeof(true_type);

private:
  template <typename U>
  static false_type HasIteratorCat(...);

  template <typename U>
  static true_type HasIteratorCat(typename U::iterator_category* = 0);
};

template <typename Iterator, bool>
class NuoIteratorTraitsImpl {};

template <typename Iterator>
class NuoIteratorTraitsImpl<Iterator, true>
{
public:
  using iterator_category = typename Iterator::iterator_category;
  using value_type        = typename Iterator::value_type;
  using pointer           = typename Iterator::pointer;
  using reference         = typename Iterator::reference;
  using difference_type   = typename Iterator::difference_type;
};

template <typename Iterator, bool>
class NuoIteratorTraitsHelper {};

template <typename Iterator>
class NuoIteratorTraitsHelper<Iterator, true>
  : public NuoIteratorTraitsImpl<Iterator,
      std::is_convertible<
        typename Iterator::iterator_category, NuoInputIteratorTag>::value ||
      std::is_convertible<
        typename Iterator::iterator_category, NuoOutputIteratorTag>::value
    >
{
};

template <typename Iterator>
class NuoIteratorTraits :
  public NuoIteratorTraitsHelper<Iterator, NuoHasIteratorCat<Iterator>::kValue>
{
};

template <typename T>
class NuoIteratorTraits<T*>
{
public:
  using iterator_category = NuoRandomAccessIteratorTag;
  using value_type        = T;
  using pointer           = T*;
  using reference         = T&;
  using difference_type   = ptrdiff_t;
};

template <typename T, typename U, bool = NuoHasIteratorCat<NuoIteratorTraits<T>>::kValue>
class NuoHasIteratorCatOf
  : public NuoBoolConstant<std::is_convertible<
      typename NuoIteratorTraits<T>::iterator_category, U>::value>
{
};

template <typename T, typename U>
class NuoHasIteratorCatOf<T, U, false> : public NuoFalseType {};

/* iterator traits */
template <typename Iter>
class NuoIsExactlyInputIterator :
  public NuoBoolConstant<
      NuoHasIteratorCatOf<Iter, NuoInputIteratorTag>::kValue &&
    !NuoHasIteratorCatOf<Iter, NuoForwardIteratorTag>::kValue
  >
{
};

template <typename Iter>
class NuoIsInputIterator :
  public NuoHasIteratorCatOf<Iter, NuoInputIteratorTag>::kValue
{ 
};

template <typename Iter>
class NuoIsOutputIterator :
  public NuoHasIteratorCatOf<Iter, NuoOutputIteratorTag>::kValue
{
};

template <typename Iter>
class NuoIsForwardIterator :
  public NuoHasIteratorCatOf<Iter, NuoForwardIteratorTag>::kValue
{
};

template <typename Iter>
class NuoIsBidirectionalIterator :
  public NuoHasIteratorCatOf<Iter, NuoBidirectionalIteratorTag>::kValue
{
};

template <typename Iter>
class NuoIsRandomAccessIterator :
  public NuoHasIteratorCatOf<Iter, NuoRandomAccessIteratorTag>::kValue
{
};

/* get iterator category */
template <typename Iterator>
typename NuoIteratorTraits<Iterator>::iterator_category
NuoIteratorCategory(const Iterator&)
{
  using Category = typename NuoIteratorTraits<Iterator>::iterator_category;
  return Category();
}

/* get iterator distance type (difference_type) */
template <typename Iterator>
typename NuoIteratorTraits<Iterator>::difference_type*
NuoDistanceType(const Iterator&)
{
  return static_cast<
    typename NuoIteratorTraits<Iterator>::difference_type*>(nullptr);
}

/* get iterator value type */
template <typename Iterator>
typename NuoIteratorTraits<Iterator>::value_type*
NuoValueType(const Iterator&)
{
  return static_cast<
    typename NuoIteratorTraits<Iterator>::value_type*>(nullptr);
}

/* make iterator forward n steps */
/* input iterator tag version */
template <typename InputIterator, typename Distance>
void NuoAdvanceDispatch(InputIterator& iter, Distance dis, NuoInputIteratorTag)
{
  if (dis < 0)
    return;

  while (dis--)
    iter++;
}

/* bidirectional iterator tag type */
template <typename BidirectionalIterator, typename Distance>
void NuoAdvanceDispatch(
  BidirectionalIterator& iter,
  Distance dis,
  NuoBidirectionalIteratorTag)
{
  if (dis >= 0)
  {
    while (dis--)
      iter++;
  }
  else
  {
    while (dis++)
      iter--;
  }
}

/* random access iterator tag type */
template <typename RandomAccessIterator, typename Distance>
void NuoAdvanceDispatch(
  RandomAccessIterator& iter,
  Distance dis,
  NuoRandomAccessIteratorTag
)
{
  iter += dis;
}

/*****************************************************************************************/
/* reverse iterator */
template <typename Iterator>
class NuoReverseIterator
{
public:
  /* types */
  using iterator_category = typename NuoIteratorTraits<Iterator>::iterator_category;
  using value_type        = typename NuoIteratorTraits<Iterator>::value_type;
  using pointer           = typename NuoIteratorTraits<Iterator>::pointer;
  using reference         = typename NuoIteratorTraits<Iterator>::reference;
  using difference_type   = typename NuoIteratorTraits<Iterator>::difference_type;
  using iterator_type     = Iterator;
  using self              = NuoReverseIterator<Iterator>;

public:
  /* constructor */
  NuoReverseIterator() = default;
  explicit NuoReverseIterator(iterator_type& iter)
    : current_iter_(iter)
  {
  }
  NuoReverseIterator(const self& rhs)
    : current_iter_(rhs.current_iter_)
  {
  }

public:
  iterator_type Base() const
  {
    return current_iter_;
  }

  /* overload operator */
  reference operator*() const
  {
    Iterator tmp_iter = current_iter_;
    return *(--tmp_iter);
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  self& operator++()
  {
    current_iter_--;
    return *this;
  }

  self operator++(int)
  {
    self tmp = *this;
    current_iter_--;
    return tmp;
  }

  self& operator--()
  {
    current_iter_++;
    return *this;
  }

  self operator--(int)
  {
    self tmp = *this;
    current_iter_++;
    return tmp;
  }

  self& operator+=(difference_type n)
  {
    current_iter_ -= n;
    return *this;
  }

  self operator+(difference_type n) const
  {
    return self(current_iter_ - n);
  }

  self& operator-=(difference_type n)
  {
    current_iter_ += n;
    return *this;
  }

  self operator-(difference_type n) const
  {
    return self(current_iter_ + n);
  }

  reference operator[](difference_type n) const
  {
    return *(*this + n);
  }

private:
  Iterator current_iter_; /* current forward iterator */
};

template <typename Iterator>
typename NuoReverseIterator<Iterator>::difference_type
operator-(const NuoReverseIterator<Iterator>& lhs,
          const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() - rhs.base();
}

template <typename Iterator>
bool operator==(const NuoReverseIterator<Iterator>& lhs,
                const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() == rhs.base();
}

template <typename Iterator>
bool operator!=(const NuoReverseIterator<Iterator>& lhs,
                const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() != rhs.base();
}

template <typename Iterator>
bool operator<(const NuoReverseIterator<Iterator>& lhs,
                const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() < rhs.base();
}

template <typename Iterator>
bool operator>(const NuoReverseIterator<Iterator>& lhs,
                const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() > rhs.base();
}

template <typename Iterator>
bool operator<=(const NuoReverseIterator<Iterator>& lhs,
                const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() <= rhs.base();
}

template <typename Iterator>
bool operator>=(const NuoReverseIterator<Iterator>& lhs,
                const NuoReverseIterator<Iterator>& rhs)
{
  return lhs.base() >= rhs.base();
}

} /* namespace nuostl */