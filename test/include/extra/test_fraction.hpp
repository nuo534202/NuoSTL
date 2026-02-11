#pragma once

namespace test {

class TestFraction {
public:
    static void test_fraction();

private:
    static void test_default_constructor();
    static void test_parameterized_constructor();
    static void test_constructor_zero_denominator();
    static void test_constructor_normalize();

    static void test_operator_equal();
    static void test_operator_not_equal();
    static void test_operator_greater();
    static void test_operator_greater_equal();
    static void test_operator_less();
    static void test_operator_less_equal();

    static void test_get_up();
    static void test_get_down();
    static void test_get_fraction();
    static void test_set_up();
    static void test_set_down();
    static void test_set_down_zero();

    static void test_normalize();
};

}   /* namespace test */