#include "core/algorithms/test_nuo_sequence_operations.hpp"

#include <cassert>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <random>
#include <set>
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

void CheckRotation(const std::vector<int>& values)
{
  const std::ptrdiff_t size = static_cast<std::ptrdiff_t>(values.size());
  for (std::ptrdiff_t middle = 0; middle <= size; ++middle)
  {
    std::vector<int> rotated(values);
    std::vector<int> expected(values);
    const auto rotated_return = nuostl::NuoRotate(
      rotated.begin(), rotated.begin() + middle, rotated.end());
    const auto expected_return = std::rotate(
      expected.begin(), expected.begin() + middle, expected.end());
    assert(rotated == expected);
    assert(Offset(rotated, rotated_return) ==
           Offset(expected, expected_return));

    std::vector<int> copied(values.size() + 1, -1);
    std::vector<int> copied_expected(values.size() + 1, -1);
    const auto copied_return = nuostl::NuoRotateCopy(
      values.begin(), values.begin() + middle, values.end(), copied.begin());
    std::rotate_copy(values.begin(), values.begin() + middle, values.end(),
                     copied_expected.begin());
    assert(copied_return == copied.begin() + size);
    assert(copied == copied_expected);
  }
}

void CheckPermutation(const std::vector<int>& values)
{
  std::vector<int> next(values);
  std::vector<int> next_expected(values);
  assert(nuostl::NuoNextPermutation(next.begin(), next.end()) ==
         std::next_permutation(next_expected.begin(), next_expected.end()));
  assert(next == next_expected);

  std::vector<int> previous(values);
  std::vector<int> previous_expected(values);
  assert(nuostl::NuoPrevPermutation(previous.begin(), previous.end()) ==
         std::prev_permutation(previous_expected.begin(),
                               previous_expected.end()));
  assert(previous == previous_expected);

  std::vector<int> next_greater(values);
  std::vector<int> next_greater_expected(values);
  assert(nuostl::NuoNextPermutation(next_greater.begin(), next_greater.end(),
                                    std::greater<>()) ==
         std::next_permutation(next_greater_expected.begin(),
                               next_greater_expected.end(), std::greater<>()));
  assert(next_greater == next_greater_expected);

  std::vector<int> previous_greater(values);
  std::vector<int> previous_greater_expected(values);
  assert(nuostl::NuoPrevPermutation(previous_greater.begin(),
                                    previous_greater.end(),
                                    std::greater<>()) ==
         std::prev_permutation(previous_greater_expected.begin(),
                               previous_greater_expected.end(),
                               std::greater<>()));
  assert(previous_greater == previous_greater_expected);
}

void CheckPermutationCycle(int size)
{
  std::vector<int> ordered(static_cast<std::size_t>(size));
  for (int index = 0; index < size; ++index)
  {
    ordered[static_cast<std::size_t>(index)] = index;
  }

  /* The standard library supplies the number of distinct permutations. */
  std::size_t expected_count = 0;
  std::vector<int> expected(ordered);
  do
  {
    ++expected_count;
  } while (std::next_permutation(expected.begin(), expected.end()));

  /* Walking forward from the first permutation returns to the first one. */
  std::vector<int> ascending(ordered);
  std::size_t ascending_count = 0;
  do
  {
    ++ascending_count;
  } while (nuostl::NuoNextPermutation(ascending.begin(), ascending.end()));
  assert(ascending_count == expected_count);
  assert(ascending == ordered);

  /* Walking backward from the last permutation returns to the last one. */
  std::vector<int> descending(ordered);
  nuostl::NuoReverse(descending.begin(), descending.end());
  std::vector<int> current(descending);
  std::size_t descending_count = 0;
  do
  {
    ++descending_count;
  } while (nuostl::NuoPrevPermutation(current.begin(), current.end()));
  assert(descending_count == expected_count);
  assert(current == descending);
}

void CheckShuffle()
{
  const std::vector<int> values = {0, 1, 2, 3, 4, 5, 6, 7};

  std::mt19937 generator(1234);
  std::vector<int> shuffled(values);
  nuostl::NuoShuffle(shuffled.begin(), shuffled.end(), generator);
  std::vector<int> sorted(shuffled);
  std::sort(sorted.begin(), sorted.end());
  assert(sorted == values);

  std::mt19937 repeated_generator(1234);
  std::vector<int> repeated(values);
  nuostl::NuoShuffle(repeated.begin(), repeated.end(), repeated_generator);
  assert(repeated == shuffled);

  std::set<std::vector<int>> arrangements;
  for (unsigned int seed = 0; seed < 64; ++seed)
  {
    std::mt19937 seed_generator(seed);
    std::vector<int> candidate(values);
    nuostl::NuoShuffle(candidate.begin(), candidate.end(), seed_generator);
    std::vector<int> candidate_sorted(candidate);
    std::sort(candidate_sorted.begin(), candidate_sorted.end());
    assert(candidate_sorted == values);
    arrangements.insert(candidate);
  }
  assert(arrangements.size() > 1);

  std::vector<int> empty;
  std::mt19937 empty_generator(9);
  nuostl::NuoShuffle(empty.begin(), empty.end(), empty_generator);
  assert(empty.empty());

  std::vector<int> single = {42};
  nuostl::NuoShuffle(single.begin(), single.end(), empty_generator);
  assert(single.size() == 1 && single[0] == 42);
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
    CheckRotation(values);
    CheckPermutation(values);
  }

  CheckPermutationCycle(1);
  CheckPermutationCycle(2);
  CheckPermutationCycle(5);
  CheckShuffle();

  for (const std::vector<int>& left : sequences)
  {
    for (const std::vector<int>& right : sequences)
    {
      CheckSetRelations(left, right);
    }
  }
}

} /* namespace test */
