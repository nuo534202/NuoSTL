#pragma once

namespace test
{

class TestNuoSourceLocation
{
public:
  static void TestSuite();

private:
  static void TestDefaultConstruction();
  static void TestCurrent();
  static void TestDefaultArgumentCapture();
  static void TestConstexprCapture();
};

} /* namespace test */
