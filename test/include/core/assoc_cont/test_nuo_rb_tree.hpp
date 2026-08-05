#ifndef NUOSTL_TEST_CORE_ASSOC_CONT_TEST_NUO_RB_TREE_HPP_
#define NUOSTL_TEST_CORE_ASSOC_CONT_TEST_NUO_RB_TREE_HPP_

namespace test
{

class TestNuoRbTree
{
private:
  static void test_value_traits_and_node_algorithms();
  static void test_constructor_assignment_and_swap();
  static void test_unique_insertion();
  static void test_multi_insertion();
  static void test_iterators();
  static void test_lookup_and_bounds();
  static void test_erase();
  static void test_pair_values_and_comparison();
  static void test_rebalance_stress();

public:
  static void test_nuo_rb_tree();
};

} /* namespace test */

#endif
