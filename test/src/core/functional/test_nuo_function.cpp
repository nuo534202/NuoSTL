#include "core/functional/test_nuo_function.hpp"

#include <assert.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "core/functional/nuo_function.hpp"

namespace test
{

namespace
{

int Add(int first, int second)
{
  return first + second;
}

struct Widget
{
  int value = 7;

  int Get() const
  {
    return value;
  }

  void Set(int new_value)
  {
    value = new_value;
  }
};

struct Counter
{
  int count = 0;

  int operator()(int increment)
  {
    count += increment;
    return count;
  }
};

struct MoveOnlyCallable
{
  MoveOnlyCallable() = default;
  MoveOnlyCallable(MoveOnlyCallable&&) = default;
  MoveOnlyCallable& operator=(MoveOnlyCallable&&) = default;
  MoveOnlyCallable(const MoveOnlyCallable&) = delete;
  MoveOnlyCallable& operator=(const MoveOnlyCallable&) = delete;

  int operator()() const
  {
    return 1;
  }
};

struct ReferenceTarget
{
  int value = 0;

  int& Get()
  {
    return value;
  }
};

template <typename Exception, typename Function>
void ExpectThrows(Function function)
{
  bool caught = false;
  try
  {
    function();
  }
  catch (const Exception&)
  {
    caught = true;
  }
  assert(caught);
}

} /* namespace */

void TestNuoFunction::TestDefaultState()
{
  nuostl::NuoFunction<int(int, int)> empty;
  assert(!empty);
  assert(!static_cast<bool>(empty));
  assert(empty.TargetType() == typeid(void));
  assert(empty.Target<int (*)(int, int)>() == nullptr);

  const nuostl::NuoFunction<int(int, int)>& const_empty = empty;
  assert(const_empty.Target<int (*)(int, int)>() == nullptr);
  assert(const_empty.TargetType() == typeid(void));

  ExpectThrows<std::bad_function_call>([&]()
  {
    (void)empty(1, 2);
  });

  nuostl::NuoFunction<int(int, int)> null(nullptr);
  assert(!null);
  assert(null == nullptr);
  assert(nullptr == null);
  assert(!(null != nullptr));
}

void TestNuoFunction::TestFunctionPointers()
{
  using FunctionPointer = int (*)(int, int);

  nuostl::NuoFunction<int(int, int)> function = &Add;
  assert(function);
  assert(function(2, 3) == 5);
  assert(function.Target<FunctionPointer>() != nullptr);
  assert(*function.Target<FunctionPointer>() == &Add);
  assert(function.TargetType() == typeid(FunctionPointer));
  assert(function.Target<int (*)(int)>() == nullptr);

  FunctionPointer null_pointer = nullptr;
  nuostl::NuoFunction<int(int, int)> from_null = null_pointer;
  assert(!from_null);

  function = null_pointer;
  assert(!function);

  function = &Add;
  assert(function(4, 5) == 9);
}

void TestNuoFunction::TestCallables()
{
  Counter counter;
  nuostl::NuoFunction<int(int)> by_value = counter;
  assert(by_value(2) == 2);
  assert(by_value(3) == 5);
  assert(counter.count == 0);
  assert(by_value.Target<Counter>() != nullptr);
  assert(by_value.Target<Counter>()->count == 5);

  nuostl::NuoFunction<int(int)> by_reference = std::ref(counter);
  assert(by_reference(1) == 1);
  assert(counter.count == 1);
  assert(by_reference.Target<std::reference_wrapper<Counter>>() != nullptr);
  assert(by_reference.TargetType() ==
         typeid(std::reference_wrapper<Counter>));

  auto lambda = [](int value)
  {
    return value * 2;
  };
  nuostl::NuoFunction<int(int)> from_lambda = lambda;
  assert(from_lambda(21) == 42);
  assert(from_lambda.Target<decltype(lambda)>() != nullptr);
  assert(from_lambda.Target<int (*)(int)>() == nullptr);

  nuostl::NuoFunction<int()> mutable_lambda = [count = 0]() mutable
  {
    return ++count;
  };
  const nuostl::NuoFunction<int()>& const_lambda = mutable_lambda;
  assert(const_lambda() == 1);
  assert(const_lambda() == 2);

  Widget widget;
  nuostl::NuoFunction<int(const Widget&)> getter = &Widget::Get;
  assert(getter(widget) == 7);
  nuostl::NuoFunction<int(Widget*)> getter_pointer = &Widget::Get;
  assert(getter_pointer(&widget) == 7);
  nuostl::NuoFunction<void(Widget&, int)> setter = &Widget::Set;
  setter(widget, 9);
  assert(widget.value == 9);

  std::function<int(int)> standard = [](int value)
  {
    return value + 1;
  };
  nuostl::NuoFunction<int(int)> from_standard = standard;
  assert(from_standard(1) == 2);
  assert(from_standard.Target<std::function<int(int)>>() != nullptr);
}

void TestNuoFunction::TestCopyAndMove()
{
  nuostl::NuoFunction<int()> original = []()
  {
    return 7;
  };
  nuostl::NuoFunction<int()> copy(original);
  assert(copy() == 7);
  assert(original() == 7);

  nuostl::NuoFunction<int()> assigned;
  assigned = copy;
  assert(assigned() == 7);

  nuostl::NuoFunction<int()> moved(std::move(original));
  assert(!original);
  assert(moved() == 7);

  nuostl::NuoFunction<int()> move_assigned;
  move_assigned = std::move(moved);
  assert(!moved);
  assert(move_assigned() == 7);

  nuostl::NuoFunction<int()> self = []()
  {
    return 5;
  };
  assert(self() == 5);

  nuostl::NuoFunction<int()> first = [value = 0]() mutable
  {
    return ++value;
  };
  nuostl::NuoFunction<int()> second = first;
  assert(first() == 1);
  assert(first() == 2);
  assert(second() == 1);
  assert(first() == 3);

  nuostl::NuoFunction<int()> destination;
  destination = std::move(first);
  assert(!first);
  assert(destination() == 4);
}

void TestNuoFunction::TestAssignment()
{
  nuostl::NuoFunction<int(int, int)> function;
  function = [](int first, int second)
  {
    return first + second + 3;
  };
  assert(function(4, 0) == 7);

  function = &Add;
  assert(function(4, 0) == 4);

  function = nullptr;
  assert(!function);
  ExpectThrows<std::bad_function_call>([&]()
  {
    (void)function(1, 1);
  });

  using FunctionPointer = int (*)(int, int);
  FunctionPointer null_pointer = nullptr;
  function = &Add;
  function = null_pointer;
  assert(!function);

  void (Widget::*null_member)(int) = nullptr;
  nuostl::NuoFunction<void(Widget&, int)> member_function = &Widget::Set;
  member_function = null_member;
  assert(!member_function);

  Counter counter;
  nuostl::NuoFunction<int(int)> reference;
  reference = std::ref(counter);
  assert(reference(2) == 2);
  assert(counter.count == 2);
}

void TestNuoFunction::TestSwap()
{
  nuostl::NuoFunction<int()> first = []()
  {
    return 1;
  };
  nuostl::NuoFunction<int()> second = []()
  {
    return 2;
  };

  first.Swap(second);
  assert(first() == 2);
  assert(second() == 1);

  nuostl::NuoSwap(first, second);
  assert(first() == 1);
  assert(second() == 2);

  using std::swap;
  swap(first, second);
  assert(first() == 2);
  assert(second() == 1);

  nuostl::NuoFunction<int()> empty;
  first.Swap(empty);
  assert(!first);
  assert(empty() == 2);
}

void TestNuoFunction::TestTargets()
{
  nuostl::NuoFunction<int(int)> empty;
  assert(empty.Target<Counter>() == nullptr);

  nuostl::NuoFunction<int(int)> function = Counter{};
  assert(function.TargetType() == typeid(Counter));
  assert(function.Target<Counter>() != nullptr);
  assert(function.Target<Counter>()->count == 0);

  function.Target<Counter>()->count = 5;
  assert(function(1) == 6);

  const nuostl::NuoFunction<int(int)>& const_function = function;
  assert(const_function.Target<Counter>() != nullptr);
  assert(const_function.Target<Counter>()->count == 6);
  assert(const_function.Target<int (*)(int)>() == nullptr);
  assert(const_function.TargetType() == typeid(Counter));

  assert(function.Target<const Counter>() != nullptr);
  assert(function.Target<volatile Counter>() != nullptr);
  assert(function.Target<const volatile Counter>() != nullptr);
  assert(const_function.Target<const Counter>() != nullptr);
}

void TestNuoFunction::TestArgumentsAndReturns()
{
  nuostl::NuoFunction<int(std::unique_ptr<int>)> consume =
    [](std::unique_ptr<int> value)
  {
    return *value;
  };
  std::unique_ptr<int> value = std::make_unique<int>(42);
  assert(consume(std::move(value)) == 42);
  assert(value == nullptr);

  nuostl::NuoFunction<int&(int&)> identity = [](int& number) -> int&
  {
    return number;
  };
  int number = 5;
  identity(number) = 9;
  assert(number == 9);

  nuostl::NuoFunction<ReferenceTarget&()> reference_target =
    []() -> ReferenceTarget&
    {
      static ReferenceTarget target;
      return target;
    };
  reference_target().value = 11;
  assert(reference_target().value == 11);

  int calls = 0;
  nuostl::NuoFunction<void()> action = [&calls]()
  {
    ++calls;
  };
  action();
  assert(calls == 1);

  nuostl::NuoFunction<std::string(int)> text = [](int count)
  {
    return std::string(static_cast<std::size_t>(count), 'x');
  };
  assert(text(3) == "xxx");

  nuostl::NuoFunction<long()> converted = []()
  {
    return 1;
  };
  assert(converted() == 1L);

  nuostl::NuoFunction<int()> throwing = []() -> int
  {
    throw std::runtime_error("failure");
  };
  ExpectThrows<std::runtime_error>([&]()
  {
    (void)throwing();
  });

  nuostl::NuoFunction<int(int, int)> function_reference = std::ref(Add);
  assert(function_reference(1, 2) == 3);
  assert(function_reference.Target<
           std::reference_wrapper<int(int, int)>>() != nullptr);

  int discarded_calls = 0;
  nuostl::NuoFunction<void()> discard = [&discarded_calls]()
  {
    ++discarded_calls;
    return 42;
  };
  discard();
  assert(discarded_calls == 1);

  nuostl::NuoFunction<int()> empty_source;
  nuostl::NuoFunction<int()> empty_copy(empty_source);
  assert(!empty_copy);
  empty_copy = empty_source;
  assert(!empty_copy);

  nuostl::NuoFunction<int()> value_source = []()
  {
    return 1;
  };
  nuostl::NuoFunction<int()> empty_move_source;
  value_source = std::move(empty_move_source);
  assert(!value_source);
}

void TestNuoFunction::TestTraits()
{
  using Function = nuostl::NuoFunction<void()>;

  static_assert(std::is_default_constructible_v<Function>);
  static_assert(std::is_copy_constructible_v<Function>);
  static_assert(std::is_copy_assignable_v<Function>);
  static_assert(std::is_move_constructible_v<Function>);
  static_assert(std::is_move_assignable_v<Function>);
  static_assert(std::is_nothrow_move_constructible_v<Function>);
  static_assert(std::is_nothrow_move_assignable_v<Function>);
  static_assert(std::is_nothrow_destructible_v<Function>);
  static_assert(!std::is_convertible_v<Function, bool>);
  static_assert(std::is_constructible_v<bool, Function>);
  static_assert(!std::is_constructible_v<Function, MoveOnlyCallable>);
}

void TestNuoFunction::TestComparisonsWithNull()
{
  nuostl::NuoFunction<void()> function;
  assert(function == nullptr);
  assert(nullptr == function);
  assert(!(function != nullptr));
  assert(!(nullptr != function));

  function = []()
  {
  };
  assert(!(function == nullptr));
  assert(!(nullptr == function));
  assert(function != nullptr);
  assert(nullptr != function);
}

void TestNuoFunction::TestSuite()
{
  TestDefaultState();
  TestFunctionPointers();
  TestCallables();
  TestCopyAndMove();
  TestAssignment();
  TestSwap();
  TestTargets();
  TestArgumentsAndReturns();
  TestTraits();
  TestComparisonsWithNull();
}

} /* namespace test */
