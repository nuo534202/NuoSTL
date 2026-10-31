#include "core/seq_cont/test_nuo_list.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "core/seq_cont/nuo_list.hpp"
#include "test_input_iterator.hpp"
#include "test_framework.hpp"

namespace test
{

namespace
{

static_assert(std::is_nothrow_move_constructible_v<nuostl::NuoList<int>>);
static_assert(std::is_nothrow_move_assignable_v<nuostl::NuoList<int>>);

struct BidirectionalListValue
{
  static inline int live_count = 0;
  static inline int copies_before_throw = -1;

  BidirectionalListValue();
  BidirectionalListValue(const BidirectionalListValue& other);
  ~BidirectionalListValue();
};

BidirectionalListValue::BidirectionalListValue()
{
  ++live_count;
}

BidirectionalListValue::BidirectionalListValue(const BidirectionalListValue&)
{
  if (copies_before_throw == 0)
  {
    throw std::runtime_error("copy failed");
  }
  if (copies_before_throw > 0)
  {
    --copies_before_throw;
  }
  ++live_count;
}

BidirectionalListValue::~BidirectionalListValue()
{
  --live_count;
}

struct MoveTrackingValue
{
  static inline int copies = 0;
  static inline int moves = 0;

  MoveTrackingValue() = default;
  explicit MoveTrackingValue(int value) : value(value)
  {
  }

  MoveTrackingValue(const MoveTrackingValue& other) : value(other.value)
  {
    ++copies;
  }

  MoveTrackingValue(MoveTrackingValue&& other) noexcept : value(other.value)
  {
    ++moves;
  }

  int value = 0;
};

struct MultiArgumentValue
{
  MultiArgumentValue(int value, const std::string& label)
    : value(value)
    , label(label)
  {
  }

  int value;
  std::string label;
};

class CopiesBeforeThrowGuard
{
public:
  explicit CopiesBeforeThrowGuard(int copies)
    : previous_(BidirectionalListValue::copies_before_throw)
  {
    BidirectionalListValue::copies_before_throw = copies;
  }

  ~CopiesBeforeThrowGuard()
  {
    BidirectionalListValue::copies_before_throw = previous_;
  }

private:
  int previous_;
};

template <typename Iterator>
std::vector<typename nuostl::NuoIteratorTraits<Iterator>::value_type>
RangeValues(Iterator first, Iterator last)
{
  /* Use only for plain values outside exception-count scopes. */
  using Value = typename nuostl::NuoIteratorTraits<Iterator>::value_type;
  static_assert(std::is_same_v<Value, int>,
                "RangeValues only supports int test values.");
  std::vector<Value> values;
  for (; first != last; ++first)
  {
    values.push_back(*first);
  }
  return values;
}

std::vector<int> ListValues(const nuostl::NuoList<int>& list)
{
  return RangeValues(list.Cbegin(), list.Cend());
}

std::vector<int> ReverseListValues(const nuostl::NuoList<int>& list)
{
  return RangeValues(list.Crbegin(), list.Crend());
}

template <typename Iterator, typename Value>
void CheckIteratorValue(Iterator iterator, Iterator end, const Value& expected)
{
  NUO_TEST_CHECK(iterator != end);
  if (iterator != end)
  {
    NUO_TEST_CHECK(*iterator == expected);
  }
}

constexpr std::array<int, 2> kSourceValues = {6, 7};

template <typename Operation>
void ExpectListInsertFailure(nuostl::NuoList<BidirectionalListValue>& list,
                             int copies_before_failure,
                             Operation&& operation)
{
  const auto size_before = list.Size();
  const int live_before = BidirectionalListValue::live_count;
  std::vector<BidirectionalListValue*> surviving;
  for (auto it = list.Begin(); it != list.End(); ++it)
  {
    surviving.push_back(&*it);
  }

  CopiesBeforeThrowGuard guard(copies_before_failure);
  bool caught = false;
  try
  {
    nuostl::NuoForward<Operation>(operation)();
  }
  catch (const std::runtime_error& error)
  {
    caught = true;
    NUO_TEST_CHECK(std::string(error.what()) == "copy failed");
  }

  NUO_TEST_CHECK(caught);
  if (!caught)
    return;
  NUO_TEST_CHECK(list.Size() == size_before);
  NUO_TEST_CHECK(BidirectionalListValue::live_count == live_before);
  if (list.Size() == size_before)
  {
    std::size_t index = 0;
    for (auto it = list.Begin(); it != list.End(); ++it, ++index)
    {
      NUO_TEST_CHECK(index < surviving.size());
      if (index < surviving.size())
      {
        NUO_TEST_CHECK(&*it == surviving[index]);
      }
    }
    NUO_TEST_CHECK(index == surviving.size());
  }
}

} /* namespace */

void TestNuoList::TestResourceLifetime()
{
  using List = nuostl::NuoList<BidirectionalListValue>;
  {
    BidirectionalListValue value;
    List list(3, value);
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 4);
    list.PopFront();
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 3);
    list.PopBack();
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 2);
    list.Clear();
    list.Clear();
    NUO_TEST_CHECK(list.Empty());
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 1);
    list.PushBack(value);
    List source(3, value);
    list = source;
    NUO_TEST_CHECK(list.Size() == 3);
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 7);

    BidirectionalListValue::copies_before_throw = 1;
    bool caught = false;
    try
    {
      list = source;
    }
    catch (const std::runtime_error&)
    {
      caught = true;
    }
    NUO_TEST_CHECK(caught);
    NUO_TEST_CHECK(list.Empty());
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 4);
    BidirectionalListValue::copies_before_throw = -1;
    list.PushBack(value);

    BidirectionalListValue::copies_before_throw = 1;
    caught = false;
    try
    {
      List failing(3, value);
    }
    catch (const std::runtime_error&)
    {
      caught = true;
    }
    NUO_TEST_CHECK(caught);
    NUO_TEST_CHECK(BidirectionalListValue::live_count == 5);
    BidirectionalListValue::copies_before_throw = -1;

    ExpectListInsertFailure(list, 1, [&]()
    {
      list.Insert(list.Cbegin(), 3, value);
    });

    const int live_before_range_source = BidirectionalListValue::live_count;
    List range_source(3, value);
    NUO_TEST_CHECK(BidirectionalListValue::live_count ==
                   live_before_range_source + 3);
    ExpectListInsertFailure(list, 1, [&]()
    {
      list.Insert(list.Begin(), range_source.Begin(), range_source.End());
    });

    ExpectListInsertFailure(list, 0, [&]()
    {
      list.Insert(list.Cbegin(), value);
    });

    std::array<BidirectionalListValue, 3> input_values{};
    SinglePassInputIterator<BidirectionalListValue> input_first(
        input_values.data());
    SinglePassInputIterator<BidirectionalListValue> input_last(
        input_values.data() + input_values.size());
    auto operation_token = std::make_unique<int>(0);
    ExpectListInsertFailure(
        list, 1,
        [&, token = nuostl::NuoMove(operation_token)]() mutable
        {
          NUO_TEST_CHECK(*token == 0);
          list.Insert(list.Cbegin(), input_first, input_last);
        });

    int forward_count = 0;
    for (List::const_iterator it = list.Cbegin(); it != list.Cend(); ++it)
    {
      ++forward_count;
    }
    int reverse_count = 0;
    for (List::const_reverse_iterator it = list.Crbegin();
         it != list.Crend(); ++it)
    {
      ++reverse_count;
    }
    NUO_TEST_CHECK(forward_count == 1);
    NUO_TEST_CHECK(reverse_count == 1);
  }
  NUO_TEST_CHECK(BidirectionalListValue::live_count == 0);
  nuostl::NuoList<std::string> strings(4, std::string(200, 'x'));
  strings.Clear();
  strings.PushBack(std::string(300, 'y'));
}

