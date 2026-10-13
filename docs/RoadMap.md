# Road Map

## 1. C++ STL Core Components

### Data Types (TBD)

- [x] NuoAny – Similar to `std::any`
- [x] NuoOptional – Similar to `std::optional`
- [x] NuoPair – Similar to `std::pair`
- [x] NuoString – Similar to `std::string`
- [x] NuoTuple – Similar to `std::tuple`
- [x] NuoVariant – Similar to `std::variant`

### Sequence Containers

- [x] NuoArray – Similar to `std::array`
- [x] NuoDeque – Similar to `std::deque`
- [x] NuoForwardList – Similar to `std::forward_list`
- [x] NuoList – Similar to `std::list`
- [x] NuoPriorityQueue – Similar to `std::priority_queue`
  - [x] NuoHeap
- [x] NuoQueue – Similar to `std::queue`
- [x] NuoStack – Similar to `std::stack`
- [x] NuoStringView – Similar to `std::string_view`
- [x] NuoVector – Similar to `std::vector`
  - TODO: replace `to_string` with `nuo_to_string`, refactor
- [x] NuoVectorBool – Similar to `std::vector<bool>`

### Associative Containers

- [x] NuoUnorderedMap – Similar to `std::unordered_map`
- [x] NuoUnorderedMultimap – Similar to `std::unordered_multimap`
- [x] NuoUnorderedMultiset – Similar to `std::unordered_multiset`
- [x] NuoUnorderedSet – Similar to `std::unordered_set`
- [x] NuoMap – Similar to `std::map` (wraps `NuoRbTree`)
- [x] NuoMultimap – Similar to `std::multimap` (wraps `NuoRbTree`)
- [x] NuoMultiset – Similar to `std::multiset` (wraps `NuoRbTree`)
- [x] NuoSet – Similar to `std::set` (wraps `NuoRbTree`)

### Iterators

- [x] Bidirectional Iterator
- [x] Forward Iterator
- [x] Input Iterator
- [x] Iterator Traits
- [x] Output Iterator
- [x] Random Access Iterator

### Algorithms (TBD)

- [x] NuoAccumulate – Similar to `std::accumulate`
- [x] NuoAdjacentFind – Similar to `std::adjacent_find`
- [x] NuoAllOf – Similar to `std::all_of`
- [x] NuoAnyOf – Similar to `std::any_of`
- [x] NuoBinarySearch – Similar to `std::binary_search`
  - [x] NuoLowerBound – Similar to `std::lower_bound`
  - [x] NuoUpperBound – Similar to `std::upper_bound`
  - [x] NuoEqualRange – Similar to `std::equal_range`
- [x] NuoCopy – Similar to `std::copy`
- [x] NuoCount – Similar to `std::count`
- [x] NuoCountIf – Similar to `std::count_if`
- [x] NuoFind – Similar to `std::find`
- [x] NuoFindEnd – Similar to `std::find_end`
- [x] NuoFindFirstOf – Similar to `std::find_first_of`
- [x] NuoFindIf – Similar to `std::find_if`
- [x] NuoFindIfNot – Similar to `std::find_if_not`
- [x] NuoForEach – Similar to `std::for_each`
- [x] NuoGenerate – Similar to `std::generate`
- [x] NuoGenerateN – Similar to `std::generate_n`
- [x] NuoIncludes – Similar to `std::includes`
- [x] NuoIsPermutation – Similar to `std::is_permutation`
- [x] NuoMax – Similar to `std::max`
- [x] NuoMaxElement – Similar to `std::max_element`
- [x] NuoMerge – Similar to `std::merge`
- [x] NuoMin – Similar to `std::min`
- [x] NuoMinElement – Similar to `std::min_element`
- [x] NuoNoneOf – Similar to `std::none_of`
- [x] NuoRemove – Similar to `std::remove`
- [x] NuoRemoveCopy – Similar to `std::remove_copy`
- [x] NuoRemoveCopyIf – Similar to `std::remove_copy_if`
- [x] NuoRemoveIf – Similar to `std::remove_if`
- [x] NuoReplace – Similar to `std::replace`
- [x] NuoReplaceCopy – Similar to `std::replace_copy`
- [x] NuoReplaceCopyIf – Similar to `std::replace_copy_if`
- [x] NuoReplaceIf – Similar to `std::replace_if`
- [x] NuoReverse – Similar to `std::reverse`
- [x] NuoReverseCopy – Similar to `std::reverse_copy`
- [x] NuoSearch – Similar to `std::search`
- [x] NuoSearchN – Similar to `std::search_n`
- [x] NuoSort – Similar to `std::sort`
- [x] NuoTransform – Similar to `std::transform`
- [x] NuoUnique – Similar to `std::unique`
- [x] NuoUniqueCopy – Similar to `std::unique_copy`
- [x] NuoNextPermutation – Similar to `std::next_permutation`
- [x] NuoPrevPermutation – Similar to `std::prev_permutation`
- [x] NuoRotate – Similar to `std::rotate`
  - [x] NuoRotateCopy – Similar to `std::rotate_copy`
- [x] NuoSetDifference – Similar to `std::set_difference`
- [x] NuoSetIntersection – Similar to `std::set_intersection`
- [x] NuoSetSymmetricDifference – Similar to `std::set_symmetric_difference`
- [x] NuoSetUnion – Similar to `std::set_union`
- [x] NuoShuffle – Similar to `std::shuffle`

### Numeric Algorithms

- [x] NuoAccumulate – Similar to `std::accumulate` (declared with the utility algorithms)
- [x] NuoAdjacentDifference – Similar to `std::adjacent_difference`
- [x] NuoInnerProduct – Similar to `std::inner_product`
- [x] NuoIota – Similar to `std::iota`
- [x] NuoPartialSum – Similar to `std::partial_sum`

### Function Objects (Functors)

- [x] NuoDivides – Similar to `std::divides`
- [x] NuoEqualTo – Similar to `std::equal_to`
- [x] NuoGreater – Similar to `std::greater`
- [x] NuoMinus – Similar to `std::minus`
- [x] NuoModulus – Similar to `std::modulus`
- [x] NuoMultiplies – Similar to `std::multiplies`
- [x] NuoLess – Similar to `std::less`
- [x] NuoPlus – Similar to `std::plus`

### Allocators

- [x] Default Allocator – Similar to `std::allocator`  
- [x] NuoBlockPool – Fixed-size block pool used by `NuoPoolAllocator`
- [x] NuoPoolAllocator – Allocator that serves single objects from a memory pool

## 2. Extra Components (TBD)

- [x] BigInteger Class – Arbitrary precision integer type ([API](api/extra/big_integer.md))
- [x] Complex Number Class – Similar to `std::complex` but extended ([API](api/extra/complex.md))
- [x] Fraction Class – Rational number representation ([API](api/extra/fraction.md))
- [x] Matrix Class – Linear algebra support ([API](api/extra/matrix.md))
- [x] Polynomial Class – Symbolic polynomial arithmetic ([API](api/extra/polynomial.md))
