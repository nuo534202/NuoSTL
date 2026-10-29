#include "core/assoc_cont/test_nuo_rb_tree.hpp"

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "core/assoc_cont/nuo_map.hpp"
#include "core/assoc_cont/nuo_rb_tree.hpp"
#include "core/assoc_cont/nuo_set.hpp"
#include "test_framework.hpp"

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

struct StatefulCompare
{
  StatefulCompare() = default;
  StatefulCompare(const StatefulCompare&) = default;
  StatefulCompare& operator=(const StatefulCompare&) = default;

  StatefulCompare(StatefulCompare&& rhs) noexcept
  {
    rhs.callable_ = false;
  }

  StatefulCompare& operator=(StatefulCompare&& rhs) noexcept
  {
    callable_ = rhs.callable_;
    rhs.callable_ = false;
    return *this;
  }

  bool operator()(int lhs, int rhs) const
  {
    if (!callable_)
    {
      throw std::runtime_error("moved-from comparator is not callable");
    }
    return lhs < rhs;
  }

 private:
  bool callable_ = true;
};

struct ThrowingCompare
{
  static bool throw_on_copy;
  static bool throw_on_move_assignment;

  ThrowingCompare() = default;

  ThrowingCompare(const ThrowingCompare&)
  {
    if (throw_on_copy)
    {
      throw std::runtime_error("comparator copy failed");
    }
  }

  ThrowingCompare& operator=(const ThrowingCompare&)
  {
    if (throw_on_copy)
    {
      throw std::runtime_error("comparator assignment failed");
    }
    return *this;
  }

  ThrowingCompare(ThrowingCompare&&) noexcept(false)
  {
  }
  ThrowingCompare& operator=(ThrowingCompare&&)
  {
    if (throw_on_move_assignment)
    {
      throw std::runtime_error("comparator move assignment failed");
    }
    return *this;
  }

  bool operator()(int lhs, int rhs) const noexcept
  {
    return lhs < rhs;
  }
};

bool ThrowingCompare::throw_on_copy = false;
bool ThrowingCompare::throw_on_move_assignment = false;

/* This comparator never throws, but its move constructor is declared as
   potentially throwing to verify noexcept propagation. */
struct NoexceptFalseMoveCompare
{
  NoexceptFalseMoveCompare() = default;
  NoexceptFalseMoveCompare(const NoexceptFalseMoveCompare&) = default;
  NoexceptFalseMoveCompare& operator=(const NoexceptFalseMoveCompare&) =
      default;
  NoexceptFalseMoveCompare(NoexceptFalseMoveCompare&&) noexcept(false)
  {
  }
  NoexceptFalseMoveCompare& operator=(NoexceptFalseMoveCompare&&) = default;

  bool operator()(int lhs, int rhs) const noexcept
  {
    return lhs < rhs;
  }
};

struct ThrowingOrderCompare
{
  static bool throw_on_comparison;

  bool operator()(int lhs, int rhs) const
  {
    if (throw_on_comparison)
    {
      throw std::runtime_error("comparison failed");
    }
    return lhs < rhs;
  }
};

bool ThrowingOrderCompare::throw_on_comparison = false;

class ComparatorThrowGuard
{
 public:
  ComparatorThrowGuard(bool throw_on_copy, bool throw_on_move_assignment)
      : previous_throw_on_copy_(ThrowingCompare::throw_on_copy),
        previous_throw_on_move_assignment_(
            ThrowingCompare::throw_on_move_assignment)
  {
    ThrowingCompare::throw_on_copy = throw_on_copy;
    ThrowingCompare::throw_on_move_assignment = throw_on_move_assignment;
  }

  ~ComparatorThrowGuard()
  {
    ThrowingCompare::throw_on_copy = previous_throw_on_copy_;
    ThrowingCompare::throw_on_move_assignment =
        previous_throw_on_move_assignment_;
  }

  ComparatorThrowGuard(const ComparatorThrowGuard&) = delete;
  ComparatorThrowGuard& operator=(const ComparatorThrowGuard&) = delete;

 private:
  bool previous_throw_on_copy_;
  bool previous_throw_on_move_assignment_;
};

class ComparisonThrowGuard
{
 public:
  explicit ComparisonThrowGuard(bool throw_on_comparison)
      : previous_(ThrowingOrderCompare::throw_on_comparison)
  {
    ThrowingOrderCompare::throw_on_comparison = throw_on_comparison;
  }

  ~ComparisonThrowGuard()
  {
    ThrowingOrderCompare::throw_on_comparison = previous_;
  }

  ComparisonThrowGuard(const ComparisonThrowGuard&) = delete;
  ComparisonThrowGuard& operator=(const ComparisonThrowGuard&) = delete;

 private:
  bool previous_;
};

template <typename Operation>
bool ThrowsRuntimeError(Operation&& operation)
{
  try
  {
    nuostl::NuoForward<Operation>(operation)();
    return false;
  }
  catch (const std::runtime_error&)
  {
    return true;
  }
  catch (...)
  {
    NUO_TEST_CHECK(false);
    return false;
  }
}

