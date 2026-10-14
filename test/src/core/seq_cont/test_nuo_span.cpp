#include "core/seq_cont/test_nuo_span.hpp"

#include <assert.h>

#include <array>
#include <cstddef>
#include <span>
#include <type_traits>
#include <vector>

#include "core/seq_cont/nuo_span.hpp"

namespace test
{

void TestNuoSpan::TestConstruction()
{
  int values[3] = {1, 2, 3};
  nuostl::NuoSpan<int, 3> fixed(values);
  assert(fixed.Size() == 3);
  assert(fixed.Data() == values);
  static_assert(decltype(fixed)::extent == 3);

  nuostl::NuoSpan<int> dynamic(values);
  assert(dynamic.Size() == 3);
  assert(dynamic.Data() == values);
  static_assert(decltype(dynamic)::extent == nuostl::NuoDynamicExtent);
  static_assert(std::dynamic_extent == nuostl::NuoDynamicExtent);

  std::array<int, 4> standard_array = {4, 5, 6, 7};
  nuostl::NuoSpan<int, 4> fixed_array(standard_array);
  nuostl::NuoSpan<int> dynamic_array(standard_array);
  assert(fixed_array.Size() == 4 && fixed_array[0] == 4);
  assert(dynamic_array.Size() == 4 && dynamic_array[3] == 7);

  const std::array<int, 4> const_array = {8, 9, 10, 11};
  nuostl::NuoSpan<const int, 4> fixed_const_array(const_array);
  nuostl::NuoSpan<const int> dynamic_const_array(const_array);
  assert(fixed_const_array.Size() == 4 && fixed_const_array[0] == 8);
  assert(dynamic_const_array.Size() == 4 && dynamic_const_array[3] == 11);

  std::vector<int> vector = {1, 2, 3, 4};
  nuostl::NuoSpan<int> vector_span(vector);
  nuostl::NuoSpan<const int> const_vector_span(vector);
  assert(vector_span.Size() == 4 && vector_span.Data() == vector.data());
  assert(const_vector_span.Size() == 4);

  const std::vector<int> const_vector = {5, 6};
  nuostl::NuoSpan<const int> from_const_vector(const_vector);
  assert(from_const_vector.Size() == 2 && from_const_vector[1] == 6);

  int* pointer = values;
  nuostl::NuoSpan<int> pointer_span(pointer, 2);
  assert(pointer_span.Size() == 2 && pointer_span[1] == 2);

  nuostl::NuoSpan<int> iterator_span(vector.begin(), vector.end());
  assert(iterator_span.Size() == 4 && iterator_span[3] == 4);

  nuostl::NuoSpan<const int> converted(fixed);
  assert(converted.Size() == 3 && converted.Data() == fixed.Data());
  nuostl::NuoSpan<int> dynamic_from_fixed(fixed);
  assert(dynamic_from_fixed.Size() == 3);

  nuostl::NuoSpan<int, 0> empty_fixed;
  nuostl::NuoSpan<int> empty_dynamic;
  assert(empty_fixed.Empty() && empty_fixed.Data() == nullptr);
  assert(empty_fixed.Size() == 0);
  assert(empty_dynamic.Empty() && empty_dynamic.Size() == 0);

  static_assert(!std::is_default_constructible_v<nuostl::NuoSpan<int, 3>>);
  static_assert(!std::is_constructible_v<nuostl::NuoSpan<int>,
                                         const int (&)[3]>);
  static_assert(!std::is_constructible_v<nuostl::NuoSpan<int, 4>,
                                         int (&)[3]>);
  static_assert(!std::is_constructible_v<nuostl::NuoSpan<const int>,
                                         std::vector<int>>);
  static_assert(std::is_constructible_v<nuostl::NuoSpan<const int>,
                                        std::vector<int>&>);
  static_assert(!std::is_constructible_v<nuostl::NuoSpan<const int>,
                                         std::array<int, 3>>);
  static_assert(std::is_constructible_v<nuostl::NuoSpan<const int>,
                                        std::array<int, 3>&>);
}

void TestNuoSpan::TestObservers()
{
  int values[4] = {1, 2, 3, 4};
  nuostl::NuoSpan<int> span(values);

  assert(span.Size() == 4);
  assert(span.SizeBytes() == 4 * sizeof(int));
  assert(!span.Empty());
  assert(span.Data() == values);
  assert(span[0] == 1);
  assert(span[3] == 4);
  assert(span.Front() == 1);
  assert(span.Back() == 4);
  assert(span.Begin() == values);
  assert(span.End() == values + 4);
  assert(*span.RBegin() == 4);
  assert(*(span.REnd() - 1) == 1);

  int sum = 0;
  for (int* iterator = span.Begin(); iterator != span.End(); ++iterator)
  {
    sum += *iterator;
  }
  assert(sum == 10);
}

void TestNuoSpan::TestSubviews()
{
  int values[5] = {1, 2, 3, 4, 5};
  nuostl::NuoSpan<int> span(values);

  const auto first = span.First(2);
  assert(first.Size() == 2);
  assert(first[0] == 1 && first[1] == 2);

  const auto last = span.Last(2);
  assert(last.Size() == 2);
  assert(last[0] == 4 && last[1] == 5);

  const auto middle = span.Subspan(1, 3);
  assert(middle.Size() == 3);
  assert(middle[0] == 2 && middle[2] == 4);

  const auto tail = span.Subspan(2);
  assert(tail.Size() == 3 && tail[0] == 3);

  assert(span.Subspan(5).Empty());

  nuostl::NuoSpan<int, 5> fixed(values);
  assert(fixed.First(3).Size() == 3);
  assert(fixed.Last(3).Size() == 3);
  assert(fixed.Last(3)[0] == 3);
  assert(fixed.Subspan(2, 2).Size() == 2);
}

void TestNuoSpan::TestMutation()
{
  int values[3] = {1, 2, 3};
  nuostl::NuoSpan<int> span(values);

  span[0] = 10;
  span.Front() = 11;
  span.Back() = 13;
  assert(values[0] == 11 && values[1] == 2 && values[2] == 13);

  for (int* iterator = span.Begin(); iterator != span.End(); ++iterator)
  {
    *iterator += 1;
  }
  assert(values[0] == 12 && values[1] == 3 && values[2] == 14);

  const auto part = span.Subspan(1, 2);
  part[0] = 20;
  assert(values[1] == 20 && values[2] == 14);

  const int const_values[3] = {1, 2, 3};
  nuostl::NuoSpan<const int> const_span(const_values);
  static_assert(std::is_same_v<decltype(const_span[0]), const int&>);
  assert(const_span[0] == 1);
  assert(const_span.Front() == 1);
}

void TestNuoSpan::TestDeductionGuides()
{
  int values[3] = {1, 2, 3};
  nuostl::NuoSpan deduced_array(values);
  static_assert(std::is_same_v<decltype(deduced_array),
                               nuostl::NuoSpan<int, 3>>);

  std::array<int, 2> standard_array = {4, 5};
  nuostl::NuoSpan deduced_std_array(standard_array);
  static_assert(std::is_same_v<decltype(deduced_std_array),
                               nuostl::NuoSpan<int, 2>>);

  const std::array<int, 2> const_standard_array = {6, 7};
  nuostl::NuoSpan deduced_const_array(const_standard_array);
  static_assert(std::is_same_v<decltype(deduced_const_array),
                               nuostl::NuoSpan<const int, 2>>);

  std::vector<int> vector = {8, 9};
  nuostl::NuoSpan deduced_vector(vector);
  static_assert(std::is_same_v<decltype(deduced_vector),
                               nuostl::NuoSpan<int>>);
}

void TestNuoSpan::TestConstexprOperations()
{
  static constexpr int values[3] = {1, 2, 3};
  constexpr nuostl::NuoSpan<const int, 3> fixed(values);
  static_assert(fixed.Size() == 3);
  static_assert(fixed.SizeBytes() == 3 * sizeof(int));
  static_assert(!fixed.Empty());
  static_assert(fixed[1] == 2);
  static_assert(fixed.Front() == 1);
  static_assert(fixed.Back() == 3);
  static_assert(fixed.Data() == values);
  static_assert(fixed.Begin() == values);
  static_assert(fixed.End() == values + 3);

  constexpr auto part = fixed.Subspan(1, 2);
  static_assert(part.Size() == 2);
  static_assert(part[0] == 2 && part[1] == 3);

  constexpr nuostl::NuoSpan<const int> dynamic(values);
  static_assert(dynamic.Size() == 3);
  static_assert(dynamic.First(2).Size() == 2);
  static_assert(dynamic.Last(2)[0] == 2);
}

void TestNuoSpan::TestStdParity()
{
  int values[5] = {1, 2, 3, 4, 5};
  std::span<int> standard(values);
  nuostl::NuoSpan<int> nuo(values);

  assert(standard.size() == nuo.Size());
  assert(standard.size_bytes() == nuo.SizeBytes());
  assert(standard.data() == nuo.Data());
  assert(standard.empty() == nuo.Empty());
  assert(standard.front() == nuo.Front());
  assert(standard.back() == nuo.Back());
  for (std::size_t index = 0; index < standard.size(); ++index)
  {
    assert(standard[index] == nuo[index]);
  }

  assert(standard.first(2).size() == nuo.First(2).Size());
  assert(standard.last(2).size() == nuo.Last(2).Size());
  assert(standard.subspan(1, 3).size() == nuo.Subspan(1, 3).Size());
  assert(standard.subspan(2).size() == nuo.Subspan(2).Size());
  assert(standard.first(2)[1] == nuo.First(2)[1]);
  assert(standard.last(2)[0] == nuo.Last(2)[0]);
  assert(standard.subspan(1, 3)[2] == nuo.Subspan(1, 3)[2]);

  std::vector<int> vector = {6, 7, 8};
  std::span<int> standard_vector(vector);
  nuostl::NuoSpan<int> nuo_vector(vector);
  assert(standard_vector.size() == nuo_vector.Size());
  assert(standard_vector.data() == nuo_vector.Data());
}

void TestNuoSpan::TestSuite()
{
  TestConstruction();
  TestObservers();
  TestSubviews();
  TestMutation();
  TestDeductionGuides();
  TestConstexprOperations();
  TestStdParity();
}

} /* namespace test */
