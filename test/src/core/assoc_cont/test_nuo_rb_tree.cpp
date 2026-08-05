#include "core/assoc_cont/test_nuo_rb_tree.hpp"

#include <assert.h>

#include <string>
#include <type_traits>

#include "core/assoc_cont/nuo_rb_tree.hpp"

namespace test
{
namespace
{

struct ReverseInt
{
  bool operator()(int lhs, int rhs) const
  {
    return lhs > rhs;
  }
};

template <typename Tree>
int ValidateSubtree(typename Tree::base_ptr node,
                    typename Tree::base_ptr parent,
                    const typename Tree::key_compare& comp,
                    typename Tree::size_type& node_count)
{
  if (node == nullptr)
    return 1;

  assert(node->parent_ == parent);
  ++node_count;

  using Traits = typename Tree::value_traits;
  const auto& key = Traits::GetKey(node->ToNode()->value_);

  if (node->left_ != nullptr)
  {
    const auto& left_key = Traits::GetKey(node->left_->ToNode()->value_);
    assert(!comp(key, left_key));
  }
  if (node->right_ != nullptr)
  {
    const auto& right_key = Traits::GetKey(node->right_->ToNode()->value_);
    assert(!comp(right_key, key));
  }

  if (nuostl::NuoRbTreeIsRed(node))
  {
    assert(node->left_ == nullptr || !nuostl::NuoRbTreeIsRed(node->left_));
    assert(node->right_ == nullptr || !nuostl::NuoRbTreeIsRed(node->right_));
  }

  const int left_black_height = ValidateSubtree<Tree>(node->left_, node, comp, node_count);
  const int right_black_height = ValidateSubtree<Tree>(node->right_, node, comp, node_count);
  assert(left_black_height == right_black_height);
  return left_black_height + (nuostl::NuoRbTreeIsRed(node) ? 0 : 1);
}

template <typename Tree>
void ValidateTree(const Tree& tree)
{
  using Traits = typename Tree::value_traits;
  typename Tree::base_ptr header = tree.End().node_;
  assert(header != nullptr);
  assert(nuostl::NuoRbTreeIsRed(header));

  if (tree.Empty())
  {
    assert(tree.Size() == 0);
    assert(header->parent_ == nullptr);
    assert(header->left_ == header);
    assert(header->right_ == header);
    assert(tree.Begin() == tree.End());
    return;
  }

  typename Tree::base_ptr root = header->parent_;
  assert(root != nullptr);
  assert(root->parent_ == header);
  assert(!nuostl::NuoRbTreeIsRed(root));
  assert(header->left_ == nuostl::NuoRbTreeMin(root));
  assert(header->right_ == nuostl::NuoRbTreeMax(root));
  assert(tree.Begin().node_ == header->left_);

  auto last = tree.End();
  --last;
  assert(last.node_ == header->right_);

  const auto comp = tree.KeyComp();
  typename Tree::size_type node_count = 0;
  const int black_height = ValidateSubtree<Tree>(root, header, comp, node_count);
  assert(black_height > 1);
  assert(node_count == tree.Size());

  auto previous = tree.Begin();
  auto current = previous;
  ++current;
  while (current != tree.End())
  {
    assert(!comp(Traits::GetKey(*current), Traits::GetKey(*previous)));
    previous = current;
    ++current;
  }

  typename Tree::size_type reverse_count = 0;
  for (auto iterator = tree.RBegin(); iterator != tree.REnd(); ++iterator)
    ++reverse_count;
  assert(reverse_count == tree.Size());
}

template <typename Tree>
void InsertUniqueAndValidate(Tree& tree, int value)
{
  auto result = tree.InsertUnique(value);
  assert(result.second);
  assert(*result.first == value);
  ValidateTree(tree);
}

} /* namespace */

void TestNuoRbTree::test_value_traits_and_node_algorithms()
{
  using Pair = nuostl::nuo_pair<const int, std::string>;
  static_assert(!nuostl::NuoRbTreeValueTraits<int>::kIsPair);
  static_assert(nuostl::NuoRbTreeValueTraits<Pair>::kIsPair);
  static_assert(std::is_same_v<nuostl::NuoRbTreeValueTraits<int>::key_type, int>);
  static_assert(
    std::is_same_v<nuostl::NuoRbTreeValueTraits<Pair>::key_type, const int>);

  Pair pair(7, "seven");
  assert(nuostl::NuoRbTreeValueTraits<Pair>::GetKey(pair) == 7);

  using Node = nuostl::NuoRbTreeNode<int>;
  using Base = nuostl::NuoRbTreeNodeBase<int>;
  Base header{};
  Node a{};
  Node x{};
  Node b{};
  Node y{};
  a.value_ = 1;
  x.value_ = 2;
  b.value_ = 3;
  y.value_ = 4;

  Base* root = x.ToBase();
  header.parent_ = root;
  x.parent_ = &header;
  x.left_ = a.ToBase();
  x.right_ = y.ToBase();
  a.parent_ = x.ToBase();
  y.parent_ = x.ToBase();
  y.left_ = b.ToBase();
  b.parent_ = y.ToBase();

  assert(nuostl::NuoRbTreeMin(root) == a.ToBase());
  assert(nuostl::NuoRbTreeMax(root) == y.ToBase());
  assert(nuostl::NuoRbTreeIsLChild(a.ToBase()));
  assert(nuostl::NuoRbTreeNext(a.ToBase()) == x.ToBase());
  assert(nuostl::NuoRbTreeNext(x.ToBase()) == b.ToBase());

  nuostl::NuoRbTreeSetRed(x.ToBase());
  assert(nuostl::NuoRbTreeIsRed(x.ToBase()));
  nuostl::NuoRbTreeSetBlack(x.ToBase());
  assert(!nuostl::NuoRbTreeIsRed(x.ToBase()));

  nuostl::NuoRbTreeRotateLeft(x.ToBase(), root);
  assert(root == y.ToBase());
  assert(y.left_ == x.ToBase());
  assert(x.parent_ == y.ToBase());
  assert(x.right_ == b.ToBase());
  assert(b.parent_ == x.ToBase());

  nuostl::NuoRbTreeRotateRight(y.ToBase(), root);
  assert(root == x.ToBase());
  assert(x.right_ == y.ToBase());
  assert(y.parent_ == x.ToBase());
  assert(y.left_ == b.ToBase());
  assert(b.parent_ == y.ToBase());
}

void TestNuoRbTree::test_constructor_assignment_and_swap()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  ValidateTree(tree);
  assert(tree.GetAllocator().Allocate(0) == nullptr);
  assert(tree.MaxSize() > 0);

