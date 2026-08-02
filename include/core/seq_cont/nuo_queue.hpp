#pragma once

#include "core/seq_cont/nuo_deque.hpp"
#include "utils/nuo_algorithm.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

/*****************************************************************************/
/* queue class definition */
template <typename T, typename Container = NuoDeque<T>>
class NuoQueue
{
public:
  /* types */
  using value_type      = typename Container::value_type;
  using reference       = typename Container::reference;
  using const_reference = typename Container::const_reference;
  using size_type       = typename Container::size_type;
  using container_type  = Container;

protected:
  Container c; /* underlying container */

public:
  /* construct / copy / destroy */
  constexpr NuoQueue();
  constexpr explicit NuoQueue(const Container& container);
  constexpr explicit NuoQueue(Container&& container);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
  constexpr NuoQueue(InputIter first, InputIter last);
  constexpr NuoQueue(const NuoQueue& rhs);
  constexpr NuoQueue(NuoQueue&& rhs);
  constexpr NuoQueue& operator=(const NuoQueue& rhs);
  constexpr NuoQueue& operator=(NuoQueue&& rhs);
  constexpr ~NuoQueue() = default;

  /* capacity */
  constexpr bool Empty() const noexcept;
  constexpr size_type Size() const noexcept;

  /* element access */
  constexpr reference Front();
  constexpr const_reference Front() const;
  constexpr reference Back();
  constexpr const_reference Back() const;

  /* modifiers */
  constexpr void Push(const value_type& value);
  constexpr void Push(value_type&& value);
  template <typename... Args>
  constexpr reference Emplace(Args&&... args);
  constexpr void Pop();

  /* swap */
  constexpr void Swap(NuoQueue& rhs) noexcept;

  /* comparison operators (friend functions) */
  template <typename T1, typename C1>
  friend bool operator==(const NuoQueue<T1, C1>& lhs,
                         const NuoQueue<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator!=(const NuoQueue<T1, C1>& lhs,
                         const NuoQueue<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator<(const NuoQueue<T1, C1>& lhs,
                        const NuoQueue<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator>(const NuoQueue<T1, C1>& lhs,
                        const NuoQueue<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator<=(const NuoQueue<T1, C1>& lhs,
                         const NuoQueue<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator>=(const NuoQueue<T1, C1>& lhs,
                         const NuoQueue<T1, C1>& rhs);
};

/* overload NuoSwap */
template <typename T, typename Container>
void NuoSwap(NuoQueue<T, Container>& lhs, NuoQueue<T, Container>& rhs) noexcept
{
  lhs.Swap(rhs);
}

/* overload comparison operators */
template <typename T, typename Container>
bool operator==(const NuoQueue<T, Container>& lhs,
                const NuoQueue<T, Container>& rhs)
{
  return lhs.Size() == rhs.Size() &&
    NuoEqual(lhs.c.Begin(), lhs.c.End(), rhs.c.Begin());
}

template <typename T, typename Container>
bool operator!=(const NuoQueue<T, Container>& lhs,
                const NuoQueue<T, Container>& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Container>
bool operator<(const NuoQueue<T, Container>& lhs,
               const NuoQueue<T, Container>& rhs)
{
  return NuoLexicographicalCompare(
    lhs.c.Begin(), lhs.c.End(), rhs.c.Begin(), rhs.c.End());
}

template <typename T, typename Container>
bool operator>(const NuoQueue<T, Container>& lhs,
               const NuoQueue<T, Container>& rhs)
{
  return rhs < lhs;
}

template <typename T, typename Container>
bool operator<=(const NuoQueue<T, Container>& lhs,
                const NuoQueue<T, Container>& rhs)
{
  return !(rhs < lhs);
}

template <typename T, typename Container>
bool operator>=(const NuoQueue<T, Container>& lhs,
                const NuoQueue<T, Container>& rhs)
{
  return !(lhs < rhs);
}

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
template <typename Container>
NuoQueue(Container) -> NuoQueue<typename Container::value_type, Container>;

template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
NuoQueue(InputIter, InputIter)
  -> NuoQueue<typename NuoIteratorTraits<InputIter>::value_type>;
#endif

/*****************************************************************************/
/* queue class implementation */
template <typename T, typename Container>
constexpr NuoQueue<T, Container>::NuoQueue()
  : c()
{
}

template <typename T, typename Container>
constexpr NuoQueue<T, Container>::NuoQueue(const Container& container)
  : c(container)
{
}

template <typename T, typename Container>
constexpr NuoQueue<T, Container>::NuoQueue(Container&& container)
  : c(NuoMove(container))
{
}

template <typename T, typename Container>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type>
constexpr NuoQueue<T, Container>::NuoQueue(InputIter first, InputIter last)
  : c(first, last)
{
}

template <typename T, typename Container>
constexpr NuoQueue<T, Container>::NuoQueue(const NuoQueue& rhs)
  : c(rhs.c)
{
}

template <typename T, typename Container>
constexpr NuoQueue<T, Container>::NuoQueue(NuoQueue&& rhs)
  : c(NuoMove(rhs.c))
{
}

template <typename T, typename Container>
constexpr NuoQueue<T, Container>&
NuoQueue<T, Container>::operator=(const NuoQueue& rhs)
{
  c = rhs.c;
  return *this;
}

template <typename T, typename Container>
constexpr NuoQueue<T, Container>&
NuoQueue<T, Container>::operator=(NuoQueue&& rhs)
{
  c = NuoMove(rhs.c);
  return *this;
}

template <typename T, typename Container>
constexpr bool NuoQueue<T, Container>::Empty() const noexcept
{
  return c.Empty();
}

template <typename T, typename Container>
constexpr typename NuoQueue<T, Container>::size_type
NuoQueue<T, Container>::Size() const noexcept
{
  return c.Size();
}

template <typename T, typename Container>
constexpr typename NuoQueue<T, Container>::reference
NuoQueue<T, Container>::Front()
{
  return c.Front();
}

template <typename T, typename Container>
constexpr typename NuoQueue<T, Container>::const_reference
NuoQueue<T, Container>::Front() const
{
  return c.Front();
}

template <typename T, typename Container>
constexpr typename NuoQueue<T, Container>::reference
NuoQueue<T, Container>::Back()
{
  return c.Back();
}

template <typename T, typename Container>
constexpr typename NuoQueue<T, Container>::const_reference
NuoQueue<T, Container>::Back() const
{
  return c.Back();
}

template <typename T, typename Container>
constexpr void NuoQueue<T, Container>::Push(const value_type& value)
{
  c.push_back(value);
}

template <typename T, typename Container>
constexpr void NuoQueue<T, Container>::Push(value_type&& value)
{
  c.push_back(NuoMove(value));
}

template <typename T, typename Container>
template <typename... Args>
constexpr typename NuoQueue<T, Container>::reference
NuoQueue<T, Container>::Emplace(Args&&... args)
{
  return c.EmplaceBack(NuoForward<Args>(args)...);
}

template <typename T, typename Container>
constexpr void NuoQueue<T, Container>::Pop()
{
  c.PopFront();
}

template <typename T, typename Container>
constexpr void NuoQueue<T, Container>::Swap(NuoQueue& rhs) noexcept
{
  NuoSwap(c, rhs.c);
}

} /* namespace nuostl */