void TestNuoList::test_constructor()
{
  /* default constructor */
  nuostl::NuoList<int> l1;
  NUO_TEST_CHECK(l1.Empty());
  NUO_TEST_CHECK(l1.Size() == 0);
  NUO_TEST_CHECK(l1.Begin() == l1.End());
  NUO_TEST_CHECK(l1.Rbegin() == l1.Rend());

  /* fill constructor */
  nuostl::NuoList<int> l2(3, 7);
  NUO_TEST_CHECK(!l2.Empty());
  NUO_TEST_CHECK(l2.Size() == 3);
  NUO_TEST_CHECK(l2.Front() == 7);
  NUO_TEST_CHECK(l2.Back() == 7);

  /* initializer-list constructor */
  nuostl::NuoList<int> l3{9, 8, 7};
  NUO_TEST_CHECK(l3.Size() == 3);
  NUO_TEST_CHECK(l3.Front() == 9);
  NUO_TEST_CHECK(l3.Back() == 7);

  /* initializer-list constructor with string */
  nuostl::NuoList<std::string> l4{"a", "b"};
  NUO_TEST_CHECK(l4.Size() == 2);
  NUO_TEST_CHECK(l4.Front() == "a");
  NUO_TEST_CHECK(l4.Back() == "b");
}

void TestNuoList::test_assign()
{
  nuostl::NuoList<int> l1;

  /* assign n copies */
  l1.Assign(4, 5);
  NUO_TEST_CHECK(l1.Size() == 4);
  NUO_TEST_CHECK(l1.Front() == 5);
  NUO_TEST_CHECK(l1.Back() == 5);

  /* assign by initializer list */
  l1.Assign({1, 3, 5, 7});
  NUO_TEST_CHECK(l1.Size() == 4);
  NUO_TEST_CHECK(l1.Front() == 1);
  NUO_TEST_CHECK(l1.Back() == 7);

  /* assign one element */
  l1.Assign(1, 42);
  NUO_TEST_CHECK(l1.Size() == 1);
  NUO_TEST_CHECK(l1.Front() == 42);
  NUO_TEST_CHECK(l1.Back() == 42);
}

