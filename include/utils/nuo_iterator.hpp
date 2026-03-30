#pragma once

#include <stddef.h>

namespace nuostl
{

template <typename T>
class NuoIterator
{
public:
  using value_type  = T;
  using pointer     = T*;
  using reference   = T&;
  using difference_type = ptrdiff_t;
};

} /* namespace nuostl */