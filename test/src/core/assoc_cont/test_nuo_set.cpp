#include "./core/assoc_cont/test_nuo_set.hpp"

#include <assert.h>

#include <string>
#include <vector>

#include "nuostl.hpp"


using nuostl::NuoSet;
using nuostl::NuoMultiset;
using nuostl::NuoLess;


/* ------------------------------------------------- */
/* Test nuo set */
void test::Test_Nuo_Set::test_nuo_set() {
    test_constructor();
    test_assignment();
    test_iterator();
    test_capacity();
    test_insert();
    test_emplace();
    test_erase();
    test_lookup();
    test_swap();
    test_comparison();

    test_multiset();
}

/* ------------------------------------------------- */
/* Test constructors */
void test::Test_Nuo_Set::test_constructor() {
    /* default constructor */
    NuoSet<int> s1;
    assert(s1.Empty());
    assert(s1.Size() == 0);

    /* range constructor */
    std::vector<int> v{5, 1, 4, 2, 3, 2, 1};
    NuoSet<int> s2(v.begin(), v.end());
    assert(s2.Size() == 5);  /* Unique values are 1, 2, 3, 4, and 5. */

    /* initializer_list constructor */
    NuoSet<int> s3{3, 1, 2, 3, 3};
    assert(s3.Size() == 3);

    /* copy constructor */
    NuoSet<int> s4(s3);
    assert(s4.Size() == s3.Size());
    assert(s4 == s3);

    /* move constructor */
    NuoSet<int> s5(NuoMove(s4));
    assert(s5.Size() == 3);
    assert(s4.Empty());  /* The source is empty after the move. */

    /* string set */
    NuoSet<std::string> ss;
    ss.Insert("banana");
    ss.Insert("apple");
    ss.Insert("cherry");
    assert(ss.Size() == 3);
    assert(*ss.Begin() == "apple");
}

/* ------------------------------------------------- */
/* Test assignment */
void test::Test_Nuo_Set::test_assignment() {
    NuoSet<int> s1{1, 2, 3};
    NuoSet<int> s2;

    /* copy assignment */
    s2 = s1;
    assert(s2.Size() == 3);
    assert(s2 == s1);

    /* move assignment */
    NuoSet<int> s3;
    s3 = NuoMove(s2);
    assert(s3.Size() == 3);
    assert(s2.Empty());

    /* initializer_list assignment */
    s3 = {9, 8, 7, 9};
    assert(s3.Size() == 3);
    assert(s3.Count(9) == 1);
}

/* ------------------------------------------------- */
/* Test iterators */
void test::Test_Nuo_Set::test_iterator() {
    NuoSet<int> s{5, 1, 3, 2, 4};

    /* forward iteration must be ascending */
    int expect = 1;
    for (auto it = s.Begin(); it != s.End(); ++it) {
        assert(*it == expect);
        ++expect;
    }

    /* reverse iteration must be descending */
    expect = 5;
    for (auto rit = s.RBegin(); rit != s.REnd(); ++rit) {
        assert(*rit == expect);
        --expect;
    }

    /* const iterators */
    const NuoSet<int>& cs = s;
    assert(*cs.CBegin() == 1);
    assert(*cs.CRBegin() == 5);

    /* operator++/-- round trip */
    auto it = s.Begin();
    auto back = it++;
    assert(*back == 1);
    assert(*it == 2);
    --it;
    assert(*it == 1);

    /* iterator equality across const/non-const */
    assert(s.Begin() == s.CBegin());
    assert(s.End() == s.CEnd());
}

/* ------------------------------------------------- */
/* Test capacity */
void test::Test_Nuo_Set::test_capacity() {
    NuoSet<int> s;
    assert(s.Empty());
    assert(s.MaxSize() > 0);

    s.Insert(1);
    assert(!s.Empty());
    assert(s.Size() == 1);

    for (int i = 2; i <= 100; ++i) {
        s.Insert(i);
    }
    assert(s.Size() == 100);

    s.Clear();
    assert(s.Empty());
    assert(s.Size() == 0);
}

/* ------------------------------------------------- */
/* Test insert */
void test::Test_Nuo_Set::test_insert() {
    NuoSet<int> s;

    /* insert unique value */
    auto res1 = s.Insert(10);
    assert(res1.second);
    assert(*res1.first == 10);

    /* insert duplicate value fails */
    auto res2 = s.Insert(10);
    assert(!res2.second);
    assert(*res2.first == 10);  /* Points to the existing element. */
    assert(s.Size() == 1);

    /* insert with hint */
    auto it = s.Insert(s.Begin(), 5);
    assert(*it == 5);
    assert(s.Size() == 2);

    /* insert rvalue */
    auto res3 = s.Insert(7);
    assert(res3.second);
    assert(s.Size() == 3);

    /* range insert with duplicates */
    std::vector<int> v{5, 8, 9, 8, 10};
    s.Insert(v.begin(), v.end());
    assert(s.Size() == 5);  /* {5, 7, 8, 9, 10}; 5 and 10 exist, and 8 is deduplicated. */

    /* sorted order maintained */
    int prev = -1;
    for (auto it2 = s.Begin(); it2 != s.End(); ++it2) {
        assert(*it2 > prev);
        prev = *it2;
    }
}

/* ------------------------------------------------- */
/* Test emplace */
void test::Test_Nuo_Set::test_emplace() {
    NuoSet<int> s;

    auto res1 = s.Emplace(3);
    assert(res1.second);
    assert(*res1.first == 3);

    auto res2 = s.Emplace(3);
    assert(!res2.second);

    auto it = s.EmplaceHint(s.End(), 1);
    assert(*it == 1);
    assert(s.Size() == 2);
}

