#pragma once

namespace test
{

class TestNuoAllocatorTraits
{
public:
  static void TestSuite();

private:
  static void TestPointerTraits();
  static void TestToAddress();
  static void TestAllocatorTraitsTypes();
  static void TestAllocatorTraitsOperations();
  static void TestOwnerLess();
};

} /* namespace test */
