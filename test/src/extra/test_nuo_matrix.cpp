#include "extra/test_nuo_matrix.hpp"

#include <cassert>

#include <cmath>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "extra/nuo_complex.hpp"
#include "extra/nuo_matrix.hpp"

namespace test
{

namespace
{

using Matrix = nuostl::NuoMatrix<double>;

/* Deterministic pseudo random integers, kept small so that every determinant
   stays exactly representable in a double. */
class Random
{
public:
  explicit Random(unsigned int seed) : state_(seed)
  {
  }

  int Next()
  {
    state_ = state_ * 1103515245u + 12345u;
    return static_cast<int>((state_ >> 16) % 7) - 3;
  }

private:
  unsigned int state_;
};

Matrix RandomMatrix(Random& random, std::size_t size)
{
  Matrix matrix(size, size);
  for (std::size_t row = 0; row < size; ++row)
  {
    for (std::size_t col = 0; col < size; ++col)
    {
      matrix(row, col) = static_cast<double>(random.Next());
    }
  }
  return matrix;
}

bool NearlyEqual(double lhs, double rhs, double tolerance)
{
  return std::fabs(lhs - rhs) <= tolerance * (1.0 + std::fabs(rhs));
}

bool IsIdentity(const Matrix& matrix, double tolerance)
{
  if (matrix.Rows() != matrix.Cols())
  {
    return false;
  }
  for (std::size_t row = 0; row < matrix.Rows(); ++row)
  {
    for (std::size_t col = 0; col < matrix.Cols(); ++col)
    {
      const double expected = row == col ? 1.0 : 0.0;
      if (!NearlyEqual(matrix(row, col), expected, tolerance))
      {
        return false;
      }
    }
  }
  return true;
}

} /* namespace */

void TestNuoMatrix::TestConstruction()
{
  const Matrix empty;
  assert(empty.Empty());
  assert(empty.Rows() == 0 && empty.Cols() == 0 && empty.Size() == 0);

  const Matrix zero(2, 3);
  assert(zero.Rows() == 2 && zero.Cols() == 3 && zero.Size() == 6);
  for (std::size_t row = 0; row < zero.Rows(); ++row)
  {
    for (std::size_t col = 0; col < zero.Cols(); ++col)
    {
      assert(zero(row, col) == 0.0);
    }
  }

  const Matrix filled(2, 2, 7.5);
  for (std::size_t row = 0; row < filled.Rows(); ++row)
  {
    for (std::size_t col = 0; col < filled.Cols(); ++col)
    {
      assert(filled(row, col) == 7.5);
    }
  }

  const Matrix listed{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  assert(listed.Rows() == 2 && listed.Cols() == 3);
  assert(listed(0, 0) == 1.0 && listed(0, 2) == 3.0);
  assert(listed(1, 0) == 4.0 && listed(1, 2) == 6.0);

  const Matrix single{{4.0}};
  assert(single.Rows() == 1 && single.Cols() == 1 && single(0, 0) == 4.0);

  const Matrix identity = Matrix::Identity(3);
  assert(identity.Rows() == 3 && identity.Cols() == 3);
  for (std::size_t row = 0; row < 3; ++row)
  {
    for (std::size_t col = 0; col < 3; ++col)
    {
      assert(identity(row, col) == (row == col ? 1.0 : 0.0));
    }
  }

  const Matrix copied(listed);
  assert(copied == listed);
  Matrix assigned;
  assigned = copied;
  assert(assigned == listed);

  /* Ragged initializer lists are rejected. */
  bool caught = false;
  try
  {
    const Matrix ragged{{1.0, 2.0}, {3.0}};
    (void)ragged;
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);
}

void TestNuoMatrix::TestElementAccess()
{
  Matrix matrix(2, 3);
  matrix(0, 0) = 1.0;
  matrix(1, 2) = 6.0;
  assert(matrix(0, 0) == 1.0 && matrix(1, 2) == 6.0);
  assert(matrix.At(1, 2) == 6.0);

  matrix.At(0, 1) = 2.5;
  assert(matrix(0, 1) == 2.5);

  bool caught = false;
  try
  {
    matrix.At(2, 0);
  }
  catch (const std::out_of_range&)
  {
    caught = true;
  }
  assert(caught);

  caught = false;
  try
  {
    matrix.At(0, 3);
  }
  catch (const std::out_of_range&)
  {
    caught = true;
  }
  assert(caught);

  const Matrix& const_matrix = matrix;
  assert(const_matrix.At(0, 1) == 2.5);

  matrix.Fill(9.0);
  for (std::size_t row = 0; row < matrix.Rows(); ++row)
  {
    for (std::size_t col = 0; col < matrix.Cols(); ++col)
    {
      assert(matrix(row, col) == 9.0);
    }
  }

  matrix.Resize(3, 2);
  assert(matrix.Rows() == 3 && matrix.Cols() == 2);
  assert(matrix(0, 0) == 9.0 && matrix(2, 1) == 0.0);

  matrix.Resize(0, 0);
  assert(matrix.Empty());
}

void TestNuoMatrix::TestArithmetic()
{
  const Matrix a{{1.0, 2.0}, {3.0, 4.0}};
  const Matrix b{{5.0, 6.0}, {7.0, 8.0}};

  const Matrix sum = a + b;
  assert(sum == Matrix({{6.0, 8.0}, {10.0, 12.0}}));
  const Matrix difference = a - b;
  assert(difference == Matrix({{-4.0, -4.0}, {-4.0, -4.0}}));
  assert(-a == Matrix({{-1.0, -2.0}, {-3.0, -4.0}}));

  const Matrix scaled = a * 2.0;
  assert(scaled == Matrix({{2.0, 4.0}, {6.0, 8.0}}));
  assert(2.0 * a == scaled);
  const Matrix halved = a / 2.0;
  assert(halved == Matrix({{0.5, 1.0}, {1.5, 2.0}}));

  const Matrix product = a * b;
  assert(product == Matrix({{19.0, 22.0}, {43.0, 50.0}}));

  /* Non square shapes multiply when the inner dimensions agree. */
  const Matrix tall{{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
  const Matrix wide{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  const Matrix outer = tall * wide;
  assert(outer.Rows() == 3 && outer.Cols() == 3);
  assert(outer(0, 0) == 9.0 && outer(2, 2) == 51.0);

  Matrix accumulated(a);
  accumulated += b;
  assert(accumulated == sum);
  accumulated -= b;
  assert(accumulated == a);
  accumulated *= 2.0;
  assert(accumulated == scaled);
  accumulated /= 2.0;
  assert(accumulated == a);

  assert(a != b && a == a);
  assert(a != Matrix(2, 2));

  bool caught = false;
  try
  {
    const Matrix mismatched = a + Matrix(3, 3);
    (void)mismatched;
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);

  caught = false;
  try
  {
    const Matrix mismatched = a * Matrix(3, 3);
    (void)mismatched;
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);
}

void TestNuoMatrix::TestStructure()
{
  const Matrix matrix{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

  const Matrix transposed = matrix.Transpose();
  assert(transposed.Rows() == 3 && transposed.Cols() == 2);
  assert(transposed(0, 0) == 1.0 && transposed(0, 1) == 4.0);
  assert(transposed(2, 0) == 3.0 && transposed(2, 1) == 6.0);
  assert(transposed.Transpose() == matrix);

  const Matrix square{{1.0, 2.0}, {3.0, 4.0}};
  assert(square.Trace() == 5.0);
  assert(Matrix::Identity(4).Trace() == 4.0);

  bool caught = false;
  try
  {
    matrix.Trace();
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);

  Matrix swappable{{1.0, 2.0}, {3.0, 4.0}};
  swappable.SwapRows(0, 1);
  assert(swappable == Matrix({{3.0, 4.0}, {1.0, 2.0}}));
  swappable.SwapRows(1, 1);
  assert(swappable == Matrix({{3.0, 4.0}, {1.0, 2.0}}));

  caught = false;
  try
  {
    swappable.SwapRows(0, 2);
  }
  catch (const std::out_of_range&)
  {
    caught = true;
  }
  assert(caught);

  /* (A * B)^T == B^T * A^T */
  Random random(11);
  for (int trial = 0; trial < 32; ++trial)
  {
    const Matrix left = RandomMatrix(random, 3);
    const Matrix right = RandomMatrix(random, 3);
    assert((left * right).Transpose() ==
           right.Transpose() * left.Transpose());
  }
}

void TestNuoMatrix::TestDeterminant()
{
  assert(Matrix({{7.0}}).Determinant() == 7.0);
  assert(Matrix({{1.0, 2.0}, {3.0, 4.0}}).Determinant() == -2.0);
  assert(Matrix({{2.0, 0.0, 0.0}, {0.0, 3.0, 0.0}, {0.0, 0.0, 4.0}})
           .Determinant() == 24.0);
  assert(Matrix({{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 10.0}})
           .Determinant() == -3.0);
  assert(Matrix({{1.0, 2.0}, {2.0, 4.0}}).Determinant() == 0.0);
  assert(Matrix::Identity(5).Determinant() == 1.0);

  /* A zero pivot in the leading position still has to be handled. */
  assert(Matrix({{0.0, 2.0}, {3.0, 4.0}}).Determinant() == -6.0);

  bool caught = false;
  try
  {
    Matrix(2, 3).Determinant();
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);

  Random random(29);
  for (int trial = 0; trial < 64; ++trial)
  {
    const Matrix left = RandomMatrix(random, 3);
    const Matrix right = RandomMatrix(random, 3);
    assert(NearlyEqual((left * right).Determinant(),
                       left.Determinant() * right.Determinant(), 1e-9));
    assert(NearlyEqual(left.Transpose().Determinant(), left.Determinant(),
                       1e-9));
    const Matrix doubled = left * 2.0;
    assert(NearlyEqual(doubled.Determinant(),
                       8.0 * left.Determinant(), 1e-9));
  }
}

void TestNuoMatrix::TestInverse()
{
  assert(IsIdentity(Matrix({{4.0}}).Inverse() * Matrix({{4.0}}), 1e-12));
  assert(Matrix::Identity(3).Inverse() == Matrix::Identity(3));

  const Matrix matrix{{1.0, 2.0}, {3.0, 4.0}};
  const Matrix inverse = matrix.Inverse();
  assert(NearlyEqual(inverse(0, 0), -2.0, 1e-12));
  assert(NearlyEqual(inverse(0, 1), 1.0, 1e-12));
  assert(NearlyEqual(inverse(1, 0), 1.5, 1e-12));
  assert(NearlyEqual(inverse(1, 1), -0.5, 1e-12));
  assert(IsIdentity(matrix * inverse, 1e-12));
  assert(IsIdentity(inverse * matrix, 1e-12));

  bool caught = false;
  try
  {
    Matrix({{1.0, 2.0}, {2.0, 4.0}}).Inverse();
  }
  catch (const std::domain_error&)
  {
    caught = true;
  }
  assert(caught);

  caught = false;
  try
  {
    Matrix(2, 3).Inverse();
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);

  /* Random integer matrices: A * A^-1 is the identity whenever A is regular. */
  Random random(97);
  int regular = 0;
  for (int trial = 0; trial < 64; ++trial)
  {
    const Matrix sample = RandomMatrix(random, 3);
    if (sample.Determinant() == 0.0)
    {
      continue;
    }
    ++regular;
    const Matrix sample_inverse = sample.Inverse();
    assert(IsIdentity(sample * sample_inverse, 1e-9));
    assert(IsIdentity(sample_inverse * sample, 1e-9));
  }
  assert(regular > 40);
}

void TestNuoMatrix::TestPower()
{
  const Matrix matrix{{1.0, 1.0}, {0.0, 1.0}};

  assert(Matrix::Identity(2).Pow(0) == Matrix::Identity(2));
  assert(matrix.Pow(1) == matrix);
  const Matrix squared = matrix * matrix;
  assert(matrix.Pow(2) == squared);
  assert(matrix.Pow(3) == squared * matrix);
  assert(matrix.Pow(10) == Matrix({{1.0, 10.0}, {0.0, 1.0}}));

  bool caught = false;
  try
  {
    Matrix(2, 3).Pow(2);
  }
  catch (const std::invalid_argument&)
  {
    caught = true;
  }
  assert(caught);
}

void TestNuoMatrix::TestGenericElementType()
{
  using Complex = nuostl::NuoComplex<double>;
  using ComplexMatrix = nuostl::NuoMatrix<Complex>;

  /* The element type only needs value semantics, so NuoComplex works too. */
  const ComplexMatrix matrix{{Complex(1.0, 1.0), Complex(0.0, 0.0)},
                             {Complex(0.0, 0.0), Complex(2.0, 0.0)}};
  assert(matrix.Rows() == 2 && matrix.Cols() == 2);
  assert(matrix(0, 0) == Complex(1.0, 1.0));

  const ComplexMatrix identity = ComplexMatrix::Identity(2);
  assert(identity * matrix == matrix);

  const Complex determinant = matrix.Determinant();
  assert(nuostl::NuoNearlyEqual(determinant, Complex(2.0, 2.0), 1e-12));

  const ComplexMatrix inverse = matrix.Inverse();
  const ComplexMatrix product = matrix * inverse;
  for (std::size_t row = 0; row < 2; ++row)
  {
    for (std::size_t col = 0; col < 2; ++col)
    {
      const Complex expected = row == col ? Complex(1.0, 0.0)
                                          : Complex(0.0, 0.0);
      assert(nuostl::NuoNearlyEqual(product(row, col), expected, 1e-12));
    }
  }
}

void TestNuoMatrix::TestOutput()
{
  std::ostringstream stream;
  stream << Matrix({{1.0, 2.0}, {3.0, 4.0}});
  assert(stream.str() == "[[1, 2], [3, 4]]");

  std::ostringstream empty_stream;
  empty_stream << Matrix();
  assert(empty_stream.str() == "[]");
}

void TestNuoMatrix::TestSuite()
{
  TestConstruction();
  TestElementAccess();
  TestArithmetic();
  TestStructure();
  TestDeterminant();
  TestInverse();
  TestPower();
  TestGenericElementType();
  TestOutput();
}

} /* namespace test */