/* ------------------------------------------------- */
/* Test erase */
void test::Test_Nuo_Set::test_erase() {
    NuoSet<int> s;
    for (int i = 0; i < 10; ++i) {
        s.Insert(i);
    }
    assert(s.Size() == 10);

    /* erase by key */
    assert(s.Erase(5) == 1);
    assert(s.Erase(5) == 0);  /* The second erase returns 0. */
    assert(s.Size() == 9);
    assert(s.Count(5) == 0);

    /* erase by iterator */
    auto it = s.Find(0);
    assert(it != s.End());
    auto next = s.Erase(it);
    assert(*next == 1);
    assert(s.Size() == 8);

    /* erase range */
    auto first = s.Find(2);
    auto last = s.Find(7);
    s.Erase(first, last);
    assert(s.Size() == 4);  /* Remaining values are {1, 7, 8, 9}. */

    /* erase to end via range */
    auto f2 = s.Find(8);
    s.Erase(f2, s.End());
    assert(s.Size() == 2);  /* Remaining values are {1, 7}. */

    /* clear */
    s.Clear();
    assert(s.Empty());
}

/* ------------------------------------------------- */
/* Test lookup */
void test::Test_Nuo_Set::test_lookup() {
    NuoSet<int> s;
    for (int i = 0; i < 20; ++i) {
        s.Insert(i * 2);  /* Even values from 0 through 38. */
    }

    /* find */
    assert(s.Find(10) != s.End());
    assert(*s.Find(10) == 10);
    assert(s.Find(11) == s.End());

    /* count (unique) */
    assert(s.Count(10) == 1);
    assert(s.Count(11) == 0);

    /* lower_bound / upper_bound */
    assert(*s.LowerBound(9) == 10);
    assert(*s.LowerBound(10) == 10);
    assert(*s.UpperBound(10) == 12);
    assert(s.UpperBound(38) == s.End());

    /* equal_range */
    auto er = s.EqualRange(10);
    assert(*er.first == 10);
    assert(er.second == ++NuoSet<int>::iterator(er.first) || *er.second == 12);

    /* equal_range for missing key */
    auto er2 = s.EqualRange(11);
    assert(*er2.first == 12);
    assert(er2.first == er2.second);
}

/* ------------------------------------------------- */
/* Test swap */
void test::Test_Nuo_Set::test_swap() {
    NuoSet<int> s1{1, 2, 3};
    NuoSet<int> s2{4, 5};

    s1.Swap(s2);
    assert(s1.Size() == 2);
    assert(s2.Size() == 3);
    assert(*s1.Begin() == 4);
    assert(*s2.Begin() == 1);

    /* free swap function */
    nuostl::swap(s1, s2);
    assert(s1.Size() == 3);
    assert(s2.Size() == 2);
}

/* ------------------------------------------------- */
/* Test comparison */
void test::Test_Nuo_Set::test_comparison() {
    NuoSet<int> s1{1, 2, 3};
    NuoSet<int> s2{1, 2, 3};
    NuoSet<int> s3{1, 2, 4};
    NuoSet<int> s4{1, 2};

    assert(s1 == s2);
    assert(!(s1 != s2));
    assert(s1 != s3);
    assert(s1 < s3);          /* Equal prefix, then 3 < 4. */
    assert(s4 < s1);          /* Proper prefix. */
    assert(s3 > s1);
    assert(s1 <= s2);
    assert(s1 >= s2);
}

/* ------------------------------------------------- */
/* Test multiset */
void test::Test_Nuo_Set::test_multiset() {
    /* default constructor */
    NuoMultiset<int> ms1;
    assert(ms1.Empty());

    /* duplicates are allowed */
    NuoMultiset<int> ms;
    for (int i = 0; i < 10; ++i) {
        ms.Insert(i % 4);  /* Values 0, 1, 2, and 3 each occur two or three times. */
    }
    assert(ms.Size() == 10);
    assert(ms.Count(0) == 3);
    assert(ms.Count(1) == 3);
    assert(ms.Count(2) == 2);
    assert(ms.Count(3) == 2);

    /* insert returns iterator directly */
    auto it = ms.Insert(5);
    assert(*it == 5);
    assert(ms.Size() == 11);

    /* range insert */
    std::vector<int> v{7, 7, 7};
    ms.Insert(v.begin(), v.end());
    assert(ms.Size() == 14);
    assert(ms.Count(7) == 3);

    /* emplace */
    auto it2 = ms.Emplace(9);
    assert(*it2 == 9);
    assert(ms.Count(9) == 1);

    /* erase by key removes all */
    assert(ms.Erase(7) == 3);
    assert(ms.Count(7) == 0);
    assert(ms.Size() == 12);

    /* erase by iterator removes one */
    auto it3 = ms.Find(5);
    assert(it3 != ms.End());
    ms.Erase(it3);
    assert(ms.Count(5) == 0);
    assert(ms.Size() == 11);

    /* equal_range covers all duplicates */
    auto er = ms.EqualRange(0);
    int n = 0;
    for (auto e = er.first; e != er.second; ++e) {
        assert(*e == 0);
        ++n;
    }
    assert(n == static_cast<int>(ms.Count(0)));

    /* copy / move */
    NuoMultiset<int> ms2(ms);
    assert(ms2 == ms);
    NuoMultiset<int> ms3(NuoMove(ms2));
    assert(ms3 == ms);
    assert(ms2.Empty());

    /* erase range */
    auto first = ms.LowerBound(1);
    auto last = ms.UpperBound(1);
    ms.Erase(first, last);
    assert(ms.Count(1) == 0);

    /* clear */
    ms.Clear();
    assert(ms.Empty());
}
