#pragma once

#include "utils/nuo_type_traits.hpp"

namespace nuostl
{

/* destroy */
template <typename T>
void NuoDestroy(T* ptr);

template <typename ForwardIter>
void NuoDestroy(ForwardIter first, ForwardIter last);

/* destroy single object */
template <typename T>
void NuoDestroySingle(T*, std::true_type);

template <typename T>
void NuoDestroyDingle(T* ptr, std::false_type);

/* destroy category */
template <typename ForwardIter>
void NuoDestroyCat(ForwardIter, ForwardIter, std::true_type);

template <typename ForwardIter>
void NuoDestroyCat(ForwardIter first, ForwardIter last, std::false_type);


/* implementation */
/* destroy*/
template <typename T>
void NuoDestroy(T* ptr)
{
  NuoDestroySingle(ptr, std::is_trivially_destructible<T>());
}

template <typename ForwardIter>
void NuoDestroy(ForwardIter first, ForwardIter last)
{
  NuoDestroyCat(
    first,
    last,
    std::is_trivially_destructible<
      typename NuoIteratorTraits<ForwardIter>::value_type>{}
  );
}

/* destroy single object */
template <typename T>
void NuoDestroySingle(T*, std::true_type)
{
}

template <typename T>
void NuoDestroySingle(T* ptr, std::false_type)
{
  if (ptr == nullptr)
    return;
  ptr->~T();
}

/* destroy category */
template <typename ForwardIter>
void NuoDestroyCat(ForwardIter, ForwardIter, std::true_type)
{
}

template <typename ForwardIter>
void NuoDestroyCat(ForwardIter first, ForwardIter last, std::false_type)
{
  for (ForwardIter it = first; it != last; it++)
    NuoDestroy(&*first);
}

} /* namespace nuostl */