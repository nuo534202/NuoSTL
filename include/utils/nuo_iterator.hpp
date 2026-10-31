#pragma once

#include <stddef.h>
#include <type_traits>
#include <utility>

#include "utils/nuo_type_traits.hpp"

namespace nuostl
{

/* 5 iterator tags */
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
  template <typename U>
  static false_type HasIteratorCat(...);

  template <typename U>
  static true_type HasIteratorCat(typename U::iterator_category* = 0);

  static const bool kValue = sizeof(HasIteratorCat<T>(0)) == sizeof(true_type);
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
  using value_type        = typename std::remove_cv<T>::type;
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
  public NuoHasIteratorCatOf<Iter, NuoInputIteratorTag>
{ 
};

template <typename Iter>
class NuoIsOutputIterator :
  public NuoHasIteratorCatOf<Iter, NuoOutputIteratorTag>
{
};

template <typename Iter>
class NuoIsForwardIterator :
  public NuoHasIteratorCatOf<Iter, NuoForwardIteratorTag>
{
};

template <typename Iter>
class NuoIsBidirectionalIterator :
  public NuoHasIteratorCatOf<Iter, NuoBidirectionalIteratorTag>
{
};

template <typename Iter>
class NuoIsRandomAccessIterator :
  public NuoHasIteratorCatOf<Iter, NuoRandomAccessIteratorTag>
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

/* calculate the distance of iterator */
template <typename InputIterator>
typename NuoIteratorTraits<InputIterator>::difference_type
NuoDistanceDispatch(
  InputIterator first,
  InputIterator last,
  NuoInputIteratorTag)
{
  typename NuoIteratorTraits<InputIterator>::difference_type n = 0;

  while (first != last)
    first++, n++;

  return n;
}

template <typename RandomIterator>
typename NuoIteratorTraits<RandomIterator>::difference_type
NuoDistanceDispatch(
  RandomIterator first,
  RandomIterator last,
  NuoRandomAccessIteratorTag)
{
  return last - first;
}

template <typename InputIterator>
typename NuoIteratorTraits<InputIterator>::difference_type
NuoDistance(InputIterator first, InputIterator last)
{
  return NuoDistanceDispatch(first, last, NuoIteratorCategory(first));
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

template <typename InputIterator, typename Distance>
void NuoAdvance(InputIterator& iter, Distance dis)
{
  NuoAdvanceDispatch(iter, dis, NuoIteratorCategory(iter));
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
  explicit NuoReverseIterator(const iterator_type& iter)
    : current_iter_(iter)
  {
  }
  NuoReverseIterator(const self& rhs)
    : current_iter_(rhs.current_iter_)
  {
  }
  template <typename OtherIterator,
            typename std::enable_if<
              std::is_convertible<OtherIterator, Iterator>::value &&
              std::is_convertible<
                typename NuoIteratorTraits<OtherIterator>::reference,
                typename NuoIteratorTraits<Iterator>::reference>::value &&
              std::is_convertible<
                typename NuoIteratorTraits<OtherIterator>::pointer,
                typename NuoIteratorTraits<Iterator>::pointer>::value,
              int>::type = 0>
  NuoReverseIterator(const NuoReverseIterator<OtherIterator>& rhs)
    noexcept(noexcept(rhs.Base()) &&
             std::is_nothrow_constructible_v<Iterator, OtherIterator>)
    : current_iter_(rhs.Base())
  {
  }

public:
  iterator_type Base() const
    noexcept(std::is_nothrow_copy_constructible_v<iterator_type>)
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

namespace detail
{

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoHasIteratorLeftEquality : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorLeftEquality<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<LeftIterator>() ==
      std::declval<RightIterator>())>>
  : public NuoBoolConstant<std::is_convertible<
      decltype(std::declval<LeftIterator>() ==
               std::declval<RightIterator>()),
      bool>::value>
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorRightEquality
  : public NuoHasIteratorLeftEquality<
      RightIterator, LeftIterator>
{
};

template <typename LeftIterator, typename RightIterator>
class NuoIsIteratorEqualityComparable
  : public NuoBoolConstant<
      NuoHasIteratorLeftEquality<
        LeftIterator, RightIterator>::kValue ||
      NuoHasIteratorRightEquality<
        LeftIterator, RightIterator>::kValue>
{
};

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoHasIteratorLeftInequality : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorLeftInequality<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<LeftIterator>() !=
      std::declval<RightIterator>())>>
  : public NuoBoolConstant<std::is_convertible<
      decltype(std::declval<LeftIterator>() !=
               std::declval<RightIterator>()),
      bool>::value>
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorRightInequality
  : public NuoHasIteratorLeftInequality<
      RightIterator, LeftIterator>
{
};

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoHasIteratorLess : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorLess<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<LeftIterator>() <
      std::declval<RightIterator>())>>
  : public NuoBoolConstant<std::is_convertible<
      decltype(std::declval<LeftIterator>() <
               std::declval<RightIterator>()),
      bool>::value>
{
};

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoHasIteratorGreater : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorGreater<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<LeftIterator>() >
      std::declval<RightIterator>())>>
  : public NuoBoolConstant<std::is_convertible<
      decltype(std::declval<LeftIterator>() >
               std::declval<RightIterator>()),
      bool>::value>
{
};

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoHasIteratorLessEqual : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoHasIteratorGreaterEqual : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorGreaterEqual<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<LeftIterator>() >=
      std::declval<RightIterator>())>>
  : public NuoBoolConstant<std::is_convertible<
      decltype(std::declval<LeftIterator>() >=
               std::declval<RightIterator>()),
      bool>::value>
{
};

