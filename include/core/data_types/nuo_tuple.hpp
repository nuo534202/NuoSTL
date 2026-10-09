#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace nuostl
{

/* cppreference declaration: template< class... Types > class tuple;
   https://en.cppreference.com/w/cpp/utility/tuple.html
   Adapted to the NuoSTL name; elements are stored independently below. */
template <class... Types>
class NuoTuple;

namespace NuoTupleDetail
{

template <std::size_t Index, class Type>
struct Element
{
  constexpr Element() requires std::is_default_constructible_v<Type>
    : value()
  {
  }

  template <class Value>
  explicit constexpr Element(Value&& value)
    : value(std::forward<Value>(value))
  {
  }

  [[no_unique_address]] Type value;
};

template <class Indices, class... Types>
struct Storage;

template <std::size_t... Indices, class... Types>
struct Storage<std::index_sequence<Indices...>, Types...>
  : Element<Indices, Types>...
{
  constexpr Storage() requires (std::is_default_constructible_v<Types> && ...)
    : Element<Indices, Types>()...
  {
  }

  template <class... Values>
  explicit constexpr Storage(std::in_place_t, Values&&... values)
    : Element<Indices, Types>(std::forward<Values>(values))...
  {
  }
};

template <class Type>
struct IsTuple : std::false_type
{
};

template <class... Types>
struct IsTuple<NuoTuple<Types...>> : std::true_type
{
};

template <class Left, class Right>
constexpr auto Compare(const Left& left, const Right& right)
{
  if constexpr (std::three_way_comparable_with<Left, Right>)
  {
    return left <=> right;
  }
  else
  {
    if (left < right)
    {
      return std::weak_ordering::less;
    }
    if (right < left)
    {
      return std::weak_ordering::greater;
    }
    return std::weak_ordering::equivalent;
  }
}

} /* namespace NuoTupleDetail */

template <class... Types>
class NuoTuple
{
  using Storage = NuoTupleDetail::Storage<
    std::index_sequence_for<Types...>, Types...>;

  template <std::size_t Index>
  using ElementType = std::tuple_element_t<Index, std::tuple<Types...>>;

  template <class Type>
  static constexpr std::size_t TypeIndex()
  {
    static_assert((std::size_t{0} + ... + std::is_same_v<Type, Types>) == 1,
                  "Type must occur exactly once in NuoTuple");
    constexpr bool matches[] = {std::is_same_v<Type, Types>..., false};
    for (std::size_t index = 0; index < sizeof...(Types); ++index)
    {
      if (matches[index])
      {
        return index;
      }
    }
    return sizeof...(Types);
  }

  template <class Other, std::size_t... Indices>
  constexpr NuoTuple(std::index_sequence<Indices...>, Other&& other)
    : values_(std::in_place,
              std::forward<Other>(other).template Get<Indices>()...)
  {
  }

  template <class Other, std::size_t... Indices>
  constexpr void Assign(Other&& other, std::index_sequence<Indices...>)
  {
    ((Get<Indices>() =
      std::forward<Other>(other).template Get<Indices>()), ...);
  }

  template <std::size_t... Indices>
  constexpr void SwapElements(NuoTuple& other,
                             std::index_sequence<Indices...>)
  {
    using std::swap;
    (swap(Get<Indices>(), other.template Get<Indices>()), ...);
  }

public:
  constexpr NuoTuple() noexcept(
    (std::is_nothrow_default_constructible_v<Types> && ...))
    requires (std::is_default_constructible_v<Types> && ...)
    : values_()
  {
  }

  template <class... Values>
    requires (sizeof...(Values) == sizeof...(Types) &&
              sizeof...(Types) > 0 &&
              !((sizeof...(Values) == 1) &&
                (std::is_same_v<NuoTuple,
                                std::remove_cvref_t<Values>> && ...)) &&
              (std::is_constructible_v<Types, Values&&> && ...))
  explicit(!(std::is_convertible_v<Values&&, Types> && ...))
  constexpr NuoTuple(Values&&... values) noexcept(
    (std::is_nothrow_constructible_v<Types, Values&&> && ...))
    : values_(std::in_place, std::forward<Values>(values)...)
  {
  }

  explicit(!(std::is_convertible_v<const Types&, Types> && ...))
  constexpr NuoTuple(const Types&... values) noexcept(
    (std::is_nothrow_copy_constructible_v<Types> && ...))
    requires (sizeof...(Types) > 0 &&
              (std::is_copy_constructible_v<Types> && ...))
    : values_(std::in_place, values...)
  {
  }

  constexpr NuoTuple(const NuoTuple&) = default;
  constexpr NuoTuple(NuoTuple&&) = default;
  ~NuoTuple() = default;

  template <class... OtherTypes>
    requires (sizeof...(Types) == sizeof...(OtherTypes) &&
              (std::is_constructible_v<Types, const OtherTypes&> && ...) &&
              !(sizeof...(Types) == 1 &&
                (std::is_constructible_v<Types,
                                        const NuoTuple<OtherTypes...>&> || ...)))
  explicit(!(std::is_convertible_v<const OtherTypes&, Types> && ...))
  constexpr NuoTuple(const NuoTuple<OtherTypes...>& other) noexcept(
    (std::is_nothrow_constructible_v<Types, const OtherTypes&> && ...))
    : NuoTuple(std::index_sequence_for<Types...>{}, other)
  {
  }

  template <class... OtherTypes>
    requires (sizeof...(Types) == sizeof...(OtherTypes) &&
              (std::is_constructible_v<Types, OtherTypes&&> && ...) &&
              !(sizeof...(Types) == 1 &&
                (std::is_constructible_v<Types,
                                        NuoTuple<OtherTypes...>&&> || ...)))
  explicit(!(std::is_convertible_v<OtherTypes&&, Types> && ...))
  constexpr NuoTuple(NuoTuple<OtherTypes...>&& other) noexcept(
    (std::is_nothrow_constructible_v<Types, OtherTypes&&> && ...))
    : NuoTuple(std::index_sequence_for<Types...>{}, std::move(other))
  {
  }

  constexpr NuoTuple& operator=(const NuoTuple& other)
    requires (std::is_assignable_v<Types&, const Types&> && ...)
  {
    Assign(other, std::index_sequence_for<Types...>{});
    return *this;
  }

  constexpr NuoTuple& operator=(NuoTuple&& other) noexcept(
    (std::is_nothrow_assignable_v<Types&, Types&&> && ...))
    requires (std::is_assignable_v<Types&, Types&&> && ...)
  {
    Assign(std::move(other), std::index_sequence_for<Types...>{});
    return *this;
  }

  template <class... OtherTypes>
    requires (sizeof...(Types) == sizeof...(OtherTypes) &&
              (std::is_assignable_v<Types&, const OtherTypes&> && ...))
  constexpr NuoTuple& operator=(const NuoTuple<OtherTypes...>& other)
  {
    Assign(other, std::index_sequence_for<Types...>{});
    return *this;
  }

  template <class... OtherTypes>
    requires (sizeof...(Types) == sizeof...(OtherTypes) &&
              (std::is_assignable_v<Types&, OtherTypes&&> && ...))
  constexpr NuoTuple& operator=(NuoTuple<OtherTypes...>&& other) noexcept(
    (std::is_nothrow_assignable_v<Types&, OtherTypes&&> && ...))
  {
    Assign(std::move(other), std::index_sequence_for<Types...>{});
    return *this;
  }

  template <std::size_t Index>
  constexpr ElementType<Index>& Get() & noexcept
  {
    return static_cast<NuoTupleDetail::Element<Index, ElementType<Index>>&>(
      values_).value;
  }

  template <std::size_t Index>
  constexpr const ElementType<Index>& Get() const& noexcept
  {
    return static_cast<const NuoTupleDetail::Element<
      Index, ElementType<Index>>&>(values_).value;
  }

  template <std::size_t Index>
  constexpr ElementType<Index>&& Get() && noexcept
  {
    return std::forward<ElementType<Index>>(Get<Index>());
  }

  template <std::size_t Index>
  constexpr const ElementType<Index>&& Get() const&& noexcept
  {
    return std::forward<const ElementType<Index>>(Get<Index>());
  }

  template <class Type>
  constexpr decltype(auto) Get() & noexcept
  {
    return Get<TypeIndex<Type>()>();
  }

  template <class Type>
  constexpr decltype(auto) Get() const& noexcept
  {
    return Get<TypeIndex<Type>()>();
  }

  template <class Type>
  constexpr decltype(auto) Get() && noexcept
  {
    return std::move(*this).template Get<TypeIndex<Type>()>();
  }

  template <class Type>
  constexpr decltype(auto) Get() const&& noexcept
  {
    return std::move(*this).template Get<TypeIndex<Type>()>();
  }

  /* The lowercase name is required by the structured binding protocol. */
  template <std::size_t Index>
  constexpr decltype(auto) get() & noexcept
  {
    return Get<Index>();
  }

  template <std::size_t Index>
  constexpr decltype(auto) get() const& noexcept
  {
    return Get<Index>();
  }

  template <std::size_t Index>
  constexpr decltype(auto) get() && noexcept
  {
    return std::move(*this).template Get<Index>();
  }

  template <std::size_t Index>
  constexpr decltype(auto) get() const&& noexcept
  {
    return std::move(*this).template Get<Index>();
  }

  constexpr void Swap(NuoTuple& other) noexcept(
    (std::is_nothrow_swappable_v<Types> && ...))
    requires (std::is_swappable_v<Types> && ...)
  {
    SwapElements(other, std::index_sequence_for<Types...>{});
  }

private:
  Storage values_;
};

template <class... Types>
NuoTuple(Types...) -> NuoTuple<Types...>;

template <std::size_t Index, class Tuple>
  requires NuoTupleDetail::IsTuple<std::remove_cvref_t<Tuple>>::value
constexpr decltype(auto) NuoGet(Tuple&& tuple) noexcept
{
  return std::forward<Tuple>(tuple).template Get<Index>();
}

template <class Type, class Tuple>
  requires NuoTupleDetail::IsTuple<std::remove_cvref_t<Tuple>>::value
constexpr decltype(auto) NuoGet(Tuple&& tuple) noexcept
{
  return std::forward<Tuple>(tuple).template Get<Type>();
}

template <class... Types>
constexpr auto NuoMakeTuple(Types&&... values)
{
  return NuoTuple<std::unwrap_ref_decay_t<Types>...>(
    std::forward<Types>(values)...);
}

template <class... Types>
constexpr auto NuoTie(Types&... values) noexcept
{
  return NuoTuple<Types&...>(values...);
}

template <class... Types>
constexpr auto NuoForwardAsTuple(Types&&... values) noexcept
{
  return NuoTuple<Types&&...>(std::forward<Types>(values)...);
}

namespace NuoTupleDetail
{

/* Map each flat output index to its source tuple without intermediate values. */
template <std::size_t Index, class First, class... Rest>
constexpr decltype(auto) CatGet(First&& first, Rest&&... rest)
{
  constexpr std::size_t size = std::tuple_size_v<std::remove_cvref_t<First>>;
  if constexpr (Index < size)
  {
    return std::forward<First>(first).template Get<Index>();
  }
  else
  {
    return CatGet<Index - size>(std::forward<Rest>(rest)...);
  }
}

template <std::size_t Index, class First, class... Rest>
constexpr auto CatElement()
{
  constexpr std::size_t size = std::tuple_size_v<std::remove_cvref_t<First>>;
  if constexpr (Index < size)
  {
    return std::type_identity<
      std::tuple_element_t<Index, std::remove_cvref_t<First>>>{};
  }
  else
  {
    return CatElement<Index - size, Rest...>();
  }
}

template <std::size_t... Indices, class... Tuples>
constexpr auto Cat(std::index_sequence<Indices...>, Tuples&&... tuples)
{
  using Result = NuoTuple<
    typename decltype(CatElement<Indices, Tuples...>())::type...>;
  return Result(CatGet<Indices>(std::forward<Tuples>(tuples)...)...);
}

} /* namespace NuoTupleDetail */

template <class... Tuples>
  requires (NuoTupleDetail::IsTuple<std::remove_cvref_t<Tuples>>::value && ...)
constexpr auto NuoTupleCat(Tuples&&... tuples)
{
  constexpr std::size_t size =
    (std::size_t{0} + ... + std::tuple_size_v<std::remove_cvref_t<Tuples>>);
  return NuoTupleDetail::Cat(std::make_index_sequence<size>{},
                              std::forward<Tuples>(tuples)...);
}

template <class Function, class Tuple, std::size_t... Indices>
constexpr decltype(auto) NuoApplyImpl(Function&& function, Tuple&& tuple,
                                      std::index_sequence<Indices...>)
{
  return std::invoke(std::forward<Function>(function),
                     std::forward<Tuple>(tuple).template Get<Indices>()...);
}

template <class Function, class Tuple>
  requires NuoTupleDetail::IsTuple<std::remove_cvref_t<Tuple>>::value
constexpr decltype(auto) NuoApply(Function&& function, Tuple&& tuple)
{
  return NuoApplyImpl(
    std::forward<Function>(function), std::forward<Tuple>(tuple),
    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
}

template <class Type, class Tuple, std::size_t... Indices>
constexpr Type NuoMakeFromTupleImpl(Tuple&& tuple,
                                    std::index_sequence<Indices...>)
{
  return Type(std::forward<Tuple>(tuple).template Get<Indices>()...);
}

template <class Type, class Tuple>
  requires NuoTupleDetail::IsTuple<std::remove_cvref_t<Tuple>>::value
constexpr Type NuoMakeFromTuple(Tuple&& tuple)
{
  return NuoMakeFromTupleImpl<Type>(
    std::forward<Tuple>(tuple),
    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
}

namespace NuoTupleDetail
{

template <class Left, class Right, std::size_t... Indices>
constexpr bool Equal(const Left& left, const Right& right,
                     std::index_sequence<Indices...>)
{
  return ((left.template Get<Indices>() == right.template Get<Indices>()) && ...);
}

template <class Category, std::size_t Index, class Left, class Right>
constexpr Category CompareTuples(const Left& left, const Right& right)
{
  if constexpr (Index == std::tuple_size_v<std::remove_cvref_t<Left>>)
  {
    return Category::equivalent;
  }
  else
  {
    const Category result = Compare(left.template Get<Index>(),
                                    right.template Get<Index>());
    if (result != 0)
    {
      return result;
    }
    return CompareTuples<Category, Index + 1>(left, right);
  }
}

template <class Left, class Right, std::size_t... Indices>
constexpr auto CompareTuples(const Left& left, const Right& right,
                             std::index_sequence<Indices...>)
{
  using Category = std::common_comparison_category_t<
    decltype(Compare(left.template Get<Indices>(),
                     right.template Get<Indices>()))...>;
  return CompareTuples<Category, 0>(left, right);
}

} /* namespace NuoTupleDetail */

template <class... Left, class... Right>
  requires (sizeof...(Left) == sizeof...(Right))
constexpr bool operator==(const NuoTuple<Left...>& left,
                          const NuoTuple<Right...>& right)
{
  return NuoTupleDetail::Equal(left, right, std::index_sequence_for<Left...>{});
}

template <class... Left, class... Right>
  requires (sizeof...(Left) == sizeof...(Right))
constexpr auto operator<=>(const NuoTuple<Left...>& left,
                           const NuoTuple<Right...>& right)
{
  if constexpr (sizeof...(Left) == 0)
  {
    return std::strong_ordering::equal;
  }
  else
  {
    return NuoTupleDetail::CompareTuples(
      left, right, std::index_sequence_for<Left...>{});
  }
}

template <class... Types>
constexpr void NuoSwap(NuoTuple<Types...>& left, NuoTuple<Types...>& right)
  noexcept(noexcept(left.Swap(right)))
  requires (std::is_swappable_v<Types> && ...)
{
  left.Swap(right);
}

/* Enable unqualified swap through argument-dependent lookup. */
template <class... Types>
constexpr void swap(NuoTuple<Types...>& left, NuoTuple<Types...>& right)
  noexcept(noexcept(left.Swap(right)))
  requires (std::is_swappable_v<Types> && ...)
{
  left.Swap(right);
}

} /* namespace nuostl */

namespace std
{

template <class... Types>
struct tuple_size<nuostl::NuoTuple<Types...>>
  : integral_constant<size_t, sizeof...(Types)>
{
};

template <size_t Index, class... Types>
struct tuple_element<Index, nuostl::NuoTuple<Types...>>
  : tuple_element<Index, tuple<Types...>>
{
};

} /* namespace std */
