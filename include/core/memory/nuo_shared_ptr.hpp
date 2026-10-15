#pragma once

#include <atomic>
#include <compare>
#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <ostream>
#include <type_traits>
#include <utility>

#include "core/memory/nuo_unique_ptr.hpp"
#include "core/memory/nuo_allocator_traits.hpp"

namespace nuostl
{

template <typename T>
class NuoSharedPtr;

template <typename T>
class NuoWeakPtr;

template <typename T>
class NuoEnableSharedFromThis;

namespace detail
{

struct NuoAdoptSharedTag
{
};

struct NuoAdoptWeakTag
{
};

struct NuoNoEnableSharedBase
{
};

template <typename T>
NuoEnableSharedFromThis<T>* NuoEnableSharedBase(
  NuoEnableSharedFromThis<T>* pointer) noexcept
{
  return pointer;
}

inline NuoNoEnableSharedBase NuoEnableSharedBase(...) noexcept
{
  return NuoNoEnableSharedBase{};
}

/* The control block owns the shared and weak reference counts. The initial
   shared count represents the first owner, and the initial weak count keeps
   the control block alive while shared owners exist. */
class NuoSharedControl
{
public:
  void AddShared() noexcept
  {
    shared_count_.fetch_add(1, std::memory_order_relaxed);
  }

  void AddWeak() noexcept
  {
    weak_count_.fetch_add(1, std::memory_order_relaxed);
  }

  bool TryAddShared() noexcept
  {
    long count = shared_count_.load(std::memory_order_relaxed);
    while (count > 0)
    {
      if (shared_count_.compare_exchange_weak(
            count, count + 1, std::memory_order_acq_rel,
            std::memory_order_relaxed))
      {
        return true;
      }
    }
    return false;
  }

  bool ReleaseShared() noexcept
  {
    if (shared_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
      Dispose();
      return ReleaseWeak();
    }
    return false;
  }

  bool ReleaseWeak() noexcept
  {
    if (weak_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
      Destroy();
      return true;
    }
    return false;
  }

  long UseCount() const noexcept
  {
    return shared_count_.load(std::memory_order_acquire);
  }

  long WeakCount() const noexcept
  {
    return weak_count_.load(std::memory_order_acquire);
  }

protected:
  NuoSharedControl() = default;
  virtual ~NuoSharedControl() = default;

  virtual void Dispose() noexcept = 0;
  virtual void Destroy() noexcept = 0;

private:
  std::atomic<long> shared_count_{1};
  std::atomic<long> weak_count_{1};
};

template <typename Pointer, typename Deleter>
class NuoSharedPointerControl final : public NuoSharedControl
{
public:
  template <typename D>
  NuoSharedPointerControl(Pointer pointer, D&& deleter)
    : pointer_(pointer)
    , deleter_(std::forward<D>(deleter))
  {
  }

protected:
  ~NuoSharedPointerControl() override = default;

  void Dispose() noexcept override
  {
    deleter_(pointer_);
  }

  void Destroy() noexcept override
  {
    delete this;
  }

private:
  Pointer pointer_;
  [[no_unique_address]] Deleter deleter_;
};

template <typename T>
class NuoSharedObjectControl final : public NuoSharedControl
{
public:
  template <typename... Args>
  explicit NuoSharedObjectControl(Args&&... args)
  {
    std::construct_at(StoragePointer(), std::forward<Args>(args)...);
  }

  T* Object() noexcept
  {
    return std::launder(StoragePointer());
  }

protected:
  ~NuoSharedObjectControl() override = default;

  void Dispose() noexcept override
  {
    std::destroy_at(Object());
  }

  void Destroy() noexcept override
  {
    delete this;
  }

private:
  T* StoragePointer() noexcept
  {
    return reinterpret_cast<T*>(storage_);
  }

