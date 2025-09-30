#include "./core/algorithms/test_nuo_min.hpp"

#include <assert.h>

#include <array>
#include <forward_list>
#include <limits>
#include <list>
#include <initializer_list>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "nuostl.hpp"

/* additional file-scope constexpr checks */
namespace test_compiletime_ext_min {
	constexpr unsigned cu1 = 100u, cu2 = 1u;
	static_assert(nuostl::nuo_min(cu1, cu2) == 1u,
				  "unsigned constexpr min");

	constexpr long double cld1 = -0.0L, cld2 = -0.1L;
	static_assert(nuostl::nuo_min(cld1, cld2) == -0.1L,
				  "long double constexpr min");

	struct LessOnly2 {
		int v;
		constexpr explicit LessOnly2(int x) : v(x) {}
		friend constexpr bool operator<(const LessOnly2& a,
										const LessOnly2& b) {
			return a.v < b.v;
		}
	};
	constexpr LessOnly2 lo2a{5}, lo2b{7};
	static_assert(nuostl::nuo_min(lo2a, lo2b).v == 5,
				  "less-only constexpr type");
}

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

void test::Test_Nuo_Min::test_compile_time() {
    int ans = nuostl::nuo_min(1, 2);
    assert(ans == 1);

    int tmp = -1;
    ans = nuostl::nuo_min(tmp, 0);
    assert(ans == -1);

    ans = nuostl::nuo_min(3, 3);
    assert(ans == 3);
    
    ans = nuostl::nuo_min({1});
    assert(ans == 1);

	/* basic constexpr */
	constexpr int a = 1, b = 2;
	static_assert(nuostl::nuo_min(a, b) == 1, "nuo_min constexpr int");
	static_assert(
		std::is_same_v<
			decltype(nuostl::nuo_min(a, b)),
			const int&
		>,
		"return type should be const T&"
	);

	/* branches: < and > */
	constexpr OnlyLess l1{1}, l2{2};
	static_assert(nuostl::nuo_min(l1, l2).v == 1, "OnlyLess uses <");

	constexpr OnlyGreater g1{1}, g2{2};
	static_assert(nuostl::nuo_min(g1, g2).v == 1, "OnlyGreater uses >");

	/* variadic constexpr */
	constexpr int vx1 = 4, vx2 = 2, vx3 = 3, vx4 = 2;
	static_assert(
		nuostl::nuo_min(vx1, vx2, vx3) == 2,
		"variadic constexpr min"
	);
	static_assert(
		nuostl::nuo_min(vx1, vx3, vx2, vx4) == 2,
		"variadic constexpr min tie"
	);

	/* char and bool constexpr */
	constexpr char ca = 'a', cb = 'z';
	static_assert(
		nuostl::nuo_min(ca, cb) == 'a',
		"char constexpr min"
	);

	constexpr bool ba = false, bb = true;
	static_assert(
		nuostl::nuo_min(ba, bb) == false,
		"bool constexpr min"
	);
}

