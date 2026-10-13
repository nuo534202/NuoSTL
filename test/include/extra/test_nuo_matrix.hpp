#pragma once

namespace test
{

class TestNuoMatrix
{
public:
  static void TestSuite();

private:
  static void TestConstruction();
  static void TestElementAccess();
  static void TestArithmetic();
  static void TestStructure();
  static void TestDeterminant();
  static void TestInverse();
  static void TestPower();
  static void TestGenericElementType();
  static void TestOutput();
};

} /* namespace test */
