#pragma once

namespace test
{

class TestNuoSpan
{
public:
  static void TestSuite();

private:
  static void TestConstruction();
  static void TestObservers();
  static void TestSubviews();
  static void TestMutation();
  static void TestDeductionGuides();
  static void TestConstexprOperations();
  static void TestStdParity();
};

} /* namespace test */
