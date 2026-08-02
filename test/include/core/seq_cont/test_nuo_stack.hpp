#pragma once

namespace test
{

class TestNuoStack
{
private:
  static void test_constructor();
  static void test_assign();
  static void test_capacity_and_access();
  static void test_push_pop();
  static void test_emplace();
  static void test_comparison_and_swap();

public:
  static void test_nuo_stack();
};

} /* namespace test */