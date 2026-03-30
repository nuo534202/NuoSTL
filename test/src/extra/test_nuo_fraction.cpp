#include "extra/test_nuo_fraction.hpp"

#include <assert.h>
#include <math.h>

#include <limits>
#include <stdexcept>

#include "extra/nuo_fraction.hpp"

namespace test {

void TestFraction::test_fraction()
{
    /* Constructors */
    test_default_constructor();
    test_parameterized_constructor();
    test_constructor_zero_denominator();
    test_constructor_normalize();

    /* Comparison operators */
    test_operator_equal();
    test_operator_not_equal();
    test_operator_greater();
    test_operator_greater_equal();
    test_operator_less();
    test_operator_less_equal();

    /* Getters and setters */
    test_get_up();
    test_get_down();
    test_get_fraction();
    test_set_up();
    test_set_down();
    test_set_down_zero();

    /* Normalize */
    test_normalize();
}

void TestFraction::test_default_constructor()
{
    nuostl::Fraction f;
    assert(f.GetUp() == 0);
    assert(f.GetDown() == 1);
    assert(f.GetFraction() == 0.0);
}

void TestFraction::test_parameterized_constructor()
{
    /* basic positive fraction */
    nuostl::Fraction f1(1, 2);
    assert(f1.GetUp() == 1);
    assert(f1.GetDown() == 2);

    /* negative numerator */
    nuostl::Fraction f2(-3, 4);
    assert(f2.GetUp() == -3);
    assert(f2.GetDown() == 4);

    /* zero numerator */
    nuostl::Fraction f3(0, 5);
    assert(f3.GetUp() == 0);
    assert(f3.GetDown() == 5);

    /* negative denominator triggers Normalize: sign moves to numerator */
    nuostl::Fraction f4(3, -4);
    assert(f4.GetUp() == -3);
    assert(f4.GetDown() == 4);

    /* both negative: Normalize makes denominator positive */
    nuostl::Fraction f5(-7, -3);
    assert(f5.GetUp() == 7);
    assert(f5.GetDown() == 3);

    /* denominator = 1 (integer fraction) */
    nuostl::Fraction f6(42, 1);
    assert(f6.GetUp() == 42);
    assert(f6.GetDown() == 1);

    /* large values near int64 limits */
    nuostl::int64 big = std::numeric_limits<nuostl::int64>::max() / 2;
    nuostl::Fraction f7(big, big);
    assert(f7.GetUp() == big);
    assert(f7.GetDown() == big);

    /* denominator = -1 */
    nuostl::Fraction f8(5, -1);
    assert(f8.GetUp() == -5);
    assert(f8.GetDown() == 1);
}

void TestFraction::test_constructor_zero_denominator()
{
    bool caught = false;
    try
    {
        nuostl::Fraction f(1, 0);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught);

    /* zero numerator with zero denominator should still throw */
    caught = false;
    try
    {
        nuostl::Fraction f(0, 0);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught);

    /* negative numerator with zero denominator */
    caught = false;
    try
    {
        nuostl::Fraction f(-5, 0);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught);
}

void TestFraction::test_constructor_normalize()
{
    /* negative denominator => sign flips */
    nuostl::Fraction f1(1, -2);
    assert(f1.GetUp() == -1);
    assert(f1.GetDown() == 2);

    /* both negative => both positive */
    nuostl::Fraction f2(-1, -2);
    assert(f2.GetUp() == 1);
    assert(f2.GetDown() == 2);

    /* positive denominator stays unchanged */
    nuostl::Fraction f3(-3, 7);
    assert(f3.GetUp() == -3);
    assert(f3.GetDown() == 7);

    /* zero numerator with negative denominator */
    nuostl::Fraction f4(0, -10);
    assert(f4.GetUp() == 0);
    assert(f4.GetDown() == 10);
}

void TestFraction::test_operator_equal()
{
    /* identical fractions */
    nuostl::Fraction a(1, 2);
    nuostl::Fraction b(1, 2);
    assert(a == b);

    /* same value after normalization (sign flip) */
    nuostl::Fraction c(-3, 4);
    nuostl::Fraction d(3, -4);
    assert(c == d);

    /* zero fractions with different denominators */
    nuostl::Fraction z1(0, 1);
    nuostl::Fraction z2(0, 99);
    /* Note: structural equality — up_ and down_ must match */
    assert(!(z1 == z2));  /* 0/1 != 0/99 structurally */

    /* negative fractions equal */
    nuostl::Fraction e(-5, 7);
    nuostl::Fraction f(-5, 7);
    assert(e == f);

    /* self-comparison */
    nuostl::Fraction g(42, 13);
    assert(g == g);

    /* different fractions are not equal */
    nuostl::Fraction h(1, 3);
    nuostl::Fraction i(1, 4);
    assert(!(h == i));
}

void TestFraction::test_operator_not_equal()
{
    nuostl::Fraction a(1, 2);
    nuostl::Fraction b(1, 3);
    assert(a != b);

    /* same fraction is not unequal */
    nuostl::Fraction c(7, 11);
    assert(!(c != c));

    /* different signs */
    nuostl::Fraction d(1, 2);
    nuostl::Fraction e(-1, 2);
    assert(d != e);

    /* same magnitude, different sign */
    nuostl::Fraction f(3, 4);
    nuostl::Fraction g(-3, 4);
    assert(f != g);
}

void TestFraction::test_operator_greater()
{
    /* basic positive comparison */
    nuostl::Fraction a(3, 4);
    nuostl::Fraction b(1, 2);
    assert(a > b);
    assert(!(b > a));

    /* equal fractions */
    nuostl::Fraction c(1, 2);
    assert(!(c > c));

    /* negative < positive */
    nuostl::Fraction d(-1, 2);
    nuostl::Fraction e(1, 2);
    assert(!(d > e));
    assert(e > d);

    /* both negative: -1/4 > -1/2 */
    nuostl::Fraction f(-1, 4);
    nuostl::Fraction g(-1, 2);
    assert(f > g);
    assert(!(g > f));

    /* zero vs positive */
    nuostl::Fraction z(0, 1);
    nuostl::Fraction p(1, 100);
    assert(p > z);
    assert(!(z > p));

    /* zero vs negative */
    nuostl::Fraction n(-1, 100);
    assert(z > n);

    /* large numerators, close values */
    nuostl::Fraction h(999999999, 1000000000);
    nuostl::Fraction i(999999998, 1000000000);
    assert(h > i);
    assert(!(i > h));

    /* cross-sign comparison with large values */
    nuostl::int64 big = std::numeric_limits<nuostl::int64>::max() / 2;
    nuostl::Fraction j(big, 1);
    nuostl::Fraction k(-1, 1);
    assert(j > k);
}

void TestFraction::test_operator_greater_equal()
{
    /* strictly greater */
    nuostl::Fraction a(3, 4);
    nuostl::Fraction b(1, 4);
    assert(a >= b);

    /* equal */
    nuostl::Fraction c(5, 7);
    assert(c >= c);

    /* not greater or equal */
    assert(!(b >= a));

    /* negative equal */
    nuostl::Fraction d(-2, 3);
    nuostl::Fraction e(-2, 3);
    assert(d >= e);
    assert(e >= d);

    /* zero >= negative */
    nuostl::Fraction z(0, 1);
    nuostl::Fraction n(-1, 1);
    assert(z >= n);

    /* positive >= zero */
    nuostl::Fraction p(1, 1);
    assert(p >= z);
}

void TestFraction::test_operator_less()
{
    /* basic comparison */
    nuostl::Fraction a(1, 4);
    nuostl::Fraction b(3, 4);
    assert(a < b);
    assert(!(b < a));

    /* equal fractions */
    nuostl::Fraction c(2, 3);
    assert(!(c < c));

    /* negative < positive */
    nuostl::Fraction d(-1, 2);
    nuostl::Fraction e(1, 2);
    assert(d < e);
    assert(!(e < d));

    /* both negative: -3/4 < -1/4 */
    nuostl::Fraction f(-3, 4);
    nuostl::Fraction g(-1, 4);
    assert(f < g);
    assert(!(g < f));

    /* zero < positive */
    nuostl::Fraction z(0, 1);
    nuostl::Fraction p(1, 1000000);
    assert(z < p);

    /* negative < zero */
    nuostl::Fraction n(-1, 1000000);
    assert(n < z);

    /* close fractions with large denominators */
    nuostl::Fraction h(1, 1000000000);
    nuostl::Fraction i(2, 1000000000);
    assert(h < i);
}

void TestFraction::test_operator_less_equal()
{
    /* strictly less */
    nuostl::Fraction a(1, 4);
    nuostl::Fraction b(3, 4);
    assert(a <= b);

    /* equal */
    nuostl::Fraction c(7, 11);
    assert(c <= c);

    /* not less or equal */
    assert(!(b <= a));

    /* negative <= zero */
    nuostl::Fraction n(-1, 1);
    nuostl::Fraction z(0, 1);
    assert(n <= z);

    /* zero <= positive */
    nuostl::Fraction p(1, 1);
    assert(z <= p);

    /* both negative, equal */
    nuostl::Fraction d(-5, 3);
    nuostl::Fraction e(-5, 3);
    assert(d <= e);
    assert(e <= d);
}

void TestFraction::test_get_up()
{
    nuostl::Fraction f1(7, 3);
    assert(f1.GetUp() == 7);

    nuostl::Fraction f2(-7, 3);
    assert(f2.GetUp() == -7);

    nuostl::Fraction f3(0, 1);
    assert(f3.GetUp() == 0);

    /* after normalization */
    nuostl::Fraction f4(5, -2);
    assert(f4.GetUp() == -5);
}

void TestFraction::test_get_down()
{
    nuostl::Fraction f1(7, 3);
    assert(f1.GetDown() == 3);

    /* after normalization, denominator is positive */
    nuostl::Fraction f2(7, -3);
    assert(f2.GetDown() == 3);

    nuostl::Fraction f3(0, 42);
    assert(f3.GetDown() == 42);
}

void TestFraction::test_get_fraction()
{
    nuostl::Fraction f1(1, 2);
    assert(std::fabs(f1.GetFraction() - 0.5) < 1e-9);

    nuostl::Fraction f2(1, 3);
    assert(std::fabs(f2.GetFraction() - (1.0 / 3.0)) < 1e-9);

    nuostl::Fraction f3(-1, 4);
    assert(std::fabs(f3.GetFraction() - (-0.25)) < 1e-9);

    nuostl::Fraction f4(0, 7);
    assert(f4.GetFraction() == 0.0);

    nuostl::Fraction f5(7, 1);
    assert(std::fabs(f5.GetFraction() - 7.0) < 1e-9);
}

void TestFraction::test_set_up()
{
    nuostl::Fraction f(1, 2);
    f.SetUp(5);
    assert(f.GetUp() == 5);
    assert(f.GetDown() == 2);

    /* set to zero */
    f.SetUp(0);
    assert(f.GetUp() == 0);

    /* set to negative */
    f.SetUp(-100);
    assert(f.GetUp() == -100);

    /* large value */
    nuostl::int64 big = std::numeric_limits<nuostl::int64>::max();
    f.SetUp(big);
    assert(f.GetUp() == big);
}

void TestFraction::test_set_down()
{
    nuostl::Fraction f(1, 2);
    f.SetDown(5);
    assert(f.GetDown() == 5);
    assert(f.GetUp() == 1);

    /* set to 1 */
    f.SetDown(1);
    assert(f.GetDown() == 1);

    /* large value */
    nuostl::int64 big = std::numeric_limits<nuostl::int64>::max();
    f.SetDown(big);
    assert(f.GetDown() == big);
}

void TestFraction::test_set_down_zero()
{
    nuostl::Fraction f(1, 2);
    bool caught = false;
    try {
        f.SetDown(0);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    /* original value should be unchanged after failed set */
    assert(f.GetDown() == 2);
}

/* Normalize: negative denominator gets flipped      */
void TestFraction::test_normalize()
{
    /* negative denominator => sign flips to numerator */
    nuostl::Fraction f1(3, -5);
    assert(f1.GetUp() == -3);
    assert(f1.GetDown() == 5);

    /* both negative => both positive */
    nuostl::Fraction f2(-3, -5);
    assert(f2.GetUp() == 3);
    assert(f2.GetDown() == 5);

    /* already normal: positive denominator stays */
    nuostl::Fraction f3(4, 7);
    assert(f3.GetUp() == 4);
    assert(f3.GetDown() == 7);

    /* negative numerator, positive denominator stays */
    nuostl::Fraction f4(-4, 7);
    assert(f4.GetUp() == -4);
    assert(f4.GetDown() == 7);

    /* zero numerator with negative denominator */
    nuostl::Fraction f5(0, -3);
    assert(f5.GetUp() == 0);
    assert(f5.GetDown() == 3);

    /* denominator = -1 */
    nuostl::Fraction f6(1, -1);
    assert(f6.GetUp() == -1);
    assert(f6.GetDown() == 1);

    /* large negative denominator */
    nuostl::int64 big = std::numeric_limits<nuostl::int64>::max();
    nuostl::Fraction f7(1, -big);
    assert(f7.GetUp() == -1);
    assert(f7.GetDown() == big);

    /* verify Normalize is applied after arithmetic that may
       produce negative denominator (division by negative fraction) */
    nuostl::Fraction f8(1, 2);
    nuostl::Fraction neg(-3, 4);
    nuostl::Fraction result = f8;
    result / neg;
    /* After /: up = 1*4 = 4, down = 2*(-3) = -6
       Normalize should make down positive.
       After gcd reduction the result should have down_ > 0 */
    assert(result.GetDown() > 0);
}

}   /* namespace test */