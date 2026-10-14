#pragma once

namespace test
{

class TestNuoUniquePtr
{
public:
  static void TestSuite();

private:
  static void TestDefaultDelete();
  static void TestConstructionAndAccess();
  static void TestMoveSemantics();
  static void TestResetAndRelease();
  static void TestCustomDeleter();
  static void TestPolymorphism();
  static void TestArrays();
  static void TestMakeUnique();
  static void TestComparisons();
  static void TestHashAndLayout();
  static void TestOutput();
};

} /* namespace test */
