#include "extra/test_nuo_big_integer.hpp"

#include <cassert>

#include <algorithm>
#include <climits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "extra/nuo_big_integer.hpp"

namespace test
{

namespace
{

/* Deterministic pseudo random generator for the differential checks. */
class Random
{
public:
  explicit Random(unsigned int seed) : state_(seed)
  {
  }

  unsigned int Next()
  {
    state_ = state_ * 1103515245u + 12345u;
    return state_ >> 16;
  }

  long long NextSigned(long long magnitude)
  {
    const unsigned int bound = static_cast<unsigned int>(magnitude + 1);
    const long long value = static_cast<long long>(Next() % bound);
    return Next() % 2 == 0 ? value : -value;
  }

private:
  unsigned int state_;
};

std::string Repeat(char digit, std::size_t count)
{
  return std::string(count, digit);
}

} /* namespace */

void TestNuoBigInteger::TestConstruction()
{
  const nuostl::NuoBigInteger zero;
  assert(zero.IsZero());
  assert(!zero.IsNegative());
  assert(zero.ToString() == "0");
  assert(zero.DigitCount() == 1);

  assert(nuostl::NuoBigInteger(0).ToString() == "0");
  assert(nuostl::NuoBigInteger(7).ToString() == "7");
  assert(nuostl::NuoBigInteger(-7).ToString() == "-7");
  assert(nuostl::NuoBigInteger(1000000000).ToString() == "1000000000");
  assert(nuostl::NuoBigInteger(-1000000001).ToString() == "-1000000001");

  assert(nuostl::NuoBigInteger(42u).ToString() == "42");
  assert(nuostl::NuoBigInteger(42l).ToString() == "42");
  assert(nuostl::NuoBigInteger(42ul).ToString() == "42");
  assert(nuostl::NuoBigInteger(42ll).ToString() == "42");
  assert(nuostl::NuoBigInteger(42ull).ToString() == "42");

  assert(nuostl::NuoBigInteger(INT_MIN).ToString() == std::to_string(INT_MIN));
  assert(nuostl::NuoBigInteger(INT_MAX).ToString() == std::to_string(INT_MAX));
  assert(nuostl::NuoBigInteger(LLONG_MIN).ToString() ==
         std::to_string(LLONG_MIN));
  assert(nuostl::NuoBigInteger(LLONG_MAX).ToString() ==
         std::to_string(LLONG_MAX));
  assert(nuostl::NuoBigInteger(ULLONG_MAX).ToString() ==
         std::to_string(ULLONG_MAX));
  assert(nuostl::NuoBigInteger(ULONG_MAX).ToString() ==
         std::to_string(ULONG_MAX));

  const nuostl::NuoBigInteger copied(zero);
  assert(copied.IsZero());
  const nuostl::NuoBigInteger moved(nuostl::NuoBigInteger(-5));
  assert(moved.ToString() == "-5");
}

void TestNuoBigInteger::TestStringParsing()
{
  assert(nuostl::NuoBigInteger(std::string("0")).ToString() == "0");
  assert(nuostl::NuoBigInteger(std::string("-0")).ToString() == "0");
  assert(nuostl::NuoBigInteger(std::string("+0")).ToString() == "0");
  assert(nuostl::NuoBigInteger(std::string("000123")).ToString() == "123");
  assert(nuostl::NuoBigInteger(std::string("-000123")).ToString() == "-123");
  assert(nuostl::NuoBigInteger(std::string("123456789")).ToString() ==
         "123456789");
  assert(nuostl::NuoBigInteger(std::string("1000000000")).ToString() ==
         "1000000000");
  const std::string twenty_digits = "98765432109876543210";
  assert(nuostl::NuoBigInteger(twenty_digits).ToString() == twenty_digits);
  assert(nuostl::NuoBigInteger("-" + twenty_digits).ToString() ==
         "-" + twenty_digits);
  assert(nuostl::NuoBigInteger(std::string("+42")).ToString() == "42");
  assert(nuostl::NuoBigInteger("31415926535897932384626433832795").ToString() ==
         "31415926535897932384626433832795");

  const std::vector<std::string> invalid = {
    "", "+", "-", " 12", "12 ", "1a", "a1", "--1", "++1", "-+1", "+-1", "1.5",
    "0x10", "١٢٣"};
  for (const std::string& text : invalid)
  {
    bool caught = false;
    try
    {
      nuostl::NuoBigInteger value(text);
      (void)value;
    }
    catch (const std::invalid_argument&)
    {
      caught = true;
    }
    assert(caught);
  }

  /* Parsing round trips through ToString for long inputs. */
  Random random(2024);
  for (int trial = 0; trial < 32; ++trial)
  {
    std::string text;
    const int length = 1 + static_cast<int>(random.Next() % 200);
    if (random.Next() % 2 == 0)
    {
      text.push_back('-');
    }
    text.push_back(static_cast<char>('1' + random.Next() % 9));
    for (int index = 1; index < length; ++index)
    {
      text.push_back(static_cast<char>('0' + random.Next() % 10));
    }
    const nuostl::NuoBigInteger value(text);
    assert(value.ToString() == text);
  }
}

void TestNuoBigInteger::TestConversion()
{
  assert(nuostl::NuoBigInteger(0).ToLongLong() == 0);
  assert(nuostl::NuoBigInteger(-1).ToLongLong() == -1);
  assert(nuostl::NuoBigInteger(LLONG_MAX).ToLongLong() == LLONG_MAX);
  assert(nuostl::NuoBigInteger(LLONG_MIN).ToLongLong() == LLONG_MIN);

  bool caught = false;
  try
  {
    nuostl::NuoBigInteger(std::string("9223372036854775808")).ToLongLong();
  }
  catch (const std::overflow_error&)
  {
    caught = true;
  }
  assert(caught);

  caught = false;
  try
  {
    nuostl::NuoBigInteger(std::string("-9223372036854775809")).ToLongLong();
  }
  catch (const std::overflow_error&)
  {
    caught = true;
  }
  assert(caught);

  assert(nuostl::NuoBigInteger(9).DigitCount() == 1);
  assert(nuostl::NuoBigInteger(10).DigitCount() == 2);
  assert(nuostl::NuoBigInteger(-100).DigitCount() == 3);
  assert(nuostl::NuoBigInteger(std::string(Repeat('9', 100))).DigitCount() ==
         100);
  assert(nuostl::NuoBigInteger(std::string("1" + Repeat('0', 100)))
           .DigitCount() == 101);
}

void TestNuoBigInteger::TestComparison()
{
  const nuostl::NuoBigInteger zero;
  const nuostl::NuoBigInteger small(10);
  const nuostl::NuoBigInteger big(1000000000000LL);
  const nuostl::NuoBigInteger negative(-10);

  assert(zero == nuostl::NuoBigInteger(0));
  assert(zero != small);
  assert(negative < zero);
  assert(negative < small);
  assert(small < big);
  assert(big > small);
  assert(small <= small);
  assert(small >= small);
  assert(zero == nuostl::NuoBigInteger(std::string("+0")));
  assert(nuostl::NuoBigInteger(std::string("-0")) == zero);

  Random random(7);
  for (int trial = 0; trial < 512; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    const long long right = random.NextSigned(1000000000);
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);
    assert((big_left < big_right) == (left < right));
    assert((big_left <= big_right) == (left <= right));
    assert((big_left > big_right) == (left > right));
    assert((big_left >= big_right) == (left >= right));
    assert((big_left == big_right) == (left == right));
    assert((big_left != big_right) == (left != right));
  }

