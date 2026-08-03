#pragma once

namespace test
{

class TestNuoForwardList
{
private:
  static void test_constructor();
  static void test_assign();
  static void test_iterator_and_access();
  static void test_push_front();
  static void test_insert_after();
  static void test_erase_after();
  static void test_clear_and_swap();
  static void test_splice();
  static void test_remove_and_unique();
  static void test_merge_and_sort_reverse();

public:
  static void test_nuo_forward_list();
};

} /* namespace test */