#pragma once

#include <stddef.h>

#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>

#include "utils/nuo_construct.hpp"
#include "utils/nuo_destroy.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

template <typename InputIterator, typename OutputIterator>
OutputIterator NuoCopy(InputIterator first, InputIterator last,
                       OutputIterator output)
{
  for (; first != last; ++first, ++output)
  {
    *output = *first;
  }
  return output;
}

template <typename BidirectionalIterator1, typename BidirectionalIterator2>
BidirectionalIterator2 NuoCopyBackward(BidirectionalIterator1 first,
                                       BidirectionalIterator1 last,
                                       BidirectionalIterator2 output_last)
{
  while (first != last)
  {
    *--output_last = *--last;
  }
  return output_last;
}

template <typename ForwardIterator, typename T>
void NuoFill(ForwardIterator first, ForwardIterator last, const T& value)
{
  for (; first != last; ++first)
  {
    *first = value;
  }
}

template <typename OutputIterator, typename Size, typename T>
OutputIterator NuoFillN(OutputIterator first, Size count, const T& value)
{
  for (; count > 0; --count, ++first)
  {
    *first = value;
  }
  return first;
}

template <typename InputIterator1, typename InputIterator2, typename BinaryPred>
bool NuoEqual(InputIterator1 first1, InputIterator1 last1,
              InputIterator2 first2, BinaryPred pred)
{
  for (; first1 != last1; ++first1, ++first2)
  {
    if (!pred(*first1, *first2))
    {
      return false;
    }
  }
  return true;
}

template <typename InputIterator1, typename InputIterator2>
bool NuoEqual(InputIterator1 first1, InputIterator1 last1,
              InputIterator2 first2)
{
  return NuoEqual(first1, last1, first2, std::equal_to<>());
}

template <typename InputIterator1, typename InputIterator2, typename Compare>
bool NuoLexicographicalCompare(InputIterator1 first1, InputIterator1 last1,
                               InputIterator2 first2, InputIterator2 last2,
                               Compare comp)
{
  for (; first1 != last1 && first2 != last2; ++first1, ++first2)
  {
    if (comp(*first1, *first2))
    {
      return true;
    }
    if (comp(*first2, *first1))
    {
      return false;
    }
  }
  return first1 == last1 && first2 != last2;
}

template <typename InputIterator1, typename InputIterator2>
bool NuoLexicographicalCompare(InputIterator1 first1, InputIterator1 last1,
                               InputIterator2 first2, InputIterator2 last2)
{
  return NuoLexicographicalCompare(first1, last1, first2, last2,
                                   std::less<>());
}

template <typename InputIterator, typename T, typename BinaryOperation>
T NuoAccumulate(InputIterator first, InputIterator last, T initial,
                BinaryOperation operation)
{
  for (; first != last; ++first)
  {
    initial = operation(NuoMove(initial), *first);
  }
  return initial;
}

template <typename InputIterator, typename T>
T NuoAccumulate(InputIterator first, InputIterator last, T initial)
{
  return NuoAccumulate(first, last, NuoMove(initial), std::plus<>());
}

template <typename InputIterator, typename T>
InputIterator NuoFind(InputIterator first, InputIterator last, const T& value)
{
  for (; first != last; ++first)
  {
    if (*first == value)
    {
      return first;
    }
  }
  return last;
}

template <typename InputIterator, typename Function>
Function NuoForEach(InputIterator first, InputIterator last, Function function)
{
  for (; first != last; ++first)
  {
    function(*first);
  }
  return function;
}

template <typename InputIterator, typename OutputIterator, typename UnaryOp>
OutputIterator NuoTransform(InputIterator first, InputIterator last,
                            OutputIterator output, UnaryOp operation)
{
  for (; first != last; ++first, ++output)
  {
    *output = operation(*first);
  }
  return output;
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator, typename BinaryOp>
OutputIterator NuoTransform(InputIterator1 first1, InputIterator1 last1,
                            InputIterator2 first2, OutputIterator output,
                            BinaryOp operation)
{
  for (; first1 != last1; ++first1, ++first2, ++output)
  {
    *output = operation(*first1, *first2);
  }
  return output;
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator, typename Compare>
OutputIterator NuoMerge(InputIterator1 first1, InputIterator1 last1,
                        InputIterator2 first2, InputIterator2 last2,
                        OutputIterator output, Compare comp)
{
  while (first1 != last1 && first2 != last2)
  {
    if (comp(*first2, *first1))
    {
      *output++ = *first2++;
    }
    else
    {
      *output++ = *first1++;
    }
  }
  output = NuoCopy(first1, last1, output);
  return NuoCopy(first2, last2, output);
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator>
OutputIterator NuoMerge(InputIterator1 first1, InputIterator1 last1,
                        InputIterator2 first2, InputIterator2 last2,
                        OutputIterator output)
{
  return NuoMerge(first1, last1, first2, last2, output, std::less<>());
}

template <typename ForwardIterator, typename T, typename Compare>
bool NuoBinarySearch(ForwardIterator first, ForwardIterator last,
                     const T& value, Compare comp)
{
  using Difference =
    typename std::iterator_traits<ForwardIterator>::difference_type;
  using Category =
    typename std::iterator_traits<ForwardIterator>::iterator_category;
  Difference count = 0;
  if constexpr (std::is_base_of_v<NuoRandomAccessIteratorTag, Category> ||
                std::is_base_of_v<std::random_access_iterator_tag, Category>)
  {
    count = last - first;
  }
  else
  {
    for (ForwardIterator current = first; current != last; ++current)
    {
      ++count;
    }
  }
  while (count > 0)
  {
    auto step = count / 2;
    ForwardIterator middle = first;
    if constexpr (std::is_base_of_v<NuoRandomAccessIteratorTag, Category> ||
                  std::is_base_of_v<std::random_access_iterator_tag, Category>)
    {
      middle += step;
    }
    else
    {
      for (decltype(step) offset = 0; offset < step; ++offset)
      {
        ++middle;
      }
    }
    if (comp(*middle, value))
    {
      first = ++middle;
      count -= step + 1;
    }
    else
    {
      count = step;
    }
  }
  return first != last && !comp(value, *first);
}

template <typename ForwardIterator, typename T>
bool NuoBinarySearch(ForwardIterator first, ForwardIterator last,
                     const T& value)
{
  return NuoBinarySearch(first, last, value, std::less<>());
}

template <typename InputIterator, typename OutputIterator>
OutputIterator NuoUninitializedCopy(InputIterator first, InputIterator last,
                                    OutputIterator output)
{
  OutputIterator current = output;
  try
  {
    for (; first != last; ++first, ++current)
    {
      NuoConstruct(&*current, *first);
    }
    return current;
  }
  catch (...)
  {
    std::destroy(output, current);
    throw;
  }
}

template <typename ForwardIterator, typename T>
void NuoUninitializedFill(ForwardIterator first, ForwardIterator last,
                          const T& value)
{
  ForwardIterator current = first;
  try
  {
    for (; current != last; ++current)
    {
      NuoConstruct(&*current, value);
    }
  }
  catch (...)
  {
    std::destroy(first, current);
    throw;
  }
}

template <typename ForwardIterator, typename Size, typename T>
ForwardIterator NuoUninitializedFillN(ForwardIterator first, Size count,
                                      const T& value)
{
  ForwardIterator current = first;
  try
  {
    for (; count > 0; --count, ++current)
    {
      NuoConstruct(&*current, value);
    }
    return current;
  }
  catch (...)
  {
    std::destroy(first, current);
    throw;
  }
}

} /* namespace nuostl */
