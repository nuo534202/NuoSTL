#pragma once

namespace test
{

class TestNuoFunction
{
public:
  static void TestSuite();

private:
  static void TestDefaultState();
  static void TestFunctionPointers();
  static void TestCallables();
  static void TestCopyAndMove();
  static void TestAssignment();
  static void TestSwap();
  static void TestTargets();
  static void TestArgumentsAndReturns();
  static void TestTraits();
  static void TestComparisonsWithNull();
};

} /* namespace test */
