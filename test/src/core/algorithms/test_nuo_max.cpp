#include "./core/algorithms/test_nuo_max.hpp"

#include <assert.h>
#include <math.h>

#include <array>
#include <execution>
#include <forward_list>
#include <functional>
#include <limits>
#include <list>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "nuostl.hpp"

namespace {
    struct OnlyLess {
        int v;
        constexpr explicit OnlyLess(int x) : v(x) {}
        friend constexpr bool operator<(const OnlyLess& a,
                                        const OnlyLess& b) {
            return a.v < b.v;
        }
    };

    struct OnlyGreater {
        int v;
        constexpr explicit OnlyGreater(int x) : v(x) {}
        friend constexpr bool operator>(const OnlyGreater& a,
                                        const OnlyGreater& b) {
            return a.v > b.v;
        }
    };
}

/* SFINAE checks for callability (avoid requires-expressions in tests) */
template<typename A, typename B, typename = void>
struct can_call_nuo_max2 : std::false_type {};

template<typename A, typename B>
struct can_call_nuo_max2<
    A,
    B,
    std::void_t<decltype(
        nuostl::nuo_max(std::declval<A>(), std::declval<B>())
    )>
> : std::true_type {};

template<typename A, typename B, typename C, typename = void>
struct can_call_nuo_max3 : std::false_type {};

template<typename A, typename B, typename C>
struct can_call_nuo_max3<
    A,
    B,
    C,
    std::void_t<decltype(
        nuostl::nuo_max(
            std::declval<A>(),
            std::declval<B>(),
            std::declval<C>()
        )
    )>
> : std::true_type {};


    /* char and bool constexpr */
    constexpr char ca = 'a', cb = 'z';
    static_assert(nuostl::nuo_max(ca, cb) == 'z', "char constexpr max");

    constexpr bool ba = false, bb = true;
    static_assert(nuostl::nuo_max(ba, bb) == true, "bool constexpr max");
void test::Test_Nuo_Max::test_compile_time() {
    /* basic constexpr using lvalues */
    int ans = nuostl::nuo_max(1, 2);
    assert(ans == 2);

    int temp = -1;
    ans = nuostl::nuo_max(temp, 0);
    assert(ans == 0);

    ans = nuostl::nuo_max(3, 3);
    assert(ans == 3);
    
    ans = nuostl::nuo_max({1});
    assert(ans == 1);

    constexpr int a = 1, b = 2;
    static_assert(nuostl::nuo_max(a, b) == 2, "nuo_max constexpr int");
    static_assert(
        std::is_same_v<
            decltype(nuostl::nuo_max(a, b)),
            const int&
        >,
        "return type should be const T&"
    );

    /* use < branch */
    constexpr OnlyLess l1{1}, l2{2};
    static_assert(nuostl::nuo_max(l1, l2).v == 2, "OnlyLess should use <");

    /* use > branch */
    constexpr OnlyGreater g1{1}, g2{2};
    static_assert(nuostl::nuo_max(g1, g2).v == 2, "OnlyGreater should use >");

    /* variadic constexpr */
    constexpr int vx1 = 1, vx2 = 5, vx3 = 3, vx4 = 5;
    static_assert(
        nuostl::nuo_max(vx1, vx2, vx3) == 5,
        "variadic constexpr max"
    );
    static_assert(
        nuostl::nuo_max(vx1, vx3, vx2, vx4) == 5,
        "variadic constexpr max tie"
    );

    /* unsigned and long double constexpr */
    constexpr unsigned ua = 0u, ub = 42u;
    static_assert(nuostl::nuo_max(ua, ub) == 42u,
                  "unsigned constexpr max");

    constexpr long double lda = 1.25L, ldb = 1.24L;
    static_assert(nuostl::nuo_max(lda, ldb) == 1.25L,
                  "long double constexpr max");
}

