#include "./core/seq_cont/test_nuo_vector.hpp"

#include <assert.h>

#include <iostream>
#include <vector>

#include "nuostl.hpp"

namespace test
{

/* test correctness */
/* TODO: did not test all constructor and copy constructor */
/* TODO: change comments' type */
void Test_Nuo_Vector::test_constructor()
{    
    /* Default constructor */
    nuostl::nuo_vector<int> v1;
    assert(v1.size() == 0);
    assert(v1.empty());
    
    // Constructor with allocator
    std::allocator<int> alloc;
    nuostl::nuo_vector<int> v2(alloc);
    assert(v2.size() == 0);
    assert(v2.empty());
    
    // Constructor with count (default-initialized)
    nuostl::nuo_vector<int> v3(5);
    assert(v3.size() == 5);
    assert(v3.capacity() >= 5);
    for (size_t i = 0; i < v3.size(); ++i) {
        assert(v3[i] == 0);
    }
    
    // Constructor with count and value
    nuostl::nuo_vector<int> v4(10, 42);
    assert(v4.size() == 10);
    assert(v4.capacity() >= 10);
    for (size_t i = 0; i < v4.size(); ++i) {
        assert(v4[i] == 42);
    }
    
    // Constructor with iterators
    std::vector<int> std_vec = {1, 2, 3, 4, 5};
    nuostl::nuo_vector<int> v5(std_vec.begin(), std_vec.end());
    assert(v5.size() == 5);
    for (size_t i = 0; i < v5.size(); ++i) {
        assert(v5[i] == std_vec[i]);
    }
    
    // Constructor with initializer_list
    nuostl::nuo_vector<int> v6 = {10, 20, 30, 40, 50};
    assert(v6.size() == 5);
    assert(v6[0] == 10);
    assert(v6[4] == 50);
    
    // Empty constructor with count = 0
    nuostl::nuo_vector<int> v7(0);
    assert(v7.size() == 0);
    assert(v7.empty());
}

void Test_Nuo_Vector::test_copy_constructor()
{    
    // Copy empty vector
    nuostl::nuo_vector<int> v1;
    nuostl::nuo_vector<int> v2(v1);
    assert(v2.size() == 0);
    assert(v2.empty());
    
    // Copy non-empty vector
    nuostl::nuo_vector<int> v3 = {1, 2, 3, 4, 5};
    nuostl::nuo_vector<int> v4(v3);
    assert(v4.size() == v3.size());
    for (size_t i = 0; i < v4.size(); ++i) {
        assert(v4[i] == v3[i]);
    }
    
    // Verify deep copy (modify one, check other unchanged)
    v4[0] = 100;
    assert(v3[0] == 1);
    assert(v4[0] == 100);
    
    // Copy large vector
    nuostl::nuo_vector<int> v5(1000, 42);
    nuostl::nuo_vector<int> v6(v5);
    assert(v6.size() == 1000);
    for (size_t i = 0; i < v6.size(); ++i) {
        assert(v6[i] == 42);
    }
}

void Test_Nuo_Vector::test_destructor()
{    
    // Test that destructor doesn't crash with empty vector
    nuostl::nuo_vector<int> v1;
    
    // Test that destructor properly cleans up non-empty vector
    nuostl::nuo_vector<int> v2(100, 42);
    
    // Test with complex types
    nuostl::nuo_vector<std::string> v3 = {"hello", "world", "test"};
    
    // Test with large vector
    nuostl::nuo_vector<int> v4(10000);
}

void Test_Nuo_Vector::test_operator_assign()
{
    // Copy assignment from empty to empty
    nuostl::nuo_vector<int> v1;
    nuostl::nuo_vector<int> v2;
    v2 = v1;
    assert(v2.size() == 0);
    
    // Copy assignment from non-empty to empty
    nuostl::nuo_vector<int> v3 = {1, 2, 3, 4, 5};
    nuostl::nuo_vector<int> v4;
    v4 = v3;
    assert(v4.size() == 5);
    for (size_t i = 0; i < v4.size(); ++i) {
        assert(v4[i] == v3[i]);
    }
    
    // Copy assignment from empty to non-empty
    nuostl::nuo_vector<int> v5 = {10, 20, 30};
    nuostl::nuo_vector<int> v6;
    v5 = v6;
    assert(v5.size() == 0);
    assert(v5.empty());
    
    // Copy assignment from non-empty to non-empty (smaller to larger)
    nuostl::nuo_vector<int> v7 = {1, 2};
    nuostl::nuo_vector<int> v8 = {10, 20, 30, 40, 50};
    v8 = v7;
    assert(v8.size() == 2);
    assert(v8[0] == 1);
    assert(v8[1] == 2);
    
    // Copy assignment from non-empty to non-empty (larger to smaller)
    nuostl::nuo_vector<int> v9 = {1, 2, 3, 4, 5, 6, 7};
    nuostl::nuo_vector<int> v10 = {10, 20};
    v10 = v9;
    assert(v10.size() == 7);
    for (size_t i = 0; i < v10.size(); ++i) {
        assert(v10[i] == v9[i]);
    }
    
    // Self-assignment
    nuostl::nuo_vector<int> v11 = {1, 2, 3};
    nuostl::nuo_vector<int> temp_vec;

#ifdef __clang__    
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

    temp_vec = v11 = v11;

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

    assert(v11.size() == 3);
    assert(temp_vec.size() == 3);
    assert(temp_vec[0] == 1), assert(v11[0] == 1);
    assert(temp_vec[1] == 2), assert(v11[1] == 2);
    assert(temp_vec[2] == 3), assert(v11[2] == 3);
    
    // Move assignment
    nuostl::nuo_vector<int> v12 = {1, 2, 3, 4, 5};
    nuostl::nuo_vector<int> v13;
    v13 = std::move(v12);
    assert(v13.size() == 5);
    assert(v13[0] == 1);
    assert(v13[4] == 5);
    
    // Initializer list assignment
    nuostl::nuo_vector<int> v14;
    v14 = {100, 200, 300};
    assert(v14.size() == 3);
    assert(v14[0] == 100);
    assert(v14[2] == 300);
}

void Test_Nuo_Vector::test_assign()
{
    // Assign count and value
    nuostl::nuo_vector<int> v1;
    v1.assign(5, 42);
    assert(v1.size() == 5);
    for (size_t i = 0; i < v1.size(); ++i) {
        assert(v1[i] == 42);
    }
    
    // Assign to non-empty vector
    nuostl::nuo_vector<int> v2 = {1, 2, 3};
    v2.assign(10, 99);
    assert(v2.size() == 10);
    for (size_t i = 0; i < v2.size(); ++i) {
        assert(v2[i] == 99);
    }
    
    // Assign smaller count
    nuostl::nuo_vector<int> v3 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    v3.assign(3, 7);
    assert(v3.size() == 3);
    for (size_t i = 0; i < v3.size(); ++i) {
        assert(v3[i] == 7);
    }
    
    // Assign with iterators
    std::vector<int> src = {10, 20, 30, 40, 50};
    nuostl::nuo_vector<int> v4;
    v4.assign(src.begin(), src.end());
    assert(v4.size() == 5);
    for (size_t i = 0; i < v4.size(); ++i) {
        assert(v4[i] == src[i]);
    }
    
    // Assign with initializer_list
    nuostl::nuo_vector<int> v5 = {1, 2, 3};
    v5.assign({100, 200, 300, 400});
    assert(v5.size() == 4);
    assert(v5[0] == 100);
    assert(v5[3] == 400);
    
    // Assign zero elements
    nuostl::nuo_vector<int> v6 = {1, 2, 3};
    v6.assign(0, 42);
    assert(v6.size() == 0);
    assert(v6.empty());
}

void Test_Nuo_Vector::test_assign_range()
{
    #if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    // Assign from vector range
    std::vector<int> src = {1, 2, 3, 4, 5};
    nuostl::nuo_vector<int> v1;
    v1.assign_range(src);
    assert(v1.size() == 5);
    for (size_t i = 0; i < v1.size(); ++i) {
        assert(v1[i] == src[i]);
    }
    
    // Assign to non-empty vector
    nuostl::nuo_vector<int> v2 = {10, 20, 30};
    std::vector<int> src2 = {100, 200};
    v2.assign_range(src2);
    assert(v2.size() == 2);
    assert(v2[0] == 100);
    assert(v2[1] == 200);
    
    // Assign empty range
    nuostl::nuo_vector<int> v3 = {1, 2, 3};
    std::vector<int> src3;
    v3.assign_range(src3);
    assert(v3.size() == 0);
    assert(v3.empty());
    #endif
}

void Test_Nuo_Vector::test_get_allocator()
{
    // Test default allocator
    nuostl::nuo_vector<int> v1;
    auto alloc1 = v1.get_allocator();
    static_assert(std::is_same_v<decltype(alloc1), std::allocator<int>>);
    
    // Test custom allocator
    std::allocator<int> custom_alloc;
    nuostl::nuo_vector<int> v2(custom_alloc);
    auto alloc2 = v2.get_allocator();
    static_assert(std::is_same_v<decltype(alloc2), std::allocator<int>>);
}

void Test_Nuo_Vector::test_iterators()
{
    // Test begin/end
    nuostl::nuo_vector<int> v1 = {1, 2, 3, 4, 5};
    int sum = 0;
    for (auto it = v1.begin(); it != v1.end(); ++it) {
        sum += *it;
    }
    assert(sum == 15);
    
    // Test const iterators
    const nuostl::nuo_vector<int> v2 = {10, 20, 30};
    sum = 0;
    for (auto it = v2.begin(); it != v2.end(); ++it) {
        sum += *it;
    }
    assert(sum == 60);
    
    // Test cbegin/cend
    nuostl::nuo_vector<int> v3 = {1, 2, 3};
    sum = 0;
    for (auto it = v3.cbegin(); it != v3.cend(); ++it) {
        sum += *it;
    }
    assert(sum == 6);
    
    // Test reverse iterators
    nuostl::nuo_vector<int> v4 = {1, 2, 3, 4, 5};
    std::vector<int> reversed;
    for (auto it = v4.rbegin(); it != v4.rend(); ++it) {
        reversed.push_back(*it);
    }
    assert(reversed.size() == 5);
    assert(reversed[0] == 5);
    assert(reversed[4] == 1);
    
    // Test const reverse iterators
    const nuostl::nuo_vector<int> v5 = {10, 20, 30};
    reversed.clear();
    for (auto it = v5.rbegin(); it != v5.rend(); ++it) {
        reversed.push_back(*it);
    }
    assert(reversed[0] == 30);
    assert(reversed[2] == 10);
    
    // Test crbegin/crend
    nuostl::nuo_vector<int> v6 = {100, 200, 300};
    reversed.clear();
    for (auto it = v6.crbegin(); it != v6.crend(); ++it) {
        reversed.push_back(*it);
    }
    assert(reversed[0] == 300);
    assert(reversed[2] == 100);
    
    // Test empty vector iterators
    nuostl::nuo_vector<int> v7;
    assert(v7.begin() == v7.end());
    assert(v7.cbegin() == v7.cend());
    assert(v7.rbegin() == v7.rend());
    assert(v7.crbegin() == v7.crend());
    
    // Test iterator modification
    nuostl::nuo_vector<int> v8 = {1, 2, 3};
    for (auto it = v8.begin(); it != v8.end(); ++it) {
        *it *= 2;
    }
    assert(v8[0] == 2);
    assert(v8[1] == 4);
    assert(v8[2] == 6);
}

void Test_Nuo_Vector::test_capacity()
{
    // Test empty()
    nuostl::nuo_vector<int> v1;
    assert(v1.empty());
    v1.push_back(1);
    assert(!v1.empty());
    
    // Test size()
    nuostl::nuo_vector<int> v2 = {1, 2, 3, 4, 5};
    assert(v2.size() == 5);
    
    // Test capacity()
    nuostl::nuo_vector<int> v3;
    assert(v3.capacity() == 0);
    v3.reserve(10);
    assert(v3.capacity() >= 10);
    
    // Test reserve()
    nuostl::nuo_vector<int> v4 = {1, 2, 3};
    v4.reserve(100);
    assert(v4.capacity() >= 100);
    assert(v4.size() == 3);
    assert(v4[0] == 1);
    
    // Test reserve() with smaller capacity (should not reduce)
    nuostl::nuo_vector<int> v5;
    v5.reserve(100);
    size_t cap = v5.capacity();
    v5.reserve(10);
    assert(v5.capacity() == cap);
    
    // Test shrink_to_fit()
    nuostl::nuo_vector<int> v6;
    v6.reserve(100);
    v6.push_back(1);
    v6.push_back(2);
    v6.shrink_to_fit();
    assert(v6.capacity() >= 2);
    assert(v6.size() == 2);
    
    // Test max_size()
    nuostl::nuo_vector<int> v7;
    assert(v7.max_size() > 0);
    
    // Test resize() - increase size
    nuostl::nuo_vector<int> v8 = {1, 2, 3};
    v8.resize(5);
    assert(v8.size() == 5);
    assert(v8[0] == 1);
    assert(v8[4] == 0);
    
    // Test resize() - decrease size
    nuostl::nuo_vector<int> v9 = {1, 2, 3, 4, 5};
    v9.resize(2);
    assert(v9.size() == 2);
    assert(v9[0] == 1);
    assert(v9[1] == 2);
    
    // Test resize() with value
    nuostl::nuo_vector<int> v10 = {1, 2};
    v10.resize(5, 99);
    assert(v10.size() == 5);
    assert(v10[0] == 1);
    assert(v10[2] == 99);
    assert(v10[4] == 99);
}

void Test_Nuo_Vector::test_elem_access_by_at()
{
    // Test at() read access
    nuostl::nuo_vector<int> v1 = {10, 20, 30, 40, 50};
    assert(v1.at(0) == 10);
    assert(v1.at(2) == 30);
    assert(v1.at(4) == 50);
    
    // Test at() write access
    v1.at(1) = 99;
    assert(v1.at(1) == 99);
    
    // Test const at()
    const nuostl::nuo_vector<int> v2 = {1, 2, 3};
    assert(v2.at(0) == 1);
    assert(v2.at(2) == 3);
    
    // Test at() out of range exception
    nuostl::nuo_vector<int> v3 = {1, 2, 3};
    bool exception_thrown = false;
    try {
        int val = v3.at(1000);
        (void)val;
    } catch (const std::out_of_range& e) {
        exception_thrown = true;
    }
    assert(exception_thrown);
    
    // Test at() with negative index (will throw)
    exception_thrown = false;
    try {
        int val = v3.at(-1);
        (void)val;
    } catch (const std::out_of_range& e) {
        exception_thrown = true;
    }
    assert(exception_thrown);
}

void Test_Nuo_Vector::test_elem_access_by_elem()
{
    // Test operator[] read access
    nuostl::nuo_vector<int> v1 = {10, 20, 30, 40, 50};
    assert(v1[0] == 10);
    assert(v1[2] == 30);
    assert(v1[4] == 50);
    
    // Test operator[] write access
    v1[1] = 99;
    assert(v1[1] == 99);
    
    // Test const operator[]
    const nuostl::nuo_vector<int> v2 = {1, 2, 3};
    assert(v2[0] == 1);
    assert(v2[2] == 3);
    
    // Test operator[] with all indices
    nuostl::nuo_vector<int> v3 = {100, 200, 300, 400, 500};
    for (size_t i = 0; i < v3.size(); ++i) {
        assert(v3[i] == static_cast<int>(100 * (i + 1)));
    }
    
    // Test modification via operator[]
    for (size_t i = 0; i < v3.size(); ++i) {
        v3[i] = static_cast<int>(i);
    }
    for (size_t i = 0; i < v3.size(); ++i) {
        assert(v3[i] == static_cast<int>(i));
    }
}

void Test_Nuo_Vector::test_elem_access_by_front()
{
    // Test front() read access
    nuostl::nuo_vector<int> v1 = {10, 20, 30};
    assert(v1.front() == 10);
    
    // Test front() write access
    v1.front() = 99;
    assert(v1.front() == 99);
    assert(v1[0] == 99);
    
    // Test const front()
    const nuostl::nuo_vector<int> v2 = {100, 200, 300};
    assert(v2.front() == 100);
    
    // Test front() with single element
    nuostl::nuo_vector<int> v3 = {42};
    assert(v3.front() == 42);
    v3.front() = 77;
    assert(v3.front() == 77);
    
    // Test front() with different types
    nuostl::nuo_vector<std::string> v4 = {"hello", "world"};
    assert(v4.front() == "hello");
    v4.front() = "goodbye";
    assert(v4.front() == "goodbye");
}

void Test_Nuo_Vector::test_elem_access_by_back()
{
    // Test back() read access
    nuostl::nuo_vector<int> v1 = {10, 20, 30};
    assert(v1.back() == 30);
    
    // Test back() write access
    v1.back() = 99;
    assert(v1.back() == 99);
    assert(v1[2] == 99);
    
    // Test const back()
    const nuostl::nuo_vector<int> v2 = {100, 200, 300};
    assert(v2.back() == 300);
    
    // Test back() with single element
    nuostl::nuo_vector<int> v3 = {42};
    assert(v3.back() == 42);
    v3.back() = 77;
    assert(v3.back() == 77);
    
    // Test back() with different types
    nuostl::nuo_vector<std::string> v4 = {"hello", "world"};
    assert(v4.back() == "world");
    v4.back() = "universe";
    assert(v4.back() == "universe");
    
    // Test back() after push_back
    nuostl::nuo_vector<int> v5 = {1, 2, 3};
    v5.push_back(4);
    assert(v5.back() == 4);
}

void Test_Nuo_Vector::test_data_access()
{
    // Test data() with non-empty vector
    nuostl::nuo_vector<int> v1 = {10, 20, 30, 40, 50};
    int* ptr1 = v1.data();
    assert(ptr1 != nullptr);
    assert(ptr1[0] == 10);
    assert(ptr1[4] == 50);
    
    // Test data() modification
    ptr1[2] = 99;
    assert(v1[2] == 99);
    
    // Test const data()
    const nuostl::nuo_vector<int> v2 = {100, 200, 300};
    const int* ptr2 = v2.data();
    assert(ptr2 != nullptr);
    assert(ptr2[0] == 100);
    assert(ptr2[2] == 300);
    
    // Test data() with empty vector
    nuostl::nuo_vector<int> v3;
    int* ptr3 = v3.data();
    assert(ptr3 == nullptr);
    // ptr3 may be nullptr or not, implementation-defined
    
    // Test data() pointer arithmetic
    nuostl::nuo_vector<int> v4 = {1, 2, 3, 4, 5};
    int* ptr4 = v4.data();
    for (size_t i = 0; i < v4.size(); ++i) {
        assert(*(ptr4 + i) == v4[i]);
    }
    
    // Test data() with C-style functions
    nuostl::nuo_vector<int> v5 = {5, 4, 3, 2, 1};
    int* arr = v5.data();
    int sum = 0;
    for (size_t i = 0; i < v5.size(); ++i) {
        sum += arr[i];
    }
    assert(sum == 15);
}

void Test_Nuo_Vector::test_push_back()
{
    // Test push_back on empty vector
    nuostl::nuo_vector<int> v1;
    v1.push_back(10);
    assert(v1.size() == 1);
    assert(v1[0] == 10);
    
    // Test push_back multiple times
    v1.push_back(20);
    v1.push_back(30);
    assert(v1.size() == 3);
    assert(v1[0] == 10);
    assert(v1[1] == 20);
    assert(v1[2] == 30);
    
    // Test push_back with copy
    nuostl::nuo_vector<int> v2;
    int value = 42;
    v2.push_back(value);
    assert(v2[0] == 42);
    value = 99;
    assert(v2[0] == 42);  // Should still be 42
    
    // Test push_back with move
    nuostl::nuo_vector<std::string> v3;
    std::string str = "hello";
    v3.push_back(std::move(str));
    assert(v3[0] == "hello");
    assert(str.empty() || str == "hello");  // str is in valid but unspecified state
    
    // Test push_back many elements (trigger reallocation)
    nuostl::nuo_vector<int> v4;
    for (int i = 0; i < 100; ++i) {
        v4.push_back(i);
    }
    assert(v4.size() == 100);
    for (int i = 0; i < 100; ++i) {
        assert(v4[i] == i);
    }
    
    // Test capacity growth
    nuostl::nuo_vector<int> v5;
    size_t prev_cap = v5.capacity();
    for (int i = 0; i < 10; ++i) {
        v5.push_back(i);
        if (v5.capacity() > prev_cap) {
            prev_cap = v5.capacity();
        }
    }
    assert(v5.size() == 10);
}

void Test_Nuo_Vector::test_pop_back()
{
    // Test pop_back on vector with elements
    nuostl::nuo_vector<int> v1 = {10, 20, 30};
    v1.pop_back();
    assert(v1.size() == 2);
    assert(v1[0] == 10);
    assert(v1[1] == 20);
    
    // Test pop_back until empty
    v1.pop_back();
    assert(v1.size() == 1);
    assert(v1[0] == 10);
    v1.pop_back();
    assert(v1.size() == 0);
    assert(v1.empty());
    
    // Test pop_back with strings
    nuostl::nuo_vector<std::string> v2 = {"hello", "world", "test"};
    v2.pop_back();
    assert(v2.size() == 2);
    assert(v2[0] == "hello");
    assert(v2[1] == "world");
    
    // Test push_back followed by pop_back
    nuostl::nuo_vector<int> v3;
    v3.push_back(100);
    v3.push_back(200);
    v3.pop_back();
    assert(v3.size() == 1);
    assert(v3[0] == 100);
    
    // Test multiple pop_backs
    nuostl::nuo_vector<int> v4 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 5; ++i) {
        v4.pop_back();
    }
    assert(v4.size() == 5);
    assert(v4.back() == 5);
}

