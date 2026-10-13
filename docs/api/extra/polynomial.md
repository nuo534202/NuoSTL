# Polynomial

## Purpose

`nuostl::NuoPolynomial<T>` represents a univariate polynomial with dense
coefficients. It supports symbolic addition, subtraction, multiplication,
derivatives, and quotient/remainder division, as well as numerical evaluation.
The C++ standard library has no polynomial class template; this is a NuoSTL
extension with its own interface.

## Header

```cpp
#include "extra/nuo_polynomial.hpp"
```

The universal header `nuostl.hpp` also includes this component. Because this is
a class template, declarations and implementations are in the component header.

## Representation and Coefficient Types

- Coefficients are listed in ascending power order: `{1, -2, 3}` means
  `1 - 2*x + 3*x^2`.
- Trailing coefficients equal to `T()` are removed after construction and
  modification. Internal zeros are retained.
- The zero polynomial has no stored coefficients: `Size() == 0`, `Empty()` and
  `IsZero()` are true. Its `Degree()` returns `0` by project convention; use
  `IsZero()` to distinguish it from a nonzero constant.
- Equality compares normalized coefficients exactly. No floating point
  tolerance is used, including during normalization.
- `T()` must be additive zero. Each operation requires the corresponding
  value-preserving arithmetic on const operands, equality, and copy semantics.
  Derivatives also require construction from a degree. Tests cover `int` and
  `double`, and basic arithmetic and derivatives with `NuoComplex<double>`.
- Stream output additionally requires ordering against zero and streaming of
  coefficients; complex coefficients support arithmetic but not this output
  overload. Custom types with mutating binary operators, such as the current
  `Fraction` interface, do not satisfy the coefficient requirements.
- Built-in integer arithmetic must stay within representable limits. General
  addition/multiplication overflow is not checked. Division specifically checks
  the signed minimum divided by `-1` and throws `std::overflow_error`.

## Public Interface

```cpp
template <typename T>
class NuoPolynomial;

using value_type = T;
using size_type = std::size_t;

NuoPolynomial();
NuoPolynomial(std::initializer_list<T> coefficients);

size_type Size() const noexcept;
size_type Degree() const noexcept;
bool Empty() const noexcept;
bool IsZero() const noexcept;

T Coefficient(size_type degree) const;
const T& At(size_type degree) const;
void SetCoefficient(size_type degree, const T& value);

T Evaluate(const T& value) const;
T operator()(const T& value) const;
NuoPolynomial Derivative() const;
std::pair<NuoPolynomial, NuoPolynomial> Divmod(
  const NuoPolynomial& divisor) const;

bool operator==(const NuoPolynomial& other) const;
bool operator!=(const NuoPolynomial& other) const;

NuoPolynomial& operator+=(const NuoPolynomial& other);
NuoPolynomial& operator-=(const NuoPolynomial& other);
NuoPolynomial& operator*=(const NuoPolynomial& other);
NuoPolynomial& operator*=(const T& scalar);
NuoPolynomial& operator/=(const T& scalar);
```

Copy and move operations are supplied by the compiler. Nonmember operators
provide unary `-`, polynomial `+`, `-`, `*`, `/`, `%`, scalar multiplication in
both orders, scalar division, and stream output. Binary operators preserve
their operands. Scalars must have the coefficient type for template deduction:
use `2.0 * NuoPolynomial<double>{1.0}`.

`Coefficient` returns zero for degrees beyond the stored range. `At` returns a
const reference and throws `std::out_of_range` outside that range. Use
`SetCoefficient` to change a coefficient while preserving normalization.
Setting a new high-degree coefficient fills intermediate positions with zero;
setting an unstored coefficient to zero does nothing, even for a huge degree.
An unrepresentable nonzero degree throws `std::length_error`. Allocation failure
can throw `std::bad_alloc`. A reference from `At` can be invalidated by a
modification; a setter or scalar compound operator may safely receive such a
reference as its argument.

## Construction, Access and Evaluation

`Evaluate` and `operator()` use Horner evaluation. A zero polynomial evaluates
to `T()` for every argument.

```cpp
#include <cassert>

#include "extra/nuo_polynomial.hpp"

void polynomial_access_example()
{
  using Polynomial = nuostl::NuoPolynomial<int>;
  Polynomial polynomial{1, -2, 3, 0};
  assert(polynomial.Size() == 3 && polynomial.Degree() == 2);
  assert(polynomial(2) == 9);
  assert(polynomial.Coefficient(9) == 0);

  polynomial.SetCoefficient(2, 0);
  assert(polynomial == (Polynomial{1, -2}));
  polynomial.SetCoefficient(4, 5);
  assert(polynomial.At(4) == 5 && polynomial.Coefficient(3) == 0);
}
```

