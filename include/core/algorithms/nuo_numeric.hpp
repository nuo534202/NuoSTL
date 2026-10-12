#pragma once

#include <functional>
#include <iterator>
#include <utility>

#include "utils/nuo_util.hpp"

namespace nuostl
{

/* cppreference declaration:
   template< class ForwardIt, class T >
   void iota( ForwardIt first, ForwardIt last, T value );
   https://en.cppreference.com/w/cpp/algorithm/iota.html */
template <typename ForwardIterator, typename T>
void NuoIota(ForwardIterator first, ForwardIterator last, T value)
{
  for (; first != last; ++first, ++value)
  {
    *first = value;
  }
}

/* cppreference declaration:
   template< class InputIt, class OutputIt, class BinaryOp >
   OutputIt adjacent_difference( InputIt first, InputIt last,
                                 OutputIt d_first, BinaryOp op );
   https://en.cppreference.com/w/cpp/algorithm/adjacent_difference.html
   The first value is copied unchanged; every later result compares an element
   with its predecessor held in a local, so d_first may equal first. */
template <typename InputIterator, typename OutputIterator,
          typename BinaryOperation>
OutputIterator NuoAdjacentDifference(InputIterator first, InputIterator last,
                                     OutputIterator output,
                                     BinaryOperation operation)
{
  if (first == last)
  {
    return output;
  }
  using Value = typename std::iterator_traits<InputIterator>::value_type;
  Value previous = *first;
  *output = previous;
  ++output;
  while (++first != last)
  {
    Value current = *first;
    *output = operation(current, previous);
    ++output;
    previous = NuoMove(current);
  }
  return output;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator NuoAdjacentDifference(InputIterator first, InputIterator last,
                                     OutputIterator output)
{
  return NuoAdjacentDifference(first, last, output, std::minus<>());
}

/* cppreference declaration:
   template< class InputIt, class OutputIt, class BinaryOp >
   OutputIt partial_sum( InputIt first, InputIt last, OutputIt d_first,
                         BinaryOp op );
   https://en.cppreference.com/w/cpp/algorithm/partial_sum.html
   Running totals are kept in a local so d_first may equal first. */
template <typename InputIterator, typename OutputIterator,
          typename BinaryOperation>
OutputIterator NuoPartialSum(InputIterator first, InputIterator last,
                             OutputIterator output,
                             BinaryOperation operation)
{
  if (first == last)
  {
    return output;
  }
  using Value = typename std::iterator_traits<InputIterator>::value_type;
  Value running = *first;
  *output = running;
  ++output;
  while (++first != last)
  {
    running = operation(NuoMove(running), *first);
    *output = running;
    ++output;
  }
  return output;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator NuoPartialSum(InputIterator first, InputIterator last,
                             OutputIterator output)
{
  return NuoPartialSum(first, last, output, std::plus<>());
}

/* cppreference declaration:
   template< class InputIt1, class InputIt2, class T,
             class BinaryOp1, class BinaryOp2 >
   T inner_product( InputIt1 first1, InputIt1 last1, InputIt2 first2, T init,
                    BinaryOp1 op1, BinaryOp2 op2 );
   https://en.cppreference.com/w/cpp/algorithm/inner_product.html */
template <typename InputIterator1, typename InputIterator2, typename T,
          typename BinaryOperation1, typename BinaryOperation2>
T NuoInnerProduct(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, T initial,
                  BinaryOperation1 sum_operation,
                  BinaryOperation2 product_operation)
{
  for (; first1 != last1; ++first1, ++first2)
  {
    initial = sum_operation(NuoMove(initial),
                            product_operation(*first1, *first2));
  }
  return initial;
}

template <typename InputIterator1, typename InputIterator2, typename T>
T NuoInnerProduct(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, T initial)
{
  return NuoInnerProduct(first1, last1, first2, NuoMove(initial),
                         std::plus<>(), std::multiplies<>());
}

} /* namespace nuostl */
