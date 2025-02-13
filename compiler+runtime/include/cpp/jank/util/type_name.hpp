#pragma once

#include <string_view>
#include <array>
#include <utility>

namespace jank::util
{
  template <std::size_t... Idxs>
  constexpr auto substring_as_array(std::string_view const str, std::index_sequence<Idxs...> const)
  {
    return std::array{ str[Idxs]... };
  }

  template <typename T>
  constexpr auto type_name_array()
  {
    /* Based on using this code: https://gist.github.com/jeaye/7e11ee6e78164f41a0342c166d294dd0 */
#if defined(__clang__)
    /* void foo() [T = std::basic_string<char>] */
    constexpr auto prefix{ std::string_view{ "[T = " } };
    constexpr auto suffix{ std::string_view{ "]" } };
    constexpr auto function{ std::string_view{ __PRETTY_FUNCTION__ } };
#elif defined(__GNUC__)
    /* void foo() [with T = std::__cxx11::basic_string<char>] */
    constexpr auto prefix{ std::string_view{ "with T = " } };
    constexpr auto suffix{ std::string_view{ "]" } };
    constexpr auto function{ std::string_view{ __PRETTY_FUNCTION__ } };
#elif defined(_MSC_VER)
    /* void __cdecl foo<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >>(void) */
    constexpr auto prefix{ std::string_view{ "type_name_array<" } };
    constexpr auto suffix{ std::string_view{ ">(void)" } };
    constexpr auto function{ std::string_view{ __FUNCSIG__ } };
#endif

    constexpr auto start{ function.find(prefix) + prefix.size() };
    constexpr auto end{ function.rfind(suffix) };

    static_assert(start < end);

    constexpr auto name{ function.substr(start, (end - start)) };
    return substring_as_array(name, std::make_index_sequence<name.size()>{});
  }

  template <typename T>
  struct type_name_holder
  {
    static constexpr auto value{ type_name_array<T>() };
  };

  /* This is incredibly hacky. C++ doesn't offer a standard way to get an unmangled type from
   * an arbitrary type. To work around this, we use the __PRETTY_FUNCTION__ macro, which
   * contains the current function we're in, including the template parameters. We then
   * instantiate a template using the type we want, check the __PRETTY_FUNCTION__ macro,
   * and parse out the type name. For example:
   *
   * void foo() [T = std::basic_string<char>]
   *
   * But there's a catch. Every compiler formats the text in their own way. So we need
   * special handling, depending on the compiler, to know how to pull out the type from
   * the string.
   *
   * With that done, we just do some template work to turn the string literal into a
   * constexpr static variable to which we can have a string_view. Entirely at compile-time.
   * However, we only want to keep the portion we care about in the resulting binary, not
   * the whole pretty function string, so we copy it into an array which contains only
   * the type name.
   *
   * Just do type_name<T>() and there's your string_view. */
  template <typename T>
  constexpr std::string_view type_name()
  {
    constexpr auto &value{ type_name_holder<T>::value };
    return { value.data(), value.size() };
  }
}