  alignas(T) unsigned char storage_[sizeof(T)];
};

/* A control block that allocates its own memory through the allocator that
   also constructs and destroys the owned object. */
template <typename T, typename Alloc>
class NuoAllocatedSharedObjectControl final : public NuoSharedControl
{
public:
  template <typename... Args>
  explicit NuoAllocatedSharedObjectControl(const Alloc& allocator,
                                           Args&&... args)
    : allocator_(allocator)
  {
    using ObjectAllocator = typename NuoAllocatorTraits<Alloc>::
      template rebind_alloc<T>;
    ObjectAllocator object_allocator(allocator_);
    NuoAllocatorTraits<ObjectAllocator>::construct(
      object_allocator, StoragePointer(), std::forward<Args>(args)...);
  }

  T* Object() noexcept
  {
    return std::launder(StoragePointer());
  }

protected:
  ~NuoAllocatedSharedObjectControl() override = default;

  void Dispose() noexcept override
  {
    using ObjectAllocator = typename NuoAllocatorTraits<Alloc>::
      template rebind_alloc<T>;
    ObjectAllocator object_allocator(allocator_);
    NuoAllocatorTraits<ObjectAllocator>::destroy(object_allocator, Object());
  }

  void Destroy() noexcept override
  {
    using ControlAllocator = typename NuoAllocatorTraits<Alloc>::
      template rebind_alloc<NuoAllocatedSharedObjectControl>;
    using ControlPointer =
      typename NuoAllocatorTraits<ControlAllocator>::pointer;
    ControlAllocator control_allocator(allocator_);
    this->~NuoAllocatedSharedObjectControl();
    NuoAllocatorTraits<ControlAllocator>::deallocate(
      control_allocator,
      reinterpret_cast<ControlPointer>(this), 1);
  }

private:
  T* StoragePointer() noexcept
  {
    return reinterpret_cast<T*>(storage_);
  }

  alignas(T) unsigned char storage_[sizeof(T)];
  [[no_unique_address]] Alloc allocator_;
};

} /* namespace detail */

/* The standard library class template is
   template <typename T> class shared_ptr;
   cppreference.com/w/cpp/memory/shared_ptr.html
   This object form stores the owner pointer and a shared control block. */
template <typename T>
class NuoSharedPtr
{
  static_assert(!std::is_array_v<T>,
                "NuoSharedPtr<T> requires a non-array type");
  static_assert(!std::is_reference_v<T>,
                "NuoSharedPtr cannot own a reference");

  template <typename U>
  friend class NuoSharedPtr;

  template <typename U>
  friend class NuoWeakPtr;

  template <typename U, typename... Args>
    requires (!std::is_array_v<U> && !std::is_void_v<U> &&
              !std::is_reference_v<U> && !std::is_function_v<U>)
  friend NuoSharedPtr<U> NuoMakeShared(Args&&... args);

  template <typename U, typename A, typename... Args>
  friend NuoSharedPtr<U> NuoAllocateShared(const A& allocator,
                                           Args&&... args);

public:
  using element_type = T;
  using weak_type = NuoWeakPtr<T>;

  constexpr NuoSharedPtr() noexcept = default;

  constexpr NuoSharedPtr(std::nullptr_t) noexcept
  {
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  explicit NuoSharedPtr(Y* pointer)
  {
    Initialize(pointer, NuoDefaultDelete<Y>());
  }

  template <typename Y, typename D>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr(Y* pointer, D deleter)
  {
    Initialize(pointer, std::forward<D>(deleter));
  }

  NuoSharedPtr(const NuoSharedPtr& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddShared();
    }
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr(const NuoSharedPtr<Y>& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddShared();
    }
  }

  NuoSharedPtr(NuoSharedPtr&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr(NuoSharedPtr<Y>&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  template <typename Y, typename D>
    requires (std::is_convertible_v<typename NuoUniquePtr<Y, D>::pointer, T*> &&
              !std::is_array_v<Y>)
  NuoSharedPtr(NuoUniquePtr<Y, D>&& other)
  {
    std::decay_t<D> deleter = std::move(other.GetDeleter());
    Y* pointer = other.Release();
    Initialize(pointer, std::move(deleter));
  }

  template <typename Y>
  NuoSharedPtr(const NuoSharedPtr<Y>& other, T* pointer) noexcept
    : pointer_(pointer)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddShared();
    }
  }

