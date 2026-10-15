#pragma once

#include <cstdint>
#include <source_location>

namespace nuostl
{

/* The standard library declaration is
   struct source_location;
   cppreference.com/w/cpp/utility/source_location.html
   The standard facility is used to capture the current location so that the
   column is available on every supported compiler. */
class NuoSourceLocation
{
public:
  static consteval NuoSourceLocation Current(
    std::source_location location = std::source_location::current()) noexcept
  {
    return NuoSourceLocation(location.line(), location.column(),
                             location.file_name(),
                             location.function_name());
  }

  constexpr NuoSourceLocation() noexcept = default;

  constexpr std::uint_least32_t Line() const noexcept
  {
    return line_;
  }

  constexpr std::uint_least32_t Column() const noexcept
  {
    return column_;
  }

  constexpr const char* FileName() const noexcept
  {
    return file_;
  }

  constexpr const char* FunctionName() const noexcept
  {
    return function_;
  }

private:
  constexpr NuoSourceLocation(std::uint_least32_t line,
                              std::uint_least32_t column,
                              const char* file,
                              const char* function) noexcept
    : line_(line)
    , column_(column)
    , file_(file)
    , function_(function)
  {
  }

  std::uint_least32_t line_ = 0;
  std::uint_least32_t column_ = 0;
  const char* file_ = "";
  const char* function_ = "";
};

} /* namespace nuostl */
