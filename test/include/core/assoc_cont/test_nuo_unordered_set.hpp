#pragma once

namespace test
{
class TestNuoUnorderedSet
{
private:
  static void test_constructor_and_assignment();
  static void test_iterators_and_capacity();
  static void test_insert_and_emplace();
  static void test_erase_and_clear();
  static void test_lookup();
  static void test_bucket_and_hash_policy();
  static void test_swap_and_merge();
  static void test_custom_hash_and_move_only();

public:
  static void test_nuo_unordered_set();
};

class TestNuoUnorderedMultiset
{
private:
  static void test_constructor_and_assignment();
  static void test_iterators_and_capacity();
  static void test_insert_and_emplace();
  static void test_erase_and_clear();
  static void test_lookup();
  static void test_bucket_and_hash_policy();
  static void test_swap_and_merge();
  static void test_custom_hash_and_move_only();

public:
  static void test_nuo_unordered_multiset();
};
} /* namespace test */