#pragma once

namespace test
{

class TestNuoMemory
{
public:
  static void TestSuite();

private:
  static void TestAddressof();
  static void TestConstructAtAndDestroyAt();
  static void TestDestroyN();
  static void TestUninitializedCopy();
  static void TestUninitializedCopyN();
  static void TestUninitializedFill();
  static void TestUninitializedFillN();
  static void TestUninitializedMove();
  static void TestUninitializedMoveN();
  static void TestUninitializedDefaultConstruct();
  static void TestUninitializedValueConstruct();
  static void TestExceptionSafety();
  static void TestConstexprAddressof();
};

} /* namespace test */