void test::Test_Nuo_Max::test_nuo_max_basic() {
    /* ints */
    int a = 3, b = 7;
    const int& r1 = nuostl::nuo_max(a, b);
    assert(r1 == 7);
    assert(&r1 == &b);

    /* equal values should return first argument reference */

    /* chars */
    char c1 = 'A', c2 = 'a';
    const char& rc = nuostl::nuo_max(c1, c2);
    assert(rc == 'a');
    assert(&rc == &c2);

    /* bools */
    bool t = true, f = false;
    const bool& rb = nuostl::nuo_max(t, f);
    assert(rb == true);
    assert(&rb == &t);

    /* long long boundaries */
    long long ll1 = std::numeric_limits<long long>::min();
    long long ll2 = std::numeric_limits<long long>::max();
    const long long& rll = nuostl::nuo_max(ll1, ll2);
    assert(rll == ll2);

    /* reversed order type via < */
    struct RevByLess {
        int v;
        bool operator<(const RevByLess& y) const { return v > y.v; }
    };
    RevByLess rbl1{3}, rbl2{7};
    const RevByLess& rr = nuostl::nuo_max(rbl1, rbl2);
    assert(rr.v == 3);
    int c = 5, d = 5;
    const int& r2 = nuostl::nuo_max(c, d);
    assert(r2 == 5);
    assert(&r2 == &c);

    /* doubles */
    double x = -1.5, y = -3.2;
    const double& r3 = nuostl::nuo_max(x, y);
    assert(r3 == -1.5);
    assert(&r3 == &x);

    /* strings (lexicographic) */
    std::string s1 = "apple", s2 = "banana";
    const std::string& rs =
        nuostl::nuo_max(s1, s2);
    assert(rs == s2);
    assert(&rs == &s2);
    
        /* size_t */
        size_t zs1 = 0u, zs2 = static_cast<size_t>(-1);
        const size_t& rz = nuostl::nuo_max(zs1, zs2);
        assert(rz == zs2);
        assert(&rz == &zs2);
    
        /* long values */
        long la = -1000L, lb = -999L;
        const long& rl = nuostl::nuo_max(la, lb);
        assert(rl == lb);
        assert(&rl == &lb);
    /* variadic runtime, first max reference is preserved */
    int v1 = 9, v2 = 1, v3 = 9, v4 = 3;
    const int& rv = nuostl::nuo_max(v1, v2, v3, v4);
    assert(rv == 9);
    assert(&rv == &v1);

    /* negative numbers */
    int n1 = -10, n2 = -3, n3 = -7;
    const int& rn = nuostl::nuo_max(n1, n2, n3);
    assert(rn == -3);
    assert(&rn == &n2);

    /* initializer_list: ints */
    int maxm = nuostl::nuo_max({1, 3, 2, 9, 1, 5, 10});
    assert(maxm == 10);

    /* empty list returns value-initialized */
    int empty = nuostl::nuo_max(std::initializer_list<int>{});
    assert(empty == int{});

    /* initializer_list: strings */
    auto maxs = nuostl::nuo_max({std::string("a"),
                                 std::string("abc"),
                                 std::string("ab")});
    assert(maxs == std::string("abc"));

    /* many ints */
    int mx1 = nuostl::nuo_max({-10, -3, -7, 0, 1, 2, 3});
    assert(mx1 == 3);

    int mx2 = nuostl::nuo_max(
        {1,2,3,4,5,6,7,8,9,10,11,12,13}
    );
    assert(mx2 == 13);

    /* repeated maxima by value semantics */
    int mx3 = nuostl::nuo_max({5, 9, 9, 1, 9});
    assert(mx3 == 9);

    /* floating list */
    auto mxf = nuostl::nuo_max({-1.0, -0.5, -2.0, 0.0});
    assert(mxf == 0.0);

    /* floats and long doubles two-arg */
    float ff1 = -0.25f, ff2 = -0.24f;
    const float& rf = nuostl::nuo_max(ff1, ff2);
    assert(rf == ff2);
    assert(&rf == &ff2);

    long double ld1 = 10.0L, ld2 = 9.9999L;
    const long double& rld = nuostl::nuo_max(ld1, ld2);
    assert(rld == ld1);
    assert(&rld == &ld1);

    /* unsigned */
    unsigned ua = 0u, ub = 1u;
    const unsigned& ru = nuostl::nuo_max(ua, ub);
    assert(ru == ub);
    assert(&ru == &ub);

    /* NaN behavior: comparisons are false, returns second argument */
    double nan = std::numeric_limits<double>::quiet_NaN();
    double d0 = 0.0;
    const double& rnan1 = nuostl::nuo_max(nan, d0);
    assert(&rnan1 == &d0);
    const double& rnan2 = nuostl::nuo_max(d0, nan);
    assert(&rnan2 == &nan);
}

