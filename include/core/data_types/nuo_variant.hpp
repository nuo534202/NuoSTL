#pragma once

#include <compare>
#include <cstddef>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace nuostl
{

/* cppreference declaration: template< class... Types > class variant;
   https://en.cppreference.com/w/cpp/utility/variant.html
   Adapted to NuoSTL naming and an independently managed union below. */
template <class... Types>
class NuoVariant;

/* cppreference declaration: struct monostate {};
   https://en.cppreference.com/w/cpp/utility/variant/monostate.html */
struct NuoMonostate
{
  constexpr auto operator<=>(const NuoMonostate&) const = default;
};

/* cppreference declaration: class bad_variant_access : public std::exception;
   https://en.cppreference.com/w/cpp/utility/variant/bad_variant_access.html */
class NuoBadVariantAccess : public std::exception
{
public:
  const char* what() const noexcept override;
};

inline constexpr std::size_t NuoVariantNpos = static_cast<std::size_t>(-1);

namespace NuoVariantDetail
{

template <class T, class... Types>
inline constexpr std::size_t kCount = (std::size_t{0} + ... +
                                      std::size_t{std::is_same_v<T, Types>});

template <class T, class... Types>
consteval std::size_t TypeIndex()
{
  static_assert(kCount<T, Types...> == 1,
                "The requested type must occur exactly once");
  const bool matches[] = {std::is_same_v<T, Types>...};
  for (std::size_t index = 0; index < sizeof...(Types); ++index)
  {
    if (matches[index])
    {
      return index;
    }
  }
  return NuoVariantNpos;
}

template <class T>
struct ArrayProbe
{
  T values[1];
};

/* Only non-narrowing implicit conversions participate in overload resolution. */
template <std::size_t Index, class T, class Source>
struct Candidate
{
  static void Select();
};

template <std::size_t Index, class T, class Source>
  requires requires { ArrayProbe<T>{{std::declval<Source>()}}; }
struct Candidate<Index, T, Source>
{
  static std::integral_constant<std::size_t, Index> Select(T);
};

template <class Source, class Indices, class... Types>
struct Candidates;

template <class Source, std::size_t... Indices, class... Types>
struct Candidates<Source, std::index_sequence<Indices...>, Types...>
  : Candidate<Indices, Types, Source>...
{
  using Candidate<Indices, Types, Source>::Select...;
};

template <class Source, class... Types>
using Selected = decltype(Candidates<Source, std::index_sequence_for<Types...>,
                                    Types...>::Select(std::declval<Source>()));

template <class... Types>
union Storage;

template <>
union Storage<>
{
  unsigned char empty;
  constexpr Storage() : empty(0)
  {
  }
};

template <class First, class... Rest>
union Storage<First, Rest...>
{
  unsigned char empty;
  First value;
  Storage<Rest...> rest;

  constexpr Storage() : empty(0)
  {
  }

  constexpr ~Storage()
    requires (std::is_trivially_destructible_v<First> &&
              (std::is_trivially_destructible_v<Rest> && ...)) = default;

  constexpr ~Storage()
    requires (!(std::is_trivially_destructible_v<First> &&
                (std::is_trivially_destructible_v<Rest> && ...)))
  {
  }

  template <std::size_t Index, class... Args>
  constexpr decltype(auto) Construct(Args&&... args)
  {
    if constexpr (Index == 0)
    {
      return *std::construct_at(std::addressof(value),
                                std::forward<Args>(args)...);
    }
    else
    {
      std::construct_at(std::addressof(rest));
      return rest.template Construct<Index - 1>(std::forward<Args>(args)...);
    }
  }

  template <std::size_t Index>
  constexpr decltype(auto) Get() &
  {
    if constexpr (Index == 0)
    {
      return (value);
    }
    else
    {
      return rest.template Get<Index - 1>();
    }
  }

  template <std::size_t Index>
  constexpr decltype(auto) Get() const&
  {
    if constexpr (Index == 0)
    {
      return (value);
    }
    else
    {
      return rest.template Get<Index - 1>();
    }
  }
};

/* Dispatch by index rather than type so duplicate alternatives remain distinct. */
template <class Result, std::size_t Count, std::size_t Index = 0, class Function>
constexpr Result Dispatch(std::size_t active, Function&& function)
{
  if (active == Index)
  {
    return std::forward<Function>(function).template operator()<Index>();
  }
  if constexpr (Index + 1 < Count)
  {
    return Dispatch<Result, Count, Index + 1>(active,
                                              std::forward<Function>(function));
  }
  else
  {
    throw NuoBadVariantAccess();
  }
}

struct Access;

} /* namespace NuoVariantDetail */

template <class... Types>
class NuoVariant
{
  static_assert(sizeof...(Types) > 0, "NuoVariant requires an alternative");
  static_assert((std::is_object_v<Types> && ...), "Alternatives must be objects");
  static_assert((!std::is_array_v<Types> && ...), "Arrays are not alternatives");
  static_assert((std::is_destructible_v<Types> && ...),
                "Alternatives must be destructible");

  template <std::size_t Index>
  using Alternative = std::tuple_element_t<Index, std::tuple<Types...>>;

public:
  static constexpr std::size_t Size = sizeof...(Types);

  constexpr NuoVariant() noexcept(std::is_nothrow_default_constructible_v<
                                   Alternative<0>>)
    requires std::is_default_constructible_v<Alternative<0>>
  {
    Construct<0>();
  }

  constexpr NuoVariant(const NuoVariant&)
    requires ((std::is_trivially_copy_constructible_v<Types> && ...) &&
              (std::is_trivially_destructible_v<Types> && ...)) = default;

  constexpr NuoVariant(const NuoVariant& other)
    requires ((std::is_copy_constructible_v<Types> && ...) &&
              !((std::is_trivially_copy_constructible_v<Types> && ...) &&
                (std::is_trivially_destructible_v<Types> && ...)))
  {
    if (!other.ValuelessByException())
    {
      Dispatch(other.index_, [this, &other]<std::size_t Index>()
      {
        Construct<Index>(other.storage_.template Get<Index>());
      });
    }
  }

  constexpr NuoVariant(const NuoVariant&)
    requires (!(std::is_copy_constructible_v<Types> && ...)) = delete;

  constexpr NuoVariant(NuoVariant&&)
    requires ((std::is_trivially_move_constructible_v<Types> && ...) &&
              (std::is_trivially_destructible_v<Types> && ...)) = default;

  constexpr NuoVariant(NuoVariant&& other)
    noexcept((std::is_nothrow_move_constructible_v<Types> && ...))
    requires ((std::is_move_constructible_v<Types> && ...) &&
              !((std::is_trivially_move_constructible_v<Types> && ...) &&
                (std::is_trivially_destructible_v<Types> && ...)))
  {
    if (!other.ValuelessByException())
    {
      Dispatch(other.index_, [this, &other]<std::size_t Index>()
      {
        Construct<Index>(std::move(other.storage_.template Get<Index>()));
      });
    }
  }

  template <class Source, std::size_t Index =
            NuoVariantDetail::Selected<Source&&, Types...>::value>
    requires (!std::is_same_v<std::remove_cvref_t<Source>, NuoVariant> &&
              std::is_constructible_v<Alternative<Index>, Source&&>)
  constexpr NuoVariant(Source&& value)
    noexcept(std::is_nothrow_constructible_v<Alternative<Index>, Source&&>)
  {
    Construct<Index>(std::forward<Source>(value));
  }

  template <std::size_t Index, class... Args>
    requires (Index < Size && std::is_constructible_v<Alternative<Index>, Args...>)
  explicit constexpr NuoVariant(std::in_place_index_t<Index>, Args&&... args)
  {
    Construct<Index>(std::forward<Args>(args)...);
  }

  template <std::size_t Index, class U, class... Args>
    requires (Index < Size && std::is_constructible_v<Alternative<Index>,
              std::initializer_list<U>&, Args...>)
  explicit constexpr NuoVariant(std::in_place_index_t<Index>,
                                std::initializer_list<U> values, Args&&... args)
  {
    Construct<Index>(values, std::forward<Args>(args)...);
  }

  template <class T, class... Args>
    requires (NuoVariantDetail::kCount<T, Types...> == 1 &&
              std::is_constructible_v<T, Args...>)
  explicit constexpr NuoVariant(std::in_place_type_t<T>, Args&&... args)
    : NuoVariant(std::in_place_index<NuoVariantDetail::TypeIndex<T, Types...>()>,
                 std::forward<Args>(args)...)
  {
  }

  template <class T, class U, class... Args>
    requires (NuoVariantDetail::kCount<T, Types...> == 1 &&
              std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
  explicit constexpr NuoVariant(std::in_place_type_t<T>,
                                std::initializer_list<U> values, Args&&... args)
    : NuoVariant(std::in_place_index<NuoVariantDetail::TypeIndex<T, Types...>()>,
                 values, std::forward<Args>(args)...)
  {
  }

  constexpr ~NuoVariant()
    requires (std::is_trivially_destructible_v<Types> && ...) = default;

  constexpr ~NuoVariant()
    requires (!(std::is_trivially_destructible_v<Types> && ...))
  {
    Reset();
  }

  constexpr NuoVariant& operator=(const NuoVariant&)
    requires ((std::is_trivially_copy_constructible_v<Types> && ...) &&
              (std::is_trivially_copy_assignable_v<Types> && ...) &&
              (std::is_trivially_destructible_v<Types> && ...)) = default;

  constexpr NuoVariant& operator=(const NuoVariant& other)
    requires ((std::is_copy_constructible_v<Types> && ...) &&
              (std::is_copy_assignable_v<Types> && ...) &&
              !((std::is_trivially_copy_constructible_v<Types> && ...) &&
                (std::is_trivially_copy_assignable_v<Types> && ...) &&
                (std::is_trivially_destructible_v<Types> && ...)))
  {
    if (this == &other)
    {
      return *this;
    }
    if (other.ValuelessByException())
    {
      Reset();
    }
    else
    {
      Dispatch(other.index_, [this, &other]<std::size_t Index>()
      {
        Assign<Index>(other.storage_.template Get<Index>());
      });
    }
    return *this;
  }

  constexpr NuoVariant& operator=(const NuoVariant&)
    requires (!((std::is_copy_constructible_v<Types> && ...) &&
                (std::is_copy_assignable_v<Types> && ...))) = delete;

  constexpr NuoVariant& operator=(NuoVariant&&)
    requires ((std::is_trivially_move_constructible_v<Types> && ...) &&
              (std::is_trivially_move_assignable_v<Types> && ...) &&
              (std::is_trivially_destructible_v<Types> && ...)) = default;

  constexpr NuoVariant& operator=(NuoVariant&& other)
    noexcept((std::is_nothrow_move_constructible_v<Types> && ...) &&
             (std::is_nothrow_move_assignable_v<Types> && ...))
    requires ((std::is_move_constructible_v<Types> && ...) &&
              (std::is_move_assignable_v<Types> && ...) &&
              !((std::is_trivially_move_constructible_v<Types> && ...) &&
                (std::is_trivially_move_assignable_v<Types> && ...) &&
                (std::is_trivially_destructible_v<Types> && ...)))
  {
    if (this == &other)
    {
      return *this;
    }
    if (other.ValuelessByException())
    {
      Reset();
    }
    else
    {
      Dispatch(other.index_, [this, &other]<std::size_t Index>()
      {
        if (index_ == Index)
        {
          storage_.template Get<Index>() =
            std::move(other.storage_.template Get<Index>());
        }
        else
        {
          Emplace<Index>(std::move(other.storage_.template Get<Index>()));
        }
      });
    }
    return *this;
  }

  constexpr NuoVariant& operator=(NuoVariant&&)
    requires (!((std::is_move_constructible_v<Types> && ...) &&
                (std::is_move_assignable_v<Types> && ...))) = delete;

  template <class Source, std::size_t Index =
            NuoVariantDetail::Selected<Source&&, Types...>::value>
    requires (!std::is_same_v<std::remove_cvref_t<Source>, NuoVariant> &&
              std::is_constructible_v<Alternative<Index>, Source&&> &&
              std::is_assignable_v<Alternative<Index>&, Source&&>)
  constexpr NuoVariant& operator=(Source&& value)
    noexcept(std::is_nothrow_constructible_v<Alternative<Index>, Source&&> &&
             std::is_nothrow_assignable_v<Alternative<Index>&, Source&&>)
  {
    Assign<Index>(std::forward<Source>(value));
    return *this;
  }

  constexpr std::size_t Index() const noexcept
  {
    return index_;
  }

  constexpr bool ValuelessByException() const noexcept
  {
    return index_ == NuoVariantNpos;
  }

  template <std::size_t Index, class... Args>
    requires (Index < Size && std::is_constructible_v<Alternative<Index>, Args...>)
  constexpr Alternative<Index>& Emplace(Args&&... args)
  {
    Reset();
    return Construct<Index>(std::forward<Args>(args)...);
  }

  template <std::size_t Index, class U, class... Args>
    requires (Index < Size && std::is_constructible_v<Alternative<Index>,
              std::initializer_list<U>&, Args...>)
  constexpr Alternative<Index>& Emplace(std::initializer_list<U> values,
                                        Args&&... args)
  {
    Reset();
    return Construct<Index>(values, std::forward<Args>(args)...);
  }

  template <class T, class... Args>
    requires (NuoVariantDetail::kCount<T, Types...> == 1 &&
              std::is_constructible_v<T, Args...>)
  constexpr T& Emplace(Args&&... args)
  {
    return Emplace<NuoVariantDetail::TypeIndex<T, Types...>()>(
      std::forward<Args>(args)...);
  }

  template <class T, class U, class... Args>
    requires (NuoVariantDetail::kCount<T, Types...> == 1 &&
              std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
  constexpr T& Emplace(std::initializer_list<U> values, Args&&... args)
  {
    return Emplace<NuoVariantDetail::TypeIndex<T, Types...>()>(
      values, std::forward<Args>(args)...);
  }

  constexpr void Swap(NuoVariant& other)
    noexcept((std::is_nothrow_move_constructible_v<Types> && ...) &&
             (std::is_nothrow_swappable_v<Types> && ...))
    requires ((std::is_move_constructible_v<Types> && ...) &&
              (std::is_swappable_v<Types> && ...))
  {
    if (this == &other)
    {
      return;
    }
    if (index_ == other.index_)
    {
      if (!ValuelessByException())
      {
        Dispatch(index_, [this, &other]<std::size_t Index>()
        {
          using std::swap;
          swap(storage_.template Get<Index>(),
               other.storage_.template Get<Index>());
        });
      }
      return;
    }
    NuoVariant temporary(std::move(*this));
    Reset();
    MoveConstruct(other);
    other.Reset();
    other.MoveConstruct(temporary);
  }

private:
  friend struct NuoVariantDetail::Access;

  template <class Function>
  static constexpr void Dispatch(std::size_t index, Function&& function)
  {
    NuoVariantDetail::Dispatch<void, Size>(index,
                                          std::forward<Function>(function));
  }

  template <std::size_t Index, class... Args>
  constexpr Alternative<Index>& Construct(Args&&... args)
  {
    auto& value = storage_.template Construct<Index>(std::forward<Args>(args)...);
    index_ = Index;
    return value;
  }

  template <std::size_t Index, class Source>
  constexpr void Assign(Source&& source)
  {
    using T = Alternative<Index>;
    if (index_ == Index)
    {
      storage_.template Get<Index>() = std::forward<Source>(source);
    }
    else if constexpr (std::is_nothrow_constructible_v<T, Source> ||
                       !std::is_nothrow_move_constructible_v<T>)
    {
      Emplace<Index>(std::forward<Source>(source));
    }
    else
    {
      T temporary(std::forward<Source>(source));
      Emplace<Index>(std::move(temporary));
    }
  }

  constexpr void Reset() noexcept
  {
    if (!ValuelessByException())
    {
      Dispatch(index_, [this]<std::size_t Index>()
      {
        std::destroy_at(std::addressof(storage_.template Get<Index>()));
      });
      index_ = NuoVariantNpos;
    }
  }

  constexpr void MoveConstruct(NuoVariant& other)
  {
    if (!other.ValuelessByException())
    {
      Dispatch(other.index_, [this, &other]<std::size_t Index>()
      {
        Construct<Index>(std::move(other.storage_.template Get<Index>()));
      });
    }
  }

  NuoVariantDetail::Storage<Types...> storage_;
  std::size_t index_ = NuoVariantNpos;
};

namespace NuoVariantDetail
{

struct Access
{
  template <std::size_t Index, class Variant>
  static constexpr decltype(auto) Get(Variant&& value)
  {
    if constexpr (std::is_lvalue_reference_v<Variant&&>)
    {
      return value.storage_.template Get<Index>();
    }
    else
    {
      return std::move(value.storage_.template Get<Index>());
    }
  }
};

} /* namespace NuoVariantDetail */

/* cppreference declaration:
   template< class T > struct variant_size;
   https://en.cppreference.com/w/cpp/utility/variant/variant_size.html */
template <class Variant>
struct NuoVariantSize;

template <class... Types>
struct NuoVariantSize<NuoVariant<Types...>>
  : std::integral_constant<std::size_t, sizeof...(Types)>
{
};

template <class Variant>
struct NuoVariantSize<const Variant> : NuoVariantSize<Variant>
{
};

template <class Variant>
struct NuoVariantSize<volatile Variant> : NuoVariantSize<Variant>
{
};

template <class Variant>
struct NuoVariantSize<const volatile Variant> : NuoVariantSize<Variant>
{
};

template <class Variant>
inline constexpr std::size_t NuoVariantSizeV = NuoVariantSize<Variant>::value;

/* cppreference declaration:
   template< std::size_t I, class T > struct variant_alternative;
   https://en.cppreference.com/w/cpp/utility/variant/variant_alternative.html */
template <std::size_t Index, class Variant>
struct NuoVariantAlternative;

template <std::size_t Index, class... Types>
struct NuoVariantAlternative<Index, NuoVariant<Types...>>
  : std::tuple_element<Index, std::tuple<Types...>>
{
};

template <std::size_t Index, class Variant>
struct NuoVariantAlternative<Index, const Variant>
{
  using type = std::add_const_t<
    typename NuoVariantAlternative<Index, Variant>::type>;
};

template <std::size_t Index, class Variant>
struct NuoVariantAlternative<Index, volatile Variant>
{
  using type = std::add_volatile_t<
    typename NuoVariantAlternative<Index, Variant>::type>;
};

template <std::size_t Index, class Variant>
struct NuoVariantAlternative<Index, const volatile Variant>
{
  using type = std::add_cv_t<
    typename NuoVariantAlternative<Index, Variant>::type>;
};

template <std::size_t Index, class Variant>
using NuoVariantAlternativeT =
  typename NuoVariantAlternative<Index, Variant>::type;

/* cppreference declaration:
   template< std::size_t I, class... Types >
   constexpr variant_alternative_t<I, variant<Types...>>&
     get( variant<Types...>& v );
   https://en.cppreference.com/w/cpp/utility/variant/get.html */
template <std::size_t Index, class... Types>
constexpr decltype(auto) NuoGet(NuoVariant<Types...>& value)
{
  static_assert(Index < sizeof...(Types), "Alternative index is out of range");
  if (value.Index() != Index)
  {
    throw NuoBadVariantAccess();
  }
  return NuoVariantDetail::Access::Get<Index>(value);
}

template <class T, class... Types>
constexpr decltype(auto) NuoGet(NuoVariant<Types...>& value)
{
  return NuoGet<NuoVariantDetail::TypeIndex<T, Types...>()>(value);
}

template <std::size_t Index, class... Types>
constexpr decltype(auto) NuoGet(const NuoVariant<Types...>& value)
{
  static_assert(Index < sizeof...(Types), "Alternative index is out of range");
  if (value.Index() != Index)
  {
    throw NuoBadVariantAccess();
  }
  return NuoVariantDetail::Access::Get<Index>(value);
}

template <class T, class... Types>
constexpr decltype(auto) NuoGet(const NuoVariant<Types...>& value)
{
  return NuoGet<NuoVariantDetail::TypeIndex<T, Types...>()>(value);
}

template <std::size_t Index, class... Types>
constexpr decltype(auto) NuoGet(NuoVariant<Types...>&& value)
{
  static_assert(Index < sizeof...(Types), "Alternative index is out of range");
  if (value.Index() != Index)
  {
    throw NuoBadVariantAccess();
  }
  return NuoVariantDetail::Access::Get<Index>(std::move(value));
}

template <class T, class... Types>
constexpr decltype(auto) NuoGet(NuoVariant<Types...>&& value)
{
  return NuoGet<NuoVariantDetail::TypeIndex<T, Types...>()>(std::move(value));
}

template <std::size_t Index, class... Types>
constexpr decltype(auto) NuoGet(const NuoVariant<Types...>&& value)
{
  static_assert(Index < sizeof...(Types), "Alternative index is out of range");
  if (value.Index() != Index)
  {
    throw NuoBadVariantAccess();
  }
  return NuoVariantDetail::Access::Get<Index>(std::move(value));
}

template <class T, class... Types>
constexpr decltype(auto) NuoGet(const NuoVariant<Types...>&& value)
{
  return NuoGet<NuoVariantDetail::TypeIndex<T, Types...>()>(std::move(value));
}

/* cppreference declaration:
   template< std::size_t I, class... Types >
   constexpr add_pointer_t<variant_alternative_t<I, variant<Types...>>>
   get_if( variant<Types...>* pv ) noexcept;
   https://en.cppreference.com/w/cpp/utility/variant/get_if.html */
template <std::size_t Index, class... Types>
constexpr auto NuoGetIf(NuoVariant<Types...>* value) noexcept
  -> NuoVariantAlternativeT<Index, NuoVariant<Types...>>*
{
  if (value == nullptr || value->Index() != Index)
  {
    return nullptr;
  }
  return std::addressof(NuoVariantDetail::Access::Get<Index>(*value));
}

template <class T, class... Types>
constexpr T* NuoGetIf(NuoVariant<Types...>* value) noexcept
{
  return NuoGetIf<NuoVariantDetail::TypeIndex<T, Types...>()>(value);
}

template <std::size_t Index, class... Types>
constexpr auto NuoGetIf(const NuoVariant<Types...>* value) noexcept
  -> NuoVariantAlternativeT<Index, const NuoVariant<Types...>>*
{
  if (value == nullptr || value->Index() != Index)
  {
    return nullptr;
  }
  return std::addressof(NuoVariantDetail::Access::Get<Index>(*value));
}

template <class T, class... Types>
constexpr const T* NuoGetIf(const NuoVariant<Types...>* value) noexcept
{
  return NuoGetIf<NuoVariantDetail::TypeIndex<T, Types...>()>(value);
}

/* cppreference declaration:
   template< class T, class... Types >
   constexpr bool holds_alternative( const variant<Types...>& v ) noexcept;
   https://en.cppreference.com/w/cpp/utility/variant/holds_alternative.html */
template <class T, class... Types>
constexpr bool NuoHoldsAlternative(const NuoVariant<Types...>& value) noexcept
{
  return value.Index() == NuoVariantDetail::TypeIndex<T, Types...>();
}

namespace NuoVariantDetail
{

/* Recurse over the Cartesian product without copying the visitor or elements. */
template <class Result, class Function>
constexpr Result Visit(Function&& function)
{
  if constexpr (std::is_void_v<Result>)
  {
    std::invoke(std::forward<Function>(function));
  }
  else
  {
    return std::invoke(std::forward<Function>(function));
  }
}

template <class Result, class Function, class Variant, class... Rest>
constexpr Result Visit(Function&& function, Variant&& value, Rest&&... rest)
{
  return Dispatch<Result, NuoVariantSizeV<std::remove_cvref_t<Variant>>>(
    value.Index(), [&]<std::size_t Index>() -> Result
    {
      auto bound = [&function, &value](auto&&... args) -> decltype(auto)
      {
        return std::invoke(std::forward<Function>(function),
          Access::Get<Index>(std::forward<Variant>(value)),
          std::forward<decltype(args)>(args)...);
      };
      return Visit<Result>(bound, std::forward<Rest>(rest)...);
    });
}

template <class Result, class Function, class... Variants>
constexpr Result VisitExact(Function&& function, Variants&&... variants)
{
  auto checked = [&function](auto&&... args) -> decltype(auto)
  {
    static_assert(std::is_same_v<Result,
      std::invoke_result_t<Function, decltype(args)...>>,
      "Every visitor invocation must have the same return type");
    return std::invoke(std::forward<Function>(function),
                       std::forward<decltype(args)>(args)...);
  };
  return Visit<Result>(checked, std::forward<Variants>(variants)...);
}

} /* namespace NuoVariantDetail */

/* cppreference declaration:
   template< class Visitor, class... Variants >
   constexpr visit_impl visit( Visitor&& vis, Variants&&... vars );
   https://en.cppreference.com/w/cpp/utility/variant/visit.html */
template <class Visitor, class... Variants>
constexpr decltype(auto) NuoVisit(Visitor&& visitor, Variants&&... variants)
{
  if ((variants.ValuelessByException() || ...))
  {
    throw NuoBadVariantAccess();
  }
  using Result = std::invoke_result_t<Visitor,
    decltype(NuoGet<0>(std::forward<Variants>(variants)))...>;
  return NuoVariantDetail::VisitExact<Result>(std::forward<Visitor>(visitor),
                                             std::forward<Variants>(variants)...);
}

template <class Result, class Visitor, class... Variants>
constexpr Result NuoVisit(Visitor&& visitor, Variants&&... variants)
{
  if ((variants.ValuelessByException() || ...))
  {
    throw NuoBadVariantAccess();
  }
  return NuoVariantDetail::Visit<Result>(std::forward<Visitor>(visitor),
                                         std::forward<Variants>(variants)...);
}

template <class... Types>
constexpr bool operator==(const NuoVariant<Types...>& left,
                           const NuoVariant<Types...>& right)
{
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    /* Reserving zero for valueless makes it smaller than every alternative. */
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return left_rank == right_rank;
  }
  return NuoVariantDetail::Dispatch<bool, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>()
    {
      return NuoGet<Index>(left) == NuoGet<Index>(right);
    });
}

template <class... Types>
constexpr bool operator!=(const NuoVariant<Types...>& left,
                           const NuoVariant<Types...>& right)
{
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    /* Reserving zero for valueless makes it smaller than every alternative. */
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return left_rank != right_rank;
  }
  return NuoVariantDetail::Dispatch<bool, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>()
    {
      return NuoGet<Index>(left) != NuoGet<Index>(right);
    });
}

