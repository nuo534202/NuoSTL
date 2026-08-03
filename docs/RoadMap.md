# Road Map

## 1. C++ STL Core Components

### Data Types (TBD)

- [ ] NuoAny – Similar to `std::any`
- [ ] NuoOptional – Similar to `std::optional`
- [x] NuoPair – Similar to `std::pair`
- [ ] NuoString – Similar to `std::string`
- [ ] NuoTuple – Similar to `std::tuple`
- [ ] NuoVariant – Similar to `std::variant`

### Sequence Containers

- [x] NuoArray – Similar to `std::array`
- [x] NuoDeque – Similar to `std::deque`
- [ ] NuoForwardList – Similar to `std::forward_list`
- [x] NuoList – Similar to `std::list`
- [ ] NuoPriorityQueue – Similar to `std::priority_queue`
  - [ ] NuoHeap
- [x] NuoQueue – Similar to `std::queue`
- [ ] NuoSlist (Single Linked List)
- [x] NuoStack – Similar to `std::stack`
- [ ] NuoStringView – Similar to `std::string_view`
- [x] NuoVector – Similar to `std::vector`
  - TODO: replace `to_string` with `nuo_to_string`, refactor
- [ ] NuoVectorBool – Similar to `std::vector<bool>`

### Associative Containers

- [ ] NuoUnorderedMap – Similar to `std::unordered_map`
- [ ] NuoUnorderedMultimap – Similar to `std::unordered_multimap`
- [ ] NuoUnorderedMultiset – Similar to `std::unordered_multiset`
- [ ] NuoUnorderedSet – Similar to `std::unordered_set`
- [ ] NuoMap – Similar to `std::map`
- [ ] NuoMultimap – Similar to `std::multimap`
- [ ] NuoMultiset – Similar to `std::multiset`
- [ ] NuoSet – Similar to `std::set`

TBD: hashtable, rb-tree (red black tree).

### Iterators

- [x] Bidirectional Iterator
- [x] Forward Iterator
- [x] Input Iterator
- [x] Iterator Traits
- [x] Output Iterator
- [x] Random Access Iterator

### Algorithms (TBD)

- [ ] NuoAccumulate – Similar to `std::accumulate`
- [ ] NuoBinarySearch – Similar to `std::binary_search`
- [ ] NuoCopy – Similar to `std::copy`
- [ ] NuoFind – Similar to `std::find`
- [ ] NuoForEach – Similar to `std::for_each`
- [x] NuoMax – Similar to `std::max`
- [ ] NuoMerge – Similar to `std::merge`
- [x] NuoMin – Similar to `std::min`
- [ ] NuoSort – Similar to `std::sort`
- [ ] NuoTransform – Similar to `std::transform`

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
- [ ] Custom Memory Pool Allocator (TBD)

## 2. Extra Components (TBD)

- [ ] BigInteger Class – Arbitrary precision integer type
- [ ] Complex Number Class – Similar to `std::complex` but extended
- [x] Fraction Class – Rational number representation
- [ ] Matrix Class – Linear algebra support
- [ ] Polynomial Class – Symbolic polynomial arithmetic