template <typename Tree>
int ValidateSubtree(typename Tree::base_ptr node,
                    typename Tree::base_ptr parent,
                    const typename Tree::key_compare& comp,
                    typename Tree::size_type& node_count)
{
  if (node == nullptr)
  {
    return 1;
  }

  if (node->parent_ != parent)
  {
    NUO_TEST_CHECK(false);
    return 1;
  }
  ++node_count;

  using Traits = typename Tree::value_traits;
  const auto& key = Traits::GetKey(node->ToNode()->value_);

  if (node->left_ != nullptr)
  {
    const auto& left_key = Traits::GetKey(node->left_->ToNode()->value_);
    NUO_TEST_CHECK(!comp(key, left_key));
  }
  if (node->right_ != nullptr)
  {
    const auto& right_key = Traits::GetKey(node->right_->ToNode()->value_);
    NUO_TEST_CHECK(!comp(right_key, key));
  }

  if (nuostl::NuoRbTreeIsRed(node))
  {
    NUO_TEST_CHECK(node->left_ == nullptr ||
                   !nuostl::NuoRbTreeIsRed(node->left_));
    NUO_TEST_CHECK(node->right_ == nullptr ||
                   !nuostl::NuoRbTreeIsRed(node->right_));
  }

  const int left_black_height =
      ValidateSubtree<Tree>(node->left_, node, comp, node_count);
  const int right_black_height =
      ValidateSubtree<Tree>(node->right_, node, comp, node_count);
  NUO_TEST_CHECK(left_black_height == right_black_height);
  return left_black_height + (nuostl::NuoRbTreeIsRed(node) ? 0 : 1);
}

template <typename Tree>
void ValidateTree(const Tree& tree)
{
  using Traits = typename Tree::value_traits;
  typename Tree::base_ptr header = tree.End().node_;
  if (header == nullptr)
  {
    NUO_TEST_CHECK(tree.Empty());
    NUO_TEST_CHECK(tree.Size() == 0);
    NUO_TEST_CHECK(tree.Begin() == tree.End());
    return;
  }
  NUO_TEST_CHECK(nuostl::NuoRbTreeIsRed(header));

  if (tree.Empty())
  {
    NUO_TEST_CHECK(tree.Size() == 0);
    NUO_TEST_CHECK(header->parent_ == nullptr);
    NUO_TEST_CHECK(header->left_ == header);
    NUO_TEST_CHECK(header->right_ == header);
    NUO_TEST_CHECK(tree.Begin() == tree.End());
    return;
  }

  typename Tree::base_ptr root = header->parent_;
  NUO_TEST_CHECK(root != nullptr);
  if (root == nullptr)
  {
    return;
  }
  NUO_TEST_CHECK(root->parent_ == header);
  NUO_TEST_CHECK(!nuostl::NuoRbTreeIsRed(root));
  NUO_TEST_CHECK(header->left_ == nuostl::NuoRbTreeMin(root));
  NUO_TEST_CHECK(header->right_ == nuostl::NuoRbTreeMax(root));
  NUO_TEST_CHECK(tree.Begin().node_ == header->left_);

  auto last = tree.End();
  --last;
  NUO_TEST_CHECK(last.node_ == header->right_);

  const auto comp = tree.KeyComp();
  typename Tree::size_type node_count = 0;
  const int black_height =
      ValidateSubtree<Tree>(root, header, comp, node_count);
  NUO_TEST_CHECK(black_height > 1);
  NUO_TEST_CHECK(node_count == tree.Size());

  auto previous = tree.Begin();
  auto current = previous;
  ++current;
  while (current != tree.End())
  {
    NUO_TEST_CHECK(!comp(Traits::GetKey(*current), Traits::GetKey(*previous)));
    previous = current;
    ++current;
  }

  typename Tree::size_type reverse_count = 0;
  for (auto iterator = tree.RBegin(); iterator != tree.REnd(); ++iterator)
  {
    ++reverse_count;
  }
  NUO_TEST_CHECK(reverse_count == tree.Size());
}

template <typename Tree>
void InsertUniqueAndValidate(Tree& tree, int value)
{
  auto result = tree.InsertUnique(value);
  NUO_TEST_CHECK(result.second);
  NUO_TEST_CHECK(*result.first == value);
  ValidateTree(tree);
}

} /* namespace */

