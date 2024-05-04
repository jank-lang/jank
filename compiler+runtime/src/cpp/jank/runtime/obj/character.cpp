#include <iostream>
#include <sstream>

#include <jank/runtime/obj/character.hpp>

namespace jank::runtime
{
  obj::character::static_object(native_char const &d)
    : data{ d }
  {
  }

  native_bool obj::character::equal(object const &o) const
  {
    if(o.type != object_type::character)
    {
      return false;
    }

    auto const c(expect_object<obj::character>(&o));
    return data == c->data;
  }

  void obj::character::to_string(fmt::memory_buffer &buff) const
  {
    native_persistent_string sv;
    switch(data)
    {
      case '\n':
        sv = "newline";
        break;
      case ' ':
        sv = "space";
        break;
      case '\t':
        sv = "tab";
        break;
      case '\b':
        sv = "backspace";
        break;
      case '\f':
        sv = "formfeed";
        break;
      case '\r':
        sv = "return";
        break;
      default:
        sv = std::string(1, static_cast<char>(data));
    }
    fmt::format_to(std::back_inserter(buff), "\\{}", sv);
  }

  native_persistent_string obj::character::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::character::to_hash() const
  {
    // TODO(saket): fix this
    return hash::real(data);
  }
}