  int values[] = {4, 2, 6, 1, 3, 5, 7};
  tree.InsertUnique(values, values + 7);
  ValidateTree(tree);

  Tree copied(tree);
  assert(copied == tree);
  ValidateTree(copied);

  Tree moved(nuostl::NuoMove(copied));
  assert(moved == tree);
  assert(copied.Empty());
  ValidateTree(moved);

  Tree copy_assigned;
  copy_assigned = tree;
  assert(copy_assigned == tree);
  ValidateTree(copy_assigned);

  Tree move_assigned;
  move_assigned.InsertUnique(99);
  move_assigned = nuostl::NuoMove(copy_assigned);
  assert(move_assigned == tree);
  assert(copy_assigned.Empty());
  ValidateTree(move_assigned);

  Tree other;
  other.InsertUnique(100);
  tree.Swap(other);
  assert(tree.Size() == 1);
  assert(*tree.Begin() == 100);
  assert(other.Size() == 7);
  ValidateTree(tree);
  ValidateTree(other);

  nuostl::swap(tree, other);
  assert(tree.Size() == 7);
  assert(other.Size() == 1);
  ValidateTree(tree);
  ValidateTree(other);

  nuostl::NuoRbTree<int, ReverseInt> reversed{ReverseInt()};
  reversed.InsertUnique(values, values + 7);
  assert(*reversed.Begin() == 7);
  ValidateTree(reversed);
}

