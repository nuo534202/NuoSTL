# Complex

## Purpose

`nuostl::NuoComplex<T>` is a complex number type with the arithmetic operators and the mathematical functions of `std::complex`, plus a couple of helpers `std::complex` does not offer.

It is a class template, so the implementation lives in the header and works for `float`, `double` and `long double`.

## Important Notes

- **Header only**: `NuoComplex` is a template and is therefore implemented in the header.
- **Element access**: `real()` and `imag()` return the stored components; the setters `real(value)` and `imag(value)` replace them.
- **Comparison**: only `==` and `!=` exist, comparing both components. There is no ordering, exactly like `std::complex`.
- **Finite arguments** behave like the C library: every function was checked against `std::complex` over a grid that includes negative, zero and large values.
- **Non finite arguments** follow the C library conventions too, with two documented exceptions:
  - the branch points `atan(+-i)` and `atanh(+-1)` use the value from the standard (`+-i infinity` and `+- infinity`) instead of the artifact the platform library reports;
  - `pow` with a non finite operand, or with a zero base and a non positive real exponent, follows `exp(y * log(x))` instead of the dedicated special values of the C library.
- **`NuoAbs`** uses `std::hypot`, so it does not overflow for large components and returns `+inf` when a component is infinite.
- **`NuoNearlyEqual`** returns `false` when any component is NaN.

## Header

1. include the component header directly

```cpp
#include "extra/nuo_complex.hpp"
```

2. include the universal header `nuostl.hpp` to get all NuoSTL features

```cpp
#include "nuostl.hpp"
```

Namespace:

```cpp
nuostl::NuoComplex<T>
```

## Data Model

A value stores two components of the same type:

- `real_` (`T`)
- `imag_` (`T`)

A value whose `imag_` is zero is a real number.

## Constructors and Accessors

```cpp
constexpr NuoComplex(const T& real = T(), const T& imag = T());

template <typename U>
constexpr NuoComplex(const NuoComplex<U>& other); /* widening or narrowing */

constexpr T real() const;
constexpr T imag() const;
constexpr void real(const T& value);
constexpr void imag(const T& value);
```

```cpp
#include <assert.h>

#include "extra/nuo_complex.hpp"

void complex_constructor_examples()
{
    nuostl::NuoComplex<double> zero;
    assert(zero.real() == 0.0 && zero.imag() == 0.0);

    nuostl::NuoComplex<double> value(1.5, -2.5);
    assert(value.real() == 1.5 && value.imag() == -2.5);

    value.real(3.0);
    value.imag(4.0);
    assert(value.real() == 3.0 && value.imag() == 4.0);

    /* A NuoComplex<float> converts to NuoComplex<double>. */
    nuostl::NuoComplex<float> small(1.0f, 2.0f);
    nuostl::NuoComplex<double> widened(small);
    assert(widened.real() == 1.0 && widened.imag() == 2.0);
}
```

## Arithmetic Operators

Compound assignment:

```cpp
constexpr NuoComplex& operator=(const T& value);
constexpr NuoComplex& operator+=(const NuoComplex& other);
constexpr NuoComplex& operator-=(const NuoComplex& other);
constexpr NuoComplex& operator*=(const NuoComplex& other);
constexpr NuoComplex& operator/=(const NuoComplex& other);
```

plus the scalar versions `+=`, `-=`, `*=`, `/=` taking a `T`.

Binary operators exist for complex with complex, complex with scalar, and scalar with complex, in both orders:

```cpp
constexpr NuoComplex<T> operator+(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs);
constexpr NuoComplex<T> operator-(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs);
constexpr NuoComplex<T> operator*(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs);
constexpr NuoComplex<T> operator/(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs);
```

They are non members and never modify their operands.

```cpp
#include <assert.h>

#include "extra/nuo_complex.hpp"

/* a / b computed from the definition, for the assertion below */
namespace test
{
inline nuostl::NuoComplex<double> complex_divide(
    const nuostl::NuoComplex<double>& lhs, const nuostl::NuoComplex<double>& rhs)
{
    const double denominator = rhs.real() * rhs.real() + rhs.imag() * rhs.imag();
    return nuostl::NuoComplex<double>(
        (lhs.real() * rhs.real() + lhs.imag() * rhs.imag()) / denominator,
        (lhs.imag() * rhs.real() - lhs.real() * rhs.imag()) / denominator);
}
}

void complex_arithmetic_examples()
{
    const nuostl::NuoComplex<double> a(1.0, 2.0);
    const nuostl::NuoComplex<double> b(3.0, -4.0);

    assert(a + b == nuostl::NuoComplex<double>(4.0, -2.0));
    assert(a - b == nuostl::NuoComplex<double>(-2.0, 6.0));
    assert(a * b == nuostl::NuoComplex<double>(11.0, 2.0));
    assert(a / b == test::complex_divide(a, b));
    assert(-a == nuostl::NuoComplex<double>(-1.0, -2.0));

    /* Scalars work from either side. */
    assert(a * 2.0 == nuostl::NuoComplex<double>(2.0, 4.0));
    assert(2.0 * a == nuostl::NuoComplex<double>(2.0, 4.0));
    assert(a + 1.0 == nuostl::NuoComplex<double>(2.0, 2.0));
    assert(1.0 / a == nuostl::NuoComplex<double>(1.0, 0.0) / a);

    nuostl::NuoComplex<double> accumulated(1.0, 2.0);
    accumulated *= b;
    assert(accumulated == a * b);
}
```

