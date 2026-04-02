#pragma once

#include "utils/nuo_construct.hpp"
#include "utils/nuo_destroy.hpp"

namespace nuostl
{

template <typename T>
class NuoAllocator
{
public:
  using value_type    = T;
  using pointer       = T*;
  using const_pointer = const T*;
  using reference       = T&;
  using const_reference = const T&;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;

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
  NuoConstruct(ptr, value);
}

template <typename T>
template <typename... Args>
void NuoAllocator<T>::Construct(T* ptr, Args&& ...args)
{
  NuoConstruct(ptr, args);
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

} /* namespace nuostl */