template <typename LeftIterator, typename RightIterator>
class NuoHasIteratorLessEqual<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<LeftIterator>() <=
      std::declval<RightIterator>())>>
  : public NuoBoolConstant<std::is_convertible<
      decltype(std::declval<LeftIterator>() <=
               std::declval<RightIterator>()),
      bool>::value>
{
};

template <typename LeftIterator, typename RightIterator>
using NuoIteratorRightMinusLeftResult = decltype(
  std::declval<RightIterator>() - std::declval<LeftIterator>());

template <typename LeftIterator, typename RightIterator, typename = void>
class NuoIsIteratorDifferenceAvailable : public NuoFalseType
{
};

template <typename LeftIterator, typename RightIterator>
class NuoIsIteratorDifferenceAvailable<
    LeftIterator,
    RightIterator,
    std::void_t<NuoIteratorRightMinusLeftResult<
      LeftIterator, RightIterator>>>
  /* Reject results that cannot represent a valid iterator distance. */
  : public NuoBoolConstant<
      !std::is_void_v<NuoIteratorRightMinusLeftResult<
        LeftIterator, RightIterator>> &&
      !std::is_reference_v<NuoIteratorRightMinusLeftResult<
        LeftIterator, RightIterator>>>
{
};

} /* namespace detail */

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoIsIteratorDifferenceAvailable<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
/* Mirror the standard heterogeneous reverse_iterator subtraction contract. */
decltype(std::declval<RightIterator>() -
         std::declval<LeftIterator>())
operator-(const NuoReverseIterator<LeftIterator>& lhs,
          const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() - lhs.Base()))
{
  return rhs.Base() - lhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoHasIteratorLeftEquality<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator==(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() == rhs.Base()))
{
  return lhs.Base() == rhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoIsIteratorEqualityComparable<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorLeftInequality<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator==(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() != rhs.Base()))
{
  return !(lhs.Base() != rhs.Base());
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoIsIteratorEqualityComparable<
              LeftIterator, RightIterator>::kValue &&
            !detail::NuoHasIteratorLeftInequality<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorRightInequality<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator==(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() != lhs.Base()))
{
  return !(rhs.Base() != lhs.Base());
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoHasIteratorLeftInequality<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator!=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() != rhs.Base()))
{
  return lhs.Base() != rhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorLeftInequality<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorRightInequality<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator!=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() != lhs.Base()))
{
  return rhs.Base() != lhs.Base();
}