## Comparison

```cpp
#include <assert.h>

#include "extra/nuo_complex.hpp"

void complex_comparison_examples()
{
    const nuostl::NuoComplex<double> value(1.5, -2.5);
    assert(value == nuostl::NuoComplex<double>(1.5, -2.5));
    assert(value != nuostl::NuoComplex<double>(1.5, 2.5));

    /* A scalar compares as the complex number with a zero imaginary part. */
    assert(nuostl::NuoComplex<double>(3.0, 0.0) == 3.0);
    assert(nuostl::NuoComplex<double>(3.0, 1.0) != 3.0);
}
```

## Structure Functions

```cpp
constexpr T NuoReal(const NuoComplex<T>& value);
constexpr T NuoImag(const NuoComplex<T>& value);
constexpr NuoComplex<T> NuoConj(const NuoComplex<T>& value);
constexpr T NuoNorm(const NuoComplex<T>& value); /* squared magnitude */
T NuoAbs(const NuoComplex<T>& value);   /* magnitude */
T NuoArg(const NuoComplex<T>& value);   /* phase angle */
NuoComplex<T> NuoPolar(const T& magnitude, const T& angle = T());
NuoComplex<T> NuoProj(const NuoComplex<T>& value);
```

```cpp
#include <assert.h>
#include <cmath>
#include <limits>

#include "extra/nuo_complex.hpp"

void complex_structure_examples()
{
    const nuostl::NuoComplex<double> value(3.0, 4.0);

    assert(nuostl::NuoReal(value) == 3.0);
    assert(nuostl::NuoImag(value) == 4.0);
    assert(nuostl::NuoNorm(value) == 25.0);
    assert(nuostl::NuoAbs(value) == 5.0);
    assert(nuostl::NuoConj(value) == nuostl::NuoComplex<double>(3.0, -4.0));
    assert(std::fabs(nuostl::NuoArg(value) - std::atan2(4.0, 3.0)) < 1e-15);

    /* polar builds the value from a magnitude and an angle. */
    const nuostl::NuoComplex<double> from_polar =
        nuostl::NuoPolar(2.0, 3.14159265358979323846 / 2.0);
    assert(std::fabs(from_polar.real()) < 1e-15);
    assert(std::fabs(from_polar.imag() - 2.0) < 1e-15);

    /* proj maps every infinite value onto the positive real infinity. */
    const double infinity = std::numeric_limits<double>::infinity();
    const nuostl::NuoComplex<double> projected =
        nuostl::NuoProj(nuostl::NuoComplex<double>(-infinity, 1.0));
    assert(projected.real() == infinity && projected.imag() == 0.0);
}
```

## Analytic Functions

```cpp
NuoComplex<T> NuoExp(const NuoComplex<T>& value);
NuoComplex<T> NuoLog(const NuoComplex<T>& value);
NuoComplex<T> NuoLog10(const NuoComplex<T>& value);
NuoComplex<T> NuoSqrt(const NuoComplex<T>& value);
NuoComplex<T> NuoPow(const NuoComplex<T>& base, const NuoComplex<T>& exponent);
NuoComplex<T> NuoPow(const NuoComplex<T>& base, const T& exponent);

NuoComplex<T> NuoSin(const NuoComplex<T>& value);
NuoComplex<T> NuoCos(const NuoComplex<T>& value);
NuoComplex<T> NuoTan(const NuoComplex<T>& value);
NuoComplex<T> NuoSinh(const NuoComplex<T>& value);
NuoComplex<T> NuoCosh(const NuoComplex<T>& value);
NuoComplex<T> NuoTanh(const NuoComplex<T>& value);

NuoComplex<T> NuoAsin(const NuoComplex<T>& value);
NuoComplex<T> NuoAcos(const NuoComplex<T>& value);
NuoComplex<T> NuoAtan(const NuoComplex<T>& value);
NuoComplex<T> NuoAsinh(const NuoComplex<T>& value);
NuoComplex<T> NuoAcosh(const NuoComplex<T>& value);
NuoComplex<T> NuoAtanh(const NuoComplex<T>& value);
```