void Test_Nuo_Vector::test_emplace_back()
{
    // Test emplace_back on empty vector
    nuostl::nuo_vector<int> v1;
    v1.emplace_back(10);
    assert(v1.size() == 1);
    assert(v1[0] == 10);
    
    // Test emplace_back multiple times
    v1.emplace_back(20);
    v1.emplace_back(30);
    assert(v1.size() == 3);
    assert(v1[0] == 10);
    assert(v1[1] == 20);
    assert(v1[2] == 30);
    
    // Test emplace_back with pair
    nuostl::nuo_vector<std::pair<int, std::string>> v2;
    v2.emplace_back(1, "one");
    v2.emplace_back(2, "two");
    assert(v2.size() == 2);
    assert(v2[0].first == 1);
    assert(v2[0].second == "one");
    assert(v2[1].first == 2);
    assert(v2[1].second == "two");
    
    // Test emplace_back many elements
    nuostl::nuo_vector<int> v3;
    for (int i = 0; i < 50; ++i) {
        v3.emplace_back(i * 2);
    }
    assert(v3.size() == 50);
    for (int i = 0; i < 50; ++i) {
        assert(v3[i] == i * 2);
    }
    
    // Test emplace_back vs push_back (both should work)
    nuostl::nuo_vector<std::string> v4;
    v4.emplace_back("constructed");
    v4.push_back("copied");
    assert(v4.size() == 2);
    assert(v4[0] == "constructed");
    assert(v4[1] == "copied");
}

