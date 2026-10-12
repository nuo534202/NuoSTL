#pragma once

namespace test
{

class TestNuoBigInteger
{
public:
  static void TestSuite();

private:
  static void TestConstruction();
  static void TestStringParsing();
  static void TestConversion();
  static void TestComparison();
  static void TestAdditionAndSubtraction();
  static void TestMultiplication();
  static void TestDivisionAndRemainder();
  static void TestIncrementAndSign();
  static void TestLargeValues();
  static void TestStreamOutput();
};

} /* namespace test */
