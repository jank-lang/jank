#include <algorithm>
#include <locale>
#include <codecvt>
#include <cwctype>
#include <ranges>

#include <jank/util/string.hpp>

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

  /* Doesn't support Unicode characters. */
  void capitalize(std::string &s)
  {
    if(s.empty())
    {
      return;
    }
    s[0] = static_cast<char>(std::toupper(s[0]));
  }

  std::string ordinal_under_100(usize const n)
  {
    static std::array<std::string, 20> const ordinal{
      "zeroth",    "first",     "second",      "third",      "fourth",
      "fifth",     "sixth",     "seventh",     "eighth",     "ninth",
      "tenth",     "eleventh",  "twelfth",     "thirteenth", "fourteenth",
      "fifteenth", "sixteenth", "seventeenth", "eighteenth", "nineteenth"
    };
    static std::array<std::string, 10> const tens{
      "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
    };
    static std::array<std::string, 10> const tens_ordinal{ "",          "",           "twentieth",
                                                           "thirtieth", "fortieth",   "fiftieth",
                                                           "sixtieth",  "seventieth", "eightieth",
                                                           "ninetieth" };

    if(n < 20)
    {
      return ordinal[n];
    }
    usize const t{ n / 10 }, u{ n % 10 };
    return (u == 0) ? tens_ordinal[t] : tens[t] + "-" + ordinal[u];
  }

  std::string number_to_ordinal(usize const n)
  {
    if(n > 999)
    {
      throw std::out_of_range("Number is too large");
    }

    usize const num{ n + 1 };

    if(num == 1000)
    {
      return "One thousandth";
    }

    static std::array<std::string, 10> const cardinal
      = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };

    std::string result;
    if(num < 100)
    {
      result = ordinal_under_100(num);
    }
    else
    {
      usize const h{ num / 100 };
      usize const rem{ num % 100 };
      if(rem == 0)
      {
        result = cardinal[h] + " hundredth";
      }
      else
      {
        result = cardinal[h] + " hundred " + ordinal_under_100(rem);
      }
    }
    capitalize(result);
    return result;
  }
}
