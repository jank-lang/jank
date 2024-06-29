#include <jank/option.hpp>

namespace jank::util::character
{
  option<char> get_char_from_repr(native_persistent_string_view const &sv)
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
}
