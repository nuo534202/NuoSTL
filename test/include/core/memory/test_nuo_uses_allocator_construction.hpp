#pragma once

namespace test
{

class TestNuoUsesAllocatorConstruction
{
public:
  static void TestSuite();

private:
  static void TestGeneralConstruction();
  static void TestTupleShapes();
  static void TestPairConstruction();
  static void TestUninitializedConstruction();
};

} /* namespace test */