void TestNuoRbTree::test_value_traits_and_node_algorithms()
{
  using Pair = nuostl::nuo_pair<const int, std::string>;
  static_assert(!nuostl::NuoRbTreeValueTraits<int>::kIsPair);
  static_assert(nuostl::NuoRbTreeValueTraits<Pair>::kIsPair);
  static_assert(
      std::is_same_v<nuostl::NuoRbTreeValueTraits<int>::key_type, int>);
  static_assert(
      std::is_same_v<nuostl::NuoRbTreeValueTraits<Pair>::key_type, const int>);

  Pair pair(7, "seven");
  NUO_TEST_CHECK(nuostl::NuoRbTreeValueTraits<Pair>::GetKey(pair) == 7);

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

  NUO_TEST_CHECK(nuostl::NuoRbTreeMin(root) == a.ToBase());
  NUO_TEST_CHECK(nuostl::NuoRbTreeMax(root) == y.ToBase());
  NUO_TEST_CHECK(nuostl::NuoRbTreeIsLChild(a.ToBase()));
  NUO_TEST_CHECK(nuostl::NuoRbTreeNext(a.ToBase()) == x.ToBase());
  NUO_TEST_CHECK(nuostl::NuoRbTreeNext(x.ToBase()) == b.ToBase());

  nuostl::NuoRbTreeSetRed(x.ToBase());
  NUO_TEST_CHECK(nuostl::NuoRbTreeIsRed(x.ToBase()));
  nuostl::NuoRbTreeSetBlack(x.ToBase());
  NUO_TEST_CHECK(!nuostl::NuoRbTreeIsRed(x.ToBase()));

  nuostl::NuoRbTreeRotateLeft(x.ToBase(), root);
  NUO_TEST_CHECK(root == y.ToBase());
  NUO_TEST_CHECK(y.left_ == x.ToBase());
  NUO_TEST_CHECK(x.parent_ == y.ToBase());
  NUO_TEST_CHECK(x.right_ == b.ToBase());
  NUO_TEST_CHECK(b.parent_ == x.ToBase());

  nuostl::NuoRbTreeRotateRight(y.ToBase(), root);
  NUO_TEST_CHECK(root == x.ToBase());
  NUO_TEST_CHECK(x.right_ == y.ToBase());
  NUO_TEST_CHECK(y.parent_ == x.ToBase());
  NUO_TEST_CHECK(y.left_ == b.ToBase());
  NUO_TEST_CHECK(b.parent_ == y.ToBase());
}

void TestNuoRbTree::test_constructor_assignment_and_swap()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  ValidateTree(tree);
  NUO_TEST_CHECK(tree.GetAllocator().Allocate(0) == nullptr);
  NUO_TEST_CHECK(tree.MaxSize() > 0);

  int values[] = {4, 2, 6, 1, 3, 5, 7};
  tree.InsertUnique(values, values + 7);
  ValidateTree(tree);

  Tree copied(tree);
  NUO_TEST_CHECK(copied == tree);
  ValidateTree(copied);

  Tree moved(nuostl::NuoMove(copied));
  NUO_TEST_CHECK(moved == tree);
  NUO_TEST_CHECK(copied.Empty());
  ValidateTree(moved);

  Tree copy_assigned;
  copy_assigned = tree;
  NUO_TEST_CHECK(copy_assigned == tree);
  ValidateTree(copy_assigned);

  Tree move_assigned;
  move_assigned.InsertUnique(99);
  move_assigned = nuostl::NuoMove(copy_assigned);
  NUO_TEST_CHECK(move_assigned == tree);
  NUO_TEST_CHECK(copy_assigned.Empty());
  ValidateTree(move_assigned);

  Tree other;
  other.InsertUnique(100);
  tree.Swap(other);
  NUO_TEST_CHECK(tree.Size() == 1);
  NUO_TEST_CHECK(*tree.Begin() == 100);
  NUO_TEST_CHECK(other.Size() == 7);
  ValidateTree(tree);
  ValidateTree(other);

  nuostl::swap(tree, other);
  NUO_TEST_CHECK(tree.Size() == 7);
  NUO_TEST_CHECK(other.Size() == 1);
  ValidateTree(tree);
  ValidateTree(other);

  nuostl::NuoRbTree<int, ReverseInt> reversed{ReverseInt()};
  reversed.InsertUnique(values, values + 7);
  NUO_TEST_CHECK(*reversed.Begin() == 7);
  ValidateTree(reversed);
}