void Test_Nuo_Vector::test_append_range()
{    
    #if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    // Test append_range to empty vector
    nuostl::nuo_vector<int> v1;
    std::vector<int> src1 = {1, 2, 3};
    v1.append_range(src1);
    assert(v1.size() == 3);
    assert(v1[0] == 1);
    assert(v1[2] == 3);
    
    // Test append_range to non-empty vector
    nuostl::nuo_vector<int> v2 = {10, 20};
    std::vector<int> src2 = {30, 40, 50};
    v2.append_range(src2);
    assert(v2.size() == 5);
    assert(v2[0] == 10);
    assert(v2[1] == 20);
    assert(v2[2] == 30);
    assert(v2[4] == 50);
    
    // Test append_range with empty range
    nuostl::nuo_vector<int> v3 = {1, 2, 3};
    std::vector<int> src3;
    v3.append_range(src3);
    assert(v3.size() == 3);
    
    // Test append_range multiple times
    nuostl::nuo_vector<int> v4;
    std::vector<int> src4a = {1, 2};
    std::vector<int> src4b = {3, 4};
    std::vector<int> src4c = {5, 6};
    v4.append_range(src4a);
    v4.append_range(src4b);
    v4.append_range(src4c);
    assert(v4.size() == 6);
    for (int i = 0; i < 6; ++i) {
        assert(v4[i] == i + 1);
    }
    #endif
}

