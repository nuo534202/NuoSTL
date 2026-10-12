#include "core/algorithms/test_nuo_sequence_operations.hpp"

#include <cassert>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

constexpr int kAlphabet = 3;
constexpr int kMaxLength = 5;

bool IsEven(int value)
{
  return value % 2 == 0;
}

template <typename Container, typename Iterator>
std::ptrdiff_t Offset(Container& container, Iterator position)
{
  return std::distance(container.begin(), position);
}

void CheckInspection(const std::vector<int>& values)
{
  const auto first = values.begin();
  const auto last = values.end();
  assert(nuostl::NuoAllOf(first, last, IsEven) ==
         std::all_of(first, last, IsEven));
  assert(nuostl::NuoAnyOf(first, last, IsEven) ==
         std::any_of(first, last, IsEven));
  assert(nuostl::NuoNoneOf(first, last, IsEven) ==
         std::none_of(first, last, IsEven));
  assert(nuostl::NuoAllOf(first, last, IsEven) ==
         std::all_of(first, last, IsEven));

  assert(nuostl::NuoCount(first, last, 1) == std::count(first, last, 1));
  assert(nuostl::NuoCountIf(first, last, IsEven) ==
         std::count_if(first, last, IsEven));

  assert(Offset(values, nuostl::NuoFindIf(first, last, IsEven)) ==
         Offset(values, std::find_if(first, last, IsEven)));
  assert(Offset(values, nuostl::NuoFindIfNot(first, last, IsEven)) ==
         Offset(values, std::find_if_not(first, last, IsEven)));

  assert(Offset(values, nuostl::NuoAdjacentFind(first, last)) ==
         Offset(values, std::adjacent_find(first, last)));
  assert(Offset(values, nuostl::NuoAdjacentFind(
                         first, last, std::equal_to<>())) ==
         Offset(values, std::adjacent_find(
                          first, last, std::equal_to<>())));

  assert(Offset(values, nuostl::NuoMinElement(first, last)) ==
         Offset(values, std::min_element(first, last)));
  assert(Offset(values, nuostl::NuoMaxElement(first, last)) ==
         Offset(values, std::max_element(first, last)));
  assert(Offset(values, nuostl::NuoMinElement(
                         first, last, std::greater<>())) ==
         Offset(values, std::min_element(first, last, std::greater<>())));
  assert(Offset(values, nuostl::NuoMaxElement(
                         first, last, std::greater<>())) ==
         Offset(values, std::max_element(first, last, std::greater<>())));

  for (std::ptrdiff_t begin = 0;
       begin <= static_cast<std::ptrdiff_t>(values.size()); ++begin)
  {
    for (std::ptrdiff_t end = begin;
         end <= static_cast<std::ptrdiff_t>(values.size()); ++end)
    {
      const auto needle_begin = first + begin;
      const auto needle_end = first + end;
      assert(Offset(values, nuostl::NuoSearch(
                             first, last, needle_begin, needle_end)) ==
             Offset(values, std::search(
                              first, last, needle_begin, needle_end)));
      assert(Offset(values, nuostl::NuoSearch(
                             first, last, needle_begin, needle_end,
                             std::equal_to<>())) ==
             Offset(values, std::search(
                              first, last, needle_begin, needle_end,
                              std::equal_to<>())));
      assert(Offset(values, nuostl::NuoFindEnd(
                             first, last, needle_begin, needle_end)) ==
             Offset(values, std::find_end(
                              first, last, needle_begin, needle_end)));
      assert(Offset(values, nuostl::NuoFindFirstOf(
                             first, last, needle_begin, needle_end)) ==
             Offset(values, std::find_first_of(
                              first, last, needle_begin, needle_end)));
      assert(Offset(values, nuostl::NuoFindFirstOf(
                             first, last, needle_begin, needle_end,
                             std::equal_to<>())) ==
             Offset(values, std::find_first_of(
                              first, last, needle_begin, needle_end,
                              std::equal_to<>())));
    }
  }

  for (int count = 0; count <= 3; ++count)
  {
    for (int value = 0; value < kAlphabet; ++value)
    {
      assert(Offset(values, nuostl::NuoSearchN(
                             first, last, count, value)) ==
             Offset(values, std::search_n(first, last, count, value)));
      assert(Offset(values, nuostl::NuoSearchN(
                             first, last, count, value,
                             std::equal_to<>())) ==
             Offset(values, std::search_n(first, last, count, value,
                                          std::equal_to<>())));
    }
  }
}

