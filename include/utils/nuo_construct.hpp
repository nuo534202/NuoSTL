#pragma once

#include <new>

#include "utils/nuo_util.hpp"

namespace nuostl
{

template<typename T>
void NuoConstruct(T* ptr)
{
  ::new ((void*)ptr) T();
}

template <typename T>
void NuoConstruct(T* ptr, const T& value)
{
  ::new ((void*)ptr) T(value);
}

template <typename T>
void NuoConstruct(T* ptr, T&& value)
{
  ::new ((void*)ptr) T(NuoMove(value));
}

template <typename T, typename... Args>
void NuoConstruct(T* ptr, Args&& ...args)
{
  ::new ((void*)ptr) T(NuoForward<Args>(args)...);
}

} /* namespace nuostl */