void test::Test_Nuo_Max::test_nuo_max_custom_compare() {}

void test::Test_Nuo_Max::test_nuo_max_execution_policy() {}

void test::Test_Nuo_Max::test_nuo_max_pair() {
    /* std::pair */
    std::pair<int, int> p1{1, 10}, p2{1, 20};
    const auto& rp = nuostl::nuo_max(p1, p2);
    assert(&rp == &p2);

    /* nuostl::nuo_pair */
    nuostl::nuo_pair<int, int> np1(2, 3), np2(2, 2), np3(1, 100);
    /* compare second since first equal */
    const auto& rnp1 = nuostl::nuo_max(np1, np2);
    assert(&rnp1 == &np1);

    /* compare first */
    const auto& rnp2 = nuostl::nuo_max(np1, np3);
    assert(&rnp2 == &np1);

    /* equal pairs return first argument reference */
    std::pair<int, int> pe1{4, 4}, pe2{4, 4};
    const auto& rpe = nuostl::nuo_max(pe1, pe2);
    assert(&rpe == &pe1);

    /* strings in pair second component */
    std::pair<int, std::string> ps1{1, "a"}, ps2{1, "b"};
    const auto& rps = nuostl::nuo_max(ps1, ps2);
    assert(&rps == &ps2);

    /* nested pairs */
    std::pair<std::pair<int,int>, int> npa{{1,2}, 3};
    std::pair<std::pair<int,int>, int> npb{{1,3}, 0};
    const auto& rnp = nuostl::nuo_max(npa, npb);
    assert(&rnp == &npb);

    /* nuo_pair equal */
    nuostl::nuo_pair<int, int> npe1(5, 5), npe2(5, 5);
    const auto& rnpe = nuostl::nuo_max(npe1, npe2);
    assert(&rnpe == &npe1);

    /* mixed magnitudes */
    std::pair<long long, long long> lp1{std::numeric_limits<long long>::min(),
                                        0},
                                    lp2{std::numeric_limits<long long>::max(),
                                        0};
    const auto& rlp = nuostl::nuo_max(lp1, lp2);
    assert(&rlp == &lp2);
}

void test::Test_Nuo_Max::test_nuo_max_initializer_list() {
    /* basic list */
    int maxm = nuostl::nuo_max({1, 3, 2, 9, 1, 5, 10});
    assert(maxm == 10);

    /* empty list returns value-initialized */
    int empty = nuostl::nuo_max(std::initializer_list<int>{});
    assert(empty == int{});

    /* strings */
    auto maxs = nuostl::nuo_max({std::string("a"),
                                 std::string("abc"),
                                 std::string("ab")});
    assert(maxs == std::string("abc"));

    /* extended cases */
    int mx1 = nuostl::nuo_max({-100, -50, -1});
    assert(mx1 == -1);

    int mx2 = nuostl::nuo_max({42});
    assert(mx2 == 42);

    auto mxs = nuostl::nuo_max({std::string("zzz"),
                                 std::string("yyy"),
                                 std::string("x")});
    assert(mxs == std::string("zzz"));

    /* doubles including negatives */
    auto md = nuostl::nuo_max({-10.5, -10.6, -10.4});
    assert(md == -10.4);

    /* chars list */
    auto mc = nuostl::nuo_max({'a', 'Z', 'z'});
    assert(mc == 'z');

    /* bool list */
    auto mb = nuostl::nuo_max({false, false, true, false});
    assert(mb == true);

    /* many values to exercise loop */
    int big = nuostl::nuo_max({
        -50,-49,-48,-47,-46,-45,-44,-43,-42,-41,
        -40,-39,-38,-37,-36,-35,-34,-33,-32,-31,
        -30,-29,-28,-27,-26,-25,-24,-23,-22,-21,
        -20,-19,-18,-17,-16,-15,-14,-13,-12,-11,
        -10,-9,-8,-7,-6,-5,-4,-3,-2,-1,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    });
    assert(big == 9);
}

void test::Test_Nuo_Max::test_nuo_max() {
    test_compile_time();
    test_nuo_max_basic();
    test_nuo_max_custom_compare();
    test_nuo_max_execution_policy();
    test_nuo_max_pair();
    test_nuo_max_initializer_list();
}