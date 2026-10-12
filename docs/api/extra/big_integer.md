# BigInteger

## Purpose

`nuostl::NuoBigInteger` is an arbitrary precision signed integer.

A built-in integer type has a fixed width, so values like `100!` or `2^1000` cannot be stored. `NuoBigInteger` grows its storage on demand and keeps exact results for addition, subtraction, multiplication, division and remainder.

## Important Notes (User Responsibilities)

- **Range**: only limited by available memory. The type never silently overflows, so no range checking is required from you.
- **Canonical form**: zero has exactly one representation, and it is never negative. `"-0"`, `"+0"` and `"0"` all compare equal to the default constructed value.
- **Division by zero**: `/` and `%` throw `std::domain_error` when the divisor is zero.
- **Truncated division**: `/` truncates toward zero and `%` keeps the sign of the dividend, exactly like built-in integers. For example `-7 / 2 == -3` and `-7 % 2 == -1`.
- **Parsing**: the string constructor accepts an optional `+` or `-` followed by one or more decimal digits. Anything else, including an empty string or whitespace, throws `std::invalid_argument`.
- **Narrowing back to `long long`**: `ToLongLong()` throws `std::overflow_error` when the value does not fit.

## Header

1. include the component header directly

```cpp
#include "extra/nuo_big_integer.hpp"
```

2. include the universal header `nuostl.hpp` to get all NuoSTL features

```cpp
#include "nuostl.hpp"
```

Namespace:

```cpp
nuostl::NuoBigInteger
```

## Data Model

Conceptually a value is stored as:

- `negative_` (`bool`): the sign, always `false` for zero
- `blocks_` (`std::vector<std::uint32_t>`): the magnitude in base `10^9`, least significant block first

Blocks never have leading zeros, so an empty `blocks_` means zero.
Because every block is below `10^9`, a product of two blocks plus carry still fits comfortably into 64 bits, which is why the base is a decimal one.

## Constructors

```cpp
NuoBigInteger();                        /* 0 */
NuoBigInteger(int value);
NuoBigInteger(unsigned int value);
NuoBigInteger(long value);
NuoBigInteger(unsigned long value);
NuoBigInteger(long long value);
NuoBigInteger(unsigned long long value);
explicit NuoBigInteger(std::string_view text);
```

The integral constructors are implicit, so a built-in integer can be used directly wherever a `NuoBigInteger` is expected:

```cpp
#include <assert.h>

#include "extra/nuo_big_integer.hpp"

void big_integer_constructor_examples()
{
    nuostl::NuoBigInteger zero;
    assert(zero.ToString() == "0");

    nuostl::NuoBigInteger small(-42);
    assert(small.ToString() == "-42");

    /* The full range of the widest built-in types is supported. */
    nuostl::NuoBigInteger smallest(LLONG_MIN);
    assert(smallest.ToString() == "-9223372036854775808");

    nuostl::NuoBigInteger biggest(ULLONG_MAX);
    assert(biggest.ToString() == "18446744073709551615");

    /* Decimal literals of any length, with an optional sign. */
    nuostl::NuoBigInteger from_text("31415926535897932384626433832795");
    assert(from_text.ToString() == "31415926535897932384626433832795");

    nuostl::NuoBigInteger normalized("-000123");
    assert(normalized.ToString() == "-123");
}
```

Malformed text throws:

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/nuo_big_integer.hpp"

void big_integer_parse_errors()
{
    bool caught = false;
    try {
        nuostl::NuoBigInteger bad("12a");
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);
}
```

## Observers

```cpp
bool IsZero() const;                    /* true only for the value 0 */
bool IsNegative() const;                /* false for 0, even when built from "-0" */
std::size_t DigitCount() const;         /* decimal length, 1 for 0 */
std::string ToString() const;           /* canonical decimal text */
NuoBigInteger Abs() const;              /* magnitude as a new value */
```

```cpp
#include <assert.h>

#include "extra/nuo_big_integer.hpp"

void big_integer_observer_examples()
{
    nuostl::NuoBigInteger value("-1000000000");
    assert(!value.IsZero());
    assert(value.IsNegative());
    assert(value.DigitCount() == 10);   /* the sign is not a digit */
    assert(value.Abs().ToString() == "1000000000");
    assert(!value.Abs().IsNegative());

    nuostl::NuoBigInteger zero("-0");
    assert(zero.IsZero());
    assert(!zero.IsNegative());
}
```

## Conversion

```cpp
long long ToLongLong() const;
```

Throws `std::overflow_error` when the value is outside `[LLONG_MIN, LLONG_MAX]`:

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/nuo_big_integer.hpp"

void big_integer_conversion_examples()
{
    assert(nuostl::NuoBigInteger(-7).ToLongLong() == -7);
    assert(nuostl::NuoBigInteger("-9223372036854775808").ToLongLong() ==
           LLONG_MIN);

    bool caught = false;
    try {
        nuostl::NuoBigInteger("9223372036854775808").ToLongLong();
    } catch (const std::overflow_error&) {
        caught = true;
    }
    assert(caught);
}
```

## Comparison

`NuoBigInteger` defines `operator==` and `operator<=>`, which gives all six relational operators.
The ordering is the usual mathematical one, so negative values are smaller than zero and magnitudes only decide between equal signs.