void TestNuoList::test_iterator_and_access()
{
  nuostl::NuoList<int> l1{1, 2, 3, 4, 5};

  NUO_TEST_CHECK(l1.Begin() != l1.End());
  NUO_TEST_CHECK(!(l1.Begin() != l1.Begin()));
  int total = 0;
  for (auto it = l1.Begin(); it != l1.End(); ++it)
  {
    total += *it;
  }
  NUO_TEST_CHECK(total == 15);

  NUO_TEST_CHECK(l1.Front() == 1);
  NUO_TEST_CHECK(l1.Back() == 5);

  l1.Front() = 11;
  l1.Back() = 55;
  NUO_TEST_CHECK(l1.Front() == 11);
  NUO_TEST_CHECK(l1.Back() == 55);

  const nuostl::NuoList<int> l2{6, 7, 8};
  NUO_TEST_CHECK(l2.Begin() != l2.End());
  NUO_TEST_CHECK(!(l2.Begin() != l2.Begin()));
  total = 0;
  for (auto it = l2.Begin(); it != l2.End(); ++it)
  {
    total += *it;
  }
  NUO_TEST_CHECK(total == 21);
  NUO_TEST_CHECK(l2.Front() == 6);
  NUO_TEST_CHECK(l2.Back() == 8);
}

void TestNuoList::test_iterator_conversion()
{
  nuostl::NuoList<int> node_list{17};
  nuostl::NuoList<int>::const_iterator converted = node_list.Begin();
  NUO_TEST_CHECK(*converted == 17);
  NUO_TEST_CHECK(node_list.Begin() == node_list.Cbegin());
  NUO_TEST_CHECK(node_list.Cbegin() == node_list.Begin());
  NUO_TEST_CHECK(!(node_list.Begin() != node_list.Cbegin()));
  NUO_TEST_CHECK(!(node_list.Cbegin() != node_list.Begin()));
  NUO_TEST_CHECK(node_list.End() == node_list.Cend());
  NUO_TEST_CHECK(node_list.Cend() == node_list.End());
  NUO_TEST_CHECK(node_list.Cend() != node_list.Begin());
  NUO_TEST_CHECK(node_list.Rbegin() != node_list.Crend());
  NUO_TEST_CHECK(node_list.Rend() == node_list.Crend());
  NUO_TEST_CHECK(node_list.Crend() == node_list.Rend());
  NUO_TEST_CHECK(node_list.Crbegin() == node_list.Rbegin());
  nuostl::NuoList<int>::const_reverse_iterator converted_reverse =
      node_list.Rbegin();
  NUO_TEST_CHECK(*converted_reverse == 17);
}

void TestNuoList::test_emplace_positions()
{
  nuostl::NuoList<int> list;
  nuostl::NuoList<int>::iterator inserted = list.Emplace(list.Cend(), 2);
  CheckIteratorValue(inserted, list.End(), 2);
  NUO_TEST_CHECK(ListValues(list) == std::vector<int>{2});
  NUO_TEST_CHECK(list.Size() == 1);

  inserted = list.Emplace(list.Cbegin(), 1);
  CheckIteratorValue(inserted, list.End(), 1);
  NUO_TEST_CHECK(ListValues(list) == (std::vector<int>{1, 2}));
  NUO_TEST_CHECK(list.Size() == 2);

  nuostl::NuoList<int>::const_iterator middle = list.Cbegin();
  ++middle;
  inserted = list.Emplace(middle, 15);
  CheckIteratorValue(inserted, list.End(), 15);
  NUO_TEST_CHECK(ListValues(list) == (std::vector<int>{1, 15, 2}));
  NUO_TEST_CHECK(list.Size() == 3);

  inserted = list.Emplace(list.Cend(), 3);
  CheckIteratorValue(inserted, list.End(), 3);
  NUO_TEST_CHECK(ListValues(list) == (std::vector<int>{1, 15, 2, 3}));
  NUO_TEST_CHECK(list.Size() == 4);

  inserted = list.Emplace(middle, 14);
  CheckIteratorValue(inserted, list.End(), 14);
  inserted = list.Emplace(middle, 13);
  CheckIteratorValue(inserted, list.End(), 13);
  NUO_TEST_CHECK(ListValues(list) ==
                (std::vector<int>{1, 15, 14, 13, 2, 3}));
  NUO_TEST_CHECK(list.Size() == 6);

  int& back = list.EmplaceBack(4);
  NUO_TEST_CHECK(back == 4);
  NUO_TEST_CHECK(&back == &list.Back());
  list.EmplaceBack(5);
  NUO_TEST_CHECK(ListValues(list) ==
                (std::vector<int>{1, 15, 14, 13, 2, 3, 4, 5}));
  NUO_TEST_CHECK(list.Size() == 8);
  NUO_TEST_CHECK(ReverseListValues(list) ==
                (std::vector<int>{5, 4, 3, 2, 13, 14, 15, 1}));

  nuostl::NuoList<int> empty_back;
  int& empty_back_value = empty_back.EmplaceBack(9);
  NUO_TEST_CHECK(empty_back_value == 9);
  NUO_TEST_CHECK(&empty_back_value == &empty_back.Back());
  NUO_TEST_CHECK(ListValues(empty_back) == std::vector<int>{9});
  NUO_TEST_CHECK(empty_back.Size() == 1);

  nuostl::NuoList<int> empty_front;
  int& empty_front_value = empty_front.EmplaceFront(8);
  NUO_TEST_CHECK(empty_front_value == 8);
  NUO_TEST_CHECK(&empty_front_value == &empty_front.Front());
  NUO_TEST_CHECK(ListValues(empty_front) == std::vector<int>{8});
  NUO_TEST_CHECK(empty_front.Size() == 1);

  nuostl::NuoList<int> emplace_edges{2};
  int& front_value = emplace_edges.EmplaceFront(1);
  NUO_TEST_CHECK(front_value == 1);
  NUO_TEST_CHECK(&front_value == &emplace_edges.Front());
  NUO_TEST_CHECK(ListValues(emplace_edges) == (std::vector<int>{1, 2}));
  NUO_TEST_CHECK(emplace_edges.Size() == 2);

  nuostl::NuoList<MultiArgumentValue> forwarded_values;
  const std::string forwarded_label = "forwarded";
  nuostl::NuoList<MultiArgumentValue>::iterator forwarded =
      forwarded_values.Emplace(forwarded_values.Cend(), 11, forwarded_label);
  NUO_TEST_CHECK(forwarded != forwarded_values.End());
  if (forwarded != forwarded_values.End())
  {
    NUO_TEST_CHECK(forwarded->value == 11);
    NUO_TEST_CHECK(forwarded->label == forwarded_label);
  }
  NUO_TEST_CHECK(forwarded_values.Size() == 1);
}

