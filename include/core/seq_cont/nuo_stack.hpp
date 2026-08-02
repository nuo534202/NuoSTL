#pragma once

#include "core/seq_cont/nuo_deque.hpp"
#include "utils/nuo_algorithm.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_util.hpp"

namespace nuostl
{

/*****************************************************************************/
/* stack class definition */
template <typename T, typename Container = NuoDeque<T>>
class NuoStack
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
  constexpr NuoStack();
  constexpr explicit NuoStack(const Container& container);
  constexpr explicit NuoStack(Container&& container);
  template <typename InputIter, typename std::enable_if<
    NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
  constexpr NuoStack(InputIter first, InputIter last);
  constexpr NuoStack(const NuoStack& rhs);
  constexpr NuoStack(NuoStack&& rhs);
  constexpr NuoStack& operator=(const NuoStack& rhs);
  constexpr NuoStack& operator=(NuoStack&& rhs);
  constexpr ~NuoStack() = default;

  /* capacity */
  constexpr bool Empty() const noexcept;
  constexpr size_type Size() const noexcept;

  /* element access */
  constexpr reference Top();
  constexpr const_reference Top() const;

  /* modifiers */
  constexpr void Push(const value_type& value);
  constexpr void Push(value_type&& value);
  template <typename... Args>
  constexpr reference Emplace(Args&&... args);
  constexpr void Pop();

  /* swap */
  constexpr void Swap(NuoStack& rhs) noexcept;

  /* comparison operators (friend functions) */
  template <typename T1, typename C1>
  friend bool operator==(const NuoStack<T1, C1>& lhs,
                         const NuoStack<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator!=(const NuoStack<T1, C1>& lhs,
                         const NuoStack<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator<(const NuoStack<T1, C1>& lhs,
                        const NuoStack<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator>(const NuoStack<T1, C1>& lhs,
                        const NuoStack<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator<=(const NuoStack<T1, C1>& lhs,
                         const NuoStack<T1, C1>& rhs);
  template <typename T1, typename C1>
  friend bool operator>=(const NuoStack<T1, C1>& lhs,
                         const NuoStack<T1, C1>& rhs);
};

/* overload NuoSwap */
template <typename T, typename Container>
void NuoSwap(NuoStack<T, Container>& lhs, NuoStack<T, Container>& rhs) noexcept
{
  lhs.Swap(rhs);
}

/* overload comparison operators */
template <typename T, typename Container>
bool operator==(const NuoStack<T, Container>& lhs,
                const NuoStack<T, Container>& rhs)
{
  return lhs.Size() == rhs.Size() &&
    NuoEqual(lhs.c.Begin(), lhs.c.End(), rhs.c.Begin());
}

template <typename T, typename Container>
bool operator!=(const NuoStack<T, Container>& lhs,
                const NuoStack<T, Container>& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Container>
bool operator<(const NuoStack<T, Container>& lhs,
               const NuoStack<T, Container>& rhs)
{
  return NuoLexicographicalCompare(
    lhs.c.Begin(), lhs.c.End(), rhs.c.Begin(), rhs.c.End());
}

template <typename T, typename Container>
bool operator>(const NuoStack<T, Container>& lhs,
               const NuoStack<T, Container>& rhs)
{
  return rhs < lhs;
}

template <typename T, typename Container>
bool operator<=(const NuoStack<T, Container>& lhs,
                const NuoStack<T, Container>& rhs)
{
  return !(rhs < lhs);
}

template <typename T, typename Container>
bool operator>=(const NuoStack<T, Container>& lhs,
                const NuoStack<T, Container>& rhs)
{
  return !(lhs < rhs);
}

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
template <typename Container>
NuoStack(Container) -> NuoStack<typename Container::value_type, Container>;

template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type = 0>
NuoStack(InputIter, InputIter)
  -> NuoStack<typename NuoIteratorTraits<InputIter>::value_type>;
#endif

/*****************************************************************************/
/* stack class implementation */
template <typename T, typename Container>
constexpr NuoStack<T, Container>::NuoStack()
  : c()
{
}

template <typename T, typename Container>
constexpr NuoStack<T, Container>::NuoStack(const Container& container)
  : c(container)
{
}

template <typename T, typename Container>
constexpr NuoStack<T, Container>::NuoStack(Container&& container)
  : c(NuoMove(container))
{
}

template <typename T, typename Container>
template <typename InputIter, typename std::enable_if<
  NuoIsInputIterator<InputIter>::kValue, int>::type>
constexpr NuoStack<T, Container>::NuoStack(InputIter first, InputIter last)
  : c(first, last)
{
}

template <typename T, typename Container>
constexpr NuoStack<T, Container>::NuoStack(const NuoStack& rhs)
  : c(rhs.c)
{
}

template <typename T, typename Container>
constexpr NuoStack<T, Container>::NuoStack(NuoStack&& rhs)
  : c(NuoMove(rhs.c))
{
}

template <typename T, typename Container>
constexpr NuoStack<T, Container>&
NuoStack<T, Container>::operator=(const NuoStack& rhs)
{
  c = rhs.c;
  return *this;
}

template <typename T, typename Container>
constexpr NuoStack<T, Container>&
NuoStack<T, Container>::operator=(NuoStack&& rhs)
{
  c = NuoMove(rhs.c);
  return *this;
}

template <typename T, typename Container>
constexpr bool NuoStack<T, Container>::Empty() const noexcept
{
  return c.Empty();
}

template <typename T, typename Container>
constexpr typename NuoStack<T, Container>::size_type
NuoStack<T, Container>::Size() const noexcept
{
  return c.Size();
}

template <typename T, typename Container>
constexpr typename NuoStack<T, Container>::reference
NuoStack<T, Container>::Top()
{
  return c.Back();
}

template <typename T, typename Container>
constexpr typename NuoStack<T, Container>::const_reference
NuoStack<T, Container>::Top() const
{
  return c.Back();
}

template <typename T, typename Container>
constexpr void NuoStack<T, Container>::Push(const value_type& value)
{
  c.push_back(value);
}

template <typename T, typename Container>
constexpr void NuoStack<T, Container>::Push(value_type&& value)
{
  c.push_back(NuoMove(value));
}

template <typename T, typename Container>
template <typename... Args>
constexpr typename NuoStack<T, Container>::reference
NuoStack<T, Container>::Emplace(Args&&... args)
{
  return c.EmplaceBack(NuoForward<Args>(args)...);
}

template <typename T, typename Container>
constexpr void NuoStack<T, Container>::Pop()
{
  c.PopBack();
}

template <typename T, typename Container>
constexpr void NuoStack<T, Container>::Swap(NuoStack& rhs) noexcept
{
  NuoSwap(c, rhs.c);
}

} /* namespace nuostl */