  /* Sorting the big values must match sorting the built in values. */
  std::vector<long long> natives;
  std::vector<nuostl::NuoBigInteger> values;
  for (int trial = 0; trial < 64; ++trial)
  {
    const long long value = random.NextSigned(1000000000000LL);
    natives.push_back(value);
    values.push_back(nuostl::NuoBigInteger(value));
  }
  std::sort(natives.begin(), natives.end());
  std::sort(values.begin(), values.end());
  assert(values.size() == natives.size());
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    assert(values[index].ToString() == std::to_string(natives[index]));
  }
}

void TestNuoBigInteger::TestAdditionAndSubtraction()
{
  Random random(11);
  for (int trial = 0; trial < 1024; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    const long long right = random.NextSigned(1000000000);
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);

    assert((big_left + big_right).ToString() == std::to_string(left + right));
    assert((big_left - big_right).ToString() == std::to_string(left - right));
    assert((-big_left).ToString() == std::to_string(-left));

    nuostl::NuoBigInteger accumulated(left);
    accumulated += big_right;
    assert(accumulated.ToString() == std::to_string(left + right));
    accumulated -= big_right;
    assert(accumulated.ToString() == std::to_string(left));

    /* Borrowing across many base digits. */
    const nuostl::NuoBigInteger power(std::string("1" + Repeat('0', 40)));
    assert((power - nuostl::NuoBigInteger(1)).ToString() ==
           Repeat('9', 40));
    assert((power + nuostl::NuoBigInteger(1)).ToString() ==
           "1" + Repeat('0', 39) + "1");
    assert((nuostl::NuoBigInteger(1) - power).ToString() ==
           "-" + Repeat('9', 40));
  }

  const nuostl::NuoBigInteger zero;
  assert((zero - zero).ToString() == "0");
  assert((zero + zero).IsZero());
  assert((zero - nuostl::NuoBigInteger(5)).ToString() == "-5");
}

