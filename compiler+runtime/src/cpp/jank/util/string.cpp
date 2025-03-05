#include <algorithm>
#include <locale>
#include <codecvt>
#include <cwctype>

#include <jank/util/string.hpp>
#include <ranges>

namespace jank::util
{
  std::string to_lowercase(std::string const &s)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide{ converter.from_bytes(s) };
    std::locale const loc{ "" };

    auto &facet{ std::use_facet<std::ctype<wchar_t>>(loc) };
    facet.tolower(wide.data(), wide.data() + wide.size());

    return converter.to_bytes(wide);
  }

  std::string to_uppercase(std::string const &s)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide{ converter.from_bytes(s) };
    std::locale const loc{ "" };

    auto const &facet{ std::use_facet<std::ctype<wchar_t>>(loc) };
    facet.toupper(wide.data(), wide.data() + wide.size());

    return converter.to_bytes(wide);
  }

  /* Doesn't support Unicode whitespace. */
  void trim(std::string &s)
  {
    auto const not_space{ [](unsigned char const ch) { return !std::isspace(ch); } };
    s.erase(s.begin(), std::ranges::find_if(s, not_space));
    s.erase(std::ranges::find_if(std::ranges::reverse_view(s), not_space).base(), s.end());
  }
}