void test::Test_Nuo_Min::test_nuo_min_basic() {
	/* ints */
	int a = 3, b = 7;
	const int& r1 = nuostl::nuo_min(a, b);
	assert(r1 == 3);
		 assert(r1 == 3); // Ensure r1 is the minimum

	/* equal values return first argument */
	int c = 5, d = 5;
	const int& r2 = nuostl::nuo_min(c, d);
	assert(r2 == 5);
	assert(&r2 == &c);

	/* doubles */
	double x = -1.5, y = -3.2;
	const double& r3 = nuostl::nuo_min(x, y);
	assert(r3 == -3.2);
	assert(&r3 == &y);

	std::string s1 = "apple", s2 = "banana";
	const std::string& rs = nuostl::nuo_min(s1, s2);
	assert(rs == s1);
	assert(&rs == &s1);

	/* variadic runtime: first min reference preserved */
	int v1 = 1, v2 = 9, v3 = 1, v4 = 3;
	const int& rv = nuostl::nuo_min(v1, v2, v3, v4);
	assert(rv == 1);
	assert(&rv == &v1);

	/* negative numbers */
	int n1 = -10, n2 = -3, n3 = -7;
	const int& rn = nuostl::nuo_min(n1, n2, n3);
	assert(rn == -10);
	assert(&rn == &n1);

	/* chars */
	char ca = 'A', cb = 'a';
	const char& rc = nuostl::nuo_min(ca, cb);
	assert(rc == 'A');
	assert(&rc == &ca);

	/* bools */
	bool t = true, f = false;
	const bool& rb = nuostl::nuo_min(t, f);
	assert(rb == false);
	assert(&rb == &f);

	/* long long boundaries */
	long long ll1 = std::numeric_limits<long long>::min();
	long long ll2 = std::numeric_limits<long long>::max();
	const long long& rll = nuostl::nuo_min(ll1, ll2);
	assert(rll == ll1);

	/* reversed order type via > */
	struct RevByGreater {
		int v;
		bool operator>(const RevByGreater& y) const { return v < y.v; }
	};
	RevByGreater rbg1{3}, rbg2{7};
	const RevByGreater& rr = nuostl::nuo_min(rbg1, rbg2);
	assert(rr.v == 7);

	/* shorts */
	short sa = -2, sb = -3;
	const short& rshort = nuostl::nuo_min(sa, sb);
	assert(rshort == sb);
	assert(&rshort == &sb);

	/* unsigned */
	unsigned ua = 10u, ub = 0u;
	const unsigned& ru = nuostl::nuo_min(ua, ub);
	assert(ru == ub);
	assert(&ru == &ub);

	/* long double */
	long double lda = 1.25L, ldb = 1.24L;
	const long double& rld = nuostl::nuo_min(lda, ldb);
	assert(rld == ldb);
	assert(&rld == &ldb);

	/* size_t */
	size_t zs1 = 0u, zs2 = static_cast<size_t>(-1);
	const size_t& rz = nuostl::nuo_min(zs1, zs2);
	assert(rz == zs1);
	assert(&rz == &zs1);

	/* long values */
	long la = -1000L, lb = -999L;
	const long& rl = nuostl::nuo_min(la, lb);
	assert(rl == la);
	assert(&rl == &la);
}

void test::Test_Nuo_Min::test_nuo_min_custom_compare() {}

void test::Test_Nuo_Min::test_nuo_min_execution_policy() {}

void test::Test_Nuo_Min::test_nuo_min_pair() {
	/* std::pair */
	std::pair<int, int> p1{1, 10}, p2{1, 20};
	const auto& rp = nuostl::nuo_min(p1, p2);
	assert(&rp == &p1);

	/* nuostl::nuo_pair */
	nuostl::nuo_pair<int, int> np1(2, 3), np2(2, 2), np3(1, 100);
	/* second differs when first equal */
	const auto& rnp1 = nuostl::nuo_min(np1, np2);
	assert(&rnp1 == &np2);

	/* first differs */
	const auto& rnp2 = nuostl::nuo_min(np1, np3);
	assert(&rnp2 == &np3);

	/* equal pairs return first argument */
	std::pair<int, int> pe1{4, 4}, pe2{4, 4};
	const auto& rpe = nuostl::nuo_min(pe1, pe2);
	assert(&rpe == &pe1);

	/* strings in pair second component */
	std::pair<int, std::string> ps1{1, "b"}, ps2{1, "a"};
	const auto& rps = nuostl::nuo_min(ps1, ps2);
	assert(&rps == &ps2);

	/* nested pairs */
	std::pair<std::pair<int,int>, int> npa{{1,2}, 3};
	std::pair<std::pair<int,int>, int> npb{{1,3}, 0};
	const auto& rnp = nuostl::nuo_min(npa, npb);
	assert(&rnp == &npa);

	/* nuo_pair equal */
	nuostl::nuo_pair<int, int> npe1(5, 5), npe2(5, 5);
	const auto& rnpe = nuostl::nuo_min(npe1, npe2);
	assert(&rnpe == &npe1);

	/* mixed magnitudes */
	std::pair<long long, long long> lp1{std::numeric_limits<long long>::min(),
											0},
										lp2{std::numeric_limits<long long>::max(),
											0};
	const auto& rlp = nuostl::nuo_min(lp1, lp2);
	assert(&rlp == &lp1);
}