void TestNuoList::test_insert_overloads()
{
  nuostl::NuoList<int> mutable_position_list{2};
  nuostl::NuoList<int>::const_iterator converted =
      mutable_position_list.Begin();
  NUO_TEST_CHECK(*converted == 2);
  nuostl::NuoList<int>::iterator inserted =
      mutable_position_list.Insert(mutable_position_list.Begin(), 1);
  CheckIteratorValue(inserted, mutable_position_list.End(), 1);
  inserted = mutable_position_list.Emplace(mutable_position_list.Begin(), 0);
  CheckIteratorValue(inserted, mutable_position_list.End(), 0);
  NUO_TEST_CHECK(ListValues(mutable_position_list) ==
                (std::vector<int>{0, 1, 2}));
  NUO_TEST_CHECK(mutable_position_list.Size() == 3);

  nuostl::NuoList<int> counted{5};
  inserted = counted.Insert(counted.Cbegin(), 2, 7);
  CheckIteratorValue(inserted, counted.End(), 7);
  NUO_TEST_CHECK(inserted == counted.Begin());
  NUO_TEST_CHECK(ListValues(counted) == (std::vector<int>{7, 7, 5}));
  NUO_TEST_CHECK(counted.Size() == 3);

  nuostl::NuoList<int> zero_counted{5};
  inserted = zero_counted.Insert(zero_counted.Cend(), 0, 9);
  NUO_TEST_CHECK(inserted == zero_counted.End());
  NUO_TEST_CHECK(ListValues(zero_counted) == std::vector<int>{5});
  NUO_TEST_CHECK(zero_counted.Size() == 1);

  nuostl::NuoList<int> ranged{5};
  inserted = ranged.Insert(ranged.Cbegin(), kSourceValues.data(),
                           kSourceValues.data() + kSourceValues.size());
  CheckIteratorValue(inserted, ranged.End(), 6);
  NUO_TEST_CHECK(ListValues(ranged) == (std::vector<int>{6, 7, 5}));
  NUO_TEST_CHECK(ranged.Size() == 3);

  nuostl::NuoList<int> tail_counted{5};
  inserted = tail_counted.Insert(tail_counted.Cend(), 2, 6);
  CheckIteratorValue(inserted, tail_counted.End(), 6);
  NUO_TEST_CHECK(ListValues(tail_counted) == (std::vector<int>{5, 6, 6}));
  NUO_TEST_CHECK(tail_counted.Size() == 3);

  nuostl::NuoList<int> middle_range{1, 4};
  nuostl::NuoList<int>::const_iterator middle = middle_range.Cbegin();
  ++middle;
  inserted = middle_range.Insert(middle, kSourceValues.data(),
                                 kSourceValues.data() + kSourceValues.size());
  CheckIteratorValue(inserted, middle_range.End(), 6);
  NUO_TEST_CHECK(ListValues(middle_range) ==
                (std::vector<int>{1, 6, 7, 4}));
  NUO_TEST_CHECK(middle_range.Size() == 4);

  nuostl::NuoList<int> initialized{5};
  inserted = initialized.Insert(initialized.Cend(), {6, 7});
  CheckIteratorValue(inserted, initialized.End(), 6);
  NUO_TEST_CHECK(ListValues(initialized) == (std::vector<int>{5, 6, 7}));
  NUO_TEST_CHECK(initialized.Size() == 3);

  nuostl::NuoList<int> unchanged{5};
  inserted = unchanged.Insert(unchanged.Cend(), kSourceValues.data(),
                              kSourceValues.data());
  NUO_TEST_CHECK(inserted == unchanged.End());
  NUO_TEST_CHECK(ListValues(unchanged) == std::vector<int>{5});
  NUO_TEST_CHECK(unchanged.Size() == 1);

  MoveTrackingValue::copies = 0;
  MoveTrackingValue::moves = 0;
  nuostl::NuoList<MoveTrackingValue> tracked_values;
  MoveTrackingValue tracked_value(8);
  tracked_values.Insert(tracked_values.Cbegin(),
                        nuostl::NuoMove(tracked_value));
  tracked_values.Emplace(tracked_values.Cend(), 9);
  NUO_TEST_CHECK(MoveTrackingValue::copies == 0);
  NUO_TEST_CHECK(MoveTrackingValue::moves == 1);
  NUO_TEST_CHECK(tracked_values.Front().value == 8);
  NUO_TEST_CHECK(tracked_values.Back().value == 9);
  NUO_TEST_CHECK(tracked_values.Size() == 2);

  constexpr std::array<int, 2> input_values = {8, 9};
  SinglePassInputIterator<int> input_first(input_values.data());
  SinglePassInputIterator<int> input_last(
      input_values.data() + input_values.size());
  nuostl::NuoList<int> input_list{5};
  inserted = input_list.Insert(input_list.Cbegin(), input_first, input_last);
  CheckIteratorValue(inserted, input_list.End(), 8);
  NUO_TEST_CHECK(ListValues(input_list) == (std::vector<int>{8, 9, 5}));
  NUO_TEST_CHECK(input_list.Size() == 3);
}