/* C++20 rewrites != expressions, while relational directions remain
   distinct. Strict comparisons require a strict base comparison. */
template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoHasIteratorGreater<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator<(const NuoReverseIterator<LeftIterator>& lhs,
               const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() > rhs.Base()))
{
  return lhs.Base() > rhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorGreater<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorLess<
              RightIterator, LeftIterator>::kValue,
            int>::type = 0>
bool operator<(const NuoReverseIterator<LeftIterator>& lhs,
               const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() < lhs.Base()))
{
  return rhs.Base() < lhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoHasIteratorLess<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator>(const NuoReverseIterator<LeftIterator>& lhs,
               const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() < rhs.Base()))
{
  return lhs.Base() < rhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorLess<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorGreater<
              RightIterator, LeftIterator>::kValue,
            int>::type = 0>
bool operator>(const NuoReverseIterator<LeftIterator>& lhs,
               const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() > lhs.Base()))
{
  return rhs.Base() > lhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoHasIteratorGreaterEqual<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator<=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() >= rhs.Base()))
{
  return lhs.Base() >= rhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorGreaterEqual<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorLessEqual<
              RightIterator, LeftIterator>::kValue,
            int>::type = 0>
bool operator<=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() <= lhs.Base()))
{
  return rhs.Base() <= lhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorGreaterEqual<
              LeftIterator, RightIterator>::kValue &&
            !detail::NuoHasIteratorLessEqual<
              RightIterator, LeftIterator>::kValue &&
            detail::NuoHasIteratorLess<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator<=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() < rhs.Base()))
{
  return !(lhs.Base() < rhs.Base());
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorGreaterEqual<
              LeftIterator, RightIterator>::kValue &&
            !detail::NuoHasIteratorLessEqual<
              RightIterator, LeftIterator>::kValue &&
            !detail::NuoHasIteratorLess<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorGreater<
              RightIterator, LeftIterator>::kValue,
            int>::type = 0>
bool operator<=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() > lhs.Base()))
{
  return !(rhs.Base() > lhs.Base());
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            detail::NuoHasIteratorLessEqual<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator>=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() <= rhs.Base()))
{
  return lhs.Base() <= rhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorLessEqual<
              LeftIterator, RightIterator>::kValue &&
            detail::NuoHasIteratorGreaterEqual<
              RightIterator, LeftIterator>::kValue,
            int>::type = 0>
bool operator>=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() >= lhs.Base()))
{
  return rhs.Base() >= lhs.Base();
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorLessEqual<
              LeftIterator, RightIterator>::kValue &&
            !detail::NuoHasIteratorGreaterEqual<
              RightIterator, LeftIterator>::kValue &&
            detail::NuoHasIteratorLess<
              RightIterator, LeftIterator>::kValue,
            int>::type = 0>
bool operator>=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(rhs.Base() < lhs.Base()))
{
  return !(rhs.Base() < lhs.Base());
}

template <typename LeftIterator, typename RightIterator,
          typename std::enable_if<
            !detail::NuoHasIteratorLessEqual<
              LeftIterator, RightIterator>::kValue &&
            !detail::NuoHasIteratorGreaterEqual<
              RightIterator, LeftIterator>::kValue &&
            !detail::NuoHasIteratorLess<
              RightIterator, LeftIterator>::kValue &&
            detail::NuoHasIteratorGreater<
              LeftIterator, RightIterator>::kValue,
            int>::type = 0>
bool operator>=(const NuoReverseIterator<LeftIterator>& lhs,
                const NuoReverseIterator<RightIterator>& rhs)
  noexcept(noexcept(lhs.Base() > rhs.Base()))
{
  return !(lhs.Base() > rhs.Base());
}

} /* namespace nuostl */
