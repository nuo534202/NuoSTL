#pragma once

#include <functional>
#include <iterator>
#include <random>
#include <utility>

namespace nuostl
{

/* cppreference declaration:
   template< class InputIt, class UnaryPred >
   bool all_of( InputIt first, InputIt last, UnaryPred p );
   https://en.cppreference.com/w/cpp/algorithm/all_any_none_of.html */
template <typename InputIterator, typename UnaryPredicate>
bool NuoAllOf(InputIterator first, InputIterator last, UnaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    if (!predicate(*first))
    {
      return false;
    }
  }
  return true;
}

template <typename InputIterator, typename UnaryPredicate>
bool NuoAnyOf(InputIterator first, InputIterator last,
              UnaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    if (predicate(*first))
    {
      return true;
    }
  }
  return false;
}

template <typename InputIterator, typename UnaryPredicate>
bool NuoNoneOf(InputIterator first, InputIterator last,
               UnaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    if (predicate(*first))
    {
      return false;
    }
  }
  return true;
}

/* cppreference declaration:
   template< class InputIt, class T >
   typename iterator_traits<InputIt>::difference_type
   count( InputIt first, InputIt last, const T& value );
   https://en.cppreference.com/w/cpp/algorithm/count.html */
template <typename InputIterator, typename T>
typename std::iterator_traits<InputIterator>::difference_type
NuoCount(InputIterator first, InputIterator last, const T& value)
{
  typename std::iterator_traits<InputIterator>::difference_type result = 0;
  for (; first != last; ++first)
  {
    if (*first == value)
    {
      ++result;
    }
  }
  return result;
}

template <typename InputIterator, typename UnaryPredicate>
typename std::iterator_traits<InputIterator>::difference_type
NuoCountIf(InputIterator first, InputIterator last,
           UnaryPredicate predicate)
{
  typename std::iterator_traits<InputIterator>::difference_type result = 0;
  for (; first != last; ++first)
  {
    if (predicate(*first))
    {
      ++result;
    }
  }
  return result;
}

/* cppreference declaration:
   template< class InputIt, class UnaryPred >
   InputIt find_if( InputIt first, InputIt last, UnaryPred p );
   https://en.cppreference.com/w/cpp/algorithm/find.html */
template <typename InputIterator, typename UnaryPredicate>
InputIterator NuoFindIf(InputIterator first, InputIterator last,
                        UnaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    if (predicate(*first))
    {
      return first;
    }
  }
  return last;
}

template <typename InputIterator, typename UnaryPredicate>
InputIterator NuoFindIfNot(InputIterator first, InputIterator last,
                           UnaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    if (!predicate(*first))
    {
      return first;
    }
  }
  return last;
}

/* cppreference declaration:
   template< class ForwardIt, class BinaryPred >
   ForwardIt adjacent_find( ForwardIt first, ForwardIt last, BinaryPred p );
   https://en.cppreference.com/w/cpp/algorithm/adjacent_find.html */
template <typename ForwardIterator, typename BinaryPredicate>
ForwardIterator NuoAdjacentFind(ForwardIterator first, ForwardIterator last,
                                BinaryPredicate predicate)
{
  if (first == last)
  {
    return last;
  }
  ForwardIterator next = first;
  for (++next; next != last; ++next, ++first)
  {
    if (predicate(*first, *next))
    {
      return first;
    }
  }
  return last;
}

template <typename ForwardIterator>
ForwardIterator NuoAdjacentFind(ForwardIterator first, ForwardIterator last)
{
  return NuoAdjacentFind(first, last, std::equal_to<>());
}

/* cppreference declaration:
   template< class ForwardIt1, class ForwardIt2 >
   ForwardIt1 search( ForwardIt1 first, ForwardIt1 last,
                      ForwardIt2 s_first, ForwardIt2 s_last );
   https://en.cppreference.com/w/cpp/algorithm/search.html */
template <typename ForwardIterator1, typename ForwardIterator2,
          typename BinaryPredicate>