void TestNuoList::test_range_construction_and_assign()
{
  nuostl::NuoList<int> range_constructed(
      kSourceValues.data(), kSourceValues.data() + kSourceValues.size());
  NUO_TEST_CHECK(ListValues(range_constructed) ==
                (std::vector<int>{6, 7}));
  NUO_TEST_CHECK(range_constructed.Size() == 2);

  nuostl::NuoList<int> copy_source{1, 2, 3};
  nuostl::NuoList<int> copy_constructed(copy_source);
  copy_constructed.PushBack(4);
  copy_constructed.PopFront();
  NUO_TEST_CHECK(ListValues(copy_source) == (std::vector<int>{1, 2, 3}));
  NUO_TEST_CHECK(ListValues(copy_constructed) ==
                (std::vector<int>{2, 3, 4}));
  NUO_TEST_CHECK(copy_source.Size() == 3);
  NUO_TEST_CHECK(copy_constructed.Size() == 3);

  nuostl::NuoList<int> initializer_assigned{9, 9};
  initializer_assigned = {1, 3, 5};
  NUO_TEST_CHECK(ListValues(initializer_assigned) ==
                (std::vector<int>{1, 3, 5}));
  NUO_TEST_CHECK(initializer_assigned.Size() == 3);

  nuostl::NuoList<int> range_assigned{5};
  range_assigned.Assign(kSourceValues.data(),
                        kSourceValues.data() + kSourceValues.size());
  NUO_TEST_CHECK(ListValues(range_assigned) == (std::vector<int>{6, 7}));
  NUO_TEST_CHECK(range_assigned.Size() == 2);

  nuostl::NuoList deduced(kSourceValues.data(),
                          kSourceValues.data() + kSourceValues.size());
  NUO_TEST_CHECK(ListValues(deduced) == (std::vector<int>{6, 7}));
  NUO_TEST_CHECK(deduced.Size() == 2);

  constexpr std::array<int, 2> input_values = {8, 9};
  SinglePassInputIterator<int> constructor_first(input_values.data());
  SinglePassInputIterator<int> constructor_last(
      input_values.data() + input_values.size());
  nuostl::NuoList<int> input_constructed(constructor_first, constructor_last);
  NUO_TEST_CHECK(ListValues(input_constructed) ==
                (std::vector<int>{8, 9}));

  SinglePassInputIterator<int> assign_first(input_values.data());
  SinglePassInputIterator<int> assign_last(
      input_values.data() + input_values.size());
  nuostl::NuoList<int> input_assigned{5};
  input_assigned.Assign(assign_first, assign_last);
  NUO_TEST_CHECK(ListValues(input_assigned) ==
                (std::vector<int>{8, 9}));

}

void TestNuoList::test_push_pop()
{
  nuostl::NuoList<int> l1;

  l1.PushBack(2);
  l1.PushFront(1);
  l1.PushBack(3);

  NUO_TEST_CHECK(l1.Size() == 3);
  NUO_TEST_CHECK(l1.Front() == 1);
  NUO_TEST_CHECK(l1.Back() == 3);

  l1.PopFront();
  NUO_TEST_CHECK(l1.Size() == 2);
  NUO_TEST_CHECK(l1.Front() == 2);

  l1.PopBack();
  NUO_TEST_CHECK(l1.Size() == 1);
  NUO_TEST_CHECK(l1.Front() == 2);
  NUO_TEST_CHECK(l1.Back() == 2);

  l1.PopBack();
  NUO_TEST_CHECK(l1.Empty());
}

void TestNuoList::test_insert_erase()
{
  /* use push/pop to cover insertion/erasure semantics */
  nuostl::NuoList<int> l1;

  l1.PushBack(3);
  l1.PushFront(2);
  l1.PushFront(1);
  NUO_TEST_CHECK(l1.Size() == 3);
  NUO_TEST_CHECK(l1.Front() == 1);
  NUO_TEST_CHECK(l1.Back() == 3);

  l1.PopFront();
  NUO_TEST_CHECK(l1.Size() == 2);
  NUO_TEST_CHECK(l1.Front() == 2);

  l1.PopBack();
  NUO_TEST_CHECK(l1.Size() == 1);
  NUO_TEST_CHECK(l1.Front() == 2);
  NUO_TEST_CHECK(l1.Back() == 2);
}

