#include "core/seq_cont/test_nuo_vector_bool.hpp"

#include <cassert>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "nuostl.hpp"

namespace test
{

namespace
{

/* Counts how many elements an allocation request asks for, which exposes the
   packed word representation of nuo_vector<bool>. */
template <typename T>
class CountingAllocator
{
public:
  using value_type = T;

  template <typename U>
  struct rebind
  {
    using other = CountingAllocator<U>;
  };

  CountingAllocator() noexcept = default;

  template <typename U>
  CountingAllocator(const CountingAllocator<U>&) noexcept
  {
  }

  T* allocate(std::size_t n)
  {
    allocated_elements += n;
    ++allocation_calls;
    return std::allocator<T>().allocate(n);
  }

  void deallocate(T* pointer, std::size_t n) noexcept
  {
    deallocated_elements += n;
    ++deallocation_calls;
    std::allocator<T>().deallocate(pointer, n);
  }

  template <typename U>
  bool operator==(const CountingAllocator<U>&) const noexcept
  {
    return true;
  }

  static inline std::size_t allocated_elements = 0;
  static inline std::size_t deallocated_elements = 0;
  static inline std::size_t allocation_calls = 0;
  static inline std::size_t deallocation_calls = 0;
};

bool SampleValue(std::size_t index)
{
  return (index * 7 + 3) % 5 < 2;
}

void CheckTypeProperties()
{
  using BoolVector = nuostl::nuo_vector<bool>;
  static_assert(std::is_same_v<BoolVector::value_type, bool>);
  static_assert(std::is_same_v<BoolVector::const_reference, bool>);
  static_assert(!std::is_same_v<BoolVector::reference, bool&>);
  static_assert(std::is_class_v<BoolVector::reference>);
  static_assert(std::is_same_v<nuostl::NuoVectorBool<>,
                               nuostl::nuo_vector<bool>>);
  static_assert(std::is_same_v<
    std::iterator_traits<BoolVector::iterator>::iterator_category,
    std::random_access_iterator_tag>);
  static_assert(std::is_same_v<
    decltype(std::declval<const BoolVector&>()[0]), bool>);
  static_assert(std::is_same_v<
    decltype(std::declval<BoolVector&>()[0]), BoolVector::reference>);

  /* The specialization must stay usable for its own allocator argument. */
  static_assert(!std::is_same_v<
    nuostl::nuo_vector<bool, std::allocator<bool>>,
    nuostl::nuo_vector<int, std::allocator<int>>>);
}

void CheckConstruction()
{
  nuostl::nuo_vector<bool> empty;
  assert(empty.empty() && empty.size() == 0 && empty.capacity() == 0);
  assert(empty.begin() == empty.end() && empty.cbegin() == empty.cend());

  nuostl::nuo_vector<bool> sized(5);
  assert(sized.size() == 5);
  for (std::size_t index = 0; index < sized.size(); ++index)
  {
    assert(!sized[index]);
  }

  nuostl::nuo_vector<bool> filled(5, true);
  assert(filled.size() == 5);
  for (std::size_t index = 0; index < filled.size(); ++index)
  {
    assert(filled[index]);
  }

  const bool raw[] = {true, false, true, true};
  nuostl::nuo_vector<bool> from_range(raw, raw + 4);
  assert(from_range.size() == 4);
  assert(from_range[0] && !from_range[1] && from_range[2] && from_range[3]);

  nuostl::nuo_vector<bool> from_list{false, true, true};
  assert(from_list.size() == 3);
  assert(!from_list[0] && from_list[1] && from_list[2]);

  nuostl::nuo_vector<bool> copied(from_list);
  assert(copied.size() == from_list.size());
  for (std::size_t index = 0; index < copied.size(); ++index)
  {
    assert(copied[index] == from_list[index]);
  }
  copied[0] = true;
  assert(!from_list[0] && copied[0]);

  nuostl::nuo_vector<bool> moved(std::move(copied));
  assert(moved.size() == 3 && moved[0]);
  assert(copied.empty());
  copied.push_back(true);
  assert(copied.size() == 1 && copied[0]);

  std::allocator<bool> allocator;
  nuostl::nuo_vector<bool> with_allocator(3, false, allocator);
  assert(with_allocator.size() == 3);
  assert(with_allocator.get_allocator() == allocator);

  nuostl::nuo_vector<bool> explicit_allocator(allocator);
  assert(explicit_allocator.empty());

  const nuostl::nuo_vector<bool> const_source{true, true};
  nuostl::nuo_vector<bool> const_copied(const_source, allocator);
  assert(const_copied.size() == 2 && const_copied[0] && const_copied[1]);
}

void CheckElementAccess()
{
  nuostl::nuo_vector<bool> values(4, false);
  values[0] = true;
  values[3] = values[0];
  assert(values[0] && !values[1] && !values[2] && values[3]);
  assert(values.front() && values.back());

  values.front() = false;
  values.back() = false;
  assert(!values.front() && !values.back());

  values[1].flip();
  assert(values[1]);
  values[1].flip();
  assert(!values[1]);

  assert(~values[0]);
  values[0].flip();
  assert(values[0] && !~values[0]);

  bool caught = false;
  try
  {
    values.at(4);
  }
  catch (const std::out_of_range&)
  {
    caught = true;
  }
  assert(caught);

  const nuostl::nuo_vector<bool>& const_values = values;
  caught = false;
  try
  {
    const_values.at(4);
  }
  catch (const std::out_of_range&)
  {
    caught = true;
  }
  assert(caught);
  assert(const_values.at(1) == values[1]);
}

void CheckCapacity()
{
  nuostl::nuo_vector<bool> values;
  assert(values.max_size() > 0);

  values.reserve(200);
  assert(values.capacity() >= 200);
  assert(values.empty());
  const std::size_t reserved = values.capacity();
  assert(reserved % 64 == 0);

  values.reserve(10);
  assert(values.capacity() == reserved);

  for (std::size_t index = 0; index < 200; ++index)
  {
    values.push_back(SampleValue(index));
  }
  assert(values.size() == 200);
  assert(values.capacity() >= 200);
  for (std::size_t index = 0; index < 200; ++index)
  {
    assert(values[index] == SampleValue(index));
  }

  values.resize(50);
  values.shrink_to_fit();
  assert(values.size() == 50);
  assert(values.capacity() == 64);
  assert(values.capacity() >= values.size());
  for (std::size_t index = 0; index < 50; ++index)
  {
    assert(values[index] == SampleValue(index));
  }

  values.shrink_to_fit();
  assert(values.capacity() == 64);

  values.clear();
  values.shrink_to_fit();
  assert(values.capacity() == 0 && values.empty());
}

void CheckModifiers()
{
  nuostl::nuo_vector<bool> values;
  values.push_back(true);
  values.push_back(false);
  values.emplace_back(true);
  assert(values.size() == 3);
  assert(values[0] && !values[1] && values[2]);

  const bool inserted = values.emplace_back(0);
  assert(!inserted && values.size() == 4 && !values[3]);

  values.pop_back();
  assert(values.size() == 3);

  values.resize(6, true);
  assert(values.size() == 6);
  assert(values[3] && values[4] && values[5]);

  values.resize(5);
  assert(values.size() == 5 && values.back());

  values.resize(2);
  assert(values.size() == 2 && values[0] && !values[1]);

  values.assign(4, true);
  assert(values.size() == 4 && values[0] && values[3]);

  const bool raw[] = {false, true, false};
  values.assign(raw, raw + 3);
  assert(values.size() == 3 && !values[0] && values[1] && !values[2]);

  values.assign({true, true, true, true});
  assert(values.size() == 4 && values[3]);

  values.flip();
  assert(values.size() == 4);
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    assert(!values[index]);
  }

