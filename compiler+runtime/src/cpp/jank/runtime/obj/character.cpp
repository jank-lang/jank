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

  static native_persistent_string get_repr_from_char(char const ch)
  {
    switch(ch)
    {
      case '\n':
        return "\\newline";
      case ' ':
        return "\\space";
      case '\t':
        return "\\tab";
      case '\b':
        return "\\backspace";
      case '\f':
        return "\\formfeed";
      case '\r':
        return "\\return";
      default:
        return fmt::format("\\{}", ch);
    }
  }

  obj::character::static_object(native_persistent_string const &d)
    : data{ d }
  {
  }

  obj::character::static_object(char const ch)
    : data{ get_repr_from_char(ch) }
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
    /* TODO: This is actually to_representation, since the string version of \a is just a. */
    fmt::format_to(std::back_inserter(buff), "{}", data);
  }

  native_persistent_string const &obj::character::to_string() const
  {
    return data;
  }

  native_hash obj::character::to_hash() const
  {
    return hash::visit(get_char_from_repr(data).unwrap());
  }
}
