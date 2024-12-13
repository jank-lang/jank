#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/escape.hpp>

namespace jank::runtime::obj
{
  static native_persistent_string get_literal_from_char_bytes(native_persistent_string const &bytes)
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
          return fmt::format(R"(\{})", bytes[0]);
      }
    }
    else
    {
      return fmt::format(R"(\{})", bytes);
    }
  }

  character::character(native_persistent_string const &d)
    : data{ d }
  {
  }

  character::character(char const ch)
    : data{ 1, ch }
  {
  }

  native_bool character::equal(object const &o) const
  {
    if(o.type != object_type::character)
    {
      return false;
    }

    auto const c(expect_object<character>(&o));
    return data == c->data;
  }

  void character::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff), "{}", data);
  }

  native_persistent_string character::to_string() const
  {
    return data;
  }

  native_persistent_string character::to_code_string() const
  {
    return get_literal_from_char_bytes(data);
  }

  native_hash character::to_hash() const
  {
    return data.to_hash();
  }
}