void Test_Nuo_Vector::test_insert()
{
    // Test insert single element at beginning
    nuostl::nuo_vector<int> v1 = {2, 3, 4};
    auto it1 = v1.insert(v1.begin(), 1);
    assert(v1.size() == 4);
    assert(v1[0] == 1);
    assert(*it1 == 1);
    
    // Test insert single element at end
    nuostl::nuo_vector<int> v2 = {1, 2, 3};
    auto it2 = v2.insert(v2.end(), 4);
    assert(v2.size() == 4);
    assert(v2[3] == 4);
    assert(*it2 == 4);
    
    // Test insert single element in middle
    nuostl::nuo_vector<int> v3 = {1, 2, 4, 5};
    auto it3 = v3.insert(v3.begin() + 2, 3);
    assert(v3.size() == 5);
    assert(v3[2] == 3);
    assert(*it3 == 3);
    for (int i = 0; i < 5; ++i) {
        assert(v3[i] == i + 1);
    }
    
    // Test insert multiple copies
    nuostl::nuo_vector<int> v4 = {1, 5};
    auto it4 = v4.insert(v4.begin() + 1, 3, 99);
    assert(v4.size() == 5);
    assert(v4[0] == 1);
    assert(v4[1] == 99);
    assert(v4[2] == 99);
    assert(v4[3] == 99);
    assert(v4[4] == 5);
    assert(*it4 == 99);
    
    // Test insert zero elements
    nuostl::nuo_vector<int> v5 = {1, 2, 3};
    auto it5 = v5.insert(v5.begin() + 1, 0, 42);
    assert(it5 == v5.begin() + 1);
    assert(v5.size() == 3);
    assert(v5[0] == 1);
    assert(v5[1] == 2);
    
    // Test insert range
    nuostl::nuo_vector<int> v6 = {1, 5, 6};
    std::vector<int> src = {2, 3, 4};
    auto it6 = v6.insert(v6.begin() + 1, src.begin(), src.end());
    assert(v6.size() == 6);
    for (int i = 0; i < 6; ++i) {
        assert(v6[i] == i + 1);
    }
    assert(*it6 == 2);
    
    // Test insert initializer_list
    nuostl::nuo_vector<int> v7 = {1, 5};
    auto it7 = v7.insert(v7.begin() + 1, {2, 3, 4});
    assert(v7.size() == 5);
    for (int i = 0; i < 5; ++i) {
        assert(v7[i] == i + 1);
    }
    assert(*it7 == 2);
    
    // Test insert into empty vector
    nuostl::nuo_vector<int> v8;
    auto it8 = v8.insert(v8.begin(), 42);
    assert(v8.size() == 1);
    assert(v8[0] == 42);
    assert(*it8 == 42);
    
    // Test insert with move
    nuostl::nuo_vector<std::string> v9 = {"hello", "world"};
    std::string str = "beautiful";
    auto it9 = v9.insert(v9.begin() + 1, std::move(str));
    assert(v9.size() == 3);
    assert(v9[1] == "beautiful");
    assert(*it9 == "beautiful");
}

