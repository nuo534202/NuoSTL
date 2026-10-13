#pragma once

#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace nuostl
{

/* The standard library has no matrix class template, so this type offers the
   dense linear algebra operations directly: element access, arithmetic,
   transpose, trace, determinant, inverse and integer powers.
   Elements are stored row major, and the element type only needs value
   semantics for +, -, *, / and the equality operators. */
template <typename T>
class NuoMatrix
{
public:
  using value_type = T;
  using size_type = std::size_t;

  NuoMatrix() = default;
  NuoMatrix(size_type rows, size_type cols);
  NuoMatrix(size_type rows, size_type cols, const T& value);
  NuoMatrix(std::initializer_list<std::initializer_list<T>> rows);

  /* The identity matrix of the given order. */
  static NuoMatrix Identity(size_type size);

  size_type Rows() const noexcept;
  size_type Cols() const noexcept;
  size_type Size() const noexcept;
  bool Empty() const noexcept;

  /* Stores value in every position. */
  void Fill(const T& value);

  /* Changes the shape, keeping the overlapping block and zero filling the
     rest. */
  void Resize(size_type rows, size_type cols);

  void SwapRows(size_type first, size_type second);

  /* Unchecked access, as for std::vector::operator[]. */
  T& operator()(size_type row, size_type col) noexcept;
  const T& operator()(size_type row, size_type col) const noexcept;

  /* Checked access; throws std::out_of_range. */
  T& At(size_type row, size_type col);
  const T& At(size_type row, size_type col) const;

  bool operator==(const NuoMatrix& other) const;
  bool operator!=(const NuoMatrix& other) const;

  NuoMatrix& operator+=(const NuoMatrix& other);
  NuoMatrix& operator-=(const NuoMatrix& other);
  NuoMatrix& operator*=(const T& scalar);
  NuoMatrix& operator/=(const T& scalar);

  NuoMatrix Transpose() const;
  T Trace() const;
  T Determinant() const;
  NuoMatrix Inverse() const;
  NuoMatrix Pow(unsigned long long exponent) const;

private:
  void RequireSquare(const char* operation) const;
  size_type Index(size_type row, size_type col) const noexcept;

  std::vector<T> elements_;
  size_type rows_ = 0;
  size_type cols_ = 0;
};

/* construct / copy / destroy */

template <typename T>
NuoMatrix<T>::NuoMatrix(size_type rows, size_type cols)
  : elements_(rows * cols)
  , rows_(rows)
  , cols_(cols)
{
}

template <typename T>
NuoMatrix<T>::NuoMatrix(size_type rows, size_type cols, const T& value)
  : elements_(rows * cols, value)
  , rows_(rows)
  , cols_(cols)
{
}

template <typename T>
NuoMatrix<T>::NuoMatrix(std::initializer_list<std::initializer_list<T>> rows)
  : rows_(rows.size())
  , cols_(rows.size() == 0 ? 0 : rows.begin()->size())
{
  elements_.reserve(rows_ * cols_);
  for (const std::initializer_list<T>& row : rows)
  {
    if (row.size() != cols_)
    {
      throw std::invalid_argument("NuoMatrix: rows have different lengths");
    }
    elements_.insert(elements_.end(), row.begin(), row.end());
  }
}

template <typename T>
NuoMatrix<T> NuoMatrix<T>::Identity(size_type size)
{
  NuoMatrix result(size, size);
  for (size_type index = 0; index < size; ++index)
  {
    result(index, index) = T(1);
  }
  return result;
}

/* observers */

template <typename T>
typename NuoMatrix<T>::size_type NuoMatrix<T>::Rows() const noexcept
{
  return rows_;
}

template <typename T>
typename NuoMatrix<T>::size_type NuoMatrix<T>::Cols() const noexcept
{
  return cols_;
}

template <typename T>
typename NuoMatrix<T>::size_type NuoMatrix<T>::Size() const noexcept
{
  return elements_.size();
}

template <typename T>
bool NuoMatrix<T>::Empty() const noexcept
{
  return elements_.empty();
}

template <typename T>
void NuoMatrix<T>::Fill(const T& value)
{
  for (T& element : elements_)
  {
    element = value;
  }
}

template <typename T>
void NuoMatrix<T>::Resize(size_type rows, size_type cols)
{
  if (rows == rows_ && cols == cols_)
  {
    return;
  }
  NuoMatrix resized(rows, cols);
  const size_type shared_rows = rows < rows_ ? rows : rows_;
  const size_type shared_cols = cols < cols_ ? cols : cols_;
  for (size_type row = 0; row < shared_rows; ++row)
  {
    for (size_type col = 0; col < shared_cols; ++col)
    {
      resized(row, col) = (*this)(row, col);
    }
  }
  *this = std::move(resized);
}

template <typename T>
void NuoMatrix<T>::SwapRows(size_type first, size_type second)
{
  if (first >= rows_ || second >= rows_)
  {
    throw std::out_of_range("NuoMatrix::SwapRows: row index out of range");
  }
  if (first == second)
  {
    return;
  }
  for (size_type col = 0; col < cols_; ++col)
  {
    using std::swap;
    swap((*this)(first, col), (*this)(second, col));
  }
}

/* element access */

template <typename T>
typename NuoMatrix<T>::size_type
NuoMatrix<T>::Index(size_type row, size_type col) const noexcept
{
  return row * cols_ + col;
}

template <typename T>
T& NuoMatrix<T>::operator()(size_type row, size_type col) noexcept
{
  return elements_[Index(row, col)];
}

template <typename T>
const T& NuoMatrix<T>::operator()(size_type row, size_type col) const noexcept
{
  return elements_[Index(row, col)];
}

template <typename T>
T& NuoMatrix<T>::At(size_type row, size_type col)
{
  if (row >= rows_ || col >= cols_)
  {
    throw std::out_of_range("NuoMatrix::At: index out of range");
  }
  return (*this)(row, col);
}

template <typename T>
const T& NuoMatrix<T>::At(size_type row, size_type col) const
{
  if (row >= rows_ || col >= cols_)
  {
    throw std::out_of_range("NuoMatrix::At: index out of range");
  }
  return (*this)(row, col);
}

/* comparison */

template <typename T>
bool NuoMatrix<T>::operator==(const NuoMatrix& other) const
{
  return rows_ == other.rows_ && cols_ == other.cols_ &&
    elements_ == other.elements_;
}

template <typename T>
bool NuoMatrix<T>::operator!=(const NuoMatrix& other) const
{
  return !(*this == other);
}

/* compound assignment */

template <typename T>
NuoMatrix<T>& NuoMatrix<T>::operator+=(const NuoMatrix& other)
{
  if (rows_ != other.rows_ || cols_ != other.cols_)
  {
    throw std::invalid_argument("NuoMatrix: shapes do not match");
  }
  for (size_type index = 0; index < elements_.size(); ++index)
  {
    elements_[index] = elements_[index] + other.elements_[index];
  }
  return *this;
}

template <typename T>
NuoMatrix<T>& NuoMatrix<T>::operator-=(const NuoMatrix& other)
{
  if (rows_ != other.rows_ || cols_ != other.cols_)
  {
    throw std::invalid_argument("NuoMatrix: shapes do not match");
  }
  for (size_type index = 0; index < elements_.size(); ++index)
  {
    elements_[index] = elements_[index] - other.elements_[index];
  }
  return *this;
}

template <typename T>
NuoMatrix<T>& NuoMatrix<T>::operator*=(const T& scalar)
{
  for (T& element : elements_)
  {
    element = element * scalar;
  }
  return *this;
}

template <typename T>
NuoMatrix<T>& NuoMatrix<T>::operator/=(const T& scalar)
{
  for (T& element : elements_)
  {
    element = element / scalar;
  }
  return *this;
}

/* linear algebra */

template <typename T>
NuoMatrix<T> NuoMatrix<T>::Transpose() const
{
  NuoMatrix result(cols_, rows_);
  for (size_type row = 0; row < rows_; ++row)
  {
    for (size_type col = 0; col < cols_; ++col)
    {
      result(col, row) = (*this)(row, col);
    }
  }
  return result;
}

template <typename T>
void NuoMatrix<T>::RequireSquare(const char* operation) const
{
  if (rows_ != cols_)
  {
    throw std::invalid_argument(operation);
  }
}

template <typename T>
T NuoMatrix<T>::Trace() const
{
  RequireSquare("NuoMatrix::Trace: matrix is not square");
  T total = T();
  for (size_type index = 0; index < rows_; ++index)
  {
    total = total + (*this)(index, index);
  }
  return total;
}

/* Gaussian elimination with a nonzero pivot search, so an exact element type
   produces an exact determinant. */
template <typename T>
T NuoMatrix<T>::Determinant() const
{
  RequireSquare("NuoMatrix::Determinant: matrix is not square");
  NuoMatrix work(*this);
  T determinant = T(1);

  for (size_type pivot = 0; pivot < rows_; ++pivot)
  {
    size_type chosen = pivot;
    while (chosen < rows_ && work(chosen, pivot) == T())
    {
      ++chosen;
    }
    if (chosen == rows_)
    {
      return T();
    }
    if (chosen != pivot)
    {
      work.SwapRows(chosen, pivot);
      determinant = T() - determinant;
    }
    const T pivot_value = work(pivot, pivot);
    determinant = determinant * pivot_value;
    for (size_type row = pivot + 1; row < rows_; ++row)
    {
      const T factor = work(row, pivot) / pivot_value;
      for (size_type col = pivot; col < cols_; ++col)
      {
        work(row, col) = work(row, col) - factor * work(pivot, col);
      }
    }
  }
  return determinant;
}

/* Gauss-Jordan elimination on the matrix augmented with the identity. */
template <typename T>
NuoMatrix<T> NuoMatrix<T>::Inverse() const
{
  RequireSquare("NuoMatrix::Inverse: matrix is not square");
  const size_type size = rows_;
  const size_type width = size * 2;
  NuoMatrix augmented(size, width);
  for (size_type row = 0; row < size; ++row)
  {
    for (size_type col = 0; col < size; ++col)
    {
      augmented(row, col) = (*this)(row, col);
      augmented(row, size + col) = row == col ? T(1) : T();
    }
  }

  for (size_type pivot = 0; pivot < size; ++pivot)
  {
    size_type chosen = pivot;
    while (chosen < size && augmented(chosen, pivot) == T())
    {
      ++chosen;
    }
    if (chosen == size)
    {
      throw std::domain_error("NuoMatrix::Inverse: matrix is singular");
    }
    if (chosen != pivot)
    {
      augmented.SwapRows(chosen, pivot);
    }
    const T pivot_value = augmented(pivot, pivot);
    for (size_type col = 0; col < width; ++col)
    {
      augmented(pivot, col) = augmented(pivot, col) / pivot_value;
    }
    for (size_type row = 0; row < size; ++row)
    {
      if (row == pivot)
      {
        continue;
      }
      const T factor = augmented(row, pivot);
      if (factor == T())
      {
        continue;
      }
      for (size_type col = 0; col < width; ++col)
      {
        augmented(row, col) =
          augmented(row, col) - factor * augmented(pivot, col);
      }
    }
  }

  NuoMatrix result(size, size);
  for (size_type row = 0; row < size; ++row)
  {
    for (size_type col = 0; col < size; ++col)
    {
      result(row, col) = augmented(row, size + col);
    }
  }
  return result;
}

template <typename T>
NuoMatrix<T> NuoMatrix<T>::Pow(unsigned long long exponent) const
{
  RequireSquare("NuoMatrix::Pow: matrix is not square");
  NuoMatrix result = Identity(rows_);
  NuoMatrix factor(*this);
  while (exponent > 0)
  {
    if ((exponent & 1ull) != 0ull)
    {
      result = result * factor;
    }
    exponent >>= 1;
    if (exponent > 0)
    {
      factor = factor * factor;
    }
  }
  return result;
}

/* free operators */

template <typename T>
NuoMatrix<T> operator+(NuoMatrix<T> lhs, const NuoMatrix<T>& rhs)
{
  lhs += rhs;
  return lhs;
}

template <typename T>
NuoMatrix<T> operator-(NuoMatrix<T> lhs, const NuoMatrix<T>& rhs)
{
  lhs -= rhs;
  return lhs;
}

template <typename T>
NuoMatrix<T> operator-(const NuoMatrix<T>& value)
{
  NuoMatrix result(value);
  result *= T(-1);
  return result;
}

template <typename T>
NuoMatrix<T> operator*(const NuoMatrix<T>& lhs, const NuoMatrix<T>& rhs)
{
  if (lhs.Cols() != rhs.Rows())
  {
    throw std::invalid_argument("NuoMatrix: shapes do not match");
  }
  NuoMatrix<T> result(lhs.Rows(), rhs.Cols());
  for (typename NuoMatrix<T>::size_type row = 0; row < lhs.Rows(); ++row)
  {
    for (typename NuoMatrix<T>::size_type inner = 0; inner < lhs.Cols();
         ++inner)
    {
      const T& left = lhs(row, inner);
      if (left == T())
      {
        continue;
      }
      for (typename NuoMatrix<T>::size_type col = 0; col < rhs.Cols(); ++col)
      {
        result(row, col) = result(row, col) + left * rhs(inner, col);
      }
    }
  }
  return result;
}

template <typename T>
NuoMatrix<T> operator*(NuoMatrix<T> lhs, const T& scalar)
{
  lhs *= scalar;
  return lhs;
}

template <typename T>
NuoMatrix<T> operator*(const T& scalar, NuoMatrix<T> rhs)
{
  rhs *= scalar;
  return rhs;
}

template <typename T>
NuoMatrix<T> operator/(NuoMatrix<T> lhs, const T& scalar)
{
  lhs /= scalar;
  return lhs;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const NuoMatrix<T>& matrix)
{
  stream << '[';
  for (typename NuoMatrix<T>::size_type row = 0; row < matrix.Rows(); ++row)
  {
    if (row != 0)
    {
      stream << ", ";
    }
    stream << '[';
    for (typename NuoMatrix<T>::size_type col = 0; col < matrix.Cols(); ++col)
    {
      if (col != 0)
      {
        stream << ", ";
      }
      stream << matrix(row, col);
    }
    stream << ']';
  }
  return stream << ']';
}

} /* namespace nuostl */