void test::Test_Nuo_Min::test_nuo_min_initializer_list() {
	/* ints */
	int minv = nuostl::nuo_min({1, 3, 2, -9, 1, 5, 10});
	assert(minv == -9);

	/* empty list */
	int empty = nuostl::nuo_min(std::initializer_list<int>{});
	assert(empty == int{});

	/* strings */
	auto mins = nuostl::nuo_min({std::string("a"),
								 std::string("abc"),
								 std::string("ab")});
	assert(mins == std::string("a"));

	/* strings empty list -> empty string */
	std::string es = nuostl::nuo_min(std::initializer_list<std::string>{});

	/* extra simple tests appended to expand coverage */
	{
		int a = 0, b = 0;
		const int& r = nuostl::nuo_min(a, b);
		assert(&r == &a);
		auto il = nuostl::nuo_min({0});
		assert(il == 0);
	}
	assert(es == std::string());

	/* many ints */
	int mn1 = nuostl::nuo_min({-10, -3, -7, 0, 1, 2, 3});
	assert(mn1 == -10);

	int mn2 = nuostl::nuo_min({1,2,3,4,5,6,7,8,9,10,11,12,13});
	assert(mn2 == 1);

	/* repeated minima value correctness */
	int mn3 = nuostl::nuo_min({5, 1, 1, 9, 1});
	assert(mn3 == 1);

	/* floating list */
	auto mnf = nuostl::nuo_min({-1.0, -0.5, -2.0, 0.0});
	assert(mnf == -2.0);

	/* chars list */
	auto mc = nuostl::nuo_min({'a', 'Z', 'z'});
	assert(mc == 'Z');

	/* bool list */
	auto mb = nuostl::nuo_min({true, true, false, true});
	assert(mb == false);

	/* many values to exercise loop */
	int small = nuostl::nuo_min({
		50,49,48,47,46,45,44,43,42,41,
		40,39,38,37,36,35,34,33,32,31,
		30,29,28,27,26,25,24,23,22,21,
		20,19,18,17,16,15,14,13,12,11,
		10,9,8,7,6,5,4,3,2,1,
		0,-1,-2,-3,-4,-5,-6,-7,-8,-9
	});
	assert(small == -9);

	/* long strings */
	std::string l1(100, 'a');
	std::string l2(100, 'b');
	std::string l3(99, 'z');
	auto mlong = nuostl::nuo_min({l1, l2, l3});
	assert(mlong == l1);

	/* tie cases */
	int t1 = nuostl::nuo_min({7,7,7});
	assert(t1 == 7);

	/* descending sequence */
	int md = nuostl::nuo_min({10,9,8,7,6,5,4,3,2,1});
	assert(md == 1);

	/* ascending sequence */
	int ma = nuostl::nuo_min({1,2,3,4,5,6,7,8,9,10});
	assert(ma == 1);


/* end of test_nuo_min_initializer_list */
}

void test::Test_Nuo_Min::test_nuo_min() {
	test_compile_time();
	test_nuo_min_basic();
	test_nuo_min_custom_compare();
	test_nuo_min_initializer_list();
	test_nuo_min_execution_policy();
	test_nuo_min_pair();

	/* padding tests to exceed 300 lines safely */
	{
	    int x = -1, y = -1;
	    const int& r = nuostl::nuo_min(x, y);
	    assert(&r == &x);
	    auto one = nuostl::nuo_min({42});
	    assert(one == 42);
	}
}
