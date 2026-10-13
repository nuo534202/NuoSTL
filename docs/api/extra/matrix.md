# Matrix

## Purpose

`nuostl::NuoMatrix<T>` is a dense matrix with the linear algebra operations that `std::complex` provides for numbers and the standard library does not provide for matrices: element access, arithmetic, transpose, trace, determinant, inverse and integer powers.

The standard library has no matrix class template, so the interface is a design choice of this project rather than a copy of `std::mdspan` (which is a view, not an owning matrix).

## Important Notes

- **Header only**: `NuoMatrix` is a class template and is therefore implemented in the header.
- **Element type**: any type with value semantics for `+`, `-`, `*`, `/`, `==`, `!=` and default construction works. `double` and `NuoComplex<double>` are covered by the tests; an integral element type works for addition, multiplication and integer powers, but not for `Determinant` and `Inverse`, which divide.
- **Shape errors throw `std::invalid_argument`**: adding matrices of different shapes, multiplying matrices whose inner dimensions do not match, and calling `Trace`, `Determinant`, `Inverse` or `Pow` on a non square matrix.
- **Singular matrices**: `Inverse` throws `std::domain_error`; `Determinant` returns `0`.
- **Pivoting**: elimination picks the first nonzero entry of the current column. There is no magnitude based pivot search, so an exact element type keeps exact results, while a floating point matrix may lose accuracy on badly scaled data.
- **`operator()` is unchecked**, exactly like `std::vector::operator[]`; `At` performs the bounds check and throws `std::out_of_range`.

## Header

1. include the component header directly

```cpp
#include "extra/nuo_matrix.hpp"
```

2. include the universal header `nuostl.hpp` to get all NuoSTL features

```cpp
#include "nuostl.hpp"
```

Namespace:

```cpp
nuostl::NuoMatrix<T>
```

## Data Model

- `elements_`: the entries in row major order
- `rows_`, `cols_`: the shape

`Size()` is `rows_ * cols_`, and a default constructed matrix is empty (`Rows() == 0`).

## Constructors

```cpp
NuoMatrix();                                /* 0 x 0 */
NuoMatrix(size_type rows, size_type cols);  /* zero filled */
NuoMatrix(size_type rows, size_type cols, const T& value);
NuoMatrix(std::initializer_list<std::initializer_list<T>> rows);

static NuoMatrix Identity(size_type size);
```

A nested initializer list defines the entries row by row; rows of different lengths throw `std::invalid_argument`.

```cpp
#include <assert.h>

#include "extra/nuo_matrix.hpp"

void matrix_constructor_examples()
{
    const nuostl::NuoMatrix<double> empty;
    assert(empty.Empty() && empty.Rows() == 0 && empty.Cols() == 0);

    const nuostl::NuoMatrix<double> zero(2, 3);
    assert(zero.Rows() == 2 && zero.Cols() == 3 && zero.Size() == 6);
    assert(zero(1, 2) == 0.0);

    const nuostl::NuoMatrix<double> filled(2, 2, 7.5);
    assert(filled(0, 0) == 7.5 && filled(1, 1) == 7.5);

    const nuostl::NuoMatrix<double> listed{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    assert(listed.Rows() == 2 && listed.Cols() == 3);
    assert(listed(1, 2) == 6.0);

    const nuostl::NuoMatrix<double> identity = nuostl::NuoMatrix<double>::Identity(3);
    assert(identity(0, 0) == 1.0 && identity(0, 1) == 0.0);
}
```

## Element Access

```cpp
T& operator()(size_type row, size_type col) noexcept; /* unchecked */
const T& operator()(size_type row, size_type col) const noexcept;
T& At(size_type row, size_type col);        /* checked */
const T& At(size_type row, size_type col) const;

void Fill(const T& value);
void Resize(size_type rows, size_type cols);
void SwapRows(size_type first, size_type second);
```

`Resize` keeps the entries that stay inside the new shape and zero fills the rest.

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/nuo_matrix.hpp"

void matrix_access_examples()
{
    nuostl::NuoMatrix<double> matrix(2, 2);
    matrix(0, 0) = 1.0;
    matrix.At(1, 1) = 4.0;
    assert(matrix(0, 0) == 1.0 && matrix.At(1, 1) == 4.0);

    matrix.Resize(3, 3);
    assert(matrix.Rows() == 3 && matrix(0, 0) == 1.0 && matrix(2, 2) == 0.0);

    matrix.SwapRows(0, 2);
    assert(matrix(0, 0) == 0.0 && matrix(2, 0) == 1.0);

    matrix.Fill(2.0);
    assert(matrix(1, 1) == 2.0);

    bool caught = false;
    try {
        matrix.At(3, 0);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);
}
```

## Arithmetic

```cpp
NuoMatrix& operator+=(const NuoMatrix& other);
NuoMatrix& operator-=(const NuoMatrix& other);
NuoMatrix& operator*=(const T& scalar);
NuoMatrix& operator/=(const T& scalar);

