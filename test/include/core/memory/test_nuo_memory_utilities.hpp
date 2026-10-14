#pragma once

namespace test
{

class TestNuoMemoryUtilities
{
public:
  static void TestSuite();

private:
  static void TestAlign();
  static void TestAlignVal();
  static void TestLaunder();
  static void TestAssumeAligned();
  static void TestUsesAllocator();
  static void TestAllocatorArg();
};

} /* namespace test */
