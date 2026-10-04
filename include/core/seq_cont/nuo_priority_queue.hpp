#pragma once

#include <functional>
#include <type_traits>
#include <vector>

#include "core/algorithms/nuo_heap.hpp"
#include "core/seq_cont/nuo_deque.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

namespace NuoPriorityQueueDetail
{

template <typename Container>
auto Begin(Container& container)
{
  if constexpr (requires { container.Begin(); })
  {
    return container.Begin();
  }
  else
  {
    return container.begin();
  }
}

template <typename Container>
auto End(Container& container)
{
  if constexpr (requires { container.End(); })
  {
    return container.End();
  }
  else
  {
    return container.end();
  }
}

template <typename Container>
decltype(auto) Front(Container& container)
{
  if constexpr (requires { container.Front(); })
  {
    return container.Front();
  }
  else
  {
    return container.front();
  }
}

template <typename Container, typename Value>
void PushBack(Container& container, Value&& value)
{
  container.push_back(NuoForward<Value>(value));
}

template <typename Container, typename... Args>
decltype(auto) EmplaceBack(Container& container, Args&&... args)
{
  if constexpr (requires { container.EmplaceBack(NuoForward<Args>(args)...); })
  {
    return container.EmplaceBack(NuoForward<Args>(args)...);
  }
  else
  {
    return container.emplace_back(NuoForward<Args>(args)...);
  }
}

template <typename Container>
void PopBack(Container& container)
{
  if constexpr (requires { container.PopBack(); })
  {
    container.PopBack();
  }
  else
  {
    container.pop_back();
  }
}

template <typename Container>
auto Size(const Container& container)
{
  if constexpr (requires { container.Size(); })
  {
    return container.Size();
  }
  else
  {
    return container.size();
  }
}

template <typename Container>
bool Empty(const Container& container)
{
  if constexpr (requires { container.Empty(); })
  {
    return container.Empty();
  }
  else
  {
    return container.empty();
  }
}

template <typename Container>
constexpr bool IsNothrowSwappable()
{
  if constexpr (requires(Container& left, Container& right)
                {
                  left.Swap(right);
                })
  {
    return noexcept(std::declval<Container&>().Swap(
      std::declval<Container&>()));
  }
  else
  {
    return std::is_nothrow_swappable_v<Container>;
  }
}

template <typename Container>
void Swap(Container& left, Container& right) noexcept(
  IsNothrowSwappable<Container>())
{
  if constexpr (requires { left.Swap(right); })
  {
    left.Swap(right);
  }
  else
  {
    using std::swap;
    swap(left, right);
  }
}

} /* namespace NuoPriorityQueueDetail */

template <typename T,
          typename Container = std::vector<T>,
          typename Compare = std::less<typename Container::value_type>>
class NuoPriorityQueue
{
public:
  using value_type = typename Container::value_type;
  using container_type = Container;
  using value_compare = Compare;
  using size_type = typename Container::size_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;

  static_assert(std::is_same_v<T, value_type>,
                "NuoPriorityQueue value type must match container value_type");

  NuoPriorityQueue() = default;

  explicit NuoPriorityQueue(const Compare& compare)
    : c(), comp(compare)
  {
  }

  explicit NuoPriorityQueue(const Container& container)
    : c(container), comp()
  {
    MakeHeap();
  }

  explicit NuoPriorityQueue(Container&& container)
    : c(NuoMove(container)), comp()
  {
    MakeHeap();
  }

  NuoPriorityQueue(const Compare& compare, const Container& container)
    : c(container), comp(compare)
  {
    MakeHeap();
  }

  NuoPriorityQueue(const Compare& compare, Container&& container)
    : c(NuoMove(container)), comp(compare)
  {
    MakeHeap();
  }

  template <typename InputIterator>
  NuoPriorityQueue(InputIterator first, InputIterator last,
                   const Compare& compare = Compare(),
                   const Container& container = Container())
    : c(container), comp(compare)
  {
    for (; first != last; ++first)
    {
      NuoPriorityQueueDetail::PushBack(c, *first);
    }
    MakeHeap();
  }

  template <typename InputIterator>
  NuoPriorityQueue(InputIterator first, InputIterator last,
                   const Compare& compare, Container&& container)
    : c(NuoMove(container)), comp(compare)
  {
    for (; first != last; ++first)
    {
      NuoPriorityQueueDetail::PushBack(c, *first);
    }
    MakeHeap();
  }

  bool Empty() const noexcept
  {
    return NuoPriorityQueueDetail::Empty(c);
  }

  size_type Size() const noexcept
  {
    return NuoPriorityQueueDetail::Size(c);
  }

  const_reference Top() const
  {
    return NuoPriorityQueueDetail::Front(c);
  }

  void Push(const value_type& value)
  {
    NuoPriorityQueueDetail::PushBack(c, value);
    NuoPushHeap(NuoPriorityQueueDetail::Begin(c), NuoPriorityQueueDetail::End(c),
                comp);
  }

  void Push(value_type&& value)
  {
    NuoPriorityQueueDetail::PushBack(c, NuoMove(value));
    NuoPushHeap(NuoPriorityQueueDetail::Begin(c), NuoPriorityQueueDetail::End(c),
                comp);
  }

  template <typename... Args>
  void Emplace(Args&&... args)
  {
    NuoPriorityQueueDetail::EmplaceBack(c, NuoForward<Args>(args)...);
    NuoPushHeap(NuoPriorityQueueDetail::Begin(c), NuoPriorityQueueDetail::End(c),
                comp);
  }

  void Pop()
  {
    NuoPopHeap(NuoPriorityQueueDetail::Begin(c), NuoPriorityQueueDetail::End(c),
               comp);
    NuoPriorityQueueDetail::PopBack(c);
  }

  void Swap(NuoPriorityQueue& other) noexcept(
    NuoPriorityQueueDetail::IsNothrowSwappable<Container>() &&
    std::is_nothrow_swappable_v<Compare>)
  {
    NuoPriorityQueueDetail::Swap(c, other.c);
    using std::swap;
    swap(comp, other.comp);
  }

protected:
  Container c;
  Compare comp;

private:
  void MakeHeap()
  {
    NuoMakeHeap(NuoPriorityQueueDetail::Begin(c), NuoPriorityQueueDetail::End(c),
                comp);
  }
};

template <typename T, typename Container, typename Compare>
void NuoSwap(NuoPriorityQueue<T, Container, Compare>& left,
             NuoPriorityQueue<T, Container, Compare>& right) noexcept(
  noexcept(left.Swap(right)))
{
  left.Swap(right);
}

} /* namespace nuostl */
