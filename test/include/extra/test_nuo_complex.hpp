#pragma once

namespace test
{

class TestNuoComplex
{
public:
  static void TestSuite();

private:
  static void TestConstructionAndAccess();
  static void TestArithmetic();
  static void TestComparisonAndOutput();
  static void TestStructureFunctions();
  static void TestAnalyticFunctions();
  static void TestZeroSignConventions();
  static void TestExtensions();
};

} /* namespace test */
