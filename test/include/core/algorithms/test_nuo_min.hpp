#ifndef NUO_TEST_CORE_ALGORITHMS_NUO_MIN_HPP_
#define NUO_TEST_CORE_ALGORITHMS_NUO_MIN_HPP_

namespace test {

class Test_Nuo_Min {
private:
    static void test_compile_time();
    static void test_nuo_min_basic();
    static void test_nuo_min_custom_compare();
    static void test_nuo_min_initializer_list();
    static void test_nuo_min_execution_policy();
    static void test_nuo_min_pair();
public:
    static void test_nuo_min();
};

}   /* namespace test */

#endif