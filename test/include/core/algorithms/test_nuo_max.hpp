#ifndef NUOSTL_TEST_CORE_ALGORITHMS_NUO_MAX_HPP
#define NUOSTL_TEST_CORE_ALGORITHMS_NUO_MAX_HPP

namespace test {

class Test_Nuo_Max {
private:
    static void test_compile_time();
    static void test_nuo_max_basic();
    static void test_nuo_max_custom_compare();
    static void test_nuo_max_execution_policy();
    static void test_nuo_max_initializer_list();
    static void test_nuo_max_pair();
public:
    static void test_nuo_max();
};

}   /* namespace test */

#endif