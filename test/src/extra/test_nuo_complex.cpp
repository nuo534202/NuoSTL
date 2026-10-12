#include "extra/test_nuo_complex.hpp"

#include <cassert>

#include <cmath>
#include <complex>
#include <cstdio>
#include <limits>
#include <numbers>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "extra/nuo_complex.hpp"

namespace test
{

namespace
{

constexpr double kTolerance = 1e-12;

std::vector<double> ProbeValues()
{
  const double infinity = std::numeric_limits<double>::infinity();
  const double nan = std::numeric_limits<double>::quiet_NaN();
  return {-4.0, -3.5, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.25, 10.0,
          infinity, -infinity, nan};
}

bool ComponentMatches(double actual, double expected)
{
  if (std::isnan(expected))
  {
    return std::isnan(actual) != 0;
  }
  if (std::isinf(expected))
  {
    return actual == expected;
  }
  /* The sign of a zero result is a library convention rather than a
     mathematical value, so zeros compare equal; the conventions this
     implementation follows are pinned by explicit assertions elsewhere. */
  return std::abs(actual - expected) <= kTolerance * (1.0 + std::abs(expected));
}

void ExpectSame(const char* name, double real, double imag,
                nuostl::NuoComplex<double> actual,
                std::complex<double> expected)
{
  if (!ComponentMatches(actual.real(), expected.real()) ||
      !ComponentMatches(actual.imag(), expected.imag()))
  {
    std::fprintf(stderr,
                 "%s(%g,%g) got (%g,%g) want (%g,%g)\n", name, real, imag,
                 actual.real(), actual.imag(), expected.real(),
                 expected.imag());
    assert(false);
  }
}

/* Two branch points are excluded from the differential grid: the C library
   reports an artifact there, while this implementation follows the value
   from the standard. They are pinned by explicit assertions instead. */
bool IsLibraryArtifact(const char* name, double real, double imag)
{
  const std::string function(name);
  if (function == "atan")
  {
    return real == 0.0 && (imag == 1.0 || imag == -1.0);
  }
  if (function == "atanh")
  {
    return imag == 0.0 && (real == 1.0 || real == -1.0);
  }
  return false;
}

template <typename NuoFunction, typename StandardFunction>
void CheckUnary(const char* name, NuoFunction nuo_function,
                StandardFunction standard_function)
{
  for (double real : ProbeValues())
  {
    for (double imag : ProbeValues())
    {
      if (IsLibraryArtifact(name, real, imag))
      {
        continue;
      }
      ExpectSame(name, real, imag,
                 nuo_function(nuostl::NuoComplex<double>(real, imag)),
                 standard_function(std::complex<double>(real, imag)));
    }
  }
}

/* Two families of pow arguments are implementation defined: non finite
   operands and a zero base with a non positive real exponent, where the C
   library uses dedicated special values instead of exp(y * log(x)). They are
   left out of the differential grid and pinned by explicit assertions for the
   finite domain. */
bool IsPowArtifact(const char* name, double base_real, double base_imag,
                   double exponent_real, double exponent_imag)
{
  if (std::string(name) != "pow" && std::string(name) != "pow-scalar")
  {
    return false;
  }
  const bool finite_operands = std::isfinite(base_real) &&
    std::isfinite(base_imag) && std::isfinite(exponent_real) &&
    std::isfinite(exponent_imag);
  const bool zero_base = base_real == 0.0 && base_imag == 0.0;
  return !finite_operands || (zero_base && exponent_real <= 0.0);
}

/* Compares a function that returns a real value. */
template <typename NuoFunction, typename StandardFunction>
void CheckScalar(const char* name, NuoFunction nuo_function,
                 StandardFunction standard_function)
{
  for (double real : ProbeValues())
  {
    for (double imag : ProbeValues())
    {
      const nuostl::NuoComplex<double> value(real, imag);
      const std::complex<double> reference(real, imag);
      ExpectSame(name, real, imag,
                 nuostl::NuoComplex<double>(nuo_function(value), 0.0),
                 std::complex<double>(standard_function(reference), 0.0));
    }
  }
}

template <typename NuoFunction, typename StandardFunction>
void CheckBinary(const char* name, NuoFunction nuo_function,
                 StandardFunction standard_function)
{
  for (double left_real : ProbeValues())
  {
    for (double left_imag : ProbeValues())
    {
      for (double right_real : ProbeValues())
      {
        for (double right_imag : ProbeValues())
        {
          if (IsPowArtifact(name, left_real, left_imag, right_real,
                            right_imag))
          {
            continue;
          }
          const double real = left_real;
          const double imag = left_imag;
          ExpectSame(name, real, imag,
                     nuo_function(nuostl::NuoComplex<double>(left_real,
                                                             left_imag),
                                  nuostl::NuoComplex<double>(right_real,
                                                             right_imag)),
                     standard_function(std::complex<double>(left_real,
                                                            left_imag),
                                       std::complex<double>(right_real,
                                                            right_imag)));
        }
      }
    }
  }
}

} /* namespace */

void TestNuoComplex::TestConstructionAndAccess()
{
  using Complex = nuostl::NuoComplex<double>;
  static_assert(std::is_same_v<Complex::value_type, double>);

  const Complex zero;
  assert(zero.real() == 0.0 && zero.imag() == 0.0);

  const Complex real_only(2.5);
  assert(real_only.real() == 2.5 && real_only.imag() == 0.0);

  Complex value(1.5, -2.5);
  assert(value.real() == 1.5 && value.imag() == -2.5);
  value.real(3.0);
  value.imag(4.0);
  assert(value.real() == 3.0 && value.imag() == 4.0);

  const Complex copied(value);
  assert(copied == value);

  /* Converting constructor and mixed component arithmetic. */
  const nuostl::NuoComplex<float> small(1.0f, 2.0f);
  const Complex widened(small);
  assert(widened.real() == 1.0 && widened.imag() == 2.0);
  const Complex mixed = value + small;
  assert(mixed.real() == 4.0 && mixed.imag() == 6.0);
  static_assert(std::is_same_v<decltype(value + small),
                               nuostl::NuoComplex<double>>);

  /* Scalar assignment resets the imaginary part. */
  Complex assigned;
  assigned = 7.5;
  assert(assigned.real() == 7.5 && assigned.imag() == 0.0);

  constexpr Complex constexpr_value(1.0, 2.0);
  static_assert(constexpr_value.real() == 1.0);
  static_assert(constexpr_value.imag() == 2.0);
}

void TestNuoComplex::TestArithmetic()
{
  using Complex = nuostl::NuoComplex<double>;

  const Complex left(1.0, 2.0);
  const Complex right(3.0, -4.0);

  const Complex sum = left + right;
  assert(sum == Complex(4.0, -2.0));
  const Complex difference = left - right;
  assert(difference == Complex(-2.0, 6.0));
  const Complex product = left * right;
  assert(product == Complex(11.0, 2.0));
  const Complex quotient = left / right;
  assert(quotient == Complex(1.0 * 3.0 + 2.0 * -4.0, 2.0 * 3.0 - 1.0 * -4.0) /
         Complex(25.0, 0.0));

  assert(-left == Complex(-1.0, -2.0));
  assert(+left == left);

  Complex accumulated(left);
  accumulated += right;
  assert(accumulated == sum);
  accumulated -= right;
  assert(accumulated == left);
  accumulated *= right;
  assert(accumulated == product);
  accumulated /= right;
  assert(ComponentMatches(accumulated.real(), left.real()) &&
         ComponentMatches(accumulated.imag(), left.imag()));

  /* Self application must read both components before writing them. */
  Complex squared(1.0, 2.0);
  squared *= squared;
  assert(ComponentMatches(squared.real(), -3.0) &&
         ComponentMatches(squared.imag(), 4.0));

  Complex self_divided(1.0, 2.0);
  self_divided /= self_divided;
  assert(ComponentMatches(self_divided.real(), 1.0) &&
         ComponentMatches(self_divided.imag(), 0.0));

  /* Scalar operators in both orders. */
  assert(left + 1.0 == Complex(2.0, 2.0));
  assert(1.0 + left == Complex(2.0, 2.0));
  assert(left - 1.0 == Complex(0.0, 2.0));
  assert(1.0 - left == Complex(0.0, -2.0));
  assert(left * 2.0 == Complex(2.0, 4.0));
  assert(2.0 * left == Complex(2.0, 4.0));
  assert(left / 2.0 == Complex(0.5, 1.0));
  assert(2.0 / left == Complex(2.0) / left);
  assert(left == left && left != right);
  assert(left != 1.0 && left == Complex(1.0, 2.0));

  Complex scalar(5.0, 6.0);
  scalar += 1.0;
  scalar -= 2.0;
  scalar *= 3.0;
  scalar /= 6.0;
  assert(ComponentMatches(scalar.real(), 2.0) &&
         ComponentMatches(scalar.imag(), 3.0));
}

void TestNuoComplex::TestComparisonAndOutput()
{
  using Complex = nuostl::NuoComplex<double>;

  const Complex value(1.5, -2.5);
  assert(value == Complex(1.5, -2.5));
  assert(value != Complex(1.5, 2.5));
  assert(!(value == 1.5));
  assert(Complex(3.0, 0.0) == 3.0);
  assert(3.0 == Complex(3.0, 0.0));
  assert(Complex(3.0, 1.0) != 3.0);
  assert(3.0 != Complex(3.0, 1.0));

  std::ostringstream stream;
  stream << value;
  assert(stream.str() == "(1.5,-2.5)");

  std::ostringstream integer_stream;
  integer_stream << Complex(2.0, 0.0);
  assert(integer_stream.str() == "(2,0)");
}

void TestNuoComplex::TestStructureFunctions()
{
  CheckScalar("abs", [](const auto& z) { return nuostl::NuoAbs(z); },
              [](const auto& z) { return std::abs(z); });
  CheckScalar("arg", [](const auto& z) { return nuostl::NuoArg(z); },
              [](const auto& z) { return std::arg(z); });
  CheckScalar("norm", [](const auto& z) { return nuostl::NuoNorm(z); },
              [](const auto& z) { return std::norm(z); });
  CheckUnary("conj", [](const auto& z) { return nuostl::NuoConj(z); },
             [](const auto& z) { return std::conj(z); });
  CheckUnary("proj", [](const auto& z) { return nuostl::NuoProj(z); },
             [](const auto& z) { return std::proj(z); });

  const nuostl::NuoComplex<double> value(3.0, 4.0);
  assert(nuostl::NuoReal(value) == 3.0);
  assert(nuostl::NuoImag(value) == 4.0);
  assert(nuostl::NuoConj(value) == nuostl::NuoComplex<double>(3.0, -4.0));
  assert(nuostl::NuoNorm(value) == 25.0);
  assert(nuostl::NuoAbs(value) == 5.0);
  assert(ComponentMatches(nuostl::NuoArg(value), std::atan2(4.0, 3.0)));

  const nuostl::NuoComplex<double> from_polar =
    nuostl::NuoPolar(2.0, std::numbers::pi / 2.0);
  assert(std::abs(from_polar.real()) < 1e-15);
  assert(std::abs(from_polar.imag() - 2.0) < 1e-15);
  const nuostl::NuoComplex<double> default_angle = nuostl::NuoPolar(3.0);
  assert(default_angle == nuostl::NuoComplex<double>(3.0, 0.0));

  const double infinity = std::numeric_limits<double>::infinity();
  const nuostl::NuoComplex<double> projected = nuostl::NuoProj(
    nuostl::NuoComplex<double>(-infinity, 1.0));
  assert(projected.real() == infinity && projected.imag() == 0.0 &&
         std::signbit(projected.imag()) == false);
  const nuostl::NuoComplex<double> negative_zero = nuostl::NuoProj(
    nuostl::NuoComplex<double>(infinity, -1.0));
  assert(negative_zero.real() == infinity &&
         std::signbit(negative_zero.imag()) == true);
}

void TestNuoComplex::TestAnalyticFunctions()
{
  CheckUnary("exp", [](const auto& z) { return nuostl::NuoExp(z); },
             [](const auto& z) { return std::exp(z); });
  CheckUnary("log", [](const auto& z) { return nuostl::NuoLog(z); },
             [](const auto& z) { return std::log(z); });
  CheckUnary("log10", [](const auto& z) { return nuostl::NuoLog10(z); },
             [](const auto& z) { return std::log10(z); });
  CheckUnary("sqrt", [](const auto& z) { return nuostl::NuoSqrt(z); },
             [](const auto& z) { return std::sqrt(z); });
  CheckUnary("sin", [](const auto& z) { return nuostl::NuoSin(z); },
             [](const auto& z) { return std::sin(z); });
  CheckUnary("cos", [](const auto& z) { return nuostl::NuoCos(z); },
             [](const auto& z) { return std::cos(z); });
  CheckUnary("tan", [](const auto& z) { return nuostl::NuoTan(z); },
             [](const auto& z) { return std::tan(z); });
  CheckUnary("sinh", [](const auto& z) { return nuostl::NuoSinh(z); },
             [](const auto& z) { return std::sinh(z); });
  CheckUnary("cosh", [](const auto& z) { return nuostl::NuoCosh(z); },
             [](const auto& z) { return std::cosh(z); });
  CheckUnary("tanh", [](const auto& z) { return nuostl::NuoTanh(z); },
             [](const auto& z) { return std::tanh(z); });
  CheckUnary("asin", [](const auto& z) { return nuostl::NuoAsin(z); },
             [](const auto& z) { return std::asin(z); });
  CheckUnary("acos", [](const auto& z) { return nuostl::NuoAcos(z); },
             [](const auto& z) { return std::acos(z); });
  CheckUnary("atan", [](const auto& z) { return nuostl::NuoAtan(z); },
             [](const auto& z) { return std::atan(z); });
  CheckUnary("asinh", [](const auto& z) { return nuostl::NuoAsinh(z); },
             [](const auto& z) { return std::asinh(z); });
  CheckUnary("acosh", [](const auto& z) { return nuostl::NuoAcosh(z); },
             [](const auto& z) { return std::acosh(z); });
  CheckUnary("atanh", [](const auto& z) { return nuostl::NuoAtanh(z); },
             [](const auto& z) { return std::atanh(z); });

  CheckBinary("pow",
              [](const auto& base, const auto& exponent)
              {
                return nuostl::NuoPow(base, exponent);
              },
              [](const auto& base, const auto& exponent)
              {
                return std::pow(base, exponent);
              });

  /* Finite domain identities of the complex power. */
  const nuostl::NuoComplex<double> power_base(2.0, -1.0);
  assert(nuostl::NuoNearlyEqual(nuostl::NuoPow(power_base, 0.0),
                                nuostl::NuoComplex<double>(1.0, 0.0),
                                1e-12));
  assert(nuostl::NuoNearlyEqual(nuostl::NuoPow(power_base, 1.0), power_base,
                                1e-12));
  assert(nuostl::NuoNearlyEqual(
    nuostl::NuoPow(nuostl::NuoComplex<double>(2.0, 0.0), 10.0),
    nuostl::NuoComplex<double>(1024.0, 0.0), 1e-9));
  const nuostl::NuoComplex<double> minus_one(-1.0, 0.0);
  assert(nuostl::NuoNearlyEqual(nuostl::NuoPow(minus_one, 0.5),
                                nuostl::NuoComplex<double>(0.0, 1.0),
                                1e-12));

  /* The real exponent overload follows exp(exponent * log(base)); the base is
     kept finite because the library asserts on a NaN magnitude. */
  for (double base_real : ProbeValues())
  {
    for (double base_imag : ProbeValues())
    {
      if (!std::isfinite(base_real) || !std::isfinite(base_imag))
      {
        continue;
      }
      for (double exponent : {2.0, -1.0, 0.5})
      {
        if (IsPowArtifact("pow-scalar", base_real, base_imag, exponent, 0.0))
        {
          continue;
        }
        ExpectSame("pow-scalar", base_real, base_imag,
                   nuostl::NuoPow(nuostl::NuoComplex<double>(base_real,
                                                             base_imag),
                                  exponent),
                   std::pow(std::complex<double>(base_real, base_imag),
                            exponent));
      }
    }
  }
}

void TestNuoComplex::TestZeroSignConventions()
{
  /* csin(x + i0) keeps the signed zero of the argument and ccos(x + i0)
     always reports -i0, following the C library convention. */
  const nuostl::NuoComplex<double> sin_zero =
    nuostl::NuoSin(nuostl::NuoComplex<double>(2.0, 0.0));
  assert(sin_zero.imag() == 0.0 && !std::signbit(sin_zero.imag()));

  const nuostl::NuoComplex<double> sin_negative_zero =
    nuostl::NuoSin(nuostl::NuoComplex<double>(2.0, -0.0));
  assert(sin_negative_zero.imag() == 0.0 &&
         std::signbit(sin_negative_zero.imag()));

  const nuostl::NuoComplex<double> cos_zero =
    nuostl::NuoCos(nuostl::NuoComplex<double>(2.0, 0.0));
  assert(cos_zero.imag() == 0.0 && std::signbit(cos_zero.imag()));

  /* sqrt(-4 + i0) is +2i and sqrt(-4 - i0) is -2i. */
  const nuostl::NuoComplex<double> positive_root =
    nuostl::NuoSqrt(nuostl::NuoComplex<double>(-4.0, 0.0));
  assert(positive_root.real() == 0.0 && positive_root.imag() == 2.0);
  const nuostl::NuoComplex<double> negative_root =
    nuostl::NuoSqrt(nuostl::NuoComplex<double>(-4.0, -0.0));
  assert(negative_root.real() == 0.0 && negative_root.imag() == -2.0);

  /* Branch points follow the standard rather than the C library artifact:
     catan(+-i) is the infinite point of the imaginary axis, and catanh(+-1)
     is the real infinity. */
  const double infinity = std::numeric_limits<double>::infinity();
  const nuostl::NuoComplex<double> at_i = nuostl::NuoAtan(
    nuostl::NuoComplex<double>(0.0, 1.0));
  assert(at_i.real() == 0.0 && at_i.imag() == infinity);
  const nuostl::NuoComplex<double> at_minus_i = nuostl::NuoAtan(
    nuostl::NuoComplex<double>(0.0, -1.0));
  assert(at_minus_i.real() == 0.0 && at_minus_i.imag() == -infinity);
  const nuostl::NuoComplex<double> atanh_one = nuostl::NuoAtanh(
    nuostl::NuoComplex<double>(1.0, 0.0));
  assert(atanh_one.real() == infinity && atanh_one.imag() == 0.0);

  /* A zero magnitude keeps the sign of the zero imaginary argument. */
  const nuostl::NuoComplex<double> zero_log =
    nuostl::NuoLog(nuostl::NuoComplex<double>(0.0, -0.0));
  assert(std::isinf(zero_log.real()) && zero_log.real() < 0.0);
  assert(zero_log.imag() == 0.0 && std::signbit(zero_log.imag()));
}

void TestNuoComplex::TestExtensions()
{
  using Complex = nuostl::NuoComplex<double>;

  const Complex value(1.0, 2.0);
  assert(nuostl::NuoNearlyEqual(value, value, 0.0));
  assert(nuostl::NuoNearlyEqual(Complex(1.0, 2.0), Complex(1.0 + 1e-9, 2.0),
                                1e-6));
  assert(!nuostl::NuoNearlyEqual(Complex(1.0, 2.0), Complex(1.5, 2.0), 1e-6));
  assert(!nuostl::NuoNearlyEqual(
    Complex(std::numeric_limits<double>::quiet_NaN(), 0.0), value, 1e6));

  const Complex base(2.0, -1.0);
  assert(nuostl::NuoIntegerPow(base, 0) == Complex(1.0, 0.0));
  assert(nuostl::NuoIntegerPow(base, 1) == base);
  const Complex cubed = nuostl::NuoIntegerPow(base, 3);
  const Complex cubed_expected = base * base * base;
  assert(nuostl::NuoNearlyEqual(cubed, cubed_expected, 1e-12));
  const Complex inverted = nuostl::NuoIntegerPow(base, -2);
  const Complex inverted_expected = Complex(1.0) / (base * base);
  assert(nuostl::NuoNearlyEqual(inverted, inverted_expected, 1e-12));
  assert(nuostl::NuoIntegerPow(base, 2u) == base * base);
}

void TestNuoComplex::TestSuite()
{
  TestConstructionAndAccess();
  TestArithmetic();
  TestComparisonAndOutput();
  TestStructureFunctions();
  TestAnalyticFunctions();
  TestZeroSignConventions();
  TestExtensions();
}

} /* namespace test */
