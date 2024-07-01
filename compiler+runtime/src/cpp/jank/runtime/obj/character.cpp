#include <jank/runtime/obj/character.hpp>

namespace jank::runtime
{

  static option<char> get_char_from_repr(native_persistent_string const &sv)
  {
    if(sv.size() == 2)
    {
      return sv[1];
    }
    else if(sv == "\\newline")
    {
      return '\n';
    }
    else if(sv == "\\space")
    {
      return ' ';
    }
    else if(sv == "\\tab")
    {
      return '\t';
    }
    else if(sv == "\\backspace")
    {
      return '\b';
    }
    else if(sv == "\\formfeed")
    {
      return '\f';
    }
    else if(sv == "\\return")
    {
      return '\r';
    }

    return none;
  }

  obj::character::static_object(native_persistent_string_view const &d)
    : data{ d }
  {
  }

  obj::character::static_object(native_persistent_string const &d)
    : data{ d }
  {
  }

  obj::character::static_object(char ch)
    : data{ 1, ch }
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
    fmt::format_to(std::back_inserter(buff), "{}", data);
  }

  native_persistent_string obj::character::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::character::to_hash() const
  {
    return static_cast<native_hash>(get_char_from_repr(data).unwrap());
  }
}