void TestNuoBigInteger::TestMultiplication()
{
  Random random(13);
  for (int trial = 0; trial < 1024; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    const long long right = random.NextSigned(1000000000);
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);

    assert((big_left * big_right).ToString() == std::to_string(left * right));

    nuostl::NuoBigInteger accumulated(left);
    accumulated *= big_right;
    assert(accumulated.ToString() == std::to_string(left * right));
  }

  assert((nuostl::NuoBigInteger(0) * nuostl::NuoBigInteger(0)).IsZero());
  assert((nuostl::NuoBigInteger(-1) * nuostl::NuoBigInteger(0)).IsZero());
  assert((nuostl::NuoBigInteger(-2) * nuostl::NuoBigInteger(-3)).ToString() ==
         "6");
  assert((nuostl::NuoBigInteger(-2) * nuostl::NuoBigInteger(3)).ToString() ==
         "-6");

  /* (10^50 - 1)^2 has a well known decimal expansion. */
  const std::string nines = Repeat('9', 50);
  const nuostl::NuoBigInteger all_nines(nines);
  const nuostl::NuoBigInteger squared = all_nines * all_nines;
  const std::string expected = Repeat('9', 49) + "8" + Repeat('0', 49) + "1";
  assert(squared.ToString() == expected);
}

void TestNuoBigInteger::TestDivisionAndRemainder()
{
  Random random(17);
  for (int trial = 0; trial < 1024; ++trial)
  {
    const long long left = random.NextSigned(1000000000);
    long long right = random.NextSigned(1000000000);
    if (right == 0)
    {
      right = 1;
    }
    const nuostl::NuoBigInteger big_left(left);
    const nuostl::NuoBigInteger big_right(right);

    assert((big_left / big_right).ToString() == std::to_string(left / right));
    assert((big_left % big_right).ToString() == std::to_string(left % right));

    nuostl::NuoBigInteger quotient(left);
    quotient /= big_right;
    assert(quotient.ToString() == std::to_string(left / right));
    nuostl::NuoBigInteger remainder(left);
    remainder %= big_right;
    assert(remainder.ToString() == std::to_string(left % right));

    /* Division identity and the remainder magnitude bound. */
    const nuostl::NuoBigInteger rebuilt = quotient * big_right + remainder;
    assert(rebuilt == big_left);
    if (!remainder.IsZero())
    {
      assert(remainder.Abs() < big_right.Abs());
    }
  }

  bool caught = false;
  try
  {
    nuostl::NuoBigInteger(1) / nuostl::NuoBigInteger(0);
  }
  catch (const std::domain_error&)
  {
    caught = true;
  }
  assert(caught);

  caught = false;
  try
  {
    nuostl::NuoBigInteger(1) % nuostl::NuoBigInteger(0);
  }
  catch (const std::domain_error&)
  {
    caught = true;
  }
  assert(caught);

  /* Long division across many base digits. */
  const std::string long_dividend =
    "123456789012345678901234567890123456789012345678901234567890";
  const nuostl::NuoBigInteger dividend(long_dividend);
  const nuostl::NuoBigInteger divisor(std::string("98765432109876543210"));
  const nuostl::NuoBigInteger quotient = dividend / divisor;
  const nuostl::NuoBigInteger remainder = dividend % divisor;
  assert(quotient.ToString() == "1249999988609375000154882812384313964845");
  assert(remainder.ToString() == "31711154403171115440");
  assert(quotient * divisor + remainder == dividend);
  assert(remainder < divisor);

  assert((nuostl::NuoBigInteger(5) / nuostl::NuoBigInteger(7)).IsZero());
  assert((nuostl::NuoBigInteger(5) % nuostl::NuoBigInteger(7)).ToString() ==
         "5");
  assert((nuostl::NuoBigInteger(-7) / nuostl::NuoBigInteger(2)).ToString() ==
         "-3");
  assert((nuostl::NuoBigInteger(-7) % nuostl::NuoBigInteger(2)).ToString() ==
         "-1");
  assert((nuostl::NuoBigInteger(7) / nuostl::NuoBigInteger(-2)).ToString() ==
         "-3");
  assert((nuostl::NuoBigInteger(7) % nuostl::NuoBigInteger(-2)).ToString() ==
         "1");
}

