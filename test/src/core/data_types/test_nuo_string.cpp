#include "core/data_types/test_nuo_string.hpp"

#include <assert.h>

#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/data_types/nuo_string.hpp"
#include "utils/nuo_util.hpp"

namespace test
{

void TestNuoString::test_nuo_string()
{
  nuostl::NuoString empty;
  assert(empty.Empty());
  assert(empty.Size() == 0);
  assert(empty.Length() == 0);
  assert(empty.Begin() == empty.End());
  assert(empty.CStr()[0] == '\0');
  assert(empty.Find(nuostl::NuoStringView()) == 0);
  assert(empty.RFind(nuostl::NuoStringView()) == 0);

  nuostl::NuoString text("hello");
  assert(text.Size() == 5);
  assert(text[0] == 'h');
  assert(text.At(4) == 'o');
  assert(text.Front() == 'h');
  assert(text.Back() == 'o');
  text[0] = 'H';
  assert(text == "Hello");
  assert(text.Data() == text.CStr());
  text.Reserve(64);
  assert(text.Capacity() >= 64);
  text.Resize(7, '!');
  assert(text == "Hello!!");
  text.Resize(5);
  assert(text == "Hello");
  text.ShrinkToFit();
  assert(text.GetAllocator() == std::allocator<char>());

  bool caught_out_of_range = false;
  try
  {
    text.At(text.Size());
  }
  catch (const std::out_of_range&)
  {
    caught_out_of_range = true;
  }
  assert(caught_out_of_range);

  const char embedded[] = {'a', '\0', 'b', 'c'};
  nuostl::NuoString bounded(embedded, sizeof(embedded));
  assert(bounded.Size() == 4);
  assert(bounded[1] == '\0');
  assert(bounded.CStr()[4] == '\0');

  nuostl::NuoString repeated(3, 'x');
  assert(repeated == "xxx");
  nuostl::NuoString copied(text);
  nuostl::NuoString moved(nuostl::NuoMove(copied));
  assert(moved == text);
  assert(copied.Empty());

  std::string source = "from std::string";
  nuostl::NuoString from_std(source);
  assert(from_std == source.c_str());
  nuostl::NuoString from_view(nuostl::NuoStringView("view"));
  assert(from_view == "view");

  text.Append(" world");
  text.PushBack('!');
  assert(text == "Hello world!");
  text.PopBack();
  assert(text == "Hello world");
  text.Append(2, '?');
  assert(text == "Hello world??");
  text.Clear();
  assert(text.Empty());
  text.Assign("assigned");
  assert(text == "assigned");
  text += " + ";
  text += nuostl::NuoString("value");
  text += '!';
  assert(text == "assigned + value!");
  assert(nuostl::NuoString("left") + " right" == "left right");
  assert("left " + nuostl::NuoString("right") == "left right");
  text.Assign("assigned");

  text.Insert(8, "!");
  text.Insert(0, 2, '#');
  assert(text == "##assigned!");
  text.Erase(0, 2);
  assert(text == "assigned!");
  text.Replace(0, 8, "replaced");
  assert(text == "replaced!");

  nuostl::NuoString phrase("one two one");
  assert(phrase.Find("two") == 4);
  assert(phrase.Find('o', 1) == 6);
  assert(phrase.RFind("one") == 8);
  assert(phrase.Find("missing") == nuostl::NuoString::NPos);
  assert(phrase.Find(nuostl::NuoStringView()) == 0);
  assert(phrase.Find(nuostl::NuoStringView(), 99) == nuostl::NuoString::NPos);
  assert(phrase.RFind(nuostl::NuoStringView()) == phrase.Size());
  assert(phrase.Substr(4, 3) == "two");
  assert(phrase.StartsWith("one"));
  assert(phrase.EndsWith("one"));
  assert(phrase.View() == nuostl::NuoStringView("one two one"));
  char copy_buffer[5] = {};
  assert(phrase.Copy(copy_buffer, 3, 4) == 3);
  assert(std::string(copy_buffer, 3) == "two");

  assert(nuostl::NuoString("alpha") < nuostl::NuoString("beta"));
  assert(nuostl::NuoString("alpha") != "beta");
  assert("alpha" == nuostl::NuoString("alpha"));

  nuostl::NuoString first("first");
  nuostl::NuoString second("second");
  first.Swap(second);
  assert(first == "second");
  assert(second == "first");
  nuostl::NuoSwap(first, second);
  assert(first == "first");
  assert(second == "second");

  nuostl::NuoBasicString<wchar_t> wide(L"wide");
  static_assert(std::is_same<decltype(wide)::value_type, wchar_t>::value,
                "basic string should preserve character type");
  assert(wide.Size() == 4);
  assert(wide[0] == L'w');
}

} /* namespace test */
