#pragma once

#include <new>
#include <type_traits>

#include "utils/nuo_construct.hpp"
#include "utils/nuo_destroy.hpp"

namespace nuostl
{

template <typename T>
class NuoAllocator
{
public:
  using value_type      = T;
  using pointer         = T*;
  using const_pointer   = const T*;
  using reference       = T&;
  using const_reference = const T&;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;

  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  template <typename U>
  struct rebind
  {
    using other = NuoAllocator<U>;
  };

  NuoAllocator() noexcept = default;

  template <typename U>
  NuoAllocator(const NuoAllocator<U>&) noexcept
  {
  }

  /* Standard allocator member interface. */
  pointer allocate(size_type n);
  void deallocate(pointer ptr, size_type n);
  size_type max_size() const noexcept;

  template <typename U, typename... Args>
  void construct(U* ptr, Args&&... args);

  template <typename U>
  void destroy(U* ptr);

public:
  /* allocate memory */
  static T* Allocate();
  static T* Allocate(size_type n);

  /* deallocate memory */
  static void Deallocate(T* ptr);
  static void Deallocate(T* ptr, size_type n);

  /* construct object */
  static void Construct(T* ptr);
  static void Construct(T* ptr, const T& value);
  static void Construct(T* ptr, T&& value);

  template<typename... Args>
  static void Construct(T* ptr, Args&& ...args);

  /* destroy object */
  static void Destroy(T* ptr);
  static void Destroy(T* first, T* last);
};

template <typename T>
typename NuoAllocator<T>::pointer NuoAllocator<T>::allocate(size_type n)
{
  return static_cast<pointer>(::operator new(n * sizeof(T)));
}

template <typename T>
void NuoAllocator<T>::deallocate(pointer ptr, size_type n)
{
  (void)n;
  ::operator delete(ptr);
}

template <typename T>
typename NuoAllocator<T>::size_type
NuoAllocator<T>::max_size() const noexcept
{
  return static_cast<size_type>(-1) / sizeof(T);
}

template <typename T>
template <typename U, typename... Args>
void NuoAllocator<T>::construct(U* ptr, Args&&... args)
{
  NuoConstruct(ptr, NuoForward<Args>(args)...);
}

template <typename T>
template <typename U>
void NuoAllocator<T>::destroy(U* ptr)
{
  NuoDestroy(ptr);
}

template <typename T>
T* NuoAllocator<T>::Allocate()
{
  return static_cast<T*>(::operator new(sizeof(T)));
}

template <typename T>
T* NuoAllocator<T>::Allocate(size_type n)
{
  if (n == 0)
    return nullptr;
  return static_cast<T*>(::operator new(n * sizeof(T)));
}

template <typename T>
void NuoAllocator<T>::Deallocate(T* ptr)
{
  if (ptr == nullptr)
    return;
  ::operator delete(ptr);
}

template <typename T>
void NuoAllocator<T>::Deallocate(T* ptr, size_type n)
{
  (void)n;
  if (ptr == nullptr)
    return;
  ::operator delete(ptr);
}

template <typename T>
void NuoAllocator<T>::Construct(T* ptr)
{
  NuoConstruct(ptr);
}

template <typename T>
void NuoAllocator<T>::Construct(T* ptr, const T& value)
{
  NuoConstruct(ptr, value);
}

template <typename T>
void NuoAllocator<T>::Construct(T* ptr, T&& value)
{
  NuoConstruct(ptr, NuoMove(value));
}

template <typename T>
template <typename... Args>
void NuoAllocator<T>::Construct(T* ptr, Args&& ...args)
{
  NuoConstruct(ptr, NuoForward<Args>(args)...);
}

template <typename T>
void NuoAllocator<T>::Destroy(T* ptr)
{
  NuoDestroy(ptr);
}

template <typename T>
void NuoAllocator<T>::Destroy(T* first, T* last)
{
  NuoDestroy(first, last);
}

template <typename T, typename U>
bool operator==(const NuoAllocator<T>&,
                const NuoAllocator<U>&) noexcept
{
  return true;
}

template <typename T, typename U>
bool operator!=(const NuoAllocator<T>&,
                const NuoAllocator<U>&) noexcept
{
  return false;
}

} /* namespace nuostl */
