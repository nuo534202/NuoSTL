#pragma once

#include <stdexcept>

namespace nuostl
{
#define NUO_THROW_LENGTH_ERROR_IF(expr, msg) \
  if (expr) \
    throw std::length_error(msg)

#define NUO_THROW_RUNTIME_ERROR_IF(expr, msg) \
  if (expr) \
    throw std::runtime_error(msg)

#define NUO_THROW_OUT_OF_RANGE_IF(expr, msg) \
  if (expr) \
    throw std::out_of_range(msg)
} /* namespace nuostl */