void Test_Nuo_Vector::test_insert_range()
{
    #if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    // Test insert_range at beginning
    nuostl::nuo_vector<int> v1 = {4, 5, 6};
    std::vector<int> src1 = {1, 2, 3};
    auto it1 = v1.insert_range(v1.begin(), src1);
    assert(v1.size() == 6);
    for (int i = 0; i < 6; ++i) {
        assert(v1[i] == i + 1);
    }
    assert(*it1 == 1);
    
    // Test insert_range at end
    nuostl::nuo_vector<int> v2 = {1, 2, 3};
    std::vector<int> src2 = {4, 5, 6};
    auto it2 = v2.insert_range(v2.end(), src2);
    assert(v2.size() == 6);
    for (int i = 0; i < 6; ++i) {
        assert(v2[i] == i + 1);
    }
    assert(*it2 == 4);
    
    // Test insert_range in middle
    nuostl::nuo_vector<int> v3 = {1, 2, 5, 6};
    std::vector<int> src3 = {3, 4};
    auto it3 = v3.insert_range(v3.begin() + 2, src3);
    assert(v3.size() == 6);
    for (int i = 0; i < 6; ++i) {
        assert(v3[i] == i + 1);
    }
    assert(*it3 == 3);
    
    // Test insert_range with empty range
    nuostl::nuo_vector<int> v4 = {1, 2, 3};
    std::vector<int> src4;
    auto it4 = v4.insert_range(v4.begin() + 1, src4);
    assert(it4 == v4.begin() + 1);
    assert(v4.size() == 3);
    assert(v4[0] == 1);
    assert(v4[1] == 2);
    
    // Test insert_range into empty vector
    nuostl::nuo_vector<int> v5;
    std::vector<int> src5 = {10, 20, 30};
    auto it5 = v5.insert_range(v5.begin(), src5);
    assert(v5.size() == 3);
    assert(v5[0] == 10);
    assert(v5[2] == 30);
    assert(*it5 == 10);
    #endif
}