ForwardIterator1 NuoSearch(ForwardIterator1 first, ForwardIterator1 last,
                           ForwardIterator2 sfirst, ForwardIterator2 slast,
                           BinaryPredicate predicate)
{
  for (;; ++first)
  {
    ForwardIterator1 current = first;
    for (ForwardIterator2 needle = sfirst;; ++current, ++needle)
    {
      if (needle == slast)
      {
        return first;
      }
      if (current == last || !predicate(*current, *needle))
      {
        break;
      }
    }
    if (first == last)
    {
      return last;
    }
  }
}

template <typename ForwardIterator1, typename ForwardIterator2>
ForwardIterator1 NuoSearch(ForwardIterator1 first, ForwardIterator1 last,
                           ForwardIterator2 sfirst, ForwardIterator2 slast)
{
  return NuoSearch(first, last, sfirst, slast, std::equal_to<>());
}

/* cppreference declaration:
   template< class ForwardIt, class Size, class T >
   ForwardIt search_n( ForwardIt first, ForwardIt last,
                       Size count, const T& value );
   https://en.cppreference.com/w/cpp/algorithm/search_n.html */
template <typename ForwardIterator, typename Size, typename T,
          typename BinaryPredicate>
ForwardIterator NuoSearchN(ForwardIterator first, ForwardIterator last,
                           Size count, const T& value,
                           BinaryPredicate predicate)
{
  if (count <= 0)
  {
    return first;
  }
  for (; first != last; ++first)
  {
    if (!predicate(*first, value))
    {
      continue;
    }
    ForwardIterator current = first;
    Size matched = 0;
    while (matched != count && current != last && predicate(*current, value))
    {
      ++current;
      ++matched;
    }
    if (matched == count)
    {
      return first;
    }
  }
  return last;
}

template <typename ForwardIterator, typename Size, typename T>
ForwardIterator NuoSearchN(ForwardIterator first, ForwardIterator last,
                           Size count, const T& value)
{
  return NuoSearchN(first, last, count, value, std::equal_to<>());
}

/* cppreference declaration:
   template< class ForwardIt1, class ForwardIt2 >
   ForwardIt1 find_end( ForwardIt1 first, ForwardIt1 last,
                        ForwardIt2 s_first, ForwardIt2 s_last );
   https://en.cppreference.com/w/cpp/algorithm/find_end.html */
template <typename ForwardIterator1, typename ForwardIterator2,
          typename BinaryPredicate>
ForwardIterator1 NuoFindEnd(ForwardIterator1 first, ForwardIterator1 last,
                            ForwardIterator2 sfirst, ForwardIterator2 slast,
                            BinaryPredicate predicate)
{
  if (sfirst == slast)
  {
    return last;
  }
  ForwardIterator1 result = last;
  while (first != last)
  {
    ForwardIterator1 found = NuoSearch(first, last, sfirst, slast, predicate);
    if (found == last)
    {
      break;
    }
    result = found;
    first = found;
    ++first;
  }
  return result;
}

template <typename ForwardIterator1, typename ForwardIterator2>
ForwardIterator1 NuoFindEnd(ForwardIterator1 first, ForwardIterator1 last,
                            ForwardIterator2 sfirst, ForwardIterator2 slast)
{
  return NuoFindEnd(first, last, sfirst, slast, std::equal_to<>());
}

/* cppreference declaration:
   template< class InputIt, class ForwardIt >
   InputIt find_first_of( InputIt first, InputIt last,
                          ForwardIt s_first, ForwardIt s_last );
   https://en.cppreference.com/w/cpp/algorithm/find_first_of.html */
template <typename InputIterator, typename ForwardIterator,
          typename BinaryPredicate>
InputIterator NuoFindFirstOf(InputIterator first, InputIterator last,
                             ForwardIterator sfirst, ForwardIterator slast,
                             BinaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    for (ForwardIterator current = sfirst; current != slast; ++current)
    {
      if (predicate(*first, *current))
      {
        return first;
      }
    }
  }
  return last;
}

