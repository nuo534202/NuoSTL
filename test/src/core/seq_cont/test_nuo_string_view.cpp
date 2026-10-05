#include "core/seq_cont/test_nuo_string_view.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/seq_cont/nuo_string_view.hpp"

namespace test
{

void TestNuoStringView::test_nuo_string_view()
{
  nuostl::NuoStringView literal("hello world");
  assert(literal.Size() == 11);
  assert(literal.Length() == 11);
  assert(literal.MaxSize() >= literal.Size());
  assert(!literal.Empty());
  assert(literal.Data()[0] == 'h');
  assert(literal[1] == 'e');
  assert(literal.At(4) == 'o');
  assert(literal.Front() == 'h');
  assert(literal.Back() == 'd');
  assert(literal.Begin() == literal.Data());
  assert(literal.End() == literal.Data() + literal.Size());

  bool caught_out_of_range = false;
  try
  {
    literal.At(literal.Size());
  }
  catch (const std::out_of_range&)
  {
    caught_out_of_range = true;
  }
  assert(caught_out_of_range);

  const char bounded[] = {'a', '\0', 'b', 'c'};
  nuostl::NuoStringView bounded_view(bounded, sizeof(bounded));
  assert(bounded_view.Size() == 4);
  assert(bounded_view[1] == '\0');
  assert(bounded_view[2] == 'b');
  assert(bounded_view.Find('\0') == 1);
  assert(bounded_view.Find("\0b", 0, 2) == 1);
  assert(bounded_view.FindFirstOf(bounded, 0, 2) == 0);

  nuostl::NuoStringView empty;
  assert(empty.Empty());
  assert(empty.Size() == 0);
  assert(empty.Begin() == empty.End());

  const std::string owned = "from string";
  nuostl::NuoStringView string_view(owned);
  assert(string_view == "from string");

  size_t traversed = 0;
  for (auto it = literal.RBegin(); it != literal.REnd(); ++it)
  {
    ++traversed;
  }
  assert(traversed == literal.Size());

  nuostl::NuoStringView trimmed("--world--");
  trimmed.RemovePrefix(2);
  trimmed.RemoveSuffix(2);
  assert(trimmed == "world");
  assert(literal == "hello world");

  char copied[8] = {};
  assert(literal.Copy(copied, 5, 6) == 5);
  assert(nuostl::NuoStringView(copied, 5) == "world");
  assert(literal.Substr(6, 5) == "world");
  assert(literal.Substr(literal.Size()).Empty());

  bool copy_caught_out_of_range = false;
  try
  {
    literal.Copy(copied, 1, literal.Size() + 1);
  }
  catch (const std::out_of_range&)
  {
    copy_caught_out_of_range = true;
  }
  assert(copy_caught_out_of_range);

  bool substr_caught_out_of_range = false;
  try
  {
    literal.Substr(literal.Size() + 1);
  }
  catch (const std::out_of_range&)
  {
    substr_caught_out_of_range = true;
  }
  assert(substr_caught_out_of_range);

  nuostl::NuoStringView alpha("alpha");
  nuostl::NuoStringView alphabet("alphabet");
  assert(alpha.Compare(alphabet) < 0);
  assert(alphabet.Compare(0, alpha.Size(), alpha) == 0);
  assert(alpha.StartsWith("al"));
  assert(alphabet.EndsWith("bet"));
  assert(!alpha.StartsWith("beta"));
  assert(!alpha.EndsWith("ha!"));

  nuostl::NuoStringView phrase("one two one");
  assert(phrase.Find("two") == 4);
  assert(phrase.Find('o', 1) == 6);
  assert(phrase.Find("missing") == nuostl::NuoStringView::NPos);
  assert(phrase.Find("", phrase.Size()) == phrase.Size());
  assert(phrase.Find("", phrase.Size() + 1) == nuostl::NuoStringView::npos);
  assert(phrase.RFind("one") == 8);
  assert(phrase.RFind("one", nuostl::NuoStringView::NPos, 3) == 8);
  assert(phrase.RFind('o') == 8);
  assert(phrase.FindFirstOf("tw") == 4);
  assert(phrase.FindLastOf("oe") == 10);
  assert(phrase.FindFirstNotOf("one ") == 4);
  assert(phrase.FindLastNotOf("one") == 7);

  nuostl::NuoStringView first("first");
  nuostl::NuoStringView second("second");
  nuostl::NuoSwap(first, second);
  assert(first == "second");
  assert(second == "first");

  nuostl::NuoBasicStringView<wchar_t> wide(L"wide");
  static_assert(std::is_same<decltype(wide)::value_type, wchar_t>::value,
                "basic view should preserve character type");
  assert(wide.Size() == 4);
  assert(wide[0] == L'w');
}

} /* namespace test */
