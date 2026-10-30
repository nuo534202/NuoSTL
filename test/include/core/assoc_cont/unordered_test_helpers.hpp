#pragma once

#include <cstddef>

#include "nuostl.hpp"

namespace test
{
namespace detail
{
/* Describe the equivalent-key layout found in one bucket. */
struct EquivalentRunResult
{
  std::size_t match_count;
  bool is_contiguous;
};

/* Measure one equivalent-key run while checking for a later second run. */
template <typename Container, typename Predicate>
EquivalentRunResult InspectEquivalentRun(
    const Container& container,
    typename Container::size_type bucket,
    Predicate is_equivalent)
{
  std::size_t match_count = 0;
  bool found_equivalent = false;
  bool equivalent_run_finished = false;
  bool is_contiguous = true;
  for (typename Container::const_local_iterator it = container.CBegin(bucket);
       it != container.CEnd(bucket); ++it)
  {
    if (is_equivalent(*it))
    {
      if (equivalent_run_finished)
      {
        is_contiguous = false;
      }
      found_equivalent = true;
      ++match_count;
    }
    else if (found_equivalent)
    {
      equivalent_run_finished = true;
    }
  }
  return {match_count, found_equivalent && is_contiguous};
}

/* Count the elements returned by EqualRange. */
template <typename Container, typename Key>
std::size_t EqualRangeCount(const Container& container, const Key& key)
{
  nuostl::nuo_pair<typename Container::const_iterator,
                   typename Container::const_iterator>
      range = container.EqualRange(key);
  std::size_t count = 0;
  for (typename Container::const_iterator it = range.first;
       it != range.second; ++it)
  {
    ++count;
  }
  return count;
}
} /* namespace detail */
} /* namespace test */