void TestNuoRbTree::test_unique_insertion()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;

  int ten = 10;
  auto first = tree.InsertUnique(ten);
  NUO_TEST_CHECK(first.second);
  NUO_TEST_CHECK(*first.first == 10);
  ValidateTree(tree);

  auto duplicate = tree.InsertUnique(10);
  NUO_TEST_CHECK(!duplicate.second);
  NUO_TEST_CHECK(duplicate.first == first.first);
  NUO_TEST_CHECK(tree.Size() == 1);
  ValidateTree(tree);

  auto emplaced = tree.EmplaceUnique(5);
  NUO_TEST_CHECK(emplaced.second);
  ValidateTree(tree);

  auto begin_hint = tree.EmplaceUniqueUseHint(tree.Begin(), 1);
  NUO_TEST_CHECK(*begin_hint == 1);
  ValidateTree(tree);

  auto end_hint = tree.EmplaceUniqueUseHint(tree.End(), 30);
  NUO_TEST_CHECK(*end_hint == 30);
  ValidateTree(tree);

  auto middle_hint = tree.EmplaceUniqueUseHint(tree.Find(10), 7);
  NUO_TEST_CHECK(*middle_hint == 7);
  ValidateTree(tree);

  auto duplicate_hint = tree.InsertUnique(tree.End(), 10);
  NUO_TEST_CHECK(*duplicate_hint == 10);
  NUO_TEST_CHECK(tree.Size() == 5);
  ValidateTree(tree);

  int range[] = {20, 15, 25, 5, 15};
  tree.InsertUnique(range, range + 5);
  NUO_TEST_CHECK(tree.Size() == 8);
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
  NUO_TEST_CHECK(tree.CountMulti(4) == 3);

  auto begin_hint = tree.EmplaceMultiUseHint(tree.Begin(), 1);
  NUO_TEST_CHECK(*begin_hint == 1);
  ValidateTree(tree);

  auto end_hint = tree.EmplaceMultiUseHint(tree.End(), 8);
  NUO_TEST_CHECK(*end_hint == 8);
  ValidateTree(tree);

  auto middle_hint = tree.InsertMulti(tree.Find(4), 3);
  NUO_TEST_CHECK(*middle_hint == 3);
  ValidateTree(tree);

  int range[] = {2, 4, 6, 4, 7};
  tree.InsertMulti(range, range + 5);
  NUO_TEST_CHECK(tree.Size() == 11);
  NUO_TEST_CHECK(tree.CountMulti(4) == 5);
  ValidateTree(tree);

  auto equal = tree.EqualRangeMulti(4);
  int count = 0;
  for (auto iterator = equal.first; iterator != equal.second; ++iterator)
  {
    NUO_TEST_CHECK(*iterator == 4);
    ++count;
  }
  NUO_TEST_CHECK(count == 5);
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
    NUO_TEST_CHECK(*iterator == expected);
    ++expected;
  }

  expected = 5;
  for (auto iterator = tree.RBegin(); iterator != tree.REnd(); ++iterator)
  {
    NUO_TEST_CHECK(*iterator == expected);
    --expected;
  }

  auto iterator = tree.Begin();
  auto old = iterator++;
  NUO_TEST_CHECK(*old == 1);
  NUO_TEST_CHECK(*iterator == 2);
  --iterator;
  NUO_TEST_CHECK(*iterator == 1);

  auto last = tree.End();
  --last;
  NUO_TEST_CHECK(*last == 5);
  auto old_last = last--;
  NUO_TEST_CHECK(*old_last == 5);
  NUO_TEST_CHECK(*last == 4);

  const Tree& const_tree = tree;
  Tree::const_iterator const_iterator = tree.Begin();
  NUO_TEST_CHECK(const_iterator == const_tree.CBegin());
  NUO_TEST_CHECK(*const_tree.CBegin() == 1);
  NUO_TEST_CHECK(*const_tree.CRBegin() == 5);
  NUO_TEST_CHECK(const_tree.CEnd() == const_tree.End());
  ValidateTree(const_tree);
}

void TestNuoRbTree::test_lookup_and_bounds()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  for (int value = 0; value <= 18; value += 2)
  {
    tree.InsertUnique(value);
  }

  NUO_TEST_CHECK(tree.Find(8) != tree.End());
  NUO_TEST_CHECK(tree.Find(9) == tree.End());
  NUO_TEST_CHECK(tree.CountUnique(8) == 1);
  NUO_TEST_CHECK(tree.CountUnique(9) == 0);
  NUO_TEST_CHECK(*tree.LowerBound(7) == 8);
  NUO_TEST_CHECK(*tree.LowerBound(8) == 8);
  NUO_TEST_CHECK(*tree.UpperBound(8) == 10);
  NUO_TEST_CHECK(tree.UpperBound(18) == tree.End());

  auto present = tree.EqualRangeUnique(8);
  NUO_TEST_CHECK(*present.first == 8);
  NUO_TEST_CHECK(*present.second == 10);
  auto missing = tree.EqualRangeUnique(9);
  NUO_TEST_CHECK(missing.first == missing.second);
  NUO_TEST_CHECK(*missing.first == 10);

  const Tree& const_tree = tree;
  NUO_TEST_CHECK(*const_tree.Find(4) == 4);
  NUO_TEST_CHECK(*const_tree.LowerBound(5) == 6);
  NUO_TEST_CHECK(*const_tree.UpperBound(16) == 18);
  NUO_TEST_CHECK(*const_tree.EqualRangeUnique(11).first == 12);
  ValidateTree(tree);
}