  template <typename Y>
  NuoSharedPtr(NuoSharedPtr<Y>&& other, T* pointer) noexcept
    : pointer_(pointer)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  ~NuoSharedPtr() noexcept
  {
    Reset();
  }

  NuoSharedPtr& operator=(const NuoSharedPtr& other) noexcept
  {
    if (this != &other)
    {
      NuoSharedPtr(other).Swap(*this);
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr& operator=(const NuoSharedPtr<Y>& other) noexcept
  {
    NuoSharedPtr(other).Swap(*this);
    return *this;
  }

  NuoSharedPtr& operator=(NuoSharedPtr&& other) noexcept
  {
    if (this != &other)
    {
      Reset();
      pointer_ = other.pointer_;
      control_ = other.control_;
      other.pointer_ = nullptr;
      other.control_ = nullptr;
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr& operator=(NuoSharedPtr<Y>&& other) noexcept
  {
    Reset();
    pointer_ = other.pointer_;
    control_ = other.control_;
    other.pointer_ = nullptr;
    other.control_ = nullptr;
    return *this;
  }

  NuoSharedPtr& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  T* Get() const noexcept
  {
    return pointer_;
  }

  explicit operator bool() const noexcept
  {
    return pointer_ != nullptr;
  }

  std::add_lvalue_reference_t<T> operator*() const noexcept
    requires (!std::is_void_v<T>)
  {
    return *pointer_;
  }

  T* operator->() const noexcept
    requires (!std::is_void_v<T>)
  {
    return pointer_;
  }

  long UseCount() const noexcept
  {
    return control_ == nullptr ? 0 : control_->UseCount();
  }

  bool Unique() const noexcept
  {
    return UseCount() == 1;
  }

  void Reset() noexcept
  {
    detail::NuoSharedControl* control = control_;
    pointer_ = nullptr;
    control_ = nullptr;
    if (control != nullptr)
    {
      control->ReleaseShared();
    }
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  void Reset(Y* pointer)
  {
    NuoSharedPtr replacement(pointer);
    Swap(replacement);
  }

  template <typename Y, typename D>
    requires std::is_convertible_v<Y*, T*>
  void Reset(Y* pointer, D deleter)
  {
    NuoSharedPtr replacement(pointer, std::forward<D>(deleter));
    Swap(replacement);
  }

  void Swap(NuoSharedPtr& other) noexcept
  {
    using std::swap;
    swap(pointer_, other.pointer_);
    swap(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoSharedPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoWeakPtr<U>& other) const noexcept;

private:
  NuoSharedPtr(T* pointer, detail::NuoSharedControl* control,
               detail::NuoAdoptSharedTag) noexcept
    : pointer_(pointer)
    , control_(control)
  {
    EnableSharedFromThis(pointer);
  }

  template <typename Y, typename D>
  void Initialize(Y* pointer, D&& deleter)
  {
    using StoredDeleter = std::decay_t<D>;
    using Control = detail::NuoSharedPointerControl<Y*, StoredDeleter>;

    pointer_ = nullptr;
    control_ = nullptr;
    if (pointer == nullptr)
    {
      return;
    }

    StoredDeleter stored_deleter(std::forward<D>(deleter));
    Control* new_control = nullptr;
    try
    {
      new_control = new Control(pointer, std::move(stored_deleter));
    }
    catch (...)
    {
      stored_deleter(pointer);
      throw;
    }

    pointer_ = pointer;
    control_ = new_control;
    EnableSharedFromThis(pointer);
  }

  template <typename Y>
  void EnableSharedFromThis(Y* pointer) noexcept;

  T* pointer_ = nullptr;
  detail::NuoSharedControl* control_ = nullptr;
};

/* The standard library class template is
   template <typename T> class weak_ptr;
   cppreference.com/w/cpp/memory/weak_ptr.html
   A weak pointer observes a control block without increasing its shared
   count. */
template <typename T>
class NuoWeakPtr
{
  static_assert(!std::is_array_v<T>,
                "NuoWeakPtr<T> requires a non-array type");
  static_assert(!std::is_reference_v<T>,
                "NuoWeakPtr cannot observe a reference");

  template <typename U>
  friend class NuoWeakPtr;

  template <typename U>
  friend class NuoSharedPtr;

public:
  using element_type = T;

  constexpr NuoWeakPtr() noexcept = default;

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr(const NuoSharedPtr<Y>& shared) noexcept
    : pointer_(shared.pointer_)
    , control_(shared.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  NuoWeakPtr(const NuoWeakPtr& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr(const NuoWeakPtr<Y>& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  NuoWeakPtr(NuoWeakPtr&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr(NuoWeakPtr<Y>&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  ~NuoWeakPtr() noexcept
  {
    Reset();
  }

  NuoWeakPtr& operator=(const NuoWeakPtr& other) noexcept
  {
    if (this != &other)
    {
      NuoWeakPtr(other).Swap(*this);
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr& operator=(const NuoWeakPtr<Y>& other) noexcept
  {
    NuoWeakPtr(other).Swap(*this);
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr& operator=(const NuoSharedPtr<Y>& shared) noexcept
  {
    NuoWeakPtr(shared).Swap(*this);
    return *this;
  }

  NuoWeakPtr& operator=(NuoWeakPtr&& other) noexcept
  {
    if (this != &other)
    {
      Reset();
      pointer_ = other.pointer_;
      control_ = other.control_;
      other.pointer_ = nullptr;
      other.control_ = nullptr;
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr& operator=(NuoWeakPtr<Y>&& other) noexcept
  {
    Reset();
    pointer_ = other.pointer_;
    control_ = other.control_;
    other.pointer_ = nullptr;
    other.control_ = nullptr;
    return *this;
  }

  NuoWeakPtr& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  long UseCount() const noexcept
  {
    return control_ == nullptr ? 0 : control_->UseCount();
  }

  bool Expired() const noexcept
  {
    return UseCount() == 0;
  }

  NuoSharedPtr<T> Lock() const noexcept
  {
    if (control_ == nullptr || !control_->TryAddShared())
    {
      return NuoSharedPtr<T>();
    }
    return NuoSharedPtr<T>(pointer_, control_, detail::NuoAdoptSharedTag{});
  }

  void Reset() noexcept
  {
    detail::NuoSharedControl* control = control_;
    pointer_ = nullptr;
    control_ = nullptr;
    if (control != nullptr)
    {
      control->ReleaseWeak();
    }
  }

  void Swap(NuoWeakPtr& other) noexcept
  {
    using std::swap;
    swap(pointer_, other.pointer_);
    swap(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoSharedPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoWeakPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

private:
  NuoWeakPtr(T* pointer, detail::NuoSharedControl* control,
             detail::NuoAdoptWeakTag) noexcept
    : pointer_(pointer)
    , control_(control)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  T* pointer_ = nullptr;
  detail::NuoSharedControl* control_ = nullptr;
};

/* The array form stores an element pointer and exposes operator[]. The
   control block retains the array deleter chosen by the constructor. */
template <typename T>
class NuoSharedPtr<T[]>
{
  static_assert(!std::is_void_v<T>, "NuoSharedPtr cannot own void[]");

  template <typename U>
  friend class NuoSharedPtr;

  template <typename U>
  friend class NuoWeakPtr;

public:
  using element_type = T;
  using weak_type = NuoWeakPtr<T[]>;

  constexpr NuoSharedPtr() noexcept = default;

  constexpr NuoSharedPtr(std::nullptr_t) noexcept
  {
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  explicit NuoSharedPtr(Y* pointer)
  {
    Initialize(pointer, NuoDefaultDelete<Y[]>());
  }

  template <typename Y, typename D>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr(Y* pointer, D deleter)
  {
    Initialize(pointer, std::forward<D>(deleter));
  }

  NuoSharedPtr(const NuoSharedPtr& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddShared();
    }
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr(const NuoSharedPtr<Y[]>& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddShared();
    }
  }

  NuoSharedPtr(NuoSharedPtr&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr(NuoSharedPtr<Y[]>&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  template <typename Y, typename D>
    requires (std::is_convertible_v<typename NuoUniquePtr<Y[], D>::pointer, T*>)
  NuoSharedPtr(NuoUniquePtr<Y[], D>&& other)
  {
    std::decay_t<D> deleter = std::move(other.GetDeleter());
    Y* pointer = other.Release();
    Initialize(pointer, std::move(deleter));
  }

  template <typename Y>
  NuoSharedPtr(const NuoSharedPtr<Y[]>& other, T* pointer) noexcept
    : pointer_(pointer)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddShared();
    }
  }

  template <typename Y>
  NuoSharedPtr(NuoSharedPtr<Y[]>&& other, T* pointer) noexcept
    : pointer_(pointer)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  ~NuoSharedPtr() noexcept
  {
    Reset();
  }

  NuoSharedPtr& operator=(const NuoSharedPtr& other) noexcept
  {
    if (this != &other)
    {
      NuoSharedPtr(other).Swap(*this);
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr& operator=(const NuoSharedPtr<Y[]>& other) noexcept
  {
    NuoSharedPtr(other).Swap(*this);
    return *this;
  }

  NuoSharedPtr& operator=(NuoSharedPtr&& other) noexcept
  {
    if (this != &other)
    {
      Reset();
      pointer_ = other.pointer_;
      control_ = other.control_;
      other.pointer_ = nullptr;
      other.control_ = nullptr;
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoSharedPtr& operator=(NuoSharedPtr<Y[]>&& other) noexcept
  {
    Reset();
    pointer_ = other.pointer_;
    control_ = other.control_;
    other.pointer_ = nullptr;
    other.control_ = nullptr;
    return *this;
  }

  NuoSharedPtr& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  T* Get() const noexcept
  {
    return pointer_;
  }

  explicit operator bool() const noexcept
  {
    return pointer_ != nullptr;
  }

  T& operator[](std::size_t index) const noexcept
  {
    return pointer_[index];
  }

  long UseCount() const noexcept
  {
    return control_ == nullptr ? 0 : control_->UseCount();
  }

  bool Unique() const noexcept
  {
    return UseCount() == 1;
  }

  void Reset() noexcept
  {
    detail::NuoSharedControl* control = control_;
    pointer_ = nullptr;
    control_ = nullptr;
    if (control != nullptr)
    {
      control->ReleaseShared();
    }
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  void Reset(Y* pointer)
  {
    NuoSharedPtr replacement(pointer);
    Swap(replacement);
  }

  template <typename Y, typename D>
    requires std::is_convertible_v<Y*, T*>
  void Reset(Y* pointer, D deleter)
  {
    NuoSharedPtr replacement(pointer, std::forward<D>(deleter));
    Swap(replacement);
  }

  void Swap(NuoSharedPtr& other) noexcept
  {
    using std::swap;
    swap(pointer_, other.pointer_);
    swap(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoSharedPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoWeakPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

private:
  NuoSharedPtr(T* pointer, detail::NuoSharedControl* control,
               detail::NuoAdoptSharedTag) noexcept
    : pointer_(pointer)
    , control_(control)
  {
  }

  template <typename Y, typename D>
  void Initialize(Y* pointer, D&& deleter)
  {
    using StoredDeleter = std::decay_t<D>;
    using Control = detail::NuoSharedPointerControl<Y*, StoredDeleter>;

    pointer_ = nullptr;
    control_ = nullptr;
    if (pointer == nullptr)
    {
      return;
    }

    StoredDeleter stored_deleter(std::forward<D>(deleter));
    Control* new_control = nullptr;
    try
    {
      new_control = new Control(pointer, std::move(stored_deleter));
    }
    catch (...)
    {
      stored_deleter(pointer);
      throw;
    }

    pointer_ = pointer;
    control_ = new_control;
  }

  T* pointer_ = nullptr;
  detail::NuoSharedControl* control_ = nullptr;
};

template <typename T>
class NuoWeakPtr<T[]>
{
  static_assert(!std::is_void_v<T>, "NuoWeakPtr cannot observe void[]");

  template <typename U>
  friend class NuoWeakPtr;

  template <typename U>
  friend class NuoSharedPtr;

public:
  using element_type = T;

  constexpr NuoWeakPtr() noexcept = default;

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr(const NuoSharedPtr<Y[]>& shared) noexcept
    : pointer_(shared.pointer_)
    , control_(shared.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  NuoWeakPtr(const NuoWeakPtr& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr(const NuoWeakPtr<Y[]>& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  NuoWeakPtr(NuoWeakPtr&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr(NuoWeakPtr<Y[]>&& other) noexcept
    : pointer_(other.pointer_)
    , control_(other.control_)
  {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  ~NuoWeakPtr() noexcept
  {
    Reset();
  }

  NuoWeakPtr& operator=(const NuoWeakPtr& other) noexcept
  {
    if (this != &other)
    {
      NuoWeakPtr(other).Swap(*this);
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr& operator=(const NuoWeakPtr<Y[]>& other) noexcept
  {
    NuoWeakPtr(other).Swap(*this);
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr& operator=(const NuoSharedPtr<Y[]>& shared) noexcept
  {
    NuoWeakPtr(shared).Swap(*this);
    return *this;
  }

  NuoWeakPtr& operator=(NuoWeakPtr&& other) noexcept
  {
    if (this != &other)
    {
      Reset();
      pointer_ = other.pointer_;
      control_ = other.control_;
      other.pointer_ = nullptr;
      other.control_ = nullptr;
    }
    return *this;
  }

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  NuoWeakPtr& operator=(NuoWeakPtr<Y[]>&& other) noexcept
  {
    Reset();
    pointer_ = other.pointer_;
    control_ = other.control_;
    other.pointer_ = nullptr;
    other.control_ = nullptr;
    return *this;
  }

  NuoWeakPtr& operator=(std::nullptr_t) noexcept
  {
    Reset();
    return *this;
  }

  long UseCount() const noexcept
  {
    return control_ == nullptr ? 0 : control_->UseCount();
  }

  bool Expired() const noexcept
  {
    return UseCount() == 0;
  }

  NuoSharedPtr<T[]> Lock() const noexcept
  {
    if (control_ == nullptr || !control_->TryAddShared())
    {
      return NuoSharedPtr<T[]>();
    }
    return NuoSharedPtr<T[]>(pointer_, control_, detail::NuoAdoptSharedTag{});
  }

  void Reset() noexcept
  {
    detail::NuoSharedControl* control = control_;
    pointer_ = nullptr;
    control_ = nullptr;
    if (control != nullptr)
    {
      control->ReleaseWeak();
    }
  }

  void Swap(NuoWeakPtr& other) noexcept
  {
    using std::swap;
    swap(pointer_, other.pointer_);
    swap(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoSharedPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

  template <typename U>
  bool OwnerBefore(const NuoWeakPtr<U>& other) const noexcept
  {
    return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
  }

private:
  NuoWeakPtr(T* pointer, detail::NuoSharedControl* control,
             detail::NuoAdoptWeakTag) noexcept
    : pointer_(pointer)
    , control_(control)
  {
    if (control_ != nullptr)
    {
      control_->AddWeak();
    }
  }

  T* pointer_ = nullptr;
  detail::NuoSharedControl* control_ = nullptr;
};

/* The standard library support base is
   template <typename T> class enable_shared_from_this;
   cppreference.com/w/cpp/memory/enable_shared_from_this.html */
template <typename T>
class NuoEnableSharedFromThis
{
protected:
  constexpr NuoEnableSharedFromThis() noexcept = default;

  constexpr NuoEnableSharedFromThis(
    const NuoEnableSharedFromThis&) noexcept
  {
  }

  NuoEnableSharedFromThis& operator=(
    const NuoEnableSharedFromThis&) noexcept
  {
    return *this;
  }

  ~NuoEnableSharedFromThis() = default;

public:
  NuoSharedPtr<T> SharedFromThis()
  {
    NuoSharedPtr<T> shared = weak_this_.Lock();
    if (!shared)
    {
      throw std::bad_weak_ptr();
    }
    return shared;
  }

  NuoSharedPtr<const T> SharedFromThis() const
  {
    NuoSharedPtr<T> shared = weak_this_.Lock();
    if (!shared)
    {
      throw std::bad_weak_ptr();
    }
    return NuoSharedPtr<const T>(std::move(shared));
  }

  NuoWeakPtr<T> WeakFromThis() noexcept
  {
    return weak_this_;
  }

  NuoWeakPtr<const T> WeakFromThis() const noexcept
  {
    return weak_this_;
  }

private:
  template <typename U>
  friend class NuoSharedPtr;

  NuoWeakPtr<T> weak_this_;
};

template <typename T>
template <typename U>
void NuoSharedPtr<T>::EnableSharedFromThis(U* pointer) noexcept
{
  using BasePointer = decltype(detail::NuoEnableSharedBase(pointer));
  if constexpr (!std::is_same_v<BasePointer, detail::NuoNoEnableSharedBase>)
  {
    BasePointer base = detail::NuoEnableSharedBase(pointer);
    if (base != nullptr && base->weak_this_.Expired())
    {
      base->weak_this_ = NuoWeakPtr<U>(pointer, control_,
                                       detail::NuoAdoptWeakTag{});
    }
  }
}

template <typename T>
template <typename U>
bool NuoSharedPtr<T>::OwnerBefore(const NuoWeakPtr<U>& other) const noexcept
{
  return std::less<detail::NuoSharedControl*>{}(control_, other.control_);
}

template <typename T>
void NuoSwap(NuoSharedPtr<T>& lhs, NuoSharedPtr<T>& rhs) noexcept
{
  lhs.Swap(rhs);
}

template <typename T>
void NuoSwap(NuoWeakPtr<T>& lhs, NuoWeakPtr<T>& rhs) noexcept
{
  lhs.Swap(rhs);
}

template <typename T, typename... Args>
  requires (!std::is_array_v<T> && !std::is_void_v<T> &&
            !std::is_reference_v<T> && !std::is_function_v<T>)
NuoSharedPtr<T> NuoMakeShared(Args&&... args)
{
  using Control = detail::NuoSharedObjectControl<T>;
  Control* control = new Control(std::forward<Args>(args)...);
  return NuoSharedPtr<T>(control->Object(), control,
                         detail::NuoAdoptSharedTag{});
}

template <typename T>
  requires (std::is_array_v<T> && std::extent_v<T> == 0)
NuoSharedPtr<T> NuoMakeShared(std::size_t size)
{
  using Element = std::remove_extent_t<T>;
  return NuoSharedPtr<T>(new Element[size]());
}

template <typename T>
  requires (!std::is_array_v<T> && !std::is_void_v<T> &&
            !std::is_reference_v<T> && !std::is_function_v<T>)
NuoSharedPtr<T> NuoMakeSharedForOverwrite()
{
  return NuoSharedPtr<T>(new T);
}

template <typename T>
  requires (std::is_array_v<T> && std::extent_v<T> == 0)
NuoSharedPtr<T> NuoMakeSharedForOverwrite(std::size_t size)
{
  using Element = std::remove_extent_t<T>;
  return NuoSharedPtr<T>(new Element[size]);
}

/* The standard library declaration is
   template <typename T, typename Alloc, typename... Args>
   shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args);
   cppreference.com/w/cpp/memory/shared_ptr/allocate_shared.html
   This object form allocates the control block and the object together and
   uses the rebound allocator for construction and destruction. */
template <typename T, typename Alloc, typename... Args>
NuoSharedPtr<T> NuoAllocateShared(const Alloc& allocator, Args&&... args)
{
  static_assert(!std::is_array_v<T>,
                "NuoAllocateShared array form is not implemented");
  static_assert(!std::is_void_v<T> && !std::is_reference_v<T> &&
                !std::is_function_v<T>,
                "NuoAllocateShared requires an object type");

  using Control = detail::NuoAllocatedSharedObjectControl<T, Alloc>;
  using ControlAllocator = typename NuoAllocatorTraits<Alloc>::
    template rebind_alloc<Control>;

  ControlAllocator control_allocator(allocator);
  Control* control =
    NuoAllocatorTraits<ControlAllocator>::allocate(control_allocator, 1);
  try
  {
    ::new (static_cast<void*>(control))
      Control(allocator, std::forward<Args>(args)...);
  }
  catch (...)
  {
    NuoAllocatorTraits<ControlAllocator>::deallocate(
      control_allocator, control, 1);
    throw;
  }
  return NuoSharedPtr<T>(control->Object(), control,
                         detail::NuoAdoptSharedTag{});
}

template <typename T, typename U>
NuoSharedPtr<T> NuoStaticPointerCast(const NuoSharedPtr<U>& pointer) noexcept
{
  return NuoSharedPtr<T>(pointer, static_cast<T*>(pointer.Get()));
}

template <typename T, typename U>
NuoSharedPtr<T> NuoStaticPointerCast(NuoSharedPtr<U>&& pointer) noexcept
{
  T* casted = static_cast<T*>(pointer.Get());
  return NuoSharedPtr<T>(std::move(pointer), casted);
}

template <typename T, typename U>
NuoSharedPtr<T> NuoDynamicPointerCast(const NuoSharedPtr<U>& pointer) noexcept
{
  T* casted = dynamic_cast<T*>(pointer.Get());
  if (casted == nullptr)
  {
    return NuoSharedPtr<T>();
  }
  return NuoSharedPtr<T>(pointer, casted);
}

template <typename T, typename U>
NuoSharedPtr<T> NuoDynamicPointerCast(NuoSharedPtr<U>&& pointer) noexcept
{
  T* casted = dynamic_cast<T*>(pointer.Get());
  if (casted == nullptr)
  {
    NuoSharedPtr<U> discarded(std::move(pointer));
    return NuoSharedPtr<T>();
  }
  return NuoSharedPtr<T>(std::move(pointer), casted);
}

template <typename T, typename U>
NuoSharedPtr<T> NuoConstPointerCast(const NuoSharedPtr<U>& pointer) noexcept
{
  return NuoSharedPtr<T>(pointer, const_cast<T*>(pointer.Get()));
}

template <typename T, typename U>
NuoSharedPtr<T> NuoReinterpretPointerCast(
  const NuoSharedPtr<U>& pointer) noexcept
{
  return NuoSharedPtr<T>(pointer, reinterpret_cast<T*>(pointer.Get()));
}

template <typename T, typename U>
constexpr bool operator==(const NuoSharedPtr<T>& lhs,
                          const NuoSharedPtr<U>& rhs) noexcept
{
  return lhs.Get() == rhs.Get();
}

template <typename T, typename U>
constexpr auto operator<=>(const NuoSharedPtr<T>& lhs,
                           const NuoSharedPtr<U>& rhs) noexcept
  requires requires
  {
    std::compare_three_way{}(lhs.Get(), rhs.Get());
  }
{
  return std::compare_three_way{}(lhs.Get(), rhs.Get());
}

template <typename T>
constexpr bool operator==(const NuoSharedPtr<T>& pointer,
                          std::nullptr_t) noexcept
{
  return !pointer;
}

template <typename T>
constexpr bool operator==(std::nullptr_t,
                          const NuoSharedPtr<T>& pointer) noexcept
{
  return !pointer;
}

template <typename T>
constexpr std::strong_ordering operator<=>(const NuoSharedPtr<T>& pointer,
                                           std::nullptr_t) noexcept
{
  return pointer ? std::strong_ordering::greater
                 : std::strong_ordering::equal;
}

template <typename T>
constexpr std::strong_ordering operator<=>(std::nullptr_t,
                                           const NuoSharedPtr<T>& pointer)
  noexcept
{
  return pointer ? std::strong_ordering::less
                 : std::strong_ordering::equal;
}

template <typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& stream,
  const NuoSharedPtr<T>& pointer)
{
  return stream << pointer.Get();
}

} /* namespace nuostl */

namespace std
{

template <typename T>
struct hash<nuostl::NuoSharedPtr<T>>
{
  size_t operator()(const nuostl::NuoSharedPtr<T>& pointer) const noexcept
  {
    using Element = typename nuostl::NuoSharedPtr<T>::element_type;
    return hash<Element*>{}(pointer.Get());
  }
};

} /* namespace std */
