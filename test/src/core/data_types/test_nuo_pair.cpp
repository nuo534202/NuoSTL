#include "./core/data_types/test_nuo_pair.hpp"

#include <assert.h>

#include <string>
#include <tuple>
#include <limits>
#include <type_traits>
#include <utility>

#include "nuostl.hpp"


using nuostl::nuo_pair;
using nuostl::nuo_make_pair;


/* ------------------------------------------------- */
/* Test nuo pair */
void test::Test_Nuo_Pair::test_nuo_pair() {
    test_compile_time();

    test_constructor();
    test_copy_constructor();
    test_operator();
    test_swap();
    test_make_pair();

    test_get();
}

/* ------------------------------------------------- */
/* Test compile time */
void test::Test_Nuo_Pair::test_compile_time() {
    /* compile-time traits checks (relocated from file scope) */
    static_assert(std::tuple_size<nuostl::nuo_pair<int, double>>::value == 2, "tuple_size must be 2");
    static_assert(std::is_same_v<std::tuple_element<0, nuostl::nuo_pair<int, double>>::type, int>, "tuple_element<0> must be int");
    static_assert(std::is_same_v<std::tuple_element<1, nuostl::nuo_pair<int, double>>::type, double>, "tuple_element<1> must be double");
    constexpr nuostl::nuo_pair<int, int> constexpr_np{};
    static_assert(constexpr_np.first == 0 && constexpr_np.second == 0, "constexpr default init");
    static_assert(noexcept(nuostl::nuo_pair<int, int>()), "default constructor must be noexcept");
    static_assert(noexcept(nuostl::nuo_pair<int, int>(1, 2)), "value constructor must be noexcept");
}

/* ------------------------------------------------- */
/* Test constructors */
void test::Test_Nuo_Pair::test_constructor() {
    /* default constructor */
    nuo_pair<int, std::string> p1;
    assert(p1.first == 0);
    assert(p1.second.empty());

    /* value constructor with int + string */
    nuo_pair<int, std::string> p2(42, "hello");
    assert(p2.first == 42);
    assert(p2.second == "hello");

    /* value constructor with double + char */
    nuo_pair<double, char> p3(3.14, 'A');
    assert(p3.first == 3.14);
    assert(p3.second == 'A');

    /* value constructor with bool + int */
    nuo_pair<bool, int> p4(true, -99);
    assert(p4.first == true);
    assert(p4.second == -99);

    /* default constructor for other types */
    nuo_pair<char, bool> p6;
    assert(p6.first == '\0');
    assert(p6.second == false);

    /* negative values */
    nuo_pair<int, int> pn(-5, -7);
    assert(pn.first == -5);
    assert(pn.second == -7);

    nuo_pair<int, int> p5(0, 0);
    assert(p5.first == 0);
    assert(p5.second == 0);
}

/* ------------------------------------------------- */
/* Test copy and move constructors */
void test::Test_Nuo_Pair::test_copy_constructor() {
    nuo_pair<int, double> p1(10, 3.14);

    /* copy constructor */
    nuo_pair<int, double> p2(p1);
    assert(p2.first == 10);
    assert(p2.second == 3.14);

    /* move constructor */
    nuo_pair<int, double> p3(std::move(p1));
    assert(p3.first == 10);
    assert(p3.second == 3.14);

    /* copy assignment */
    nuo_pair<std::string, int> p4("copy", 100);
    nuo_pair<std::string, int> p5;
    p5 = p4;
    assert(p5.first == "copy");
    assert(p5.second == 100);

    /* move assignment */
    nuo_pair<std::string, int> p6(std::move(p4));
    assert(p6.first == "copy");
    assert(p6.second == 100);

    /* chain assignment with same type */
    nuo_pair<int, double> a(1, 2.5), b, c;
    c = b = a;
    assert(b.first == 1 && b.second == 2.5);
    assert(c.first == 1 && c.second == 2.5);
}