template <typename InputIterator, typename ForwardIterator>
InputIterator NuoFindFirstOf(InputIterator first, InputIterator last,
                             ForwardIterator sfirst, ForwardIterator slast)
{
  return NuoFindFirstOf(first, last, sfirst, slast, std::equal_to<>());
}

/* cppreference declaration:
   template< class ForwardIt, class Compare >
   ForwardIt min_element( ForwardIt first, ForwardIt last, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/min_element.html */
template <typename ForwardIterator, typename Compare>
ForwardIterator NuoMinElement(ForwardIterator first, ForwardIterator last,
                              Compare compare)
{
  if (first == last)
  {
    return last;
  }
  ForwardIterator smallest = first;
  while (++first != last)
  {
    if (compare(*first, *smallest))
    {
      smallest = first;
    }
  }
  return smallest;
}

template <typename ForwardIterator>
ForwardIterator NuoMinElement(ForwardIterator first, ForwardIterator last)
{
  return NuoMinElement(first, last, std::less<>());
}

/* cppreference declaration:
   template< class ForwardIt, class Compare >
   ForwardIt max_element( ForwardIt first, ForwardIt last, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/max_element.html */
template <typename ForwardIterator, typename Compare>
ForwardIterator NuoMaxElement(ForwardIterator first, ForwardIterator last,
                              Compare compare)
{
  if (first == last)
  {
    return last;
  }
  ForwardIterator largest = first;
  while (++first != last)
  {
    if (compare(*largest, *first))
    {
      largest = first;
    }
  }
  return largest;
}

template <typename ForwardIterator>
ForwardIterator NuoMaxElement(ForwardIterator first, ForwardIterator last)
{
  return NuoMaxElement(first, last, std::less<>());
}

/* cppreference declaration:
   template< class InputIt1, class InputIt2, class Compare >
   bool includes( InputIt1 first1, InputIt1 last1,
                  InputIt2 first2, InputIt2 last2, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/includes.html */
template <typename InputIterator1, typename InputIterator2, typename Compare>
bool NuoIncludes(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2,
                 Compare compare)
{
  for (; first2 != last2; ++first1)
  {
    if (first1 == last1 || compare(*first2, *first1))
    {
      return false;
    }
    if (!compare(*first1, *first2))
    {
      ++first2;
    }
  }
  return true;
}

template <typename InputIterator1, typename InputIterator2>
bool NuoIncludes(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2)
{
  return NuoIncludes(first1, last1, first2, last2, std::less<>());
}

/* cppreference declaration:
   template< class ForwardIt1, class ForwardIt2, class BinaryPred >
   bool is_permutation( ForwardIt1 first1, ForwardIt1 last1,
                        ForwardIt2 first2, ForwardIt2 last2, BinaryPred p );
   https://en.cppreference.com/w/cpp/algorithm/is_permutation.html */
template <typename ForwardIterator1, typename ForwardIterator2,
          typename BinaryPredicate>
bool NuoIsPermutation(ForwardIterator1 first1, ForwardIterator1 last1,
                      ForwardIterator2 first2, ForwardIterator2 last2,
                      BinaryPredicate predicate)
{
  if (std::distance(first1, last1) != std::distance(first2, last2))
  {
    return false;
  }
  for (ForwardIterator1 current = first1; current != last1; ++current)
  {
    typename std::iterator_traits<ForwardIterator1>::difference_type left = 0;
    for (ForwardIterator1 scan = first1; scan != last1; ++scan)
    {
      if (predicate(*current, *scan))
      {
        ++left;
      }
    }
    typename std::iterator_traits<ForwardIterator2>::difference_type right = 0;
    for (ForwardIterator2 scan = first2; scan != last2; ++scan)
    {
      if (predicate(*current, *scan))
      {
        ++right;
      }
    }
    if (left != right)
    {
      return false;
    }
  }
  return true;
}

template <typename ForwardIterator1, typename ForwardIterator2>
bool NuoIsPermutation(ForwardIterator1 first1, ForwardIterator1 last1,
                      ForwardIterator2 first2, ForwardIterator2 last2)
{
  return NuoIsPermutation(first1, last1, first2, last2, std::equal_to<>());
}