```cpp
#include <assert.h>
#include <algorithm>
#include <vector>

#include "extra/nuo_big_integer.hpp"

void big_integer_comparison_examples()
{
    nuostl::NuoBigInteger small(10);
    nuostl::NuoBigInteger large("100000000000000000000");
    nuostl::NuoBigInteger negative(-10);

    assert(negative < small);
    assert(small < large);
    assert(large > small);
    assert(small <= small);
    assert(small >= small);
    assert(small != large);
    assert(nuostl::NuoBigInteger("-0") == nuostl::NuoBigInteger());

    std::vector<nuostl::NuoBigInteger> values{large, negative, small};
    std::sort(values.begin(), values.end());
    assert(values[0] == negative && values[2] == large);
}
```

## Arithmetic Operators

Compound assignment:

```cpp
NuoBigInteger& operator+=(const NuoBigInteger& other);
NuoBigInteger& operator-=(const NuoBigInteger& other);
NuoBigInteger& operator*=(const NuoBigInteger& other);
NuoBigInteger& operator/=(const NuoBigInteger& other);
NuoBigInteger& operator%=(const NuoBigInteger& other);
```

Binary operators are non members that take the left operand by value:

```cpp
NuoBigInteger operator+(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator-(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator*(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator/(NuoBigInteger lhs, const NuoBigInteger& rhs);
NuoBigInteger operator%(NuoBigInteger lhs, const NuoBigInteger& rhs);
```

Unlike `Fraction`, the binary operators do **not** modify their operands; use `+=` and friends for in-place updates.

```cpp
#include <assert.h>

#include "extra/nuo_big_integer.hpp"

void big_integer_arithmetic_examples()
{
    const nuostl::NuoBigInteger a("100000000000000000000");
    const nuostl::NuoBigInteger b(7);

    assert((a + b).ToString() == "100000000000000000007");
    assert((a - b).ToString() == "99999999999999999993");
    assert((b - a).ToString() == "-99999999999999999993");
    assert((-a).ToString() == "-100000000000000000000");
    assert((a * b).ToString() == "700000000000000000000");

    nuostl::NuoBigInteger accumulated(1);
    accumulated += b;
    accumulated *= nuostl::NuoBigInteger(3);
    assert(accumulated.ToString() == "24");
}
```

Truncated division and the sign of the remainder follow the built-in rules:

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/nuo_big_integer.hpp"

void big_integer_division_examples()
{
    assert((nuostl::NuoBigInteger(-7) / nuostl::NuoBigInteger(2)).ToString() ==
           "-3");
    assert((nuostl::NuoBigInteger(-7) % nuostl::NuoBigInteger(2)).ToString() ==
           "-1");
    assert((nuostl::NuoBigInteger(7) % nuostl::NuoBigInteger(-2)).ToString() ==
           "1");

    /* a == (a / b) * b + a % b always holds for a non-zero b. */
    const nuostl::NuoBigInteger a("123456789012345678901234567890");
    const nuostl::NuoBigInteger b("98765432109876543210");
    assert((a / b) * b + a % b == a);

    bool caught = false;
    try {
        nuostl::NuoBigInteger(1) / nuostl::NuoBigInteger(0);
    } catch (const std::domain_error&) {
        caught = true;
    }
    assert(caught);
}
```

## Increment and Decrement

```cpp
NuoBigInteger& operator++();            /* prefix */
NuoBigInteger operator++(int);          /* postfix */
NuoBigInteger& operator--();
NuoBigInteger operator--(int);
```

```cpp
#include <assert.h>

#include "extra/nuo_big_integer.hpp"

void big_integer_increment_example()
{
    nuostl::NuoBigInteger value(9);
    assert((++value).ToString() == "10");
    assert((value--).ToString() == "10");
    assert(value.ToString() == "9");
}
```

## Utility Functions

```cpp
static NuoBigInteger Pow(const NuoBigInteger& base,
                         unsigned long long exponent);
static NuoBigInteger Gcd(const NuoBigInteger& lhs,
                         const NuoBigInteger& rhs);
```

- `Pow` uses repeated squaring and returns `1` for `0^0`.
- `Gcd` returns a non-negative result, and `Gcd(0, 0)` is `0`.

```cpp
#include <assert.h>

#include "extra/nuo_big_integer.hpp"

void big_integer_utility_examples()
{
    const nuostl::NuoBigInteger two(2);
    assert(nuostl::NuoBigInteger::Pow(two, 100).ToString() ==
           "1267650600228229401496703205376");
    assert(nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(0), 0).ToString() ==
           "1");

    assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(-12),
                                      nuostl::NuoBigInteger(18)).ToString() ==
           "6");
    assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(0),
                                      nuostl::NuoBigInteger(0)).IsZero());
}
```

## Output

```cpp
std::ostream& operator<<(std::ostream& stream, const NuoBigInteger& value);
```

It prints exactly `ToString()`:

```cpp
#include <assert.h>
#include <sstream>

#include "extra/nuo_big_integer.hpp"

void big_integer_output_example()
{
    const nuostl::NuoBigInteger value("12345678901234567890123456");
    std::ostringstream stream;
    stream << value;
    assert(stream.str() == value.ToString());
}
```

## Complexity

- Addition and subtraction are linear in the number of blocks.
- Multiplication is the schoolbook algorithm: `O(n * m)` block products for operands of `n` and `m` blocks.
- Division walks the dividend block by block and finds each quotient block by bisection over `[0, 10^9)`, so it is `O(n * m * 30)` block operations. This is slower than a normalized long division but needs no digit correction step.

Because one block holds nine decimal digits, an operand with `d` decimal digits uses roughly `d / 9` blocks.