void TestNuoList::test_splice_and_move()
{
  nuostl::NuoList<MoveTrackingValue> identity_target;
  identity_target.EmplaceBack(1);
  nuostl::NuoList<MoveTrackingValue> identity_source;
  identity_source.EmplaceBack(2);
  identity_source.EmplaceBack(3);
  MoveTrackingValue* source_first = &*identity_source.Begin();
  MoveTrackingValue::copies = 0;
  MoveTrackingValue::moves = 0;
  identity_target.Splice(identity_target.End(), identity_source);
  NUO_TEST_CHECK(MoveTrackingValue::copies == 0);
  NUO_TEST_CHECK(MoveTrackingValue::moves == 0);
  auto identity_inserted = identity_target.Begin();
  ++identity_inserted;
  NUO_TEST_CHECK(identity_inserted != identity_target.End());
  if (identity_inserted != identity_target.End())
  {
    NUO_TEST_CHECK(&*identity_inserted == source_first);
  }
  NUO_TEST_CHECK(identity_source.Empty());

  nuostl::NuoList<MoveTrackingValue> rvalue_target;
  rvalue_target.EmplaceBack(1);
  nuostl::NuoList<MoveTrackingValue> rvalue_source;
  rvalue_source.EmplaceBack(2);
  rvalue_source.EmplaceBack(3);
  auto rvalue_source_first = rvalue_source.Begin();
  MoveTrackingValue* rvalue_first = &*rvalue_source_first;
  ++rvalue_source_first;
  MoveTrackingValue* rvalue_second = &*rvalue_source_first;
  MoveTrackingValue::copies = 0;
  MoveTrackingValue::moves = 0;
  rvalue_target.Splice(rvalue_target.End(), nuostl::NuoMove(rvalue_source));
  NUO_TEST_CHECK(MoveTrackingValue::copies == 0);
  NUO_TEST_CHECK(MoveTrackingValue::moves == 0);
  auto rvalue_inserted = rvalue_target.Begin();
  ++rvalue_inserted;
  NUO_TEST_CHECK(rvalue_inserted != rvalue_target.End());
  if (rvalue_inserted != rvalue_target.End())
  {
    NUO_TEST_CHECK(&*rvalue_inserted == rvalue_first);
    ++rvalue_inserted;
  }
  NUO_TEST_CHECK(rvalue_inserted != rvalue_target.End());
  if (rvalue_inserted != rvalue_target.End())
  {
    NUO_TEST_CHECK(&*rvalue_inserted == rvalue_second);
  }
  NUO_TEST_CHECK(rvalue_source.Empty());

  nuostl::NuoList<MoveTrackingValue> single_rvalue_target;
  single_rvalue_target.EmplaceBack(1);
  single_rvalue_target.EmplaceBack(5);
  nuostl::NuoList<MoveTrackingValue> single_rvalue_source;
  single_rvalue_source.EmplaceBack(2);
  MoveTrackingValue* single_rvalue_node = &*single_rvalue_source.Begin();
  MoveTrackingValue::copies = 0;
  MoveTrackingValue::moves = 0;
  auto single_rvalue_position = single_rvalue_target.Begin();
  ++single_rvalue_position;
  single_rvalue_target.Splice(single_rvalue_position,
                              nuostl::NuoMove(single_rvalue_source),
                              single_rvalue_source.Begin());
  NUO_TEST_CHECK(MoveTrackingValue::copies == 0);
  NUO_TEST_CHECK(MoveTrackingValue::moves == 0);
  auto single_rvalue_inserted = single_rvalue_target.Begin();
  ++single_rvalue_inserted;
  NUO_TEST_CHECK(single_rvalue_inserted != single_rvalue_target.End());
  if (single_rvalue_inserted != single_rvalue_target.End())
  {
    NUO_TEST_CHECK(&*single_rvalue_inserted == single_rvalue_node);
  }
  NUO_TEST_CHECK(single_rvalue_source.Empty());

  nuostl::NuoList<MoveTrackingValue> range_rvalue_target;
  range_rvalue_target.EmplaceBack(1);
  range_rvalue_target.EmplaceBack(5);
  nuostl::NuoList<MoveTrackingValue> range_rvalue_source;
  range_rvalue_source.EmplaceBack(2);
  range_rvalue_source.EmplaceBack(3);
  range_rvalue_source.EmplaceBack(4);
  auto range_rvalue_source_first = range_rvalue_source.Begin();
  MoveTrackingValue* range_rvalue_first = &*range_rvalue_source_first;
  ++range_rvalue_source_first;
  MoveTrackingValue* range_rvalue_second = &*range_rvalue_source_first;
  ++range_rvalue_source_first;
  MoveTrackingValue* range_rvalue_third = &*range_rvalue_source_first;
  std::array<MoveTrackingValue*, 3> range_rvalue_nodes = {
      range_rvalue_first, range_rvalue_second, range_rvalue_third};
  MoveTrackingValue::copies = 0;
  MoveTrackingValue::moves = 0;
  auto range_rvalue_position = range_rvalue_target.Begin();
  ++range_rvalue_position;
  range_rvalue_target.Splice(range_rvalue_position,
                             nuostl::NuoMove(range_rvalue_source),
                             range_rvalue_source.Begin(),
                             range_rvalue_source.End());
  NUO_TEST_CHECK(MoveTrackingValue::copies == 0);
  NUO_TEST_CHECK(MoveTrackingValue::moves == 0);
  auto range_rvalue_inserted = range_rvalue_target.Begin();
  ++range_rvalue_inserted;
  for (MoveTrackingValue* node : range_rvalue_nodes)
  {
    NUO_TEST_CHECK(range_rvalue_inserted != range_rvalue_target.End());
    if (range_rvalue_inserted == range_rvalue_target.End())
    {
      break;
    }
    NUO_TEST_CHECK(&*range_rvalue_inserted == node);
    ++range_rvalue_inserted;
  }
  NUO_TEST_CHECK(range_rvalue_source.Empty());

  nuostl::NuoList<int> splice_target{1, 4};
  nuostl::NuoList<int> splice_source{2, 3};
  nuostl::NuoList<int>::const_iterator splice_position =
      splice_target.Cbegin();
  ++splice_position;
  splice_target.Splice(splice_position, nuostl::NuoMove(splice_source));
  NUO_TEST_CHECK(ListValues(splice_target) ==
                (std::vector<int>{1, 2, 3, 4}));
  NUO_TEST_CHECK(splice_target.Size() == 4);
  NUO_TEST_CHECK(splice_source.Empty());

  nuostl::NuoList<int> lvalue_target{1, 4};
  nuostl::NuoList<int> lvalue_source{2, 3};
  nuostl::NuoList<int>::const_iterator lvalue_position =
      lvalue_target.Cbegin();
  ++lvalue_position;
  lvalue_target.Splice(lvalue_position, lvalue_source);
  NUO_TEST_CHECK(ListValues(lvalue_target) ==
                (std::vector<int>{1, 2, 3, 4}));
  NUO_TEST_CHECK(lvalue_source.Empty());

  nuostl::NuoList<int> assigned_target{9};
  nuostl::NuoList<int> assigned_source{5, 6};
  assigned_target = nuostl::NuoMove(assigned_source);
  NUO_TEST_CHECK(ListValues(assigned_target) ==
                (std::vector<int>{5, 6}));
  NUO_TEST_CHECK(assigned_target.Size() == 2);
  NUO_TEST_CHECK(assigned_source.Empty());

  nuostl::NuoList<int> empty_assigned_target{3};
  nuostl::NuoList<int> empty_assigned_source;
  empty_assigned_target = nuostl::NuoMove(empty_assigned_source);
  NUO_TEST_CHECK(empty_assigned_target.Empty());
  NUO_TEST_CHECK(empty_assigned_source.Empty());
  empty_assigned_source.PushBack(4);
  NUO_TEST_CHECK(ListValues(empty_assigned_source) == std::vector<int>{4});

  nuostl::NuoList<int> move_source{5, 6};
  nuostl::NuoList<int> move_target(nuostl::NuoMove(move_source));
  NUO_TEST_CHECK(ListValues(move_target) == (std::vector<int>{5, 6}));
  NUO_TEST_CHECK(move_target.Size() == 2);
  NUO_TEST_CHECK(move_source.Empty());
  NUO_TEST_CHECK(move_source.Size() == 0);
  NUO_TEST_CHECK(move_source.Begin() == move_source.End());
  NUO_TEST_CHECK(move_source.Rbegin() == move_source.Rend());
  move_source.PushBack(7);
  NUO_TEST_CHECK(ListValues(move_source) == std::vector<int>{7});

  nuostl::NuoList<int> empty_move_source;
  nuostl::NuoList<int> empty_move_target(nuostl::NuoMove(empty_move_source));
  NUO_TEST_CHECK(empty_move_target.Empty());
  NUO_TEST_CHECK(empty_move_source.Empty());
  empty_move_source.PushBack(8);
  NUO_TEST_CHECK(ListValues(empty_move_source) == std::vector<int>{8});

  nuostl::NuoList<int> self_splice{1, 2, 3, 4, 5};
  nuostl::NuoList<int>::const_iterator self_first = self_splice.Cbegin();
  ++self_first;
  nuostl::NuoList<int>::const_iterator self_last = self_first;
  ++self_last;
  ++self_last;
  ++self_last;
  self_splice.Splice(self_splice.Cend(), self_splice, self_first, self_last);
  NUO_TEST_CHECK(ListValues(self_splice) ==
                (std::vector<int>{1, 5, 2, 3, 4}));
  NUO_TEST_CHECK(self_splice.Size() == 5);

  nuostl::NuoList<int> untouched{1, 2};
  untouched.Splice(untouched.Cend(), untouched, untouched.Cbegin(),
                   untouched.Cbegin());
  NUO_TEST_CHECK(ListValues(untouched) == (std::vector<int>{1, 2}));

  nuostl::NuoList<int> overlapping_splice{1, 2, 3, 4};
  nuostl::NuoList<int>::const_iterator overlapping_first =
      overlapping_splice.Cbegin();
  ++overlapping_first;
  nuostl::NuoList<int>::const_iterator overlapping_last =
      overlapping_first;
  ++overlapping_last;
  ++overlapping_last;
  nuostl::NuoList<int>::const_iterator overlapping_position =
      overlapping_first;
  ++overlapping_position;
  overlapping_splice.Splice(overlapping_position, overlapping_splice,
                            overlapping_first, overlapping_last);
  /* NuoList intentionally treats this invalid input overlap as a no-op. */
  NUO_TEST_CHECK(ListValues(overlapping_splice) ==
                (std::vector<int>{1, 2, 3, 4}));
  NUO_TEST_CHECK(overlapping_splice.Size() == 4);

  nuostl::NuoList<int> boundary_splice{1, 2, 3, 4};
  nuostl::NuoList<int>::const_iterator boundary_first =
      boundary_splice.Cbegin();
  ++boundary_first;
  nuostl::NuoList<int>::const_iterator boundary_last = boundary_first;
  ++boundary_last;
  ++boundary_last;
  /* NuoList deliberately treats position == first as a defensive no-op. */
  boundary_splice.Splice(boundary_first, boundary_splice, boundary_first,
                         boundary_last);
  boundary_splice.Splice(boundary_last, boundary_splice, boundary_first,
                         boundary_last);
  NUO_TEST_CHECK(ListValues(boundary_splice) ==
                (std::vector<int>{1, 2, 3, 4}));
  NUO_TEST_CHECK(boundary_splice.Size() == 4);

  nuostl::NuoList<int> forward_splice{1, 2, 3, 4, 5};
  nuostl::NuoList<int>::const_iterator forward_first = forward_splice.Cbegin();
  ++forward_first;
  ++forward_first;
  nuostl::NuoList<int>::const_iterator forward_last = forward_first;
  ++forward_last;
  ++forward_last;
  nuostl::NuoList<int>::const_iterator forward_position =
      forward_splice.Cbegin();
  ++forward_position;
  forward_splice.Splice(forward_position, forward_splice, forward_first,
                        forward_last);
  NUO_TEST_CHECK(ListValues(forward_splice) ==
                (std::vector<int>{1, 3, 4, 2, 5}));
  NUO_TEST_CHECK(forward_splice.Size() == 5);

  nuostl::NuoList<int> single_splice{7, 8};
  nuostl::NuoList<int>::const_iterator single_position =
      single_splice.Cbegin();
  nuostl::NuoList<int> single_target{5, 6};
  single_target.Splice(single_target.Cend(), nuostl::NuoMove(single_splice),
                       single_position);
  NUO_TEST_CHECK(ListValues(single_target) ==
                (std::vector<int>{5, 6, 7}));
  NUO_TEST_CHECK(ListValues(single_splice) == std::vector<int>{8});

  nuostl::NuoList<int> range_target{1, 4};
  nuostl::NuoList<int> range_source{2, 3};
  nuostl::NuoList<int>::const_iterator range_first = range_source.Cbegin();
  nuostl::NuoList<int>::const_iterator range_last = range_first;
  ++range_last;
  ++range_last;
  range_target.Splice(range_target.Cend(), nuostl::NuoMove(range_source),
                      range_first, range_last);
  NUO_TEST_CHECK(ListValues(range_target) ==
                (std::vector<int>{1, 4, 2, 3}));
  NUO_TEST_CHECK(range_target.Size() == 4);
  NUO_TEST_CHECK(range_source.Empty());
}