/* cppreference declaration:
   template< class ForwardIt, class Generator >
   void generate( ForwardIt first, ForwardIt last, Generator g );
   https://en.cppreference.com/w/cpp/algorithm/generate.html */
template <typename ForwardIterator, typename Generator>
void NuoGenerate(ForwardIterator first, ForwardIterator last,
                 Generator generator)
{
  for (; first != last; ++first)
  {
    *first = generator();
  }
}

/* cppreference declaration:
   template< class OutputIt, class Size, class Generator >
   OutputIt generate_n( OutputIt first, Size count, Generator g );
   https://en.cppreference.com/w/cpp/algorithm/generate_n.html */
template <typename OutputIterator, typename Size, typename Generator>
OutputIterator NuoGenerateN(OutputIterator first, Size count,
                            Generator generator)
{
  for (; count > 0; --count, ++first)
  {
    *first = generator();
  }
  return first;
}

/* cppreference declaration:
   template< class BidirIt >
   void reverse( BidirIt first, BidirIt last );
   https://en.cppreference.com/w/cpp/algorithm/reverse.html */
template <typename BidirectionalIterator>
void NuoReverse(BidirectionalIterator first, BidirectionalIterator last)
{
  while (first != last && first != --last)
  {
    using std::swap;
    swap(*first, *last);
    ++first;
  }
}

/* cppreference declaration:
   template< class BidirIt, class OutputIt >
   OutputIt reverse_copy( BidirIt first, BidirIt last, OutputIt d_first );
   https://en.cppreference.com/w/cpp/algorithm/reverse_copy.html */
template <typename BidirectionalIterator, typename OutputIterator>
OutputIterator NuoReverseCopy(BidirectionalIterator first,
                              BidirectionalIterator last,
                              OutputIterator output)
{
  while (first != last)
  {
    *output = *--last;
    ++output;
  }
  return output;
}

/* cppreference declaration:
   template< class ForwardIt, class BinaryPred >
   ForwardIt unique( ForwardIt first, ForwardIt last, BinaryPred p );
   https://en.cppreference.com/w/cpp/algorithm/unique.html */
template <typename ForwardIterator, typename BinaryPredicate>
ForwardIterator NuoUnique(ForwardIterator first, ForwardIterator last,
                          BinaryPredicate predicate)
{
  if (first == last)
  {
    return last;
  }
  ForwardIterator result = first;
  while (++first != last)
  {
    if (!predicate(*result, *first))
    {
      *++result = std::move(*first);
    }
  }
  return ++result;
}

template <typename ForwardIterator>
ForwardIterator NuoUnique(ForwardIterator first, ForwardIterator last)
{
  return NuoUnique(first, last, std::equal_to<>());
}

/* cppreference declaration:
   template< class InputIt, class OutputIt, class BinaryPred >
   OutputIt unique_copy( InputIt first, InputIt last,
                         OutputIt d_first, BinaryPred p );
   https://en.cppreference.com/w/cpp/algorithm/unique_copy.html */
template <typename InputIterator, typename OutputIterator,
          typename BinaryPredicate>