void CheckMutation(const std::vector<int>& values)
{
  const std::size_t size = values.size();

  std::vector<int> generated(size, -1);
  int counter = 0;
  nuostl::NuoGenerate(generated.begin(), generated.end(),
                      [&counter]
                      {
                        return counter++;
                      });
  for (std::size_t index = 0; index < size; ++index)
  {
    assert(generated[index] == static_cast<int>(index));
  }

  std::vector<int> generated_n(size + 2, -1);
  counter = 10;
  const auto generated_end = nuostl::NuoGenerateN(
    generated_n.begin(), static_cast<int>(size),
    [&counter]
    {
      return counter++;
    });
  assert(generated_end == generated_n.begin() +
         static_cast<std::ptrdiff_t>(size));
  for (std::size_t index = 0; index < size; ++index)
  {
    assert(generated_n[index] == static_cast<int>(index) + 10);
  }
  assert(generated_n[size] == -1 && generated_n[size + 1] == -1);

  std::vector<int> reversed(values);
  nuostl::NuoReverse(reversed.begin(), reversed.end());
  std::vector<int> expected(values);
  std::reverse(expected.begin(), expected.end());
  assert(reversed == expected);

  std::vector<int> reversed_copy(size, -1);
  assert(nuostl::NuoReverseCopy(values.begin(), values.end(),
                                reversed_copy.begin()) ==
         reversed_copy.end());
  assert(reversed_copy == expected);

  std::vector<int> unique(values);
  std::vector<int> unique_expected(values);
  assert(Offset(unique, nuostl::NuoUnique(unique.begin(), unique.end())) ==
         Offset(unique_expected,
                std::unique(unique_expected.begin(), unique_expected.end())));
  assert(unique == unique_expected);

  std::vector<int> unique_copy(size + 1, -1);
  std::vector<int> unique_copy_expected(size + 1, -1);
  const auto unique_copy_end = nuostl::NuoUniqueCopy(
    values.begin(), values.end(), unique_copy.begin());
  const auto unique_copy_expected_end = std::unique_copy(
    values.begin(), values.end(), unique_copy_expected.begin());
  assert(Offset(unique_copy, unique_copy_end) ==
         Offset(unique_copy_expected, unique_copy_expected_end));
  assert(unique_copy == unique_copy_expected);

  for (int old_value = 0; old_value < kAlphabet; ++old_value)
  {
    std::vector<int> replaced(values);
    std::vector<int> replaced_expected(values);
    nuostl::NuoReplace(replaced.begin(), replaced.end(), old_value, 7);
    std::replace(replaced_expected.begin(), replaced_expected.end(),
                 old_value, 7);
    assert(replaced == replaced_expected);

    std::vector<int> replaced_if(values);
    std::vector<int> replaced_if_expected(values);
    nuostl::NuoReplaceIf(replaced_if.begin(), replaced_if.end(), IsEven, 9);
    std::replace_if(replaced_if_expected.begin(), replaced_if_expected.end(),
                    IsEven, 9);
    assert(replaced_if == replaced_if_expected);

    std::vector<int> replaced_copy(size, -1);
    std::vector<int> replaced_copy_expected(size, -1);
    assert(nuostl::NuoReplaceCopy(values.begin(), values.end(),
                                  replaced_copy.begin(), old_value, 5) ==
           replaced_copy.end());
    std::replace_copy(values.begin(), values.end(),
                      replaced_copy_expected.begin(), old_value, 5);
    assert(replaced_copy == replaced_copy_expected);

    std::vector<int> removed(values);
    std::vector<int> removed_expected(values);
    assert(Offset(removed, nuostl::NuoRemove(
                               removed.begin(), removed.end(), old_value)) ==
           Offset(removed_expected,
                  std::remove(removed_expected.begin(),
                              removed_expected.end(), old_value)));
    assert(removed == removed_expected);

    std::vector<int> removed_copy(size + 1, -1);
    std::vector<int> removed_copy_expected(size + 1, -1);
    assert(Offset(removed_copy, nuostl::NuoRemoveCopy(
                                  values.begin(), values.end(),
                                  removed_copy.begin(), old_value)) ==
           Offset(removed_copy_expected,
                  std::remove_copy(values.begin(), values.end(),
                                   removed_copy_expected.begin(), old_value)));
    assert(removed_copy == removed_copy_expected);
  }

  std::vector<int> removed_if(values);
  std::vector<int> removed_if_expected(values);
  assert(Offset(removed_if, nuostl::NuoRemoveIf(
                                 removed_if.begin(), removed_if.end(),
                                 IsEven)) ==
         Offset(removed_if_expected,
                std::remove_if(removed_if_expected.begin(),
                               removed_if_expected.end(), IsEven)));
  assert(removed_if == removed_if_expected);

  std::vector<int> removed_copy_if(size + 1, -1);
  std::vector<int> removed_copy_if_expected(size + 1, -1);
  assert(Offset(removed_copy_if, nuostl::NuoRemoveCopyIf(
                                    values.begin(), values.end(),
                                    removed_copy_if.begin(), IsEven)) ==
         Offset(removed_copy_if_expected,
                std::remove_copy_if(values.begin(), values.end(),
                                    removed_copy_if_expected.begin(),
                                    IsEven)));
  assert(removed_copy_if == removed_copy_if_expected);

  std::vector<int> replaced_copy_if(size, -1);
  std::vector<int> replaced_copy_if_expected(size, -1);
  assert(nuostl::NuoReplaceCopyIf(values.begin(), values.end(),
                                  replaced_copy_if.begin(), IsEven, 4) ==
         replaced_copy_if.end());
  std::replace_copy_if(values.begin(), values.end(),
                       replaced_copy_if_expected.begin(), IsEven, 4);
  assert(replaced_copy_if == replaced_copy_if_expected);
}