template <class... Types>
constexpr bool operator<(const NuoVariant<Types...>& left,
                           const NuoVariant<Types...>& right)
{
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    /* Reserving zero for valueless makes it smaller than every alternative. */
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return left_rank < right_rank;
  }
  return NuoVariantDetail::Dispatch<bool, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>()
    {
      return NuoGet<Index>(left) < NuoGet<Index>(right);
    });
}

template <class... Types>
constexpr bool operator<=(const NuoVariant<Types...>& left,
                           const NuoVariant<Types...>& right)
{
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    /* Reserving zero for valueless makes it smaller than every alternative. */
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return left_rank <= right_rank;
  }
  return NuoVariantDetail::Dispatch<bool, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>()
    {
      return NuoGet<Index>(left) <= NuoGet<Index>(right);
    });
}

template <class... Types>
constexpr bool operator>(const NuoVariant<Types...>& left,
                           const NuoVariant<Types...>& right)
{
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    /* Reserving zero for valueless makes it smaller than every alternative. */
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return left_rank > right_rank;
  }
  return NuoVariantDetail::Dispatch<bool, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>()
    {
      return NuoGet<Index>(left) > NuoGet<Index>(right);
    });
}

template <class... Types>
constexpr bool operator>=(const NuoVariant<Types...>& left,
                           const NuoVariant<Types...>& right)
{
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    /* Reserving zero for valueless makes it smaller than every alternative. */
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return left_rank >= right_rank;
  }
  return NuoVariantDetail::Dispatch<bool, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>()
    {
      return NuoGet<Index>(left) >= NuoGet<Index>(right);
    });
}

template <class... Types>
  requires (std::three_way_comparable<Types> && ...)
constexpr auto operator<=>(const NuoVariant<Types...>& left,
                            const NuoVariant<Types...>& right)
{
  using Result = std::common_comparison_category_t<
    std::compare_three_way_result_t<Types>...>;
  if (left.ValuelessByException() || right.ValuelessByException() ||
      left.Index() != right.Index())
  {
    const auto left_rank = left.ValuelessByException() ? 0 : left.Index() + 1;
    const auto right_rank = right.ValuelessByException() ? 0 : right.Index() + 1;
    return Result(left_rank <=> right_rank);
  }
  return NuoVariantDetail::Dispatch<Result, sizeof...(Types)>(
    left.Index(), [&]<std::size_t Index>() -> Result
    {
      return NuoGet<Index>(left) <=> NuoGet<Index>(right);
    });
}

template <class... Types>
constexpr void NuoSwap(NuoVariant<Types...>& left, NuoVariant<Types...>& right)
  noexcept(noexcept(left.Swap(right)))
  requires requires { left.Swap(right); }
{
  left.Swap(right);
}

} /* namespace nuostl */