void TestNuoRbTree::test_unique_insertion()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;

  int ten = 10;
  auto first = tree.InsertUnique(ten);
  assert(first.second);
  assert(*first.first == 10);
  ValidateTree(tree);

  auto duplicate = tree.InsertUnique(10);
  assert(!duplicate.second);
  assert(duplicate.first == first.first);
  assert(tree.Size() == 1);
  ValidateTree(tree);

  auto emplaced = tree.EmplaceUnique(5);
  assert(emplaced.second);
  ValidateTree(tree);

  auto begin_hint = tree.EmplaceUniqueUseHint(tree.Begin(), 1);
  assert(*begin_hint == 1);
  ValidateTree(tree);

  auto end_hint = tree.EmplaceUniqueUseHint(tree.End(), 30);
  assert(*end_hint == 30);
  ValidateTree(tree);

  auto middle_hint = tree.EmplaceUniqueUseHint(tree.Find(10), 7);
  assert(*middle_hint == 7);
  ValidateTree(tree);

  auto duplicate_hint = tree.InsertUnique(tree.End(), 10);
  assert(*duplicate_hint == 10);
  assert(tree.Size() == 5);
  ValidateTree(tree);

  int range[] = {20, 15, 25, 5, 15};
  tree.InsertUnique(range, range + 5);
  assert(tree.Size() == 8);
  ValidateTree(tree);
}

void TestNuoRbTree::test_multi_insertion()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;

  tree.InsertMulti(4);
  tree.InsertMulti(4);
  tree.EmplaceMulti(4);
  ValidateTree(tree);
  assert(tree.CountMulti(4) == 3);

  auto begin_hint = tree.EmplaceMultiUseHint(tree.Begin(), 1);
  assert(*begin_hint == 1);
  ValidateTree(tree);

  auto end_hint = tree.EmplaceMultiUseHint(tree.End(), 8);
  assert(*end_hint == 8);
  ValidateTree(tree);

  auto middle_hint = tree.InsertMulti(tree.Find(4), 3);
  assert(*middle_hint == 3);
  ValidateTree(tree);

  int range[] = {2, 4, 6, 4, 7};
  tree.InsertMulti(range, range + 5);
  assert(tree.Size() == 11);
  assert(tree.CountMulti(4) == 5);
  ValidateTree(tree);

  auto equal = tree.EqualRangeMulti(4);
  int count = 0;
  for (auto iterator = equal.first; iterator != equal.second; ++iterator)
  {
    assert(*iterator == 4);
    ++count;
  }
  assert(count == 5);
}

void TestNuoRbTree::test_iterators()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  int values[] = {3, 1, 5, 2, 4};
  tree.InsertUnique(values, values + 5);

  int expected = 1;
  for (auto iterator = tree.Begin(); iterator != tree.End(); ++iterator)
  {
    assert(*iterator == expected);
    ++expected;
  }

  expected = 5;
  for (auto iterator = tree.RBegin(); iterator != tree.REnd(); ++iterator)
  {
    assert(*iterator == expected);
    --expected;
  }

  auto iterator = tree.Begin();
  auto old = iterator++;
  assert(*old == 1);
  assert(*iterator == 2);
  --iterator;
  assert(*iterator == 1);

  auto last = tree.End();
  --last;
  assert(*last == 5);
  auto old_last = last--;
  assert(*old_last == 5);
  assert(*last == 4);

  const Tree& const_tree = tree;
  Tree::const_iterator const_iterator = tree.Begin();
  assert(const_iterator == const_tree.CBegin());
  assert(*const_tree.CBegin() == 1);
  assert(*const_tree.CRBegin() == 5);
  assert(const_tree.CEnd() == const_tree.End());
  ValidateTree(const_tree);
}

void TestNuoRbTree::test_lookup_and_bounds()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  for (int value = 0; value <= 18; value += 2)
    tree.InsertUnique(value);

  assert(tree.Find(8) != tree.End());
  assert(tree.Find(9) == tree.End());
  assert(tree.CountUnique(8) == 1);
  assert(tree.CountUnique(9) == 0);
  assert(*tree.LowerBound(7) == 8);
  assert(*tree.LowerBound(8) == 8);
  assert(*tree.UpperBound(8) == 10);
  assert(tree.UpperBound(18) == tree.End());

  auto present = tree.EqualRangeUnique(8);
  assert(*present.first == 8);
  assert(*present.second == 10);
  auto missing = tree.EqualRangeUnique(9);
  assert(missing.first == missing.second);
  assert(*missing.first == 10);

  const Tree& const_tree = tree;
  assert(*const_tree.Find(4) == 4);
  assert(*const_tree.LowerBound(5) == 6);
  assert(*const_tree.UpperBound(16) == 18);
  assert(*const_tree.EqualRangeUnique(11).first == 12);
  ValidateTree(tree);
}