OutputIterator NuoUniqueCopy(InputIterator first, InputIterator last,
                             OutputIterator output, BinaryPredicate predicate)
{
  if (first == last)
  {
    return output;
  }
  typename std::iterator_traits<InputIterator>::value_type current = *first;
  *output = current;
  ++output;
  while (++first != last)
  {
    if (!predicate(current, *first))
    {
      current = *first;
      *output = current;
      ++output;
    }
  }
  return output;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator NuoUniqueCopy(InputIterator first, InputIterator last,
                             OutputIterator output)
{
  return NuoUniqueCopy(first, last, output, std::equal_to<>());
}

/* cppreference declaration:
   template< class ForwardIt, class T >
   void replace( ForwardIt first, ForwardIt last,
                 const T& old_value, const T& new_value );
   https://en.cppreference.com/w/cpp/algorithm/replace.html */
template <typename ForwardIterator, typename T>
void NuoReplace(ForwardIterator first, ForwardIterator last,
                const T& old_value, const T& new_value)
{
  for (; first != last; ++first)
  {
    if (*first == old_value)
    {
      *first = new_value;
    }
  }
}

template <typename ForwardIterator, typename UnaryPredicate, typename T>
void NuoReplaceIf(ForwardIterator first, ForwardIterator last,
                  UnaryPredicate predicate, const T& new_value)
{
  for (; first != last; ++first)
  {
    if (predicate(*first))
    {
      *first = new_value;
    }
  }
}

template <typename InputIterator, typename OutputIterator, typename T>
OutputIterator NuoReplaceCopy(InputIterator first, InputIterator last,
                              OutputIterator output, const T& old_value,
                              const T& new_value)
{
  for (; first != last; ++first, ++output)
  {
    *output = *first == old_value ? new_value : *first;
  }
  return output;
}

template <typename InputIterator, typename OutputIterator,
          typename UnaryPredicate, typename T>
OutputIterator NuoReplaceCopyIf(InputIterator first, InputIterator last,
                                OutputIterator output,
                                UnaryPredicate predicate,
                                const T& new_value)
{
  for (; first != last; ++first, ++output)
  {
    *output = predicate(*first) ? new_value : *first;
  }
  return output;
}

/* cppreference declaration:
   template< class ForwardIt, class T >
   ForwardIt remove( ForwardIt first, ForwardIt last, const T& value );
   https://en.cppreference.com/w/cpp/algorithm/remove.html */
template <typename ForwardIterator, typename T>
ForwardIterator NuoRemove(ForwardIterator first, ForwardIterator last,
                          const T& value)
{
  ForwardIterator result = first;
  for (; first != last; ++first)
  {
    if (!(*first == value))
    {
      *result = std::move(*first);
      ++result;
    }
  }
  return result;
}

template <typename ForwardIterator, typename UnaryPredicate>
ForwardIterator NuoRemoveIf(ForwardIterator first, ForwardIterator last,
                            UnaryPredicate predicate)
{
  ForwardIterator result = first;
  for (; first != last; ++first)
  {
    if (!predicate(*first))
    {
      *result = std::move(*first);
      ++result;
    }
  }
  return result;
}

template <typename InputIterator, typename OutputIterator, typename T>
OutputIterator NuoRemoveCopy(InputIterator first, InputIterator last,
                             OutputIterator output, const T& value)
{
  for (; first != last; ++first)
  {
    if (!(*first == value))
    {
      *output = *first;
      ++output;
    }
  }
  return output;
}

template <typename InputIterator, typename OutputIterator,
          typename UnaryPredicate>
OutputIterator NuoRemoveCopyIf(InputIterator first, InputIterator last,
                               OutputIterator output,
                               UnaryPredicate predicate)
{
  for (; first != last; ++first)
  {
    if (!predicate(*first))
    {
      *output = *first;
      ++output;
    }
  }
  return output;
}

/* cppreference declaration:
   template< class ForwardIt >
   ForwardIt rotate( ForwardIt first, ForwardIt middle, ForwardIt last );
   https://en.cppreference.com/w/cpp/algorithm/rotate.html */
template <typename ForwardIterator>
ForwardIterator NuoRotate(ForwardIterator first, ForwardIterator middle,
                          ForwardIterator last)
{
  if (first == middle)
  {
    return last;
  }
  if (middle == last)
  {
    return first;
  }
  ForwardIterator write = first;
  ForwardIterator next_read = first;
  ForwardIterator result = last;
  bool outermost = true;
  for (;;)
  {
    for (ForwardIterator read = middle; read != last; ++read, ++write)
    {
      if (write == next_read)
      {
        next_read = read;
      }
      using std::swap;
      swap(*write, *read);
    }
    if (outermost)
    {
      /* The first swap run ends where the old first element lands. */
      result = write;
      outermost = false;
    }
    if (write == next_read || next_read == last)
    {
      break;
    }
    middle = next_read;
    next_read = write;
  }
  return result;
}

/* cppreference declaration:
   template< class ForwardIt, class OutputIt >
   OutputIt rotate_copy( ForwardIt first, ForwardIt middle, ForwardIt last,
                         OutputIt d_first );
   https://en.cppreference.com/w/cpp/algorithm/rotate_copy.html */
template <typename ForwardIterator, typename OutputIterator>
OutputIterator NuoRotateCopy(ForwardIterator first, ForwardIterator middle,
                             ForwardIterator last, OutputIterator output)
{
  for (ForwardIterator current = middle; current != last; ++current)
  {
    *output = *current;
    ++output;
  }
  for (ForwardIterator current = first; current != middle; ++current)
  {
    *output = *current;
    ++output;
  }
  return output;
}

/* cppreference declaration:
   template< class BidirIt, class Compare >
   bool next_permutation( BidirIt first, BidirIt last, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/next_permutation.html */
template <typename BidirectionalIterator, typename Compare>
bool NuoNextPermutation(BidirectionalIterator first,
                        BidirectionalIterator last,
                        Compare compare)
{
  if (first == last)
  {
    return false;
  }
  BidirectionalIterator pivot = last;
  if (first == --pivot)
  {
    return false;
  }
  for (;;)
  {
    BidirectionalIterator suffix = pivot;
    --pivot;
    if (compare(*pivot, *suffix))
    {
      BidirectionalIterator successor = last;
      while (!compare(*pivot, *--successor))
      {
      }
      using std::swap;
      swap(*pivot, *successor);
      NuoReverse(suffix, last);
      return true;
    }
    if (pivot == first)
    {
      NuoReverse(first, last);
      return false;
    }
  }
}

template <typename BidirectionalIterator>
bool NuoNextPermutation(BidirectionalIterator first,
                        BidirectionalIterator last)
{
  return NuoNextPermutation(first, last, std::less<>());
}

/* cppreference declaration:
   template< class BidirIt, class Compare >
   bool prev_permutation( BidirIt first, BidirIt last, Compare comp );
   https://en.cppreference.com/w/cpp/algorithm/prev_permutation.html */
template <typename BidirectionalIterator, typename Compare>
bool NuoPrevPermutation(BidirectionalIterator first,
                        BidirectionalIterator last,
                        Compare compare)
{
  if (first == last)
  {
    return false;
  }
  BidirectionalIterator pivot = last;
  if (first == --pivot)
  {
    return false;
  }
  for (;;)
  {
    BidirectionalIterator suffix = pivot;
    --pivot;
    if (compare(*suffix, *pivot))
    {
      BidirectionalIterator successor = last;
      while (!compare(*--successor, *pivot))
      {
      }
      using std::swap;
      swap(*pivot, *successor);
      NuoReverse(suffix, last);
      return true;
    }
    if (pivot == first)
    {
      NuoReverse(first, last);
      return false;
    }
  }
}

template <typename BidirectionalIterator>
bool NuoPrevPermutation(BidirectionalIterator first,
                        BidirectionalIterator last)
{
  return NuoPrevPermutation(first, last, std::less<>());
}

/* cppreference declaration:
   template< class RandomIt, class URBG >
   void shuffle( RandomIt first, RandomIt last, URBG&& g );
   https://en.cppreference.com/w/cpp/algorithm/random_shuffle.html */
template <typename RandomIterator, typename URBG>
void NuoShuffle(RandomIterator first, RandomIterator last, URBG&& generator)
{
  using Difference =
    typename std::iterator_traits<RandomIterator>::difference_type;
  using Distribution = std::uniform_int_distribution<Difference>;
  Distribution distribution;
  for (Difference index = (last - first) - 1; index > 0; --index)
  {
    const Difference chosen = distribution(
      generator, typename Distribution::param_type(0, index));
    if (chosen != index)
    {
      using std::swap;
      swap(first[index], first[chosen]);
    }
  }
}

} /* namespace nuostl */
