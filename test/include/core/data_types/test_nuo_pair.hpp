#pragma once

namespace test {

class Test_Nuo_Pair {
private:
    static void test_compile_time(); 

    static void test_constructor();
    static void test_copy_constructor();
    static void test_operator();
    static void test_swap();
    static void test_make_pair();

    static void test_get();

public:
    static void test_nuo_pair();
};

}   /* namespace test */