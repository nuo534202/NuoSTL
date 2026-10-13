#include "extra/test_nuo_polynomial.hpp"

#include <cassert>

#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "extra/nuo_complex.hpp"
#include "extra/nuo_polynomial.hpp"

namespace test
{

namespace
{

using Polynomial = nuostl::NuoPolynomial<int>;

template <typename Exception, typename Function>
void ExpectThrows(Function function)
{
  bool caught = false;
  try
  {
    function();
  }
  catch (const Exception&)
  {
    caught = true;
  }
  assert(caught);
}

bool NearlyEqual(double lhs, double rhs)
{
  return std::fabs(lhs - rhs) <= 1e-10 * (1.0 + std::fabs(rhs));
}

} /* namespace */

void TestNuoPolynomial::TestConstruction()
{
  const Polynomial zero;
  assert(zero.IsZero());
  assert(zero.Degree() == 0);
  assert(zero.Size() == 0);
  assert(zero.Coefficient(0) == 0);
  assert(zero.Empty());
  assert((Polynomial{0, 0, 0}).IsZero());
  assert(Polynomial({}).IsZero());
  assert((Polynomial{7}).Degree() == 0);
  assert((Polynomial{7}).Size() == 1);

  const Polynomial polynomial{1, 2, 0, 0};
  assert(!polynomial.IsZero());
  assert(polynomial.Size() == 2);
  assert(polynomial.Degree() == 1);
  assert(polynomial.Coefficient(0) == 1);
  assert(polynomial.Coefficient(1) == 2);
  assert(polynomial.Coefficient(4) == 0);
  assert(polynomial.At(1) == 2);
  assert(polynomial == (Polynomial{1, 2}));
  assert(polynomial != (Polynomial{1, 3}));

  Polynomial copy = polynomial;
  copy.SetCoefficient(0, 9);
  assert(polynomial.Coefficient(0) == 1);
  Polynomial moved = std::move(copy);
  assert(moved == (Polynomial{9, 2}));
  copy = moved;
  copy = std::move(moved);
  assert(copy == (Polynomial{9, 2}));

  Polynomial mutable_polynomial;
  mutable_polynomial.SetCoefficient(3, 5);
  assert(mutable_polynomial.Size() == 4);
  assert(mutable_polynomial.Coefficient(3) == 5);
  mutable_polynomial.SetCoefficient(3, 0);
  assert(mutable_polynomial.IsZero());

  ExpectThrows<std::out_of_range>([&]()
  {
    mutable_polynomial.At(0);
  });
  ExpectThrows<std::out_of_range>([&]()
  {
    polynomial.At(polynomial.Size());
  });

  const auto largest_degree = std::numeric_limits<std::size_t>::max();
  mutable_polynomial.SetCoefficient(largest_degree, 0);
  assert(mutable_polynomial.IsZero());
  ExpectThrows<std::length_error>([&]()
  {
    mutable_polynomial.SetCoefficient(largest_degree, 1);
  });
  assert(mutable_polynomial.IsZero());

  mutable_polynomial = Polynomial{7};
  mutable_polynomial.SetCoefficient(100, mutable_polynomial.At(0));
  assert(mutable_polynomial.Coefficient(100) == 7);
}

void TestNuoPolynomial::TestEvaluation()
{
  const Polynomial polynomial{1, -2, 3};
  assert(polynomial.Evaluate(0) == 1);
  assert(polynomial.Evaluate(2) == 9);
  assert(polynomial(2) == 9);
  assert(polynomial(0) == 1);
  assert(polynomial(-2) == 17);
  assert(Polynomial()(2) == 0);
  assert((Polynomial{3})(-9) == 3);
}

void TestNuoPolynomial::TestArithmetic()
{
  const Polynomial first{1, 2, 3};
  const Polynomial second{4, -2, 1, 2};

  assert((first + second) == (Polynomial{5, 0, 4, 2}));
  assert((first - second) == (Polynomial{-3, 4, 2, -2}));
  assert((-first) == (Polynomial{-1, -2, -3}));
  assert((first * second) == (Polynomial{4, 6, 9, -2, 7, 6}));
  assert((first * 2) == (Polynomial{2, 4, 6}));
  assert(2 * first == first * 2);
  assert((first / 2) == (Polynomial{0, 1, 1}));
  assert((first - first).IsZero());
  assert((first * Polynomial()).IsZero());
  assert((Polynomial() * first).IsZero());
  assert((first * 0).IsZero());
  assert(first + Polynomial() == first);
  assert(Polynomial() - first == -first);

  Polynomial value(first);
  value += second;
  assert(value == (first + second));
  value -= second;
  assert(value == first);
  value *= 2;
  assert(value == (first * 2));
  value /= 2;
  assert(value == first);

  value = first;
  value += value;
  assert(value == first * 2);
  value -= value;
  assert(value.IsZero());
  value = first;
  value *= value;
  assert(value == first * first);

  value = Polynomial{2, 4};
  value *= value.At(0);
  assert(value == (Polynomial{4, 8}));
  value /= value.At(0);
  assert(value == (Polynomial{1, 2}));
  assert(((Polynomial{3, 1}) / 2) == (Polynomial{1}));

  ExpectThrows<std::invalid_argument>([&]()
  {
    value /= 0;
  });
  assert(value == (Polynomial{1, 2}));
  ExpectThrows<std::invalid_argument>([]()
  {
    const auto result = Polynomial() / 0;
    (void)result;
  });

  const int minimum = std::numeric_limits<int>::min();
  value = Polynomial{1, minimum};
  ExpectThrows<std::overflow_error>([&]()
  {
    value /= -1;
  });
  assert(value == (Polynomial{1, minimum}));
}

void TestNuoPolynomial::TestCalculus()
{
  const Polynomial polynomial{2, 6, 12, 8};
  assert(polynomial.Derivative() == (Polynomial{6, 24, 24}));
  assert(Polynomial().Derivative().IsZero());
  assert((Polynomial{9}).Derivative().IsZero());
  assert((Polynomial{0, 0, 1}).Derivative() == (Polynomial{0, 2}));
}

void TestNuoPolynomial::TestDivision()
{
  const Polynomial dividend{1, 2, 3};
  const Polynomial divisor{1, 1};
  const auto result = dividend.Divmod(divisor);
  assert(result.first == (Polynomial{-1, 3}));
  assert(result.second == (Polynomial{2}));
  assert((dividend / divisor) == result.first);
  assert((dividend % divisor) == result.second);
  assert((result.first * divisor + result.second) == dividend);

  const Polynomial exact{2, 3, 1};
  assert((exact / divisor) == (Polynomial{2, 1}));
  assert((exact % divisor).IsZero());

  const auto small = divisor.Divmod(dividend);
  assert(small.first.IsZero());
  assert(small.second == divisor);
  const auto zero = Polynomial().Divmod(divisor);
  assert(zero.first.IsZero() && zero.second.IsZero());
  assert(dividend / dividend == (Polynomial{1}));
  assert((dividend % dividend).IsZero());
  assert(dividend / Polynomial{1} == dividend);
  assert((dividend % Polynomial{1}).IsZero());
  assert((dividend / Polynomial{-1}) == -dividend);
  assert(((Polynomial{0, 0, 4}) / Polynomial{0, 2}) == (Polynomial{0, 2}));

  ExpectThrows<std::invalid_argument>([&]()
  {
    dividend.Divmod(Polynomial());
  });
  ExpectThrows<std::invalid_argument>([]()
  {
    Polynomial().Divmod(Polynomial());
  });
  ExpectThrows<std::domain_error>([]()
  {
    (Polynomial{1}).Divmod(Polynomial{2});
  });
  ExpectThrows<std::domain_error>([]()
  {
    (Polynomial{1, 3}).Divmod(Polynomial{1, 2});
  });
  ExpectThrows<std::domain_error>([]()
  {
    (Polynomial{1, 2}).Divmod(Polynomial{2});
  });
  ExpectThrows<std::overflow_error>([]()
  {
    (Polynomial{std::numeric_limits<int>::min()}).Divmod(Polynomial{-1});
  });
}

void TestNuoPolynomial::TestFloatingPoint()
{
  using RealPolynomial = nuostl::NuoPolynomial<double>;
  const RealPolynomial dividend{0.7, -1.3, 2.1, 3.2};
  const RealPolynomial divisor{0.1, 0.3};
  const auto result = dividend.Divmod(divisor);
  assert(result.second.IsZero() || result.second.Degree() < divisor.Degree());
  const auto reconstructed = result.first * divisor + result.second;
  for (std::size_t index = 0; index < dividend.Size(); ++index)
  {
    assert(NearlyEqual(reconstructed.Coefficient(index),
                       dividend.Coefficient(index)));
  }
  for (double x : {-2.0, 0.0, 0.5, 2.0})
  {
    assert(NearlyEqual(reconstructed(x), dividend(x)));
  }
  assert((RealPolynomial{0.0, 1e-20}).Degree() == 1);
  assert(NearlyEqual((RealPolynomial{1.0, 2.0} / 2.0)(0.0), 0.5));
  ExpectThrows<std::invalid_argument>([&]()
  {
    const auto value = dividend / 0.0;
    (void)value;
  });
  ExpectThrows<std::domain_error>([]()
  {
    const double infinity = std::numeric_limits<double>::infinity();
    (RealPolynomial{1.0, infinity}).Divmod(RealPolynomial{1.0, 1.0});
  });
  ExpectThrows<std::domain_error>([]()
  {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    (RealPolynomial{1.0}).Divmod(RealPolynomial{nan});
  });
  ExpectThrows<std::overflow_error>([]()
  {
    (RealPolynomial{std::numeric_limits<double>::max()}).Divmod(
      RealPolynomial{std::numeric_limits<double>::min()});
  });
  ExpectThrows<std::domain_error>([]()
  {
    (RealPolynomial{std::numeric_limits<double>::min()}).Divmod(
      RealPolynomial{std::numeric_limits<double>::max()});
  });
  ExpectThrows<std::overflow_error>([]()
  {
    const double maximum = std::numeric_limits<double>::max();
    (RealPolynomial{0.0, maximum}).Divmod(RealPolynomial{2.0, 1.0});
  });
}

void TestNuoPolynomial::TestGenericCoefficients()
{
  using Complex = nuostl::NuoComplex<double>;
  using ComplexPolynomial = nuostl::NuoPolynomial<Complex>;
  const Complex imaginary(0.0, 1.0);
  const ComplexPolynomial first{Complex(1.0), imaginary};
  const ComplexPolynomial second{Complex(1.0), -imaginary};
  assert(first(imaginary) == Complex());
  assert(first * second == (ComplexPolynomial{Complex(1.0), Complex(),
                                            Complex(1.0)}));
  assert(first.Derivative() == (ComplexPolynomial{imaginary}));
  const auto scaled = first * Complex(2.0);
  assert(scaled / Complex(2.0) == first);
  ExpectThrows<std::invalid_argument>([&]()
  {
    const auto value = first / Complex();
    (void)value;
  });
}

void TestNuoPolynomial::TestAlgebraicIdentities()
{
  std::mt19937 random(534202);
  std::uniform_int_distribution<int> coefficient(-3, 3);
  std::uniform_int_distribution<int> size(0, 5);
  for (int trial = 0; trial < 100; ++trial)
  {
    Polynomial first;
    Polynomial second;
    const int first_size = size(random);
    const int second_size = size(random);
    for (int index = 0; index < first_size; ++index)
    {
      first.SetCoefficient(index, coefficient(random));
    }
    for (int index = 0; index < second_size; ++index)
    {
      second.SetCoefficient(index, coefficient(random));
    }
    for (int x = -2; x <= 2; ++x)
    {
      /* Compare Horner evaluation against direct powers. */
      int sum = 0;
      int power = 1;
      for (std::size_t index = 0; index < first.Size(); ++index)
      {
        sum += first.Coefficient(index) * power;
        power *= x;
      }
      assert(first(x) == sum);
      assert((first + second)(x) == first(x) + second(x));
      assert((first * second)(x) == first(x) * second(x));
    }
    assert((first + second) - second == first);
    assert((first * second).Derivative() ==
           first.Derivative() * second + first * second.Derivative());

    /* A monic divisor guarantees exact integral long division. */
    second = Polynomial{coefficient(random), coefficient(random), 1};
    const auto result = first.Divmod(second);
    assert(result.first * second + result.second == first);
    assert(result.second.IsZero() || result.second.Degree() < second.Degree());
  }
}

void TestNuoPolynomial::TestOutput()
{
  std::ostringstream stream;
  stream << Polynomial{1, -2, 3};
  assert(stream.str() == "1 - 2*x + 3*x^2");

  std::ostringstream zero_stream;
  zero_stream << Polynomial();
  assert(zero_stream.str() == "0");

  std::ostringstream sparse_stream;
  sparse_stream << Polynomial{0, -1, 0, 2};
  assert(sparse_stream.str() == "-1*x + 2*x^3");

  const int minimum = std::numeric_limits<int>::min();
  std::ostringstream minimum_stream;
  minimum_stream << Polynomial{minimum, minimum};
  assert(minimum_stream.str() == std::to_string(minimum) + " - " +
         std::to_string(-(static_cast<long long>(minimum))) + "*x");
}

void TestNuoPolynomial::TestSuite()
{
  TestConstruction();
  TestEvaluation();
  TestArithmetic();
  TestCalculus();
  TestDivision();
  TestFloatingPoint();
  TestGenericCoefficients();
  TestAlgebraicIdentities();
  TestOutput();
}

} /* namespace test */
