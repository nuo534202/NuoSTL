#pragma once

namespace test
{

class TestNuoArray
{
private:
  static void test_constructor();
  static void test_assign();
  static void test_capacity_and_access();
  static void test_iterators();
  static void test_swap();
  static void test_comparison();

public:
  static void test_nuo_array();
};

} /* namespace test */