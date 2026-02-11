# Fraction

## Purpose

`nuostl::Fraction` is a small rational-number type.

Instead of storing a decimal value as a `double` (which may lose precision for many decimal fractions), `Fraction` stores a value as an integer numerator and denominator.
This lets you represent values like `1/3`, `2/10`, `123/1000`, etc. exactly (as long as numerator/denominator fit in integer range).

## Important Notes (User Responsibilities)

- **Range**: You must ensure that all intermediate results fit into `__int128_t` (only available on gcc and clang), and final results fit into `long long` (`int64`). If you violate this, behavior/logic may be incorrect. This library does not attempt to detect all overflows.
- **Denominator must be non-zero**: constructing or setting a denominator of `0` throws `std::invalid_argument`.
- **Normalization rule**: the implementation normalizes signs so the denominator is kept positive when `Normalize()` is executed (the sign is moved to the numerator).

## Header

1. only include `fraction.hpp`

```cpp
#include "extra/fraction.hpp"
```

2. include universal header `nuostl.hpp` to get all NuoSTL features

```cpp
#include "nuostl.hpp"
```

Namespace:

```cpp
nuostl::Fraction
```

## Data Model

Conceptually, a fraction is stored as:

- numerator: `up_` (`int64`)
- denominator: `down_` (`int64`, must not be `0`)

`Fraction` also uses 128-bit intermediates (`__int128_t`) internally for some operations to reduce the chance of overflow during cross-multiplication, but **you still must ensure the final stored values fit in `int64`.**

## Constructors

### Default constructor

Creates `0/1`.

```cpp
nuostl::Fraction f;
// f == 0/1
```

### Value constructor

```cpp
nuostl::Fraction f(up, down);
```

- Throws `std::invalid_argument` if `down == 0`.
- Calls `Normalize()` so that a negative denominator becomes positive.

Example:

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/fraction.hpp"

void fraction_constructor_examples()
{
    nuostl::Fraction a(1, 2);
    assert(a.GetUp() == 1);
    assert(a.GetDown() == 2);

    // Normalization: sign moves to numerator
    nuostl::Fraction b(3, -4);
    assert(b.GetUp() == -3);
    assert(b.GetDown() == 4);

    // Denominator = 0 throws
    bool caught = false;
    try {
        nuostl::Fraction bad(1, 0);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);
}
```

## Arithmetic Operators

`Fraction` provides:

- `Fraction& operator+(const Fraction& rhs)`
- `Fraction& operator-(const Fraction& rhs)`
- `Fraction& operator*(const Fraction& rhs)`
- `Fraction& operator/(const Fraction& rhs)`

### Important behavior

These operators **modify the left-hand side object** and return `Fraction&`.
In other words, they behave like typical `+=`, `-=`, `*=`, `/=` operators even though they are named `operator+`, `operator-`, etc.

Example:

```cpp
#include <assert.h>

#include "extra/fraction.hpp"

void fraction_arithmetic_examples()
{
    nuostl::Fraction a(1, 2);
    nuostl::Fraction b(1, 3);

    // WARNING: this mutates a
    a + b; // a becomes 1/2 + 1/3 = 5/6
    assert(a.GetUp() == 5);
    assert(a.GetDown() == 6);

    a - nuostl::Fraction(1, 6); // a becomes 4/6 -> reduced to 2/3 by gcd
    assert(a.GetUp() == 2);
    assert(a.GetDown() == 3);

    a * nuostl::Fraction(3, 2); // 2/3 * 3/2 = 1/1
    assert(a.GetUp() == 1);
    assert(a.GetDown() == 1);

    a / nuostl::Fraction(-2, 5); // 1 / (-2/5) = -5/2, denominator normalized to positive
    assert(a.GetUp() == -5);
    assert(a.GetDown() == 2);
}
```

## Assignment Operator

```cpp
Fraction& operator=(const Fraction& rhs);
```

Example:

```cpp
#include <assert.h>

#include "extra/fraction.hpp"

void fraction_assignment_example()
{
    nuostl::Fraction a(1, 2);
    nuostl::Fraction b(3, 4);
    a = b;
    assert(a == b);
}
```

## Comparison Operators

`Fraction` supports:

- `==`, `!=`, `>`, `>=`, `<`, `<=`

The ordering comparisons (`>`, `>=`, `<`, `<=`) use exact integer cross-multiplication with 128-bit intermediates.

### Equality semantics

Current implementation of `operator==` / `operator!=` compares **stored numerator and denominator directly**.
That means fractions with the same mathematical value but different stored forms are **not equal**:

```cpp
#include <assert.h>

#include "extra/fraction.hpp"

void fraction_equality_note()
{
    nuostl::Fraction a(1, 2);
    nuostl::Fraction b(2, 4);

    // a and b represent the same mathematical value,
    // but the current == checks stored form (up_, down_)
    assert(a != b);
}
```

### Basic comparison example

```cpp
#include <assert.h>

#include "extra/fraction.hpp"

void fraction_comparison_examples()
{
    nuostl::Fraction a(3, 4);
    nuostl::Fraction b(1, 2);
    assert(a > b);
    assert(b < a);
    assert(a >= a);
    assert(a <= a);

    nuostl::Fraction n1(-1, 4);
    nuostl::Fraction n2(-1, 2);
    assert(n1 > n2); // -0.25 > -0.5
}
```

## Getters / Setters

### SetUp / SetDown

```cpp
void SetUp(int64 up);
void SetDown(int64 down);
```

- `SetDown(0)` throws `std::invalid_argument`.

Example:

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/fraction.hpp"

void fraction_setter_examples()
{
    nuostl::Fraction f(1, 2);
    f.SetUp(-10);
    assert(f.GetUp() == -10);

    f.SetDown(5);
    assert(f.GetDown() == 5);

    bool caught = false;
    try {
        f.SetDown(0);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);
}
```

### GetUp / GetDown / GetFraction

```cpp
int64 GetUp() const;
int64 GetDown() const;
double GetFraction() const;
```

`GetFraction()` converts the stored rational value to `double`.

Example:

```cpp
#include <assert.h>
#include <cmath>

#include "extra/fraction.hpp"

void fraction_getter_examples()
{
    nuostl::Fraction f(1, 3);
    assert(f.GetUp() == 1);
    assert(f.GetDown() == 3);
    assert(std::fabs(f.GetFraction() - (1.0 / 3.0)) < 1e-9);
}
```

## Normalize()

`Normalize()` is an **internal** helper (private member function). Users do not call it directly.
It ensures that if the denominator is negative, both numerator and denominator are flipped so that the denominator becomes positive.

User-visible behavior:

```cpp
#include <assert.h>

#include "extra/fraction.hpp"

void fraction_normalize_behavior()
{
    nuostl::Fraction f(3, -5);
    // Normalize() runs in the constructor
    assert(f.GetUp() == -3);
    assert(f.GetDown() == 5);
}
```

## Output

`Fraction` defines a member `operator<<`, but it prints to `std::cout`.

Example:

```cpp
#include "extra/fraction.hpp"

void fraction_output_example()
{
    nuostl::Fraction f(1, 2);
    std::cout << f; // prints "1 / 2"
}
```