/* ------------------------------------------------- */
/* Test operators: comparison, arithmetic, assignment */
void test::Test_Nuo_Pair::test_operator() {
    /* arithmetic with integers */
    nuo_pair<int, int> a(2, 3);
    nuo_pair<int, int> b(2, 3);
    nuo_pair<int, int> c(5, 7);

    /* equality */
    assert(a == b);
    assert(a != c);

    /* inequality when only second differs */
    nuo_pair<int, int> a2(2, 4);
    assert(a != a2);

    /* comparison (lexicographic) */
    assert(a < c);
    assert(c > a);
    assert(a <= b);
    assert(c >= b);

    /* comparison with equal first but different second */
    nuo_pair<int, int> e1(2, 1), e2(2, 5);
    assert(e1 < e2);
    assert(!(e2 < e1));
    assert(e2 > e1);
    assert(e1 <= e2 && e2 >= e1);

    /* arithmetic operators */
    auto sum = a + c; 
    assert(sum.first == 7 && sum.second == 10);

    auto diff = c - a;
    assert(diff.first == 3 && diff.second == 4);

    auto prod = a * c;
    assert(prod.first == 10 && prod.second == 21);

    auto quot = c / a;
    assert(quot.first == 2 && quot.second == 2);

    /* arithmetic with double (non-integer division) */
    nuo_pair<double, double> ad(5.0, 7.5), bd(2.0, 2.5);
    auto qd = ad / bd;
    assert(qd.first == 2.5 && qd.second == 3.0);

    /* compound assignment */
    nuo_pair<int, int> d(1, 2);
    d += a;
    assert(d.first == 3 && d.second == 5);

    /* chaining compound assignment returns lvalue reference */
    d = nuo_pair<int, int>(1, 1);
    (d += a) += a; // add a twice
    assert(d.first == 5 && d.second == 7);

    d -= a;
    assert(d.first == 1 && d.second == 2);

    d *= a;
    assert(d.first == 2 && d.second == 6);

    d /= a;
    assert(d.first == 1 && d.second == 2);

    /* arithmetic with double */
    nuo_pair<double, double> e(1.5, 2.0);
    nuo_pair<double, double> f(0.5, 4.0);
    auto g = e + f;
    assert(g.first == 2.0 && g.second == 6.0);

    auto h = f - e;
    assert(h.first == -1.0 && h.second == 2.0);
}

/* ------------------------------------------------- */
/* Test swap */
void test::Test_Nuo_Pair::test_swap() {
    nuo_pair<int, std::string> p1(1, "one");
    nuo_pair<int, std::string> p2(2, "two");

    p1.swap(p2);
    assert(p1.first == 2 && p1.second == "two");
    assert(p2.first == 1 && p2.second == "one");

    /* swap with equal values */
    nuo_pair<int, int> a(10, 20);
    nuo_pair<int, int> b(10, 20);
    a.swap(b);
    assert(a.first == 10 && a.second == 20);
    assert(b.first == 10 && b.second == 20);

    /* self-swap no change */
    a.swap(a);
    assert(a.first == 10 && a.second == 20);

    /* compile-time checks for swap */
    static_assert(std::is_same_v<decltype(a.swap(b)), void>, "swap should return void");
    static_assert(noexcept(a.swap(b)), "swap should be noexcept when members are noexcept swappable");
}

/* ------------------------------------------------- */
/* Test nuo_make_pair */
void test::Test_Nuo_Pair::test_make_pair() {
    auto p = nuo_make_pair(99, std::string("ninety-nine"));
    assert(p.first == 99);
    assert(p.second == "ninety-nine");

    auto q = nuo_make_pair('X', 3.14);
    assert(q.first == 'X');
    assert(q.second == 3.14);

    /* const char* + int */
    auto r = nuo_make_pair("hello", 42);
    assert(std::string(r.first) == std::string("hello"));
    assert(r.second == 42);
}

/* ------------------------------------------------- */
/* Test std::get and structured bindings */
void test::Test_Nuo_Pair::test_get() {
    nuo_pair<int, std::string> p(7, "seven");

    /* std::get with index */
    assert(std::get<0>(p) == 7);
    assert(std::get<1>(p) == "seven");

    /** structured binding */
    auto [x, y] = p;
    assert(x == 7);
    assert(y == "seven");

    /* structured binding by reference */
    auto& [rx, ry] = p;
    rx = 8;
    ry = "eight";
    assert(p.first == 8 && p.second == "eight");

    /* with const object */
    const nuo_pair<int, double> q(8, 9.81);
    assert(std::get<0>(q) == 8);
    assert(std::get<1>(q) == 9.81);

    /* move semantics */
    nuo_pair<std::string, int> r("move", 123);
    auto first_val = std::get<0>(std::move(r));
    auto second_val = std::get<1>(std::move(r));
    assert(first_val == "move");
    assert(second_val == 123);
}