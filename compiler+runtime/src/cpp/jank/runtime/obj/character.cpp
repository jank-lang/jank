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

  character::character(jtl::immutable_string const &d)
    : data{ d }
  {
  }

  character::character(char const ch)
    : data{ 1, ch }
  {
  }

  bool character::equal(object const &o) const
  {
    if(o.type != object_type::character)
    {
      return false;
    }

    auto const c(expect_object<character>(&o));
    return data == c->data;
  }

  void character::to_string(util::string_builder &buff) const
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

  native_hash character::to_hash() const
  {
    return data.to_hash();
  }
}
