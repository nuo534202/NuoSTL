#pragma once

namespace test
{

class TestNuoPolynomial
{
public:
  static void TestSuite();

private:
  static void TestConstruction();
  static void TestEvaluation();
  static void TestArithmetic();
  static void TestCalculus();
  static void TestDivision();
  static void TestFloatingPoint();
  static void TestGenericCoefficients();
  static void TestAlgebraicIdentities();
  static void TestOutput();
};

} /* namespace test */
