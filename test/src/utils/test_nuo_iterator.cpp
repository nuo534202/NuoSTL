#include "utils/test_nuo_iterator.hpp"

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "core/seq_cont/nuo_deque.hpp"
#include "core/seq_cont/nuo_list.hpp"
#include "test_framework.hpp"
#include "test_input_iterator.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace test
{

namespace
{

struct ReverseLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = int*;
  using reference         = int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct ReverseRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct NarrowDifferenceIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = int;

  int id = 0;
};

struct WideDifferenceIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = long long;

  int id = 0;
};

struct UnsignedDifferenceLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::size_t;

  std::size_t id = 0;
};

struct UnsignedDifferenceRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::size_t;

  std::size_t id = 0;
};

struct ReferenceDifferenceLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;
};

struct ReferenceDifferenceRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;
};

struct VoidDifferenceLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;
};

struct VoidDifferenceRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;
};

struct NotEqualLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct NotEqualRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct ReversedNotEqualLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct ReversedNotEqualRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct LeftLessIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct LeftGreaterIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct RightGreaterIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct MirroredLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct MirroredRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct ReversedLessLeftIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct ReversedLessRightIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

struct RightLessIterator
{
  using iterator_category = nuostl::NuoBidirectionalIteratorTag;
  using value_type        = int;
  using pointer           = const int*;
  using reference         = const int&;
  using difference_type   = std::ptrdiff_t;

  int id = 0;
};

bool operator==(const ReverseRightIterator& lhs, const ReverseLeftIterator& rhs)
{
  return lhs.id == rhs.id;
}

long long operator-(const WideDifferenceIterator& lhs,
                    const NarrowDifferenceIterator& rhs)
{
  return lhs.id - rhs.id;
}

std::size_t operator-(const UnsignedDifferenceRightIterator& lhs,
                      const UnsignedDifferenceLeftIterator& rhs)
{
  return lhs.id - rhs.id;
}

bool operator!=(const NotEqualLeftIterator& lhs,
                const NotEqualRightIterator& rhs)
{
  return lhs.id != rhs.id;
}

bool operator!=(const ReversedNotEqualRightIterator& lhs,
                const ReversedNotEqualLeftIterator& rhs)
{
  return lhs.id != rhs.id;
}

bool operator<(const LeftLessIterator& lhs, const RightLessIterator& rhs)
{
  return lhs.id < rhs.id;
}

bool operator<(const RightLessIterator& lhs, const LeftLessIterator& rhs)
{
  return lhs.id < rhs.id;
}

bool operator>(const RightLessIterator& lhs, const LeftLessIterator& rhs)
{
  return lhs.id > rhs.id;
}

bool operator>(const LeftLessIterator& lhs, const RightLessIterator& rhs)
{
  return lhs.id > rhs.id;
}

bool operator>(const LeftGreaterIterator& lhs,
               const RightGreaterIterator& rhs)
{
  return lhs.id > rhs.id;
}

bool operator>(const RightGreaterIterator& lhs,
               const LeftGreaterIterator& rhs)
{
  return lhs.id > rhs.id;
}

bool operator>(const MirroredRightIterator& lhs,
               const MirroredLeftIterator& rhs)
{
  return lhs.id > rhs.id;
}

bool operator<=(const MirroredRightIterator& lhs,
                const MirroredLeftIterator& rhs)
{
  return lhs.id <= rhs.id;
}

bool operator>=(const MirroredRightIterator& lhs,
                const MirroredLeftIterator& rhs)
{
  return lhs.id >= rhs.id;
}

bool operator<(const ReversedLessRightIterator& lhs,
               const ReversedLessLeftIterator& rhs)
{
  return lhs.id < rhs.id;
}

const std::ptrdiff_t& operator-(
    const ReferenceDifferenceRightIterator&,
    const ReferenceDifferenceLeftIterator&)
{
  static const std::ptrdiff_t difference = 0;
  return difference;
}

void operator-(const VoidDifferenceRightIterator&,
               const VoidDifferenceLeftIterator&)
{
}

