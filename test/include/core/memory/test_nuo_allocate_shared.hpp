#pragma once

namespace test
{

class TestNuoAllocateShared
{
public:
  static void TestSuite();

private:
  static void TestBasicAllocation();
  static void TestWeakLifetime();
  static void TestThrowingConstruction();
  static void TestEnableSharedFromThis();
  static void TestOtherAllocators();
  static void TestStdParity();
};

} /* namespace test */