void Test_Nuo_Vector::test_erase()
{
    // Test erase single element at beginning
    nuostl::nuo_vector<int> v1 = {1, 2, 3, 4, 5};
    auto it1 = v1.erase(v1.begin());
    assert(v1.size() == 4);
    assert(v1[0] == 2);
    assert(*it1 == 2);
    
    // Test erase single element at end
    nuostl::nuo_vector<int> v2 = {1, 2, 3, 4, 5};
    auto it2 = v2.erase(v2.end() - 1);
    assert(v2.size() == 4);
    assert(v2[3] == 4);
    assert(it2 == v2.end());
    
    // Test erase single element in middle
    nuostl::nuo_vector<int> v3 = {1, 2, 3, 4, 5};
    auto it3 = v3.erase(v3.begin() + 2);
    assert(v3.size() == 4);
    assert(v3[0] == 1);
    assert(v3[1] == 2);
    assert(v3[2] == 4);
    assert(v3[3] == 5);
    assert(*it3 == 4);
    
    // Test erase range
    nuostl::nuo_vector<int> v4 = {1, 2, 3, 4, 5, 6, 7};
    auto it4 = v4.erase(v4.begin() + 2, v4.begin() + 5);
    assert(v4.size() == 4);
    assert(v4[0] == 1);
    assert(v4[1] == 2);
    assert(v4[2] == 6);
    assert(v4[3] == 7);
    assert(*it4 == 6);
    
    // Test erase entire range
    nuostl::nuo_vector<int> v5 = {1, 2, 3};
    auto it5 = v5.erase(v5.begin(), v5.end());
    assert(v5.size() == 0);
    assert(v5.empty());
    assert(it5 == v5.end());
    
    // Test erase empty range
    nuostl::nuo_vector<int> v6 = {1, 2, 3};
    auto it6 = v6.erase(v6.begin() + 1, v6.begin() + 1);
    assert(it6 == v6.begin() + 1);
    assert(v6.size() == 3);
    assert(v6[0] == 1);
    assert(v6[1] == 2);
    assert(v6[2] == 3);
    
    // Test multiple erases
    nuostl::nuo_vector<int> v7 = {1, 2, 3, 4, 5};
    v7.erase(v7.begin() + 1);
    v7.erase(v7.begin() + 1);
    assert(v7.size() == 3);
    assert(v7[0] == 1);
    assert(v7[1] == 4);
    assert(v7[2] == 5);
    
    // Test erase with strings
    nuostl::nuo_vector<std::string> v8 = {"hello", "beautiful", "world"};
    v8.erase(v8.begin() + 1);
    assert(v8.size() == 2);
    assert(v8[0] == "hello");
    assert(v8[1] == "world");
}