void CheckSetRelations(const std::vector<int>& left,
                       const std::vector<int>& right)
{
  std::vector<int> sorted_left(left);
  std::vector<int> sorted_right(right);
  std::sort(sorted_left.begin(), sorted_left.end());
  std::sort(sorted_right.begin(), sorted_right.end());

  assert(nuostl::NuoIncludes(sorted_left.begin(), sorted_left.end(),
                             sorted_right.begin(), sorted_right.end()) ==
         std::includes(sorted_left.begin(), sorted_left.end(),
                       sorted_right.begin(), sorted_right.end()));
  assert(nuostl::NuoIncludes(sorted_left.begin(), sorted_left.end(),
                             sorted_right.begin(), sorted_right.end(),
                             std::less<>()) ==
         std::includes(sorted_left.begin(), sorted_left.end(),
                       sorted_right.begin(), sorted_right.end(),
                       std::less<>()));

  assert(nuostl::NuoIsPermutation(left.begin(), left.end(),
                                  right.begin(), right.end()) ==
         std::is_permutation(left.begin(), left.end(),
                             right.begin(), right.end()));
  assert(nuostl::NuoIsPermutation(left.begin(), left.end(),
                                  right.begin(), right.end(),
                                  std::equal_to<>()) ==
         std::is_permutation(left.begin(), left.end(),
                             right.begin(), right.end(),
                             std::equal_to<>()));
}

} /* namespace */

void TestNuoSequenceOperations::TestSuite()
{
  std::vector<std::vector<int>> sequences;
  for (int length = 0; length <= kMaxLength; ++length)
  {
    std::vector<int> values(static_cast<std::size_t>(length), 0);
    int total = 1;
    for (int index = 0; index < length; ++index)
    {
      total *= kAlphabet;
    }
    for (int code = 0; code < total; ++code)
    {
      int rest = code;
      for (int index = 0; index < length; ++index)
      {
        values[static_cast<std::size_t>(index)] = rest % kAlphabet;
        rest /= kAlphabet;
      }
      sequences.push_back(values);
    }
  }

  for (const std::vector<int>& values : sequences)
  {
    CheckInspection(values);
    CheckMutation(values);
  }

  for (const std::vector<int>& left : sequences)
  {
    for (const std::vector<int>& right : sequences)
    {
      CheckSetRelations(left, right);
    }
  }
}

} /* namespace test */
