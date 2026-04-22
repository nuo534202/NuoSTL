#pragma once

namespace test
{

class TestNuoList
{
private:
  static void test_constructor();
  static void test_assign();
  static void test_iterator_and_access();
  static void test_push_pop();
  static void test_insert_erase();
  static void test_clear_and_swap();

public:
  static void test_nuo_list();
};

} /* namespace test */