void Test_Nuo_Vector::test_swap()
{
    // Test swap two non-empty vectors
    nuostl::nuo_vector<int> v1 = {1, 2, 3};
    nuostl::nuo_vector<int> v2 = {4, 5, 6, 7};
    v1.swap(v2);
    assert(v1.size() == 4);
    assert(v2.size() == 3);
    assert(v1[0] == 4);
    assert(v1[3] == 7);
    assert(v2[0] == 1);
    assert(v2[2] == 3);
    
    // Test swap empty with non-empty
    nuostl::nuo_vector<int> v3;
    nuostl::nuo_vector<int> v4 = {10, 20, 30};
    v3.swap(v4);
    assert(v3.size() == 3);
    assert(v4.size() == 0);
    assert(v4.empty());
    assert(v3[0] == 10);
    assert(v3[2] == 30);
    
    // Test swap two empty vectors
    nuostl::nuo_vector<int> v5;
    nuostl::nuo_vector<int> v6;
    v5.swap(v6);
    assert(v5.empty());
    assert(v6.empty());
    
    // Test swap with different capacities
    nuostl::nuo_vector<int> v7;
    v7.reserve(100);
    v7.push_back(1);
    nuostl::nuo_vector<int> v8 = {2, 3, 4, 5};
    size_t cap7 = v7.capacity();
    size_t cap8 = v8.capacity();
    v7.swap(v8);
    assert(cap7 == v8.capacity());
    assert(cap8 == v7.capacity());
    assert(v7.size() == 4);
    assert(v8.size() == 1);
    assert(v8[0] == 1);
    assert(v7[0] == 2);
    
    // Test std::swap with nuo_vector
    nuostl::nuo_vector<int> v9 = {1, 2, 3};
    nuostl::nuo_vector<int> v10 = {4, 5};
    std::swap(v9, v10);
    assert(v9.size() == 2);
    assert(v10.size() == 3);
    assert(v9[0] == 4);
    assert(v10[0] == 1);
    
    // Test swap with strings
    nuostl::nuo_vector<std::string> v11 = {"hello", "world"};
    nuostl::nuo_vector<std::string> v12 = {"foo", "bar", "baz"};
    v11.swap(v12);
    assert(v11.size() == 3);
    assert(v12.size() == 2);
    assert(v11[0] == "foo");
    assert(v12[0] == "hello");
}

