#pragma once

namespace test
{

class TestNuoSharedPtr
{
public:
  static void TestSuite();

private:
  static void TestConstructionAndCounting();
  static void TestMoveSemantics();
  static void TestCustomDeleter();
  static void TestMakeShared();
  static void TestAliasing();
  static void TestUniquePtrConversion();
  static void TestArrays();
  static void TestWeakPtr();
  static void TestEnableSharedFromThis();
  static void TestCyclicReferences();
  static void TestCasts();
  static void TestComparisonsAndHash();
  static void TestOutput();
};

} /* namespace test */
