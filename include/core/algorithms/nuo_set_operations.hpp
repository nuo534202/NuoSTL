#pragma once

#include <functional>

namespace nuostl
{

/* cppreference declaration:
   template< class InputIt1, class InputIt2, class OutputIt, class Compare >
   OutputIt set_union( InputIt1 first1, InputIt1 last1,
                       InputIt2 first2, InputIt2 last2,
                       OutputIt d_first, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/set_union.html */
template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator, typename Compare>
OutputIterator NuoSetUnion(InputIterator1 first1, InputIterator1 last1,
                           InputIterator2 first2, InputIterator2 last2,
                           OutputIterator output, Compare compare)
{
  while (first1 != last1 && first2 != last2)
  {
    if (compare(*first2, *first1))
    {
      *output = *first2;
      ++output;
      ++first2;
    }
    else
    {
      if (!compare(*first1, *first2))
      {
        ++first2;
      }
      *output = *first1;
      ++output;
      ++first1;
    }
  }
  while (first1 != last1)
  {
    *output = *first1;
    ++output;
    ++first1;
  }
  while (first2 != last2)
  {
    *output = *first2;
    ++output;
    ++first2;
  }
  return output;
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator>
OutputIterator NuoSetUnion(InputIterator1 first1, InputIterator1 last1,
                           InputIterator2 first2, InputIterator2 last2,
                           OutputIterator output)
{
  return NuoSetUnion(first1, last1, first2, last2, output, std::less<>());
}

/* cppreference declaration:
   template< class InputIt1, class InputIt2, class OutputIt, class Compare >
   OutputIt set_intersection( InputIt1 first1, InputIt1 last1,
                              InputIt2 first2, InputIt2 last2,
                              OutputIt d_first, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/set_intersection.html */
template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator, typename Compare>
OutputIterator NuoSetIntersection(InputIterator1 first1, InputIterator1 last1,
                                  InputIterator2 first2, InputIterator2 last2,
                                  OutputIterator output, Compare compare)
{
  while (first1 != last1 && first2 != last2)
  {
    if (compare(*first1, *first2))
    {
      ++first1;
    }
    else if (compare(*first2, *first1))
    {
      ++first2;
    }
    else
    {
      *output = *first1;
      ++output;
      ++first1;
      ++first2;
    }
  }
  return output;
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator>
OutputIterator NuoSetIntersection(InputIterator1 first1, InputIterator1 last1,
                                  InputIterator2 first2, InputIterator2 last2,
                                  OutputIterator output)
{
  return NuoSetIntersection(first1, last1, first2, last2, output,
                            std::less<>());
}

/* cppreference declaration:
   template< class InputIt1, class InputIt2, class OutputIt, class Compare >
   OutputIt set_difference( InputIt1 first1, InputIt1 last1,
                            InputIt2 first2, InputIt2 last2,
                            OutputIt d_first, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/set_difference.html */
template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator, typename Compare>
OutputIterator NuoSetDifference(InputIterator1 first1, InputIterator1 last1,
                                InputIterator2 first2, InputIterator2 last2,
                                OutputIterator output, Compare compare)
{
  while (first1 != last1 && first2 != last2)
  {
    if (compare(*first1, *first2))
    {
      *output = *first1;
      ++output;
      ++first1;
    }
    else if (compare(*first2, *first1))
    {
      ++first2;
    }
    else
    {
      ++first1;
      ++first2;
    }
  }
  while (first1 != last1)
  {
    *output = *first1;
    ++output;
    ++first1;
  }
  return output;
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator>
OutputIterator NuoSetDifference(InputIterator1 first1, InputIterator1 last1,
                                InputIterator2 first2, InputIterator2 last2,
                                OutputIterator output)
{
  return NuoSetDifference(first1, last1, first2, last2, output, std::less<>());
}

/* cppreference declaration:
   template< class InputIt1, class InputIt2, class OutputIt, class Compare >
   OutputIt set_symmetric_difference( InputIt1 first1, InputIt1 last1,
                                      InputIt2 first2, InputIt2 last2,
                                      OutputIt d_first, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/set_symmetric_difference.html */
template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator, typename Compare>
OutputIterator NuoSetSymmetricDifference(InputIterator1 first1,
                                         InputIterator1 last1,
                                         InputIterator2 first2,
                                         InputIterator2 last2,
                                         OutputIterator output,
                                         Compare compare)
{
  while (first1 != last1 && first2 != last2)
  {
    if (compare(*first1, *first2))
    {
      *output = *first1;
      ++output;
      ++first1;
    }
    else if (compare(*first2, *first1))
    {
      *output = *first2;
      ++output;
      ++first2;
    }
    else
    {
      ++first1;
      ++first2;
    }
  }
  while (first1 != last1)
  {
    *output = *first1;
    ++output;
    ++first1;
  }
  while (first2 != last2)
  {
    *output = *first2;
    ++output;
    ++first2;
  }
  return output;
}

template <typename InputIterator1, typename InputIterator2,
          typename OutputIterator>
OutputIterator NuoSetSymmetricDifference(InputIterator1 first1,
                                         InputIterator1 last1,
                                         InputIterator2 first2,
                                         InputIterator2 last2,
                                         OutputIterator output)
{
  return NuoSetSymmetricDifference(first1, last1, first2, last2, output,
                                   std::less<>());
}

} /* namespace nuostl */
