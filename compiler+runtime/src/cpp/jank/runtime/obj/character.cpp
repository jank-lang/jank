#include <cwchar>

#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/escape.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  static jtl::immutable_string get_literal_from_char_bytes(jtl::immutable_string const &bytes)
  {
    if(bytes.size() == 1)
    {
      switch(bytes[0])
      {
        case '\n':
          return R"(\newline)";
        case ' ':
          return R"(\space)";
        case '\t':
          return R"(\tab)";
        case '\b':
          return R"(\backspace)";
        case '\f':
          return R"(\formfeed)";
        case '\r':
          return R"(\return)";
        default:
          return util::format(R"(\{})", bytes[0]);
      }
    }
    else
    {
      return util::format(R"(\{})", bytes);
    }
  }

  character::character()
    : object{ obj_type, obj_behaviors }
  {
  }

  character::character(jtl::immutable_string const &d)
    : object{ obj_type, obj_behaviors }
    , data{ d }
  {
  }

  character::character(char const ch)
    : object{ obj_type, obj_behaviors }
    , data{ 1, ch }
  {
  }

  static jtl::immutable_string to_char(i64 const ch)
  {
    if(ch > 0x10FFFF)
    {
      throw std::runtime_error{ util::format("Value out of range for char: {}", ch) };
    }

    std::mbstate_t state{};
    wchar_t const wc{ static_cast<wchar_t>(ch) };
    std::string str(MB_CUR_MAX, '\0');
    auto const len{ std::wcrtomb(str.data(), wc, &state) };

    if(std::cmp_equal(len, static_cast<size_t>(-1)))
    {
      throw std::runtime_error{ util::format("Unfinished character: {}", ch) };
    }
    else if(std::cmp_equal(len, static_cast<size_t>(-2)))
    {
      throw std::runtime_error{ util::format("Invalid Unicode character: {}", ch) };
    }

    str.resize(len);

    return str;
  }

  character::character(i64 const ch)
    : object{ obj_type, obj_behaviors }
    , data{ to_char(ch) }
  {
  }

  bool character::equal(object const &o) const
  {
    if(o.type != object_type::character)
    {
      return false;
    }

    auto const c(expect_object<character>(runtime::detail::untagged(&o)));
    return data == c->data;
  }

  void character::to_string(jtl::string_builder &buff) const
  {
    buff(data);
  }

  jtl::immutable_string character::to_string() const
  {
    return data;
  }

  jtl::immutable_string character::to_code_string() const
  {
    return get_literal_from_char_bytes(data);
  }

  uhash character::to_hash() const
  {
    return data.to_hash();
  }

  i64 character::to_integer() const
  {
    std::mbstate_t state{};
    wchar_t wc{};
    std::mbrtowc(&wc, data.c_str(), data.size(), &state);
    return static_cast<i64>(wc);
  }
}