void Test_Nuo_Vector::test_clear()
{
    // Test clear on non-empty vector
    nuostl::nuo_vector<int> v1 = {1, 2, 3, 4, 5};
    size_t cap1 = v1.capacity();
    v1.clear();
    assert(v1.size() == 0);
    assert(v1.empty());
    assert(v1.capacity() == cap1);  // Capacity should remain unchanged
    
    // Test clear on empty vector
    nuostl::nuo_vector<int> v2;
    v2.clear();
    assert(v2.size() == 0);
    assert(v2.empty());
    
    // Test clear then push_back
    nuostl::nuo_vector<int> v3 = {1, 2, 3};
    v3.clear();
    v3.push_back(100);
    assert(v3.size() == 1);
    assert(v3[0] == 100);
    
    // Test clear with strings
    nuostl::nuo_vector<std::string> v4 = {"hello", "world", "test"};
    v4.clear();
    assert(v4.size() == 0);
    assert(v4.empty());
    
    // Test multiple clears
    nuostl::nuo_vector<int> v5 = {1, 2, 3};
    v5.clear();
    v5.clear();
    v5.clear();
    assert(v5.size() == 0);
    assert(v5.empty());
    
    // Test clear followed by operations
    nuostl::nuo_vector<int> v6 = {1, 2, 3, 4, 5};
    v6.clear();
    v6.push_back(10);
    v6.push_back(20);
    v6.push_back(30);
    assert(v6.size() == 3);
    assert(v6[0] == 10);
    assert(v6[2] == 30);
    
    // Test iterators after clear
    nuostl::nuo_vector<int> v7 = {1, 2, 3};
    v7.clear();
    assert(v7.begin() == v7.end());
}

/* test efficiency */

void Test_Nuo_Vector::test_nuo_vector()
{
    /* test correctness */
    test_constructor();
    test_copy_constructor();
    test_destructor();

    test_operator_assign();
    test_assign();
    test_assign_range();

    test_get_allocator();
    test_iterators();

    test_capacity();

    test_elem_access_by_at();
    test_elem_access_by_elem();
    test_elem_access_by_front();
    test_elem_access_by_back();

    test_data_access();

    test_push_back();
    test_pop_back();
    test_emplace_back();
    test_append_range();
    test_insert();
    test_insert_range();
    test_erase();
    test_swap();
    test_clear();

    /* test_efficiency */
}

};  /* namespace test */