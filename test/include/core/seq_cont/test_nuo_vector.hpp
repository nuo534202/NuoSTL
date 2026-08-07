#pragma once

namespace test
{

class Test_Nuo_Vector
{
private:
    /* test correctness */
    static void test_constructor();
    static void test_copy_constructor();
    static void test_destructor();
    
    static void test_operator_assign();
    static void test_assign();
    static void test_assign_range();

    static void test_get_allocator();
    static void test_iterators();

    static void test_capacity();

    static void test_elem_access_by_at();
    static void test_elem_access_by_elem();
    static void test_elem_access_by_front();
    static void test_elem_access_by_back();

    static void test_data_access();

    static void test_push_back();
    static void test_pop_back();
    static void test_emplace_back();
    static void test_append_range();
    static void test_insert();
    static void test_insert_range();
    static void test_erase();
    static void test_swap();
    static void test_clear();

    /* test efficiency */

public:
    static void test_nuo_vector();
};

}   /* namespace test */