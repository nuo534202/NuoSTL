#pragma once

namespace test
{

class TestNuoBitset
{
public:
  static void TestSuite();

private:
  static void TestConstruction();
  static void TestElementAccess();
  static void TestObservers();
  static void TestModifiers();
  static void TestBitwiseOperators();
  static void TestShiftOperators();
  static void TestConversions();
  static void TestStreams();
  static void TestHash();
  static void TestConstexprOperations();
};

} /* namespace test */
