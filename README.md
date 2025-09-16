# NuoSTL

NuoSTL is a C++ STL (Standard Template Library) implemented by Nuo. It is a C++ STL-like libraries finally.
The project aims to rebuild commonly used STL components from scratch using modern C++ features, mainly for learning, practice, and exploration of STL internals.
The project will also add some additional practical components that is not included in C++ STL like BigInteger.

## Road Map

### 1. C++ STL Core Components

#### Data Types
- [ ] nuo_string – Equivalent of `std::string`
- [ ] nuo_pair – Equivalent of `std::pair`
- [ ] nuo_tuple – Equivalent of `std::tuple`
- [ ] nuo_optional – Equivalent of `std::optional`
- [ ] nuo_variant – Equivalent of `std::variant`
- [ ] nuo_any – Equivalent of `std::any`

#### Sequence Containers
- [ ] nuo_vector – Equivalent of `std::vector`
- [ ] nuo_list – Equivalent of `std::list`
- [ ] nuo_forward_list – Equivalent of `std::forward_list`
- [ ] nuo_deque – Equivalent of `std::deque`
- [ ] nuo_array – Equivalent of `std::array`

#### Associative Containers
- [ ] nuo_set – Equivalent of `std::set`
- [ ] nuo_multiset – Equivalent of `std::multiset`
- [ ] nuo_map – Equivalent of `std::map`
- [ ] nuo_multimap – Equivalent of `std::multimap`

#### Unordered (Hash) Containers
- [ ] nuo_unordered_set – Equivalent of `std::unordered_set`
- [ ] nuo_unordered_multiset – Equivalent of `std::unordered_multiset`
- [ ] nuo_unordered_map – Equivalent of `std::unordered_map`
- [ ] nuo_unordered_multimap – Equivalent of `std::unordered_multimap`

#### Container Adapters
- [ ] nuo_stack – Equivalent of `std::stack`
- [ ] nuo_queue – Equivalent of `std::queue`
- [ ] nuo_priority_queue – Equivalent of `std::priority_queue`

#### Iterators
- [ ] Input Iterator
- [ ] Output Iterator
- [ ] Forward Iterator
- [ ] Bidirectional Iterator
- [ ] Random Access Iterator
- [ ] Iterator Traits

#### Algorithms
- [ ] nuo_sort – Equivalent of `std::sort`
- [ ] nuo_find – Equivalent of `std::find`
- [ ] nuo_copy – Equivalent of `std::copy`
- [ ] nuo_transform – Equivalent of `std::transform`
- [ ] nuo_accumulate – Equivalent of `std::accumulate`
- [ ] nuo_binary_search – Equivalent of `std::binary_search`
- [ ] nuo_merge – Equivalent of `std::merge`
- [ ] nuo_max / nuo_min
- [ ] nuo_for_each – Equivalent of `std::for_each`

#### Function Objects (Functors)
- [ ] nuo_plus – Equivalent of `std::plus`
- [ ] nuo_minus – Equivalent of `std::minus`
- [ ] nuo_multiplies – Equivalent of `std::multiplies`
- [ ] nuo_divides – Equivalent of `std::divides`
- [ ] nuo_modulus – Equivalent of `std::modulus`
- [ ] nuo_equal_to – Equivalent of `std::equal_to`
- [ ] nuo_less – Equivalent of `std::less`
- [ ] nuo_greater – Equivalent of `std::greater`

#### Allocators
- [ ] Default Allocator – Equivalent of `std::allocator`
- [ ] Custom Memory Pool Allocator

### 2. Additional Components
- [ ] BigInteger Class – Arbitrary precision integer type  
- [ ] Fraction Class – Rational number representation  
- [ ] Matrix Class – Linear algebra support  
- [ ] Complex Number Class – Equivalent of `std::complex` but extended  
- [ ] Polynomial Class – Symbolic polynomial arithmetic  


## License

NuoSTL is for learning and practice purpose now, so it is under [MIT License](LICENSE). Feel free to use, modify, and distribute it.
