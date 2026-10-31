#pragma once

namespace test
{

class TestNuoList
{
private:
  static void test_constructor();
  static void test_assign();
  static void test_iterator_and_access();
  static void test_iterator_conversion();
  static void test_emplace_positions();
  static void test_insert_overloads();
  static void test_range_construction_and_assign();
  static void test_push_pop();
  static void test_insert_erase();
  static void test_splice_and_move();
  static void test_clear_and_swap();
  static void TestResourceLifetime();

public:
  static void test_nuo_list();
};

} /* namespace test */