void TestNuoBigInteger::TestIncrementAndSign()
{
  nuostl::NuoBigInteger value(9);
  assert((++value).ToString() == "10");
  assert((value++).ToString() == "10");
  assert(value.ToString() == "11");
  assert((--value).ToString() == "10");
  assert((value--).ToString() == "10");
  assert(value.ToString() == "9");

  nuostl::NuoBigInteger zero;
  --zero;
  assert(zero.ToString() == "-1");
  ++zero;
  assert(zero.IsZero());

  assert(nuostl::NuoBigInteger(-15).Abs().ToString() == "15");
  assert(nuostl::NuoBigInteger(15).Abs().ToString() == "15");
  assert(nuostl::NuoBigInteger(0).Abs().IsZero());
  assert(nuostl::NuoBigInteger(-15).IsNegative());
  assert(!nuostl::NuoBigInteger(-15).Abs().IsNegative());

  assert(nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(0), 0).ToString() ==
         "1");
  assert(nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(0), 5).ToString() ==
         "0");
  assert(nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(2), 100).ToString() ==
         "1267650600228229401496703205376");
  assert(nuostl::NuoBigInteger::Pow(nuostl::NuoBigInteger(-2), 3).ToString() ==
         "-8");

  assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(12),
                                    nuostl::NuoBigInteger(18)).ToString() ==
         "6");
  assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(-12),
                                    nuostl::NuoBigInteger(18)).ToString() ==
         "6");
  assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(0),
                                    nuostl::NuoBigInteger(5)).ToString() ==
         "5");
  assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(0),
                                    nuostl::NuoBigInteger(0)).IsZero());

  Random random(23);
  for (int trial = 0; trial < 256; ++trial)
  {
    const long long left = random.NextSigned(100000);
    long long right = random.NextSigned(100000);
    if (left == 0 && right == 0)
    {
      right = 1;
    }
    long long expected = left < 0 ? -left : left;
    long long candidate = right < 0 ? -right : right;
    while (candidate != 0)
    {
      const long long next = expected % candidate;
      expected = candidate;
      candidate = next;
    }
    assert(nuostl::NuoBigInteger::Gcd(nuostl::NuoBigInteger(left),
                                      nuostl::NuoBigInteger(right))
             .ToString() == std::to_string(expected));
  }
}

void TestNuoBigInteger::TestLargeValues()
{
  /* 100! computed by repeated multiplication and compared with the exact
     decimal expansion. */
  nuostl::NuoBigInteger factorial(1);
  for (int factor = 2; factor <= 100; ++factor)
  {
    factorial *= nuostl::NuoBigInteger(factor);
  }
  assert(factorial.ToString() ==
         "93326215443944152681699238856266700490715968264381621468592963895217"
         "59999322991560894146397615651828625369792082722375825118521091686400"
         "0000000000000000000000");

  /* Division must invert multiplication for large operands. */
  Random random(29);
  for (int trial = 0; trial < 16; ++trial)
  {
    std::string left_text("1");
    std::string right_text("1");
    for (int index = 0; index < 120; ++index)
    {
      left_text.push_back(static_cast<char>('0' + random.Next() % 10));
      right_text.push_back(static_cast<char>('0' + random.Next() % 10));
    }
    const nuostl::NuoBigInteger left(left_text);
    const nuostl::NuoBigInteger right(right_text);

    const nuostl::NuoBigInteger product = left * right;
    assert(product / left == right);
    assert(product / right == left);
    assert((product % left).IsZero());
    assert((product % right).IsZero());

    const nuostl::NuoBigInteger quotient = left / right;
    const nuostl::NuoBigInteger remainder = left % right;
    assert(quotient * right + remainder == left);
    if (!remainder.IsZero())
    {
      assert(remainder < right);
    }
  }
}

void TestNuoBigInteger::TestStreamOutput()
{
  const nuostl::NuoBigInteger value(std::string("-12345678901234567890123456"));
  std::ostringstream stream;
  stream << value;
  assert(stream.str() == value.ToString());
  assert(stream.str() == "-12345678901234567890123456");

  std::ostringstream zero_stream;
  zero_stream << nuostl::NuoBigInteger();
  assert(zero_stream.str() == "0");
}

void TestNuoBigInteger::TestSuite()
{
  TestConstruction();
  TestStringParsing();
  TestConversion();
  TestComparison();
  TestAdditionAndSubtraction();
  TestMultiplication();
  TestDivisionAndRemainder();
  TestIncrementAndSign();
  TestLargeValues();
  TestStreamOutput();
}

} /* namespace test */