void TestNuoList::test_clear_and_swap()
{
  nuostl::NuoList<int> l1{1, 2, 3};
  nuostl::NuoList<int> l2{9, 8};

  l1.Swap(l2);
  NUO_TEST_CHECK(l1.Size() == 2);
  NUO_TEST_CHECK(l2.Size() == 3);
  NUO_TEST_CHECK(l1.Front() == 9);
  NUO_TEST_CHECK(l2.Front() == 1);

  l1.Swap(l1);
  NUO_TEST_CHECK(l1.Size() == 2);
  NUO_TEST_CHECK(ListValues(l1) == (std::vector<int>{9, 8}));

  nuostl::NuoList<int> empty_l1;
  l1.Swap(empty_l1);
  NUO_TEST_CHECK(l1.Empty());
  NUO_TEST_CHECK(ListValues(empty_l1) == (std::vector<int>{9, 8}));

  l1.Swap(empty_l1);
  NUO_TEST_CHECK(ListValues(l1) == (std::vector<int>{9, 8}));
  NUO_TEST_CHECK(empty_l1.Empty());

  l1.Clear();
  NUO_TEST_CHECK(l1.Empty());
  NUO_TEST_CHECK(l1.Size() == 0);
  NUO_TEST_CHECK(l1.Begin() == l1.End());
  NUO_TEST_CHECK(l1.Rbegin() == l1.Rend());

  l2.Clear();
  NUO_TEST_CHECK(l2.Empty());
}

void TestNuoList::test_nuo_list()
{
  TestResourceLifetime();
  test_constructor();
  test_assign();
  test_iterator_and_access();
  test_iterator_conversion();
  test_emplace_positions();
  test_insert_overloads();
  test_range_construction_and_assign();
  test_push_pop();
  test_insert_erase();
  test_splice_and_move();
  test_clear_and_swap();
}

} /* namespace test */
