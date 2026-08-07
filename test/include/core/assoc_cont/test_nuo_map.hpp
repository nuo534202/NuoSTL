#pragma once

namespace test
{

class TestNuoMap
{
private:
  static void test_constructor_and_assignment();
  static void test_iterators_and_capacity();
  static void test_element_access();
  static void test_insert_and_emplace();
  static void test_erase();
  static void test_lookup_and_observers();
  static void test_swap_and_comparison();
  static void test_merge();

public:
  static void test_nuo_map();
};

class TestNuoMultimap
{
private:
  static void test_constructor_and_assignment();
  static void test_iterators_and_capacity();
  static void test_insert_and_emplace();
  static void test_erase();
  static void test_lookup_and_observers();
  static void test_swap_and_comparison();
  static void test_merge();

public:
  static void test_nuo_multimap();
};

} /* namespace test */