NuoMatrix operator+(NuoMatrix lhs, const NuoMatrix& rhs);
NuoMatrix operator-(NuoMatrix lhs, const NuoMatrix& rhs);
NuoMatrix operator-(const NuoMatrix& value);
NuoMatrix operator*(const NuoMatrix& lhs, const NuoMatrix& rhs);
NuoMatrix operator*(NuoMatrix lhs, const T& scalar);
NuoMatrix operator*(const T& scalar, NuoMatrix rhs);
NuoMatrix operator/(NuoMatrix lhs, const T& scalar);

bool operator==(const NuoMatrix& other) const;
bool operator!=(const NuoMatrix& other) const;
```

The binary operators are non members and never modify their operands. Matrix multiplication requires `lhs.Cols() == rhs.Rows()`.

```cpp
#include <assert.h>

#include "extra/nuo_matrix.hpp"

void matrix_arithmetic_examples()
{
    const nuostl::NuoMatrix<double> a{{1.0, 2.0}, {3.0, 4.0}};
    const nuostl::NuoMatrix<double> b{{5.0, 6.0}, {7.0, 8.0}};

    assert((a + b) == nuostl::NuoMatrix<double>({{6.0, 8.0}, {10.0, 12.0}}));
    assert((a - b) == nuostl::NuoMatrix<double>({{-4.0, -4.0}, {-4.0, -4.0}}));
    assert((-a) == nuostl::NuoMatrix<double>({{-1.0, -2.0}, {-3.0, -4.0}}));
    assert((a * b) == nuostl::NuoMatrix<double>({{19.0, 22.0}, {43.0, 50.0}}));
    assert((a * 2.0) == nuostl::NuoMatrix<double>({{2.0, 4.0}, {6.0, 8.0}}));
    assert((2.0 * a) == (a * 2.0));
    assert((a / 2.0) == nuostl::NuoMatrix<double>({{0.5, 1.0}, {1.5, 2.0}}));
}
```

## Transpose, Trace and Powers

```cpp
NuoMatrix Transpose() const;
T Trace() const;                            /* square only */
NuoMatrix Pow(unsigned long long exponent) const; /* square only */
```

`Pow` uses repeated squaring, and `Pow(0)` is the identity.

```cpp
#include <assert.h>

#include "extra/nuo_matrix.hpp"

void matrix_structure_examples()
{
    const nuostl::NuoMatrix<double> matrix{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    const nuostl::NuoMatrix<double> transposed = matrix.Transpose();
    assert(transposed.Rows() == 3 && transposed.Cols() == 2);
    assert(transposed(0, 1) == 4.0 && transposed.Transpose() == matrix);

    const nuostl::NuoMatrix<double> square{{1.0, 2.0}, {3.0, 4.0}};
    assert(square.Trace() == 5.0);

    const nuostl::NuoMatrix<double> shear{{1.0, 1.0}, {0.0, 1.0}};
    assert(shear.Pow(3) == nuostl::NuoMatrix<double>({{1.0, 3.0}, {0.0, 1.0}}));
    assert(shear.Pow(0) == nuostl::NuoMatrix<double>::Identity(2));
}
```

## Determinant and Inverse

```cpp
T Determinant() const;                      /* square only, 0 for a singular matrix */
NuoMatrix Inverse() const;                  /* square only, throws for a singular matrix */
```

```cpp
#include <assert.h>
#include <stdexcept>

#include "extra/nuo_matrix.hpp"

void matrix_solve_examples()
{
    const nuostl::NuoMatrix<double> matrix{{1.0, 2.0}, {3.0, 4.0}};
    assert(matrix.Determinant() == -2.0);

    const nuostl::NuoMatrix<double> inverse = matrix.Inverse();
    assert((matrix * inverse) == nuostl::NuoMatrix<double>::Identity(2));
    assert((inverse * matrix) == nuostl::NuoMatrix<double>::Identity(2));

    bool caught = false;
    try {
        const nuostl::NuoMatrix<double> singular{{1.0, 2.0}, {2.0, 4.0}};
        singular.Inverse();
    } catch (const std::domain_error&) {
        caught = true;
    }
    assert(caught);
}
```

## Output

```cpp
template <typename T>
std::ostream& operator<<(std::ostream& stream, const NuoMatrix<T>& matrix);
```

The format lists the rows between square brackets.

```cpp
#include <assert.h>
#include <sstream>

#include "extra/nuo_matrix.hpp"

void matrix_output_example()
{
    std::ostringstream stream;
    stream << nuostl::NuoMatrix<double>({{1.0, 2.0}, {3.0, 4.0}});
    assert(stream.str() == "[[1, 2], [3, 4]]");
}
```

## Complexity

- Addition, subtraction and scalar operations are linear in the number of entries.
- Multiplication is `O(rows * inner * cols)`.
- `Determinant` and `Inverse` both run Gaussian elimination, so they are cubic in the order of the matrix.
- `Pow` performs `O(log exponent)` matrix multiplications.
