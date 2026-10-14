#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace nuostl
{

template <typename Signature>
class NuoFunction;

namespace detail
{

template <typename T>
struct IsNuoFunction : std::false_type
{
};

template <typename R, typename... Args>
struct IsNuoFunction<NuoFunction<R(Args...)>> : std::true_type
{
};

/* The type erased target interface used by Target<T>(). It is independent of
   the function signature, so querying a type that the stored callable cannot
   be invoked with is still well formed. */
class NuoFunctionTarget
{
public:
  virtual ~NuoFunctionTarget() = default;

  virtual const std::type_info& Type() const noexcept = 0;
  virtual void* Object() noexcept = 0;
  virtual const void* Object() const noexcept = 0;
};

/* The type erased interface of the stored callable. */
template <typename Signature>
class NuoFunctionBase;

template <typename R, typename... Args>
class NuoFunctionBase<R(Args...)> : public NuoFunctionTarget
{
public:
  virtual R Invoke(Args... args) const = 0;
  virtual NuoFunctionBase* Clone() const = 0;
};

/* The holder owns a decayed copy of the callable. The callable is mutable so
   that a const NuoFunction can invoke a mutable lambda, as std::function
   does. */
template <typename F, typename R, typename... Args>
class NuoFunctionHolder final : public NuoFunctionBase<R(Args...)>
{
public:
  explicit NuoFunctionHolder(F callable) : callable_(std::move(callable))
  {
  }

  R Invoke(Args... args) const override
  {
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(callable_, std::forward<Args>(args)...);
    }
    else
    {
      return std::invoke(callable_, std::forward<Args>(args)...);
    }
  }

  NuoFunctionBase<R(Args...)>* Clone() const override
  {
    return new NuoFunctionHolder(callable_);
  }

  const std::type_info& Type() const noexcept override
  {
    return typeid(F);
  }

  void* Object() noexcept override
  {
    return std::addressof(callable_);
  }

  const void* Object() const noexcept override
  {
    return std::addressof(callable_);
  }

private:
  mutable F callable_;
};

} /* namespace detail */

/* The standard library class template is
   template <typename R, typename... Args> class function<R(Args...)>;
   cppreference.com/w/cpp/utility/functional/function.html
   This implementation stores the callable in a heap allocated holder. There
   is no small buffer optimization and no allocator extended constructor, so
   the behavior matches std::function while every construction from a
   callable allocates. */
template <typename R, typename... Args>
class NuoFunction<R(Args...)>
{
public:
  using result_type = R;

  NuoFunction() noexcept = default;

  NuoFunction(std::nullptr_t) noexcept
  {
  }

  NuoFunction(const NuoFunction& other)
    : base_(other.base_ == nullptr ? nullptr : other.base_->Clone())
  {
  }

  NuoFunction(NuoFunction&& other) noexcept : base_(other.base_)
  {
    other.base_ = nullptr;
  }

  template <typename F>
    requires (!detail::IsNuoFunction<std::decay_t<F>>::value &&
              std::is_copy_constructible_v<std::decay_t<F>> &&
              std::is_invocable_r_v<R, std::decay_t<F>&, Args...>)
  NuoFunction(F&& callable)
  {
    Assign(std::forward<F>(callable));
  }

  ~NuoFunction()
  {
    delete base_;
  }

  NuoFunction& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  NuoFunction& operator=(const NuoFunction& other)
  {
    if (this != &other)
    {
      NuoFunction(other).Swap(*this);
    }
    return *this;
  }

  NuoFunction& operator=(NuoFunction&& other) noexcept
  {
    if (this != &other)
    {
      Reset();
      base_ = other.base_;
      other.base_ = nullptr;
    }
    return *this;
  }

  template <typename F>
    requires (!detail::IsNuoFunction<std::decay_t<F>>::value &&
              std::is_copy_constructible_v<std::decay_t<F>> &&
              std::is_invocable_r_v<R, std::decay_t<F>&, Args...>)
  NuoFunction& operator=(F&& callable)
  {
    NuoFunction(std::forward<F>(callable)).Swap(*this);
    return *this;
  }

  explicit operator bool() const noexcept
  {
    return base_ != nullptr;
  }

  R operator()(Args... args) const
  {
    if (base_ == nullptr)
    {
      throw std::bad_function_call();
    }
    if constexpr (std::is_void_v<R>)
    {
      base_->Invoke(std::forward<Args>(args)...);
    }
    else
    {
      return base_->Invoke(std::forward<Args>(args)...);
    }
  }

  void Swap(NuoFunction& other) noexcept
  {
    using std::swap;
    swap(base_, other.base_);
  }

  const std::type_info& TargetType() const noexcept
  {
    return base_ == nullptr ? typeid(void) : base_->Type();
  }

  template <typename T>
  T* Target() noexcept
  {
    if constexpr (std::is_void_v<T> || std::is_function_v<T> ||
                  std::is_array_v<T>)
    {
      return nullptr;
    }
    else if constexpr (!std::is_same_v<std::remove_cv_t<T>, T>)
    {
      return Target<std::remove_cv_t<T>>();
    }
    else
    {
      if (base_ == nullptr)
      {
        return nullptr;
      }
      auto* target = dynamic_cast<detail::NuoFunctionTarget*>(base_);
      if (target == nullptr || target->Type() != typeid(T))
      {
        return nullptr;
      }
      return static_cast<T*>(target->Object());
    }
  }

  template <typename T>
  const T* Target() const noexcept
  {
    if constexpr (std::is_void_v<T> || std::is_function_v<T> ||
                  std::is_array_v<T>)
    {
      return nullptr;
    }
    else if constexpr (!std::is_same_v<std::remove_cv_t<T>, T>)
    {
      return Target<std::remove_cv_t<T>>();
    }
    else
    {
      if (base_ == nullptr)
      {
        return nullptr;
      }
      const auto* target =
        dynamic_cast<const detail::NuoFunctionTarget*>(base_);
      if (target == nullptr || target->Type() != typeid(T))
      {
        return nullptr;
      }
      return static_cast<const T*>(target->Object());
    }
  }

private:
  template <typename F>
  void Assign(F&& callable)
  {
    using Stored = std::decay_t<F>;
    if constexpr (std::is_pointer_v<Stored> ||
                  std::is_member_pointer_v<Stored>)
    {
      if (callable == nullptr)
      {
        return;
      }
    }
    base_ = new detail::NuoFunctionHolder<Stored, R, Args...>(
      std::forward<F>(callable));
  }

  void Reset() noexcept
  {
    delete base_;
    base_ = nullptr;
  }

  detail::NuoFunctionBase<R(Args...)>* base_ = nullptr;
};

template <typename R, typename... Args>
void NuoSwap(NuoFunction<R(Args...)>& lhs,
             NuoFunction<R(Args...)>& rhs) noexcept
{
  lhs.Swap(rhs);
}

template <typename R, typename... Args>
bool operator==(const NuoFunction<R(Args...)>& function,
                std::nullptr_t) noexcept
{
  return !function;
}

template <typename R, typename... Args>
bool operator==(std::nullptr_t,
                const NuoFunction<R(Args...)>& function) noexcept
{
  return !function;
}

} /* namespace nuostl */
