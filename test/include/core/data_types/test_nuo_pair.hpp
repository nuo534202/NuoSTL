#ifndef NUOSTL_TEST_CORE_TEST_NUO_PAIR_HPP_
#define NUOSTL_TEST_CORE_TEST_NUO_PAIR_HPP_

namespace test {

class Test_Nuo_Pair {
public:
    static void test_compile_time(); 

    static void test_constructor();
    static void test_copy_constructor();
    static void test_operator();
    static void test_swap();
    static void test_make_pair();

    static void test_get();

    static void test_nuo_pair();
};

}   /* namespace test */

#endif