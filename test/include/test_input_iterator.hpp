#pragma once

#include <cstddef>
#include <memory>

#include "utils/nuo_iterator.hpp"

namespace test
{

/* A single-pass iterator whose copies share one advancing cursor. */
template <typename T>
class SinglePassInputIterator
{
public:
  using iterator_category = nuostl::NuoInputIteratorTag;
  using value_type        = T;
  using pointer           = const T*;
  using reference         = const T&;
  using difference_type   = std::ptrdiff_t;

  class PostIncrementProxy
  {
  public:
    explicit PostIncrementProxy(const T* value) : value_(value)
    {
    }

    reference operator*() const
    {
      return *value_;
    }

    pointer operator->() const
    {
      return value_;
    }

  private:
    const T* value_;
  };

  SinglePassInputIterator() = default;

  explicit SinglePassInputIterator(const T* ptr)
    : state_(std::make_shared<State>(ptr))
  {
  }

  /* Copy-like moves preserve the shared cursor required by input iterators. */
  SinglePassInputIterator(const SinglePassInputIterator&) = default;
  SinglePassInputIterator& operator=(const SinglePassInputIterator&) = default;
  SinglePassInputIterator(SinglePassInputIterator&& other) noexcept
    : state_(other.state_)
  {
  }
  SinglePassInputIterator& operator=(SinglePassInputIterator&& other) noexcept
  {
    state_ = other.state_;
    return *this;
  }

  reference operator*() const
  {
    return *state_->current_;
  }

  pointer operator->() const
  {
    return state_->current_;
  }

  SinglePassInputIterator& operator++()
  {
    ++state_->current_;
    return *this;
  }

  PostIncrementProxy operator++(int)
  {
    PostIncrementProxy previous(state_->current_);
    ++state_->current_;
    return previous;
  }

  bool operator==(const SinglePassInputIterator& rhs) const
  {
    if (!state_ || !rhs.state_)
    {
      return state_ == rhs.state_;
    }
    return state_->current_ == rhs.state_->current_;
  }

  bool operator!=(const SinglePassInputIterator& rhs) const
  {
    return !(*this == rhs);
  }

private:
  struct State
  {
    explicit State(const T* current) : current_(current)
    {
    }

    const T* current_;
  };

  std::shared_ptr<State> state_;
};

} /* namespace test */
