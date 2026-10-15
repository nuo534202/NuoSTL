#pragma once

namespace test
{

class TestNuoBit
{
public:
  static void TestSuite();

private:
  static void TestEndian();
  static void TestBitCast();
  static void TestByteswap();
  static void TestStandardUnsignedFunctions();
  static void TestExtendedWidthFunctions();
  static void TestConstraints();
};

} /* namespace test */