template <typename LeftIterator, typename RightIterator, typename = void>
struct HasReverseSubtraction : std::false_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseSubtraction<
    LeftIterator,
    RightIterator,
    std::void_t<decltype(
      std::declval<nuostl::NuoReverseIterator<LeftIterator>>() -
      std::declval<nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

enum class ReverseComparison
{
  kEqual,
  kNotEqual,
  kLess,
  kGreater,
  kLessEqual,
  kGreaterEqual,
};

template <ReverseComparison kComparison,
          typename LeftIterator,
          typename RightIterator,
          typename = void>
struct HasReverseComparison : std::false_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseComparison<ReverseComparison::kEqual,
                           LeftIterator,
                           RightIterator,
                           std::void_t<decltype(
                             std::declval<
                               nuostl::NuoReverseIterator<LeftIterator>>() ==
                             std::declval<
                               nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseComparison<ReverseComparison::kNotEqual,
                           LeftIterator,
                           RightIterator,
                           std::void_t<decltype(
                             std::declval<
                               nuostl::NuoReverseIterator<LeftIterator>>() !=
                             std::declval<
                               nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseComparison<ReverseComparison::kLess,
                           LeftIterator,
                           RightIterator,
                           std::void_t<decltype(
                             std::declval<
                               nuostl::NuoReverseIterator<LeftIterator>>() <
                             std::declval<
                               nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseComparison<ReverseComparison::kGreater,
                           LeftIterator,
                           RightIterator,
                           std::void_t<decltype(
                             std::declval<
                               nuostl::NuoReverseIterator<LeftIterator>>() >
                             std::declval<
                               nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseComparison<ReverseComparison::kLessEqual,
                           LeftIterator,
                           RightIterator,
                           std::void_t<decltype(
                             std::declval<
                               nuostl::NuoReverseIterator<LeftIterator>>() <=
                             std::declval<
                               nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

template <typename LeftIterator, typename RightIterator>
struct HasReverseComparison<ReverseComparison::kGreaterEqual,
                           LeftIterator,
                           RightIterator,
                           std::void_t<decltype(
                             std::declval<
                               nuostl::NuoReverseIterator<LeftIterator>>() >=
                             std::declval<
                               nuostl::NuoReverseIterator<RightIterator>>())>>
  : std::true_type
{
};

using MutableReverseIterator = nuostl::NuoReverseIterator<int*>;
using ConstReverseIterator = nuostl::NuoReverseIterator<const int*>;
using ListIterator = nuostl::NuoListIterator<int>;
static_assert(noexcept(std::declval<MutableReverseIterator>() ==
                       std::declval<ConstReverseIterator>()));
static_assert(noexcept(std::declval<MutableReverseIterator>() -
                       std::declval<ConstReverseIterator>()));
static_assert(noexcept(std::declval<MutableReverseIterator>() <=
                       std::declval<ConstReverseIterator>()));
static_assert(!HasReverseComparison<
              ReverseComparison::kEqual,
              ReferenceDifferenceLeftIterator,
              ReferenceDifferenceRightIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kNotEqual,
              ReferenceDifferenceLeftIterator,
              ReferenceDifferenceRightIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kLess,
              ReferenceDifferenceLeftIterator,
              ReferenceDifferenceRightIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kGreater,
              ReferenceDifferenceLeftIterator,
              ReferenceDifferenceRightIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kLessEqual,
              ReferenceDifferenceLeftIterator,
              ReferenceDifferenceRightIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kGreaterEqual,
              ReferenceDifferenceLeftIterator,
              ReferenceDifferenceRightIterator>::value);
static_assert(HasReverseComparison<
              ReverseComparison::kEqual,
              ListIterator,
              ListIterator>::value);
static_assert(HasReverseComparison<
              ReverseComparison::kNotEqual,
              ListIterator,
              ListIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kLess,
              ListIterator,
              ListIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kGreater,
              ListIterator,
              ListIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kLessEqual,
              ListIterator,
              ListIterator>::value);
static_assert(!HasReverseComparison<
              ReverseComparison::kGreaterEqual,
              ListIterator,
              ListIterator>::value);
static_assert(!noexcept(
    std::declval<nuostl::NuoReverseIterator<ReverseLeftIterator>>() ==
    std::declval<nuostl::NuoReverseIterator<ReverseRightIterator>>()));
static_assert(!noexcept(
    std::declval<nuostl::NuoReverseIterator<LeftLessIterator>>() <
    std::declval<nuostl::NuoReverseIterator<RightLessIterator>>()));

} /* namespace */

void TestNuoIterator::TestReverseIteratorOperators()
{
  using Deque = nuostl::NuoDeque<int>;
  static_assert(std::is_same_v<
                typename nuostl::NuoIteratorTraits<const int*>::value_type,
                int>);
  static_assert(std::is_same_v<
                typename nuostl::NuoIteratorTraits<const int*>::reference,
                const int&>);
  static_assert(std::is_same_v<
                typename nuostl::NuoIteratorTraits<const int*>::pointer,
                const int*>);
  static_assert(!std::is_convertible_v<typename Deque::const_reverse_iterator,
                                       typename Deque::reverse_iterator>);
  static_assert(std::is_convertible_v<typename Deque::reverse_iterator,
                                      typename Deque::const_reverse_iterator>);

  Deque deque;
  deque.push_back(3);
  deque.push_back(4);
  typename Deque::reverse_iterator deque_first = deque.Rbegin();
  typename Deque::reverse_iterator deque_second = deque_first;
  ++deque_second;
  typename Deque::const_reverse_iterator const_deque_first = deque.Crbegin();
  NUO_TEST_CHECK(deque_first == const_deque_first);
  NUO_TEST_CHECK(const_deque_first == deque_first);
  NUO_TEST_CHECK(deque_second - deque_first == 1);
  NUO_TEST_CHECK(deque_first < deque_second);
  NUO_TEST_CHECK(deque_first <= deque_second);
  NUO_TEST_CHECK(deque_second > deque_first);
  NUO_TEST_CHECK(deque_second >= deque_first);

  std::array<int, 2> values = {3, 4};
  nuostl::NuoReverseIterator<int*> mutable_reverse(values.data() + 2);
  nuostl::NuoReverseIterator<const int*> const_reverse = mutable_reverse;
  NUO_TEST_CHECK(mutable_reverse == const_reverse);
  NUO_TEST_CHECK(const_reverse == mutable_reverse);
  NUO_TEST_CHECK(!(mutable_reverse != const_reverse));
  NUO_TEST_CHECK(mutable_reverse <= const_reverse);
  NUO_TEST_CHECK(const_reverse >= mutable_reverse);
  NUO_TEST_CHECK(mutable_reverse - const_reverse == 0);

  nuostl::NuoReverseIterator<int*> mutable_front(values.data() + 1);
  nuostl::NuoReverseIterator<const int*> const_back(values.data() + 2);
  NUO_TEST_CHECK(mutable_front != const_back);
  NUO_TEST_CHECK(mutable_front > const_back);
  NUO_TEST_CHECK(const_back < mutable_front);
  NUO_TEST_CHECK(mutable_front >= const_back);
  NUO_TEST_CHECK(const_back <= mutable_front);
  NUO_TEST_CHECK(mutable_front - const_back == 1);

  static_assert(!HasReverseSubtraction<ReferenceDifferenceLeftIterator,
                                       ReferenceDifferenceRightIterator>::value);
  static_assert(!HasReverseSubtraction<VoidDifferenceLeftIterator,
                                       VoidDifferenceRightIterator>::value);

  nuostl::NuoReverseIterator<NarrowDifferenceIterator> narrow_reverse(
      NarrowDifferenceIterator{2});
  nuostl::NuoReverseIterator<WideDifferenceIterator> wide_reverse(
      WideDifferenceIterator{1});
  auto reverse_difference = narrow_reverse - wide_reverse;
  static_assert(std::is_same_v<decltype(reverse_difference), long long>);
  NUO_TEST_CHECK(reverse_difference == -1);

  nuostl::NuoReverseIterator<UnsignedDifferenceLeftIterator>
      unsigned_left(UnsignedDifferenceLeftIterator{1});
  nuostl::NuoReverseIterator<UnsignedDifferenceRightIterator>
      unsigned_right(UnsignedDifferenceRightIterator{3});
  auto unsigned_difference = unsigned_left - unsigned_right;
  static_assert(std::is_same_v<decltype(unsigned_difference), std::size_t>);
  NUO_TEST_CHECK(unsigned_difference == std::size_t{2});

  nuostl::NuoReverseIterator<ReverseLeftIterator> asymmetric_left(
      ReverseLeftIterator{3});
  nuostl::NuoReverseIterator<ReverseRightIterator> asymmetric_right(
      ReverseRightIterator{3});
  NUO_TEST_CHECK(asymmetric_left == asymmetric_right);
  NUO_TEST_CHECK(asymmetric_right == asymmetric_left);
  NUO_TEST_CHECK(!(asymmetric_left != asymmetric_right));

  nuostl::NuoReverseIterator<ReverseRightIterator> different_right(
      ReverseRightIterator{4});
  NUO_TEST_CHECK(asymmetric_left != different_right);
  NUO_TEST_CHECK(different_right != asymmetric_left);

  /* These types expose one equality direction to exercise explicit fallbacks. */
  nuostl::NuoReverseIterator<NotEqualLeftIterator> not_equal_left(
      NotEqualLeftIterator{7});
  nuostl::NuoReverseIterator<NotEqualRightIterator> not_equal_right(
      NotEqualRightIterator{7});
  NUO_TEST_CHECK(not_equal_left == not_equal_right);
  NUO_TEST_CHECK(!(not_equal_left != not_equal_right));
  nuostl::NuoReverseIterator<NotEqualRightIterator> different_not_equal_right(
      NotEqualRightIterator{8});
  NUO_TEST_CHECK(not_equal_left != different_not_equal_right);

  nuostl::NuoReverseIterator<ReversedNotEqualLeftIterator> reversed_left(
      ReversedNotEqualLeftIterator{5});
  nuostl::NuoReverseIterator<ReversedNotEqualRightIterator> reversed_right(
      ReversedNotEqualRightIterator{5});
  NUO_TEST_CHECK(reversed_left == reversed_right);
  NUO_TEST_CHECK(!(reversed_left != reversed_right));
  nuostl::NuoReverseIterator<ReversedNotEqualRightIterator>
      different_reversed_right(ReversedNotEqualRightIterator{6});
  NUO_TEST_CHECK(reversed_left != different_reversed_right);

  nuostl::NuoReverseIterator<LeftLessIterator> left_one(LeftLessIterator{1});
  nuostl::NuoReverseIterator<RightLessIterator> right_two(
      RightLessIterator{2});
  NUO_TEST_CHECK(left_one > right_two);
  NUO_TEST_CHECK(!(left_one <= right_two));

  nuostl::NuoReverseIterator<RightLessIterator> right_one(
      RightLessIterator{1});
  nuostl::NuoReverseIterator<LeftLessIterator> left_two(LeftLessIterator{2});
  NUO_TEST_CHECK(!(right_one < left_two));
  NUO_TEST_CHECK(right_one >= left_two);

  NUO_TEST_CHECK(left_one >= right_two);

  nuostl::NuoReverseIterator<LeftLessIterator> mixed_left_two(
      LeftLessIterator{2});
  nuostl::NuoReverseIterator<RightLessIterator> mixed_right_one(
      RightLessIterator{1});
  NUO_TEST_CHECK(mixed_left_two <= mixed_right_one);
  NUO_TEST_CHECK(!(mixed_left_two >= mixed_right_one));

  nuostl::NuoReverseIterator<LeftGreaterIterator> greater_left_one(
      LeftGreaterIterator{1});
  nuostl::NuoReverseIterator<RightGreaterIterator> greater_right_two(
      RightGreaterIterator{2});
  NUO_TEST_CHECK(!(greater_left_one <= greater_right_two));
  NUO_TEST_CHECK(greater_left_one >= greater_right_two);

  nuostl::NuoReverseIterator<LeftGreaterIterator> greater_left_two(
      LeftGreaterIterator{2});
  nuostl::NuoReverseIterator<RightGreaterIterator> greater_right_one(
      RightGreaterIterator{1});
  NUO_TEST_CHECK(greater_left_two <= greater_right_one);
  NUO_TEST_CHECK(!(greater_left_two >= greater_right_one));

  nuostl::NuoReverseIterator<MirroredLeftIterator> mirrored_left(
      MirroredLeftIterator{1});
  nuostl::NuoReverseIterator<MirroredRightIterator> mirrored_right(
      MirroredRightIterator{2});
  NUO_TEST_CHECK(mirrored_left > mirrored_right);
  NUO_TEST_CHECK(!(mirrored_left <= mirrored_right));
  NUO_TEST_CHECK(mirrored_left >= mirrored_right);

  nuostl::NuoReverseIterator<ReversedLessLeftIterator> reversed_less_left(
      ReversedLessLeftIterator{2});
  nuostl::NuoReverseIterator<ReversedLessRightIterator> reversed_less_right(
      ReversedLessRightIterator{1});
  NUO_TEST_CHECK(reversed_less_left < reversed_less_right);
}

void TestNuoIterator::TestSinglePassCursor()
{
  static_assert(
      std::is_default_constructible_v<SinglePassInputIterator<int>>);
  SinglePassInputIterator<int> default_first;
  SinglePassInputIterator<int> default_last;
  NUO_TEST_CHECK(default_first == default_last);
  NUO_TEST_CHECK(!(default_first != default_last));

  constexpr std::array<int, 2> values = {8, 9};
  /* Rvalues must copy the shared cursor instead of transferring it. */
  SinglePassInputIterator<int> source(values.data());
  SinglePassInputIterator<int> copy(nuostl::NuoMove(source));
  NUO_TEST_CHECK(source == copy);
  NUO_TEST_CHECK(*source == 8);
  ++copy;
  NUO_TEST_CHECK(*source == 9);

  SinglePassInputIterator<int> first(values.data());
  SinglePassInputIterator<int> last(values.data() + values.size());
  NUO_TEST_CHECK(nuostl::NuoDistance(first, last) == 2);
  NUO_TEST_CHECK(first == last);

  SinglePassInputIterator<int> cursor(values.data());
  SinglePassInputIterator<int>::PostIncrementProxy proxy = cursor++;
  NUO_TEST_CHECK(*proxy == 8);
  NUO_TEST_CHECK(*cursor == 9);
}

void TestNuoIterator::TestSuite()
{
  TestReverseIteratorOperators();
  TestSinglePassCursor();
}

} /* namespace test */