All of them implement the principal value used by the C library, so `NuoSqrt` of a negative real number is the root with a positive imaginary part, and `NuoLog` of a negative real number has the phase `+pi`.

```cpp
#include <assert.h>
#include <cmath>

#include "extra/nuo_complex.hpp"

void complex_analytic_examples()
{
    /* Euler's identity: exp(i pi) = -1. */
    const nuostl::NuoComplex<double> euler =
        nuostl::NuoExp(nuostl::NuoComplex<double>(0.0, 3.14159265358979323846));
    assert(std::fabs(euler.real() + 1.0) < 1e-15);
    assert(std::fabs(euler.imag()) < 1e-15);

    /* The square root of a negative real number is purely imaginary. */
    const nuostl::NuoComplex<double> root =
        nuostl::NuoSqrt(nuostl::NuoComplex<double>(-4.0, 0.0));
    assert(root.real() == 0.0 && root.imag() == 2.0);

    /* log and exp undo each other away from the branch cut. */
    const nuostl::NuoComplex<double> value(2.0, -3.0);
    const nuostl::NuoComplex<double> round_trip = nuostl::NuoExp(nuostl::NuoLog(value));
    assert(nuostl::NuoNearlyEqual(round_trip, value, 1e-12));

    /* asin of 2 leaves the real axis, as the real function cannot. */
    const nuostl::NuoComplex<double> arc =
        nuostl::NuoAsin(nuostl::NuoComplex<double>(2.0, 0.0));
    assert(std::fabs(arc.real() - 1.5707963267948966) < 1e-12);
    assert(std::fabs(arc.imag() - 1.3169578969248166) < 1e-12);

    /* Integer powers agree with repeated multiplication. */
    const nuostl::NuoComplex<double> base(0.5, 0.25);
    assert(nuostl::NuoNearlyEqual(
        nuostl::NuoIntegerPow(base, 3), base * base * base, 1e-12));
}
```

## Extensions beyond std::complex

```cpp
template <typename T>
bool NuoNearlyEqual(const NuoComplex<T>& lhs, const NuoComplex<T>& rhs,
                    const T& tolerance);

template <typename T, typename Integer>
NuoComplex<T> NuoIntegerPow(const NuoComplex<T>& base, Integer exponent);
```

- `NuoNearlyEqual` reports whether the distance between the two values is at most `tolerance * (1 + |rhs|)`. It returns `false` as soon as one component is NaN.
- `NuoIntegerPow` computes an exact integer power by repeated squaring and inverts the result for a negative exponent, which avoids the rounding of `NuoPow(base, T(exponent))`.

```cpp
#include <assert.h>

#include "extra/nuo_complex.hpp"

void complex_extension_examples()
{
    const nuostl::NuoComplex<double> value(1.0, 2.0);
    assert(nuostl::NuoNearlyEqual(value, value, 0.0));
    assert(nuostl::NuoNearlyEqual(value, nuostl::NuoComplex<double>(1.0 + 1e-9, 2.0), 1e-6));
    assert(!nuostl::NuoNearlyEqual(value, nuostl::NuoComplex<double>(1.5, 2.0), 1e-6));

    const nuostl::NuoComplex<double> base(2.0, -1.0);
    assert(nuostl::NuoIntegerPow(base, 0) == nuostl::NuoComplex<double>(1.0, 0.0));
    assert(nuostl::NuoIntegerPow(base, 1) == base);
    assert(nuostl::NuoNearlyEqual(
        nuostl::NuoIntegerPow(base, -2),
        nuostl::NuoComplex<double>(1.0, 0.0) / (base * base), 1e-12));
}
```

## Output

```cpp
template <typename T>
std::ostream& operator<<(std::ostream& stream, const NuoComplex<T>& value);
```

The format is `(real,imag)`, the same one `std::complex` uses.

```cpp
#include <assert.h>
#include <sstream>

#include "extra/nuo_complex.hpp"

void complex_output_example()
{
    std::ostringstream stream;
    stream << nuostl::NuoComplex<double>(1.5, -2.5);
    assert(stream.str() == "(1.5,-2.5)");
}
```

## Complexity

Every operation costs a constant number of real operations, except the analytic functions that call the corresponding real functions of `<cmath>` once or twice (for example `NuoLog` calls `hypot`, `log` and `atan2`).
