#pragma once

#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace nuostl
{

class NuoBadAnyCast : public std::bad_cast
{
public:
  const char* what() const noexcept override
  {
    return "NuoAny cast failed";
  }
};

class NuoAny
{
private:
  class StorageBase
  {
  public:
    virtual ~StorageBase() = default;
    virtual std::unique_ptr<StorageBase> Clone() const = 0;
    virtual const std::type_info& Type() const noexcept = 0;
    virtual void* Data() noexcept = 0;
    virtual const void* Data() const noexcept = 0;
  };

  template <class T>
  class Storage final : public StorageBase
  {
  public:
    template <class... Args>
    explicit Storage(Args&&... args)
      : value_(std::forward<Args>(args)...)
    {
    }

    std::unique_ptr<StorageBase> Clone() const override
    {
      return std::make_unique<Storage<T>>(value_);
    }

    const std::type_info& Type() const noexcept override
    {
      return typeid(T);
    }

    void* Data() noexcept override
    {
      return &value_;
    }

    const void* Data() const noexcept override
    {
      return &value_;
    }

    T value_;
  };

  template <class T>
  using StoredType = std::decay_t<T>;

public:
  NuoAny() noexcept = default;

  NuoAny(const NuoAny& other)
    : storage_(other.storage_ == nullptr ? nullptr : other.storage_->Clone())
  {
  }

  NuoAny(NuoAny&& other) noexcept = default;

  template <class T>
    requires (!std::is_same_v<StoredType<T>, NuoAny> &&
              std::is_constructible_v<StoredType<T>, T>)
  NuoAny(T&& value)
    : storage_(std::make_unique<Storage<StoredType<T>>>(
        std::forward<T>(value)))
  {
  }

  template <class T, class... Args>
  explicit NuoAny(std::in_place_type_t<T>, Args&&... args)
    requires std::is_constructible_v<T, Args...>
    : storage_(std::make_unique<Storage<T>>(std::forward<Args>(args)...))
  {
  }

  ~NuoAny() = default;

  NuoAny& operator=(const NuoAny& other)
  {
    if (this != &other)
    {
      NuoAny copy(other);
      Swap(copy);
    }
    return *this;
  }

  NuoAny& operator=(NuoAny&& other) noexcept = default;

  template <class T>
    requires (!std::is_same_v<StoredType<T>, NuoAny> &&
              std::is_constructible_v<StoredType<T>, T>)
  NuoAny& operator=(T&& value)
  {
    NuoAny temporary(std::forward<T>(value));
    Swap(temporary);
    return *this;
  }

  bool HasValue() const noexcept
  {
    return storage_ != nullptr;
  }

  const std::type_info& Type() const noexcept
  {
    return storage_ == nullptr ? typeid(void) : storage_->Type();
  }

  void Reset() noexcept
  {
    storage_.reset();
  }

  void Swap(NuoAny& other) noexcept
  {
    storage_.swap(other.storage_);
  }

  template <class T, class... Args>
  std::decay_t<T>& Emplace(Args&&... args)
    requires std::is_constructible_v<std::decay_t<T>, Args...>
  {
    using Stored = std::decay_t<T>;
    auto replacement = std::make_unique<Storage<Stored>>(
      std::forward<Args>(args)...);
    storage_ = std::move(replacement);
    return *static_cast<Stored*>(storage_->Data());
  }

private:
  template <class T>
  friend std::decay_t<T>* NuoAnyCast(NuoAny*) noexcept;

  template <class T>
  friend const std::decay_t<T>* NuoAnyCast(const NuoAny*) noexcept;

  std::unique_ptr<StorageBase> storage_;
};

template <class T>
std::decay_t<T>* NuoAnyCast(NuoAny* value) noexcept
{
  using Stored = std::decay_t<T>;
  if (value == nullptr || value->Type() != typeid(Stored))
  {
    return nullptr;
  }
  return static_cast<Stored*>(value->storage_->Data());
}

template <class T>
const std::decay_t<T>* NuoAnyCast(const NuoAny* value) noexcept
{
  using Stored = std::decay_t<T>;
  if (value == nullptr || value->Type() != typeid(Stored))
  {
    return nullptr;
  }
  return static_cast<const Stored*>(value->storage_->Data());
}

template <class T>
T NuoAnyCast(NuoAny& value)
{
  using Stored = std::decay_t<T>;
  Stored* pointer = NuoAnyCast<Stored>(&value);
  if (pointer == nullptr)
  {
    throw NuoBadAnyCast();
  }
  if constexpr (std::is_reference_v<T>)
  {
    return static_cast<T>(*pointer);
  }
  else
  {
    return static_cast<T>(*pointer);
  }
}

template <class T>
T NuoAnyCast(const NuoAny& value)
{
  using Stored = std::decay_t<T>;
  const Stored* pointer = NuoAnyCast<Stored>(&value);
  if (pointer == nullptr)
  {
    throw NuoBadAnyCast();
  }
  return static_cast<T>(*pointer);
}

template <class T>
T NuoAnyCast(NuoAny&& value)
{
  using Stored = std::decay_t<T>;
  Stored* pointer = NuoAnyCast<Stored>(&value);
  if (pointer == nullptr)
  {
    throw NuoBadAnyCast();
  }
  if constexpr (std::is_reference_v<T>)
  {
    return static_cast<T>(*pointer);
  }
  else
  {
    return static_cast<T>(std::move(*pointer));
  }
}

inline void NuoSwap(NuoAny& left, NuoAny& right) noexcept
{
  left.Swap(right);
}

} /* namespace nuostl */
