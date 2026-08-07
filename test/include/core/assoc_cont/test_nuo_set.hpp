#pragma once

namespace test {

class Test_Nuo_Set {
private:
    static void test_constructor();
    static void test_assignment();
    static void test_iterator();
    static void test_capacity();
    static void test_insert();
    static void test_emplace();
    static void test_erase();
    static void test_lookup();
    static void test_swap();
    static void test_comparison();

    static void test_multiset();

public:
    static void test_nuo_set();
};

}   /* namespace test */