## Arithmetic and Derivatives

Different degrees can be added and subtracted. Multiplication convolves the
coefficient sequences. Self-assignment forms such as `p += p`, `p -= p` and
`p *= p` are supported. The derivative of a constant or zero is zero.

Scalar division applies the coefficient type's division independently to each
coefficient: integer scalar division truncates toward zero. A zero scalar
divisor throws `std::invalid_argument`, even for the zero polynomial. Scalar
division leaves the polynomial unchanged if a coefficient operation throws.

```cpp
#include <cassert>

#include "extra/nuo_polynomial.hpp"

void polynomial_arithmetic_example()
{
  using Polynomial = nuostl::NuoPolynomial<int>;
  const Polynomial first{1, 2, 3};
  const Polynomial second{1, 1};

  assert(first + second == (Polynomial{2, 3, 3}));
  assert(first * second == (Polynomial{1, 3, 5, 3}));
  assert(first.Derivative() == (Polynomial{2, 6}));
  assert(first * 2 == (Polynomial{2, 4, 6}));
  assert(first / 2 == (Polynomial{0, 1, 1}));
}
```

## Quotient and Remainder

`Divmod(divisor)` returns `{quotient, remainder}`. Polynomial `/` returns the
quotient; `%` returns the remainder. For suitable exact coefficients, the result
obeys `dividend == quotient * divisor + remainder`, with zero remainder or a
remainder whose degree is lower than the divisor's degree.

- A zero polynomial divisor throws `std::invalid_argument`.
- Integral coefficients require exact division at every leading-coefficient
  step. A nondivisible step throws `std::domain_error`; polynomial division does
  not use the truncation semantics of scalar division.
- Floating point coefficients must be finite. Nonfinite inputs throw
  `std::domain_error`, and nonfinite computed quotients or remainder entries
  throw `std::overflow_error`. A quotient coefficient rounded to zero cannot
  reduce the degree and throws `std::domain_error`.
- The eliminated leading coefficient is explicitly set to zero to ensure
  progress in floating point arithmetic. The reconstruction identity is
  approximate, with accuracy subject to conditioning and rounding. Remaining
  coefficients are not trimmed by a tolerance.
- Other coefficient types must divide and cancel exactly; otherwise long
  division throws `std::domain_error`. General approximate complex long division
  is not supported by this contract.

```cpp
#include <cassert>
#include <stdexcept>

#include "extra/nuo_polynomial.hpp"

void polynomial_division_example()
{
  using Polynomial = nuostl::NuoPolynomial<int>;
  const Polynomial dividend{1, 2, 3};
  const Polynomial divisor{1, 1};
  const auto result = dividend.Divmod(divisor);
  assert(result.first == (Polynomial{-1, 3}));
  assert(result.second == (Polynomial{2}));
  assert(result.first * divisor + result.second == dividend);

  bool caught = false;
  try
  {
    (Polynomial{1}).Divmod(Polynomial{2});
  }
  catch (const std::domain_error&)
  {
    caught = true;
  }
  assert(caught);
}
```

## Output

Output lists nonzero terms in ascending degree, retaining unit coefficients.
Zero prints as `0`, and `{0, -1, 0, 2}` prints as `-1*x + 2*x^3`.

```cpp
#include <cassert>
#include <sstream>

#include "extra/nuo_polynomial.hpp"

void polynomial_output_example()
{
  std::ostringstream stream;
  stream << nuostl::NuoPolynomial<int>{1, -2, 3};
  assert(stream.str() == "1 - 2*x + 3*x^2");
}
```

## Complexity

For stored coefficient counts `n` and `m`:

- Size, degree, zero checks, and coefficient access are constant time.
- Evaluation, derivatives, and scalar operations take `O(n)` arithmetic steps.
- Addition and subtraction take `O(max(n, m))` steps.
- Multiplication takes `O(n*m)` steps and `O(n+m)` result storage.
- Long division takes `O((n-m+1)*m)` steps when `n >= m`, plus linear copying,
  and uses `O(n)` auxiliary storage.
- `SetCoefficient` can take linear time when growing or removing trailing zeros.

These counts assume constant-time coefficient operations. Root finding,
integration, multivariate polynomials, and expression parsing are outside this
component's current scope.