  values.clear();
  assert(values.empty());

  nuostl::nuo_vector<bool> other{true, false};
  values.swap(other);
  assert(values.size() == 2 && values[0] && !values[1]);
  assert(other.empty());
  swap(values, other);
  assert(values.empty() && other.size() == 2);

  values = other;
  assert(values.size() == 2);
  values = std::move(other);
  assert(values.size() == 2 && other.empty());

  nuostl::nuo_vector<bool> self{true, true};
  auto* alias = &self;
  self = *alias;
  assert(self.size() == 2 && self[0] && self[1]);
  self = std::move(*alias);
  assert(self.size() == 2 && self[0] && self[1]);

  values = {false, false, true};
  assert(values.size() == 3 && values[2]);
  assert(!values.insert(values.cbegin(), true).operator*() == false);
}

void CheckBitShifting()
{
  for (std::size_t size = 0; size <= 130; ++size)
  {
    for (std::size_t position = 0; position <= size; ++position)
    {
      for (std::size_t count = 0; count <= 3; ++count)
      {
        nuostl::nuo_vector<bool> actual;
        std::vector<bool> expected;
        for (std::size_t index = 0; index < size; ++index)
        {
          const bool value = SampleValue(index);
          actual.push_back(value);
          expected.push_back(value);
        }

        const auto actual_insert = actual.insert(
          actual.cbegin() + static_cast<std::ptrdiff_t>(position), count, true);
        expected.insert(expected.begin() +
                          static_cast<std::ptrdiff_t>(position), count, true);
        assert(actual.size() == expected.size());
        for (std::size_t index = 0; index < actual.size(); ++index)
        {
          assert(static_cast<bool>(actual[index]) == expected[index]);
        }
        assert(actual_insert == actual.begin() +
                 static_cast<std::ptrdiff_t>(position));

        const std::size_t erase_count =
          std::min<std::size_t>(count, actual.size() - position);
        const auto erase_begin =
          actual.cbegin() + static_cast<std::ptrdiff_t>(position);
        const auto erase_end = actual.cbegin() +
          static_cast<std::ptrdiff_t>(position + erase_count);
        const auto actual_erase = actual.erase(erase_begin, erase_end);
        expected.erase(expected.begin() +
                         static_cast<std::ptrdiff_t>(position),
                       expected.begin() +
                         static_cast<std::ptrdiff_t>(position + erase_count));
        assert(actual.size() == expected.size());
        for (std::size_t index = 0; index < actual.size(); ++index)
        {
          assert(static_cast<bool>(actual[index]) == expected[index]);
        }
        assert(actual_erase == actual.begin() +
                 static_cast<std::ptrdiff_t>(position));
      }

      nuostl::nuo_vector<bool> single;
      std::vector<bool> single_expected;
      for (std::size_t index = 0; index < size; ++index)
      {
        single.push_back(SampleValue(index));
        single_expected.push_back(SampleValue(index));
      }
      const auto inserted_it = single.insert(
        single.cbegin() + static_cast<std::ptrdiff_t>(position), true);
      single_expected.insert(single_expected.begin() +
                               static_cast<std::ptrdiff_t>(position), true);
      assert(single.size() == single_expected.size());
      for (std::size_t index = 0; index < single.size(); ++index)
      {
        assert(static_cast<bool>(single[index]) == single_expected[index]);
      }
      assert(inserted_it == single.begin() +
               static_cast<std::ptrdiff_t>(position));

      if (size != 0)
      {
        const std::size_t erased_position = position == size ? size - 1
                                                             : position;
        const auto erased_it = single.erase(
          single.cbegin() + static_cast<std::ptrdiff_t>(erased_position));
        single_expected.erase(single_expected.begin() +
                                static_cast<std::ptrdiff_t>(erased_position));
        assert(single.size() == single_expected.size());
        for (std::size_t index = 0; index < single.size(); ++index)
        {
          assert(static_cast<bool>(single[index]) == single_expected[index]);
        }
        assert(erased_it == single.begin() +
                 static_cast<std::ptrdiff_t>(erased_position));
      }

      nuostl::nuo_vector<bool> range(4, false);
      std::vector<bool> range_expected(4, false);
      const bool middle[] = {true, true};
      const std::size_t range_position = std::min(position, range.size());
      range.insert(range.cbegin() +
                     static_cast<std::ptrdiff_t>(range_position),
                   middle, middle + 2);
      range_expected.insert(range_expected.begin() +
                              static_cast<std::ptrdiff_t>(range_position),
                            middle, middle + 2);
      assert(range.size() == range_expected.size());
      for (std::size_t index = 0; index < range.size(); ++index)
      {
        assert(static_cast<bool>(range[index]) == range_expected[index]);
      }

      const std::size_t list_position = std::min(position, range.size());
      range.insert(range.cbegin() +
                     static_cast<std::ptrdiff_t>(list_position),
                   {true, false});
      range_expected.insert(range_expected.begin() +
                              static_cast<std::ptrdiff_t>(list_position),
                            {true, false});
      assert(range.size() == range_expected.size());
      for (std::size_t index = 0; index < range.size(); ++index)
      {
        assert(static_cast<bool>(range[index]) == range_expected[index]);
      }
    }
  }
}

void CheckIterators()
{
  nuostl::nuo_vector<bool> values{true, false, true, false, true};
  assert(std::distance(values.begin(), values.end()) == 5);
  assert(std::count(values.begin(), values.end(), true) == 3);

  std::size_t forwards = 0;
  for (nuostl::nuo_vector<bool>::const_iterator iterator = values.begin();
       iterator != values.end(); ++iterator)
  {
    assert(static_cast<bool>(*iterator) == static_cast<bool>(values[forwards]));
    ++forwards;
  }
  assert(forwards == 5);

  std::size_t backwards = values.size();
  for (nuostl::nuo_vector<bool>::iterator iterator = values.end();
       iterator != values.begin();)
  {
    --iterator;
    --backwards;
    assert(static_cast<bool>(*iterator) ==
           static_cast<bool>(values[backwards]));
  }
  assert(backwards == 0);

  nuostl::nuo_vector<bool>::iterator iterator = values.begin();
  assert(static_cast<bool>(iterator[2]));
  assert(static_cast<bool>(*(iterator + 4)));
  assert(static_cast<bool>(*(4 + iterator)));
  assert(iterator + 5 == values.end());
  assert(values.end() - iterator == 5);
  assert(iterator < iterator + 1);
  assert(iterator <= iterator);
  assert(iterator + 2 > iterator);
  assert(iterator + 2 >= iterator + 2);
  assert(iterator != iterator + 1);

  iterator += 3;
  assert(static_cast<bool>(*iterator) == false);
  iterator -= 2;
  assert(static_cast<bool>(*iterator) == static_cast<bool>(values[1]));
  assert(static_cast<bool>(*iterator++) == static_cast<bool>(values[1]));
  assert(static_cast<bool>(*iterator) == static_cast<bool>(values[2]));
  assert(static_cast<bool>(*iterator--) == static_cast<bool>(values[2]));
  assert(static_cast<bool>(*iterator) == static_cast<bool>(values[1]));

  nuostl::nuo_vector<bool>::const_iterator const_iterator = values.begin();
  assert(const_iterator == values.begin());
  assert(const_iterator != values.end());

  std::size_t reversed = 0;
  for (auto reverse = values.rbegin(); reverse != values.rend(); ++reverse)
  {
    ++reversed;
    assert(static_cast<bool>(*reverse) ==
           static_cast<bool>(values[values.size() - reversed]));
  }
  assert(reversed == values.size());

  const nuostl::nuo_vector<bool>& const_values = values;
  assert(std::count(const_values.begin(), const_values.end(), true) == 3);
  assert(std::count(const_values.crbegin(), const_values.crend(), true) == 3);
  assert(std::distance(const_values.cbegin(), const_values.cend()) == 5);

  /* Standard algorithms must work through the proxy iterators. */
  nuostl::nuo_vector<bool> sorted{true, false, true, true, false};
  std::sort(sorted.begin(), sorted.end());
  const bool expected_sorted[] = {false, false, true, true, true};
  for (std::size_t index = 0; index < sorted.size(); ++index)
  {
    assert(static_cast<bool>(sorted[index]) == expected_sorted[index]);
  }

  std::reverse(sorted.begin(), sorted.end());
  const bool expected_reversed[] = {true, true, true, false, false};
  for (std::size_t index = 0; index < sorted.size(); ++index)
  {
    assert(static_cast<bool>(sorted[index]) == expected_reversed[index]);
  }
}

void CheckPackedStorage()
{
  CountingAllocator<bool>::allocated_elements = 0;
  CountingAllocator<bool>::deallocated_elements = 0;
  CountingAllocator<bool>::allocation_calls = 0;
  CountingAllocator<bool>::deallocation_calls = 0;

  {
    /* 640 bits must be ten sixty-four bit words, not 640 elements. */
    nuostl::nuo_vector<bool, CountingAllocator<bool>> packed(640);
    assert(CountingAllocator<std::uint64_t>::allocated_elements == 10);
    assert(CountingAllocator<std::uint64_t>::allocation_calls == 1);
    assert(packed.capacity() == 640);
    assert(packed.size() == 640);

    packed[0] = true;
    packed[639] = true;
    assert(packed[0] && packed[639] && !packed[1] && !packed[638]);

    packed.reserve(1280);
    assert(CountingAllocator<std::uint64_t>::allocated_elements == 30);
    assert(packed[0] && packed[639]);

    packed.shrink_to_fit();
    assert(CountingAllocator<std::uint64_t>::allocated_elements == 40);
    assert(packed.capacity() == 640 && packed[0] && packed[639]);
  }
  assert(CountingAllocator<std::uint64_t>::deallocated_elements ==
         CountingAllocator<std::uint64_t>::allocated_elements);

  /* Pool backed storage must be released again; a single word request is
     served by the pool while several words fall back to the heap. */
  const std::size_t baseline = nuostl::NuoBlockPool::TotalBlocksInUse();
  {
    using PoolBool = nuostl::NuoPoolAllocator<bool>;
    using WordPool = nuostl::NuoPoolAllocator<std::uint64_t>;

    nuostl::nuo_vector<bool, PoolBool> single_word(32, false);
    assert(single_word.size() == 32 && single_word.capacity() == 64);
    assert(WordPool::Pool().BlocksInUse() == 1);
    for (std::size_t index = 0; index < single_word.size(); index += 2)
    {
      single_word[index] = true;
    }
    assert(single_word[0] && !single_word[1] && single_word[30] &&
           !single_word[31]);
    single_word.clear();
    single_word.shrink_to_fit();
    assert(WordPool::Pool().BlocksInUse() == 0);

    nuostl::nuo_vector<bool, PoolBool> multi_word(128, false);
    assert(multi_word.size() == 128 && multi_word.capacity() == 128);
    assert(WordPool::Pool().BlocksInUse() == 0);
    multi_word[127] = true;
    assert(multi_word[127] && !multi_word[126]);
  }
  assert(nuostl::NuoBlockPool::TotalBlocksInUse() == baseline);
}

void CheckAgainstStandard()
{
  unsigned int state = 20241012u;
  const auto next = [&state]()
  {
    state = state * 1103515245u + 12345u;
    return state >> 16;
  };

  nuostl::nuo_vector<bool> actual;
  std::vector<bool> expected;
  for (int step = 0; step < 4000; ++step)
  {
    const unsigned int choice = next() % 8;
    const bool value = next() % 2 != 0;
    const std::size_t position =
      actual.empty() ? 0 : next() % (actual.size() + 1);
    switch (choice)
    {
      case 0:
        actual.push_back(value);
        expected.push_back(value);
        break;
      case 1:
        if (!actual.empty())
        {
          actual.pop_back();
          expected.pop_back();
        }
        break;
      case 2:
        actual.insert(actual.cbegin() + static_cast<std::ptrdiff_t>(position),
                      value);
        expected.insert(expected.begin() +
                          static_cast<std::ptrdiff_t>(position), value);
        break;
      case 3:
        if (!actual.empty())
        {
          const std::size_t erasure = next() % actual.size();
          actual.erase(actual.cbegin() +
                       static_cast<std::ptrdiff_t>(erasure));
          expected.erase(expected.begin() +
                         static_cast<std::ptrdiff_t>(erasure));
        }
        break;
      case 4:
      {
        const std::size_t target = next() % 300;
        actual.resize(target, value);
        expected.resize(target, value);
        break;
      }
      case 5:
      {
        const std::size_t count = next() % 5;
        const bool filled = next() % 2 != 0;
        actual.insert(actual.cbegin() + static_cast<std::ptrdiff_t>(position),
                      count, filled);
        expected.insert(expected.begin() +
                          static_cast<std::ptrdiff_t>(position), count, filled);
        break;
      }
      case 6:
        actual.flip();
        expected.flip();
        break;
      default:
        if (!actual.empty())
        {
          const std::size_t begin = next() % actual.size();
          const std::size_t end =
            begin + (next() % (actual.size() - begin + 1));
          actual.erase(actual.cbegin() + static_cast<std::ptrdiff_t>(begin),
                       actual.cbegin() + static_cast<std::ptrdiff_t>(end));
          expected.erase(expected.begin() + static_cast<std::ptrdiff_t>(begin),
                         expected.begin() + static_cast<std::ptrdiff_t>(end));
        }
        break;
    }

    assert(actual.size() == expected.size());
    for (std::size_t index = 0; index < actual.size(); ++index)
    {
      assert(static_cast<bool>(actual[index]) == expected[index]);
    }
    assert(actual.capacity() >= actual.size());
  }
}

void CheckSelfRange()
{
  nuostl::nuo_vector<bool> values{true, false, true};
  values.insert(values.cbegin() + 1, values.cbegin(), values.cend());
  const bool duplicated[] = {true, true, false, true, false, true};
  assert(values.size() == 6);
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    assert(static_cast<bool>(values[index]) == duplicated[index]);
  }

  values.assign(values.cbegin(), values.cend());
  assert(values.size() == 6);
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    assert(static_cast<bool>(values[index]) == duplicated[index]);
  }

  nuostl::nuo_vector<bool> other{false, true};
  other.insert(other.cend(), other.begin(), other.end());
  assert(other.size() == 4);
  assert(!other[0] && other[1] && !other[2] && other[3]);

  nuostl::nuo_vector<bool> appended{true};
  appended.insert(appended.cbegin(), appended.cbegin(), appended.cend());
  assert(appended.size() == 2 && appended[0] && appended[1]);
}

} /* namespace */

void TestNuoVectorBool::TestSuite()
{
  CheckTypeProperties();
  CheckConstruction();
  CheckElementAccess();
  CheckCapacity();
  CheckModifiers();
  CheckBitShifting();
  CheckIterators();
  CheckPackedStorage();
  CheckSelfRange();
  CheckAgainstStandard();
}

} /* namespace test */
