#include "core/data_types/test_nuo_source_location.hpp"

#include <assert.h>

#include <cstdint>
#include <source_location>
#include <string>

#include "core/data_types/nuo_source_location.hpp"

namespace test
{

namespace
{

constexpr nuostl::NuoSourceLocation kConstexprLocation =
  nuostl::NuoSourceLocation::Current();

void CheckLocation(
  nuostl::NuoSourceLocation nuo = nuostl::NuoSourceLocation::Current(),
  std::source_location standard = std::source_location::current())
{
  assert(nuo.Line() == standard.line());
  assert(nuo.Column() == standard.column());
  assert(std::string(nuo.FileName()) == standard.file_name());
  assert(std::string(nuo.FunctionName()) == standard.function_name());
}

} /* namespace */

void TestNuoSourceLocation::TestDefaultConstruction()
{
  constexpr nuostl::NuoSourceLocation empty;
  static_assert(empty.Line() == 0);
  static_assert(empty.Column() == 0);
  static_assert(empty.FileName() != nullptr);
  static_assert(empty.FileName()[0] == '\0');
  static_assert(empty.FunctionName() != nullptr);
  static_assert(empty.FunctionName()[0] == '\0');

  constexpr nuostl::NuoSourceLocation copied(empty);
  static_assert(copied.Line() == 0);
  constexpr nuostl::NuoSourceLocation assigned = copied;
  static_assert(assigned.Column() == 0);
  assert(empty.FileName() != nullptr);
  assert(empty.FunctionName() != nullptr);
}

void TestNuoSourceLocation::TestCurrent()
{
  const nuostl::NuoSourceLocation location =
    nuostl::NuoSourceLocation::Current();
  assert(location.Line() > 0);
  assert(location.Column() > 0);
  assert(location.FileName() != nullptr);
  assert(std::string(location.FileName()).find(
           "test_nuo_source_location.cpp") != std::string::npos);
  assert(location.FunctionName() != nullptr);
  assert(location.FunctionName()[0] != '\0');
  assert(std::string(location.FunctionName()).find("TestCurrent") !=
         std::string::npos);
}

void TestNuoSourceLocation::TestDefaultArgumentCapture()
{
  CheckLocation();
  CheckLocation();
}

void TestNuoSourceLocation::TestConstexprCapture()
{
  static_assert(kConstexprLocation.Line() > 0);
  static_assert(kConstexprLocation.Column() > 0);
  static_assert(kConstexprLocation.FileName()[0] != '\0');
}

void TestNuoSourceLocation::TestSuite()
{
  TestDefaultConstruction();
  TestCurrent();
  TestDefaultArgumentCapture();
  TestConstexprCapture();
}

} /* namespace test */