void TestNuoRbTree::test_erase()
{
  using Tree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  Tree tree;
  int values[] = {20, 10, 30, 5, 15, 25, 35, 1, 6, 14, 16, 24, 26, 34, 36};
  tree.InsertUnique(values, values + 15);
  ValidateTree(tree);

  auto next = tree.Erase(tree.Find(1));
  NUO_TEST_CHECK(*next == 5);
  ValidateTree(tree);

  NUO_TEST_CHECK(tree.EraseUnique(5) == 1);
  NUO_TEST_CHECK(tree.EraseUnique(5) == 0);
  ValidateTree(tree);

  NUO_TEST_CHECK(tree.EraseUnique(20) == 1);
  ValidateTree(tree);
  NUO_TEST_CHECK(tree.EraseUnique(36) == 1);
  ValidateTree(tree);

  auto first = tree.LowerBound(14);
  auto last = tree.UpperBound(26);
  tree.Erase(first, last);
  ValidateTree(tree);
  NUO_TEST_CHECK(tree.Find(14) == tree.End());
  NUO_TEST_CHECK(tree.Find(26) == tree.End());

  tree.Clear();
  ValidateTree(tree);

  tree.InsertMulti(2);
  tree.InsertMulti(2);
  tree.InsertMulti(2);
  tree.InsertMulti(3);
  NUO_TEST_CHECK(tree.EraseMulti(2) == 3);
  NUO_TEST_CHECK(tree.Size() == 1);
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
  NUO_TEST_CHECK(first.second);
  unique.InsertUnique(Value(1, "one"));
  auto duplicate = unique.InsertUnique(Value(2, "ignored"));
  NUO_TEST_CHECK(!duplicate.second);
  NUO_TEST_CHECK(duplicate.first->second == "two");
  duplicate.first->second = "updated";
  NUO_TEST_CHECK(unique.Find(2)->second == "updated");
  ValidateTree(unique);

  Tree multi;
  multi.InsertMulti(Value(2, "a"));
  multi.InsertMulti(Value(2, "b"));
  multi.InsertMulti(Value(1, "one"));
  NUO_TEST_CHECK(multi.CountMulti(2) == 2);
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
  NUO_TEST_CHECK(lhs == same);
  NUO_TEST_CHECK(lhs != greater);
  NUO_TEST_CHECK(lhs < greater);
  NUO_TEST_CHECK(greater > lhs);
  NUO_TEST_CHECK(lhs <= same);
  NUO_TEST_CHECK(lhs >= same);
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
  NUO_TEST_CHECK(permuted.Size() == 127);

  for (int index = 0; index < 127; ++index)
  {
    const int value = (index * 53) % 127;
    auto iterator = permuted.Find(value);
    NUO_TEST_CHECK(iterator != permuted.End());
    permuted.Erase(iterator);
    ValidateTree(permuted);
  }
  NUO_TEST_CHECK(permuted.Empty());

  Tree ascending;
  for (int value = 0; value < 64; ++value)
  {
    InsertUniqueAndValidate(ascending, value);
  }
  for (int value = 63; value >= 0; --value)
  {
    NUO_TEST_CHECK(ascending.EraseUnique(value) == 1);
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

void TestNuoRbTree::test_move_noexcept_traits()
{
  using Tree = nuostl::NuoRbTree<int, ThrowingCompare>;
  using Map = nuostl::NuoMap<int, int, ThrowingCompare>;
  using Multimap = nuostl::NuoMultimap<int, int, ThrowingCompare>;
  using Set = nuostl::NuoSet<int, ThrowingCompare>;
  using Multiset = nuostl::NuoMultiset<int, ThrowingCompare>;

  using NoexceptFalseMoveTree =
      nuostl::NuoRbTree<int, NoexceptFalseMoveCompare>;
  using NoexceptFalseMoveMap =
      nuostl::NuoMap<int, int, NoexceptFalseMoveCompare>;
  using NoexceptFalseMoveMultimap =
      nuostl::NuoMultimap<int, int, NoexceptFalseMoveCompare>;
  using NoexceptFalseMoveSet = nuostl::NuoSet<int, NoexceptFalseMoveCompare>;
  using NoexceptFalseMoveMultiset =
      nuostl::NuoMultiset<int, NoexceptFalseMoveCompare>;
  using NoThrowTree = nuostl::NuoRbTree<int, nuostl::NuoLess<int>>;
  using NoThrowMap = nuostl::NuoMap<int, int, nuostl::NuoLess<int>>;

  static_assert(!std::is_nothrow_move_constructible_v<Tree>);
  static_assert(!std::is_nothrow_move_constructible_v<Map>);
  static_assert(!std::is_nothrow_move_constructible_v<Multimap>);
  static_assert(!std::is_nothrow_move_constructible_v<Set>);
  static_assert(!std::is_nothrow_move_constructible_v<Multiset>);
  static_assert(std::is_nothrow_move_constructible_v<NoexceptFalseMoveTree>);
  static_assert(std::is_nothrow_move_constructible_v<NoexceptFalseMoveMap>);
  static_assert(
      std::is_nothrow_move_constructible_v<NoexceptFalseMoveMultimap>);
  static_assert(std::is_nothrow_move_constructible_v<NoexceptFalseMoveSet>);
  static_assert(
      std::is_nothrow_move_constructible_v<NoexceptFalseMoveMultiset>);
  static_assert(!std::is_nothrow_move_assignable_v<Tree>);
  static_assert(!std::is_nothrow_move_assignable_v<Map>);
  static_assert(!std::is_nothrow_move_assignable_v<Multimap>);
  static_assert(!std::is_nothrow_move_assignable_v<Set>);
  static_assert(!std::is_nothrow_move_assignable_v<Multiset>);
  static_assert(std::is_nothrow_move_assignable_v<NoexceptFalseMoveTree>);
  static_assert(std::is_nothrow_move_assignable_v<NoexceptFalseMoveMap>);
  static_assert(std::is_nothrow_move_assignable_v<NoexceptFalseMoveMultimap>);
  static_assert(std::is_nothrow_move_assignable_v<NoexceptFalseMoveSet>);
  static_assert(std::is_nothrow_move_assignable_v<NoexceptFalseMoveMultiset>);
  static_assert(!noexcept(std::declval<NoexceptFalseMoveTree&>().Swap(
      std::declval<NoexceptFalseMoveTree&>())));
  static_assert(!noexcept(std::declval<NoexceptFalseMoveMap&>().Swap(
      std::declval<NoexceptFalseMoveMap&>())));
  static_assert(!noexcept(std::declval<NoexceptFalseMoveMultimap&>().Swap(
      std::declval<NoexceptFalseMoveMultimap&>())));
  static_assert(!noexcept(std::declval<NoexceptFalseMoveSet&>().Swap(
      std::declval<NoexceptFalseMoveSet&>())));
  static_assert(!noexcept(std::declval<NoexceptFalseMoveMultiset&>().Swap(
      std::declval<NoexceptFalseMoveMultiset&>())));
  static_assert(
      !noexcept(nuostl::swap(std::declval<NoexceptFalseMoveTree&>(),
                             std::declval<NoexceptFalseMoveTree&>())));
  static_assert(!noexcept(nuostl::swap(std::declval<NoexceptFalseMoveMap&>(),
                                       std::declval<NoexceptFalseMoveMap&>())));
  static_assert(
      !noexcept(nuostl::swap(std::declval<NoexceptFalseMoveMultimap&>(),
                             std::declval<NoexceptFalseMoveMultimap&>())));
  static_assert(!noexcept(nuostl::swap(std::declval<NoexceptFalseMoveSet&>(),
                                       std::declval<NoexceptFalseMoveSet&>())));
  static_assert(
      !noexcept(nuostl::swap(std::declval<NoexceptFalseMoveMultiset&>(),
                             std::declval<NoexceptFalseMoveMultiset&>())));
  static_assert(noexcept(
      std::declval<NoThrowTree&>().Swap(std::declval<NoThrowTree&>())));
  static_assert(noexcept(nuostl::swap(std::declval<NoThrowTree&>(),
                                      std::declval<NoThrowTree&>())));
  static_assert(
      noexcept(std::declval<NoThrowMap&>().Swap(std::declval<NoThrowMap&>())));
}

void TestNuoRbTree::test_tree_move_failures()
{
  using Tree = nuostl::NuoRbTree<int, ThrowingCompare>;
  using ThrowingOrderTree = nuostl::NuoRbTree<int, ThrowingOrderCompare>;

  Tree copy_failure_source;
  copy_failure_source.InsertUnique(2);
  copy_failure_source.InsertUnique(3);
  Tree copy_failure_destination;
  copy_failure_destination.InsertUnique(1);

  bool copy_caught = false;
  {
    ComparatorThrowGuard guard(true, false);
    copy_caught = ThrowsRuntimeError(
        [&]()
        { copy_failure_destination = nuostl::NuoMove(copy_failure_source); });
  }

  NUO_TEST_CHECK(copy_caught);
  if (copy_caught)
  {
    NUO_TEST_CHECK(copy_failure_destination.Size() == 1);
    if (copy_failure_destination.Size() == 1)
    {
      NUO_TEST_CHECK(*copy_failure_destination.Begin() == 1);
    }
    NUO_TEST_CHECK(copy_failure_source.Size() == 2);
    if (copy_failure_source.Size() == 2)
    {
      NUO_TEST_CHECK(*copy_failure_source.Begin() == 2);
    }
    ValidateTree(copy_failure_destination);
    ValidateTree(copy_failure_source);
  }

  Tree assignment_failure_source;
  assignment_failure_source.InsertUnique(7);
  assignment_failure_source.InsertUnique(8);
  Tree assignment_failure_destination;
  assignment_failure_destination.InsertUnique(6);

  bool assignment_caught = false;
  {
    ComparatorThrowGuard guard(false, true);
    assignment_caught = ThrowsRuntimeError(
        [&]()
        {
          assignment_failure_destination =
              nuostl::NuoMove(assignment_failure_source);
        });
  }

  NUO_TEST_CHECK(assignment_caught);
  if (assignment_caught)
  {
    NUO_TEST_CHECK(assignment_failure_destination.Empty());
    ValidateTree(assignment_failure_destination);
    NUO_TEST_CHECK(assignment_failure_source.Size() == 2);
    if (assignment_failure_source.Size() == 2)
    {
      NUO_TEST_CHECK(*assignment_failure_source.Begin() == 7);
    }
    ValidateTree(assignment_failure_source);
    assignment_failure_destination.InsertUnique(6);
    ValidateTree(assignment_failure_destination);
  }

  Tree comparator_assignment_source;
  comparator_assignment_source.InsertUnique(9);
  Tree comparator_assignment_destination;
  comparator_assignment_destination.InsertUnique(10);
  bool comparator_assignment_caught = false;
  {
    ComparatorThrowGuard guard(true, false);
    comparator_assignment_caught = ThrowsRuntimeError(
        [&]()
        { comparator_assignment_destination = comparator_assignment_source; });
  }

  NUO_TEST_CHECK(comparator_assignment_caught);
  if (comparator_assignment_caught)
  {
    NUO_TEST_CHECK(comparator_assignment_destination.Empty());
    ValidateTree(comparator_assignment_destination);
    ValidateTree(comparator_assignment_source);
    comparator_assignment_destination.InsertUnique(10);
    ValidateTree(comparator_assignment_destination);
  }

  Tree copy_construction_source;
  copy_construction_source.InsertUnique(23);
  bool copy_construction_caught = false;
  {
    ComparatorThrowGuard guard(true, false);
    copy_construction_caught = ThrowsRuntimeError(
        [&]() { Tree failed_copy(copy_construction_source); });
  }
  NUO_TEST_CHECK(copy_construction_caught);
  if (copy_construction_caught)
  {
    NUO_TEST_CHECK(copy_construction_source.Size() == 1);
    if (copy_construction_source.Size() == 1)
    {
      NUO_TEST_CHECK(*copy_construction_source.Begin() == 23);
    }
    ValidateTree(copy_construction_source);
  }

  Tree move_construction_failure_source;
  move_construction_failure_source.InsertUnique(10);
  move_construction_failure_source.InsertUnique(11);

  bool move_construction_caught = false;
  {
    ComparatorThrowGuard guard(true, false);
    move_construction_caught = ThrowsRuntimeError(
        [&]()
        {
          Tree failed_move(nuostl::NuoMove(move_construction_failure_source));
        });
  }

  NUO_TEST_CHECK(move_construction_caught);
  if (move_construction_caught)
  {
    NUO_TEST_CHECK(move_construction_failure_source.Size() == 2);
    if (move_construction_failure_source.Size() == 2)
    {
      NUO_TEST_CHECK(*move_construction_failure_source.Begin() == 10);
    }
    ValidateTree(move_construction_failure_source);
  }

  Tree swap_failure_left;
  swap_failure_left.InsertUnique(14);
  Tree swap_failure_right;
  swap_failure_right.InsertUnique(15);
  bool swap_caught = false;
  {
    ComparatorThrowGuard guard(false, true);
    swap_caught = ThrowsRuntimeError(
        [&]() { swap_failure_left.Swap(swap_failure_right); });
  }

  NUO_TEST_CHECK(swap_caught);
  if (swap_caught)
  {
    NUO_TEST_CHECK(swap_failure_left.Size() == 1);
    if (swap_failure_left.Size() == 1)
    {
      NUO_TEST_CHECK(*swap_failure_left.Begin() == 14);
    }
    NUO_TEST_CHECK(swap_failure_right.Size() == 1);
    if (swap_failure_right.Size() == 1)
    {
      NUO_TEST_CHECK(*swap_failure_right.Begin() == 15);
    }
    ValidateTree(swap_failure_left);
    ValidateTree(swap_failure_right);
  }

  ThrowingOrderTree comparison_failure_tree;
  comparison_failure_tree.InsertUnique(16);
  bool comparison_caught = false;
  {
    ComparisonThrowGuard guard(true);
    comparison_caught =
        ThrowsRuntimeError([&]() { comparison_failure_tree.InsertUnique(17); });
  }
  NUO_TEST_CHECK(comparison_caught);
  if (comparison_caught)
  {
    NUO_TEST_CHECK(comparison_failure_tree.Size() == 1);
    if (comparison_failure_tree.Size() == 1)
    {
      NUO_TEST_CHECK(*comparison_failure_tree.Begin() == 16);
    }
    ValidateTree(comparison_failure_tree);
    comparison_failure_tree.InsertUnique(17);
    NUO_TEST_CHECK(comparison_failure_tree.Size() == 2);
    ValidateTree(comparison_failure_tree);
  }

  Tree movable;
  movable.InsertUnique(4);
  {
    Tree moved(nuostl::NuoMove(movable));
    NUO_TEST_CHECK(moved.Size() == 1);
    if (moved.Size() == 1)
    {
      NUO_TEST_CHECK(*moved.Begin() == 4);
      ValidateTree(moved);
    }
    NUO_TEST_CHECK(movable.Empty());
    NUO_TEST_CHECK(movable.Size() == 0);
    NUO_TEST_CHECK(movable.Begin() == movable.End());
    NUO_TEST_CHECK(movable.Find(4) == movable.End());
    NUO_TEST_CHECK(movable.LowerBound(4) == movable.End());
    NUO_TEST_CHECK(movable.UpperBound(4) == movable.End());
    NUO_TEST_CHECK(movable.CountUnique(4) == 0);
    ValidateTree(movable);
  }

  Tree reusable;
  reusable.InsertUnique(1);
  {
    Tree moved(nuostl::NuoMove(reusable));
    NUO_TEST_CHECK(moved.Size() == 1);
    ValidateTree(moved);
  }
  NUO_TEST_CHECK(reusable.Empty());
  reusable.InsertUnique(6);
  NUO_TEST_CHECK(reusable.Size() == 1);
  if (reusable.Size() == 1)
  {
    NUO_TEST_CHECK(*reusable.Begin() == 6);
    ValidateTree(reusable);
  }

  Tree copied_empty;
  copied_empty = movable;
  NUO_TEST_CHECK(copied_empty.Empty());
  ValidateTree(copied_empty);
  copied_empty.InsertUnique(9);
  NUO_TEST_CHECK(copied_empty.Size() == 1);
  ValidateTree(copied_empty);

  Tree replacement;
  replacement.InsertUnique(5);
  movable = nuostl::NuoMove(replacement);
  NUO_TEST_CHECK(movable.Size() == 1);
  if (movable.Size() == 1)
  {
    NUO_TEST_CHECK(*movable.Begin() == 5);
    ValidateTree(movable);
  }
  NUO_TEST_CHECK(replacement.Empty());
}

void TestNuoRbTree::test_moved_from_tree_reuse()
{
  using Tree = nuostl::NuoRbTree<int, ThrowingCompare>;
  using Map = nuostl::NuoMap<int, int, ThrowingCompare>;
  using Multiset = nuostl::NuoMultiset<int, ThrowingCompare>;
  using StatefulTree = nuostl::NuoRbTree<int, StatefulCompare>;

  StatefulTree stateful_source;
  stateful_source.InsertUnique(18);
  StatefulTree stateful_owner(nuostl::NuoMove(stateful_source));
  NUO_TEST_CHECK(stateful_owner.Size() == 1);
  if (stateful_owner.Size() == 1)
  {
    NUO_TEST_CHECK(*stateful_owner.Begin() == 18);
    ValidateTree(stateful_owner);
  }
  stateful_source.InsertUnique(19);
  NUO_TEST_CHECK(stateful_source.Size() == 1);
  if (stateful_source.Size() == 1)
  {
    NUO_TEST_CHECK(*stateful_source.Begin() == 19);
    ValidateTree(stateful_source);
  }

  Tree copy_source;
  copy_source.InsertUnique(12);
  Tree moved_from_copy_destination;
  moved_from_copy_destination.InsertUnique(13);
  Tree copy_destination_owner(nuostl::NuoMove(moved_from_copy_destination));
  NUO_TEST_CHECK(copy_destination_owner.Size() == 1);
  if (copy_destination_owner.Size() == 1)
  {
    NUO_TEST_CHECK(*copy_destination_owner.Begin() == 13);
    ValidateTree(copy_destination_owner);
  }
  moved_from_copy_destination = copy_source;
  NUO_TEST_CHECK(moved_from_copy_destination.Size() == 1);
  if (moved_from_copy_destination.Size() == 1)
  {
    NUO_TEST_CHECK(*moved_from_copy_destination.Begin() == 12);
    ValidateTree(moved_from_copy_destination);
  }

  Tree moved_from_move_source;
  moved_from_move_source.InsertUnique(20);
  Tree moved_from_move_source_owner(nuostl::NuoMove(moved_from_move_source));
  NUO_TEST_CHECK(moved_from_move_source_owner.Size() == 1);
  if (moved_from_move_source_owner.Size() == 1)
  {
    NUO_TEST_CHECK(*moved_from_move_source_owner.Begin() == 20);
    ValidateTree(moved_from_move_source_owner);
  }
  Tree moved_from_move_destination;
  moved_from_move_destination.InsertUnique(21);
  moved_from_move_destination = nuostl::NuoMove(moved_from_move_source);
  NUO_TEST_CHECK(moved_from_move_destination.Empty());
  ValidateTree(moved_from_move_destination);
  moved_from_move_destination.InsertUnique(22);
  NUO_TEST_CHECK(moved_from_move_destination.Size() == 1);
  if (moved_from_move_destination.Size() == 1)
  {
    NUO_TEST_CHECK(*moved_from_move_destination.Begin() == 22);
    ValidateTree(moved_from_move_destination);
  }

  Map map_source{{1, 10}};
  Map map_owner(nuostl::NuoMove(map_source));
  NUO_TEST_CHECK(map_owner.Size() == 1);
  NUO_TEST_CHECK(map_owner.At(1) == 10);
  map_source = {{2, 20}, {3, 30}};
  NUO_TEST_CHECK(map_source.Size() == 2);
  NUO_TEST_CHECK(map_source.At(2) == 20);
  NUO_TEST_CHECK(map_source.At(3) == 30);

  Multiset multiset_source{4, 4, 5};
  Multiset multiset_owner(nuostl::NuoMove(multiset_source));
  NUO_TEST_CHECK(multiset_owner.Size() == 3);
  NUO_TEST_CHECK(multiset_owner.Count(4) == 2);
  multiset_source = {7, 7, 8};
  NUO_TEST_CHECK(multiset_source.Size() == 3);
  NUO_TEST_CHECK(multiset_source.Count(7) == 2);
}

void TestNuoRbTree::test_move_operations_exception_safety()
{
  test_move_noexcept_traits();
  test_tree_move_failures();
  test_moved_from_tree_reuse();
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
  test_move_operations_exception_safety();
}

} /* namespace test */