void TestNuoRbTree::test_erase()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  int values[] = {20, 10, 30, 5, 15, 25, 35, 1,
                  6, 14, 16, 24, 26, 34, 36};
  tree.InsertUnique(values, values + 15);
  ValidateTree(tree);

  auto next = tree.Erase(tree.Find(1));
  assert(*next == 5);
  ValidateTree(tree);

  assert(tree.EraseUnique(5) == 1);
  assert(tree.EraseUnique(5) == 0);
  ValidateTree(tree);

  assert(tree.EraseUnique(20) == 1);
  ValidateTree(tree);
  assert(tree.EraseUnique(36) == 1);
  ValidateTree(tree);

  auto first = tree.LowerBound(14);
  auto last = tree.UpperBound(26);
  tree.Erase(first, last);
  ValidateTree(tree);
  assert(tree.Find(14) == tree.End());
  assert(tree.Find(26) == tree.End());

  tree.Clear();
  ValidateTree(tree);

  tree.InsertMulti(2);
  tree.InsertMulti(2);
  tree.InsertMulti(2);
  tree.InsertMulti(3);
  assert(tree.EraseMulti(2) == 3);
  assert(tree.Size() == 1);
  ValidateTree(tree);
  tree.Erase(tree.Begin(), tree.End());
  ValidateTree(tree);
}

void TestNuoRbTree::test_pair_values_and_comparison()
{
  using Value = nuostl::nuo_pair<const int, std::string>;
  using Tree = nuostl::NuoRbTree<Value, nuostl::NuoLess<int>>;

  Tree unique;
  auto first = unique.InsertUnique(Value(2, "two"));
  assert(first.second);
  unique.InsertUnique(Value(1, "one"));
  auto duplicate = unique.InsertUnique(Value(2, "ignored"));
  assert(!duplicate.second);
  assert(duplicate.first->second == "two");
  duplicate.first->second = "updated";
  assert(unique.Find(2)->second == "updated");
  ValidateTree(unique);

  Tree multi;
  multi.InsertMulti(Value(2, "a"));
  multi.InsertMulti(Value(2, "b"));
  multi.InsertMulti(Value(1, "one"));
  assert(multi.CountMulti(2) == 2);
  ValidateTree(multi);

  using IntTree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  IntTree lhs;
  IntTree same;
  IntTree greater;
  int lhs_values[] = {1, 2, 3};
  int greater_values[] = {1, 2, 4};
  lhs.InsertUnique(lhs_values, lhs_values + 3);
  same.InsertUnique(lhs_values, lhs_values + 3);
  greater.InsertUnique(greater_values, greater_values + 3);
  assert(lhs == same);
  assert(lhs != greater);
  assert(lhs < greater);
  assert(greater > lhs);
  assert(lhs <= same);
  assert(lhs >= same);
}

void TestNuoRbTree::test_rebalance_stress()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree permuted;

  for (int index = 0; index < 127; ++index)
  {
    const int value = (index * 37) % 127;
    InsertUniqueAndValidate(permuted, value);
  }
  assert(permuted.Size() == 127);

  for (int index = 0; index < 127; ++index)
  {
    const int value = (index * 53) % 127;
    auto iterator = permuted.Find(value);
    assert(iterator != permuted.End());
    permuted.Erase(iterator);
    ValidateTree(permuted);
  }
  assert(permuted.Empty());

  Tree ascending;
  for (int value = 0; value < 64; ++value)
    InsertUniqueAndValidate(ascending, value);
  for (int value = 63; value >= 0; --value)
  {
    assert(ascending.EraseUnique(value) == 1);
    ValidateTree(ascending);
  }

  Tree duplicate_heavy;
  for (int index = 0; index < 96; ++index)
  {
    duplicate_heavy.InsertMulti((index * 17) % 11);
    ValidateTree(duplicate_heavy);
  }
  for (int key = 0; key < 11; ++key)
  {
    duplicate_heavy.EraseMulti(key);
    ValidateTree(duplicate_heavy);
  }
}

void TestNuoRbTree::test_nuo_rb_tree()
{
  test_value_traits_and_node_algorithms();
  test_constructor_assignment_and_swap();
  test_unique_insertion();
  test_multi_insertion();
  test_iterators();
  test_lookup_and_bounds();
  test_erase();
  test_pair_values_and_comparison();
  test_rebalance_stress();
}

} /* namespace test */
