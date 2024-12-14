#include <fmt/format.h>

#include <jank/util/escape.hpp>

namespace jank::util
{
  string_result<native_transient_string> unescape(native_transient_string const &input)
  {
    native_transient_string ss;
    ss.reserve(input.size());
    native_bool escape{};

    for(auto const c : input)
    {
      if(!escape)
      {
        if(c == '\\')
        {
          escape = true;
        }
        else
        {
          ss += c;
        }
      }
      else
      {
        switch(c)
        {
          case 'n':
            ss += '\n';
            break;
          case 't':
            ss += '\t';
            break;
          case 'r':
            ss += '\r';
            break;
          case '\\':
            ss += '\\';
            break;
          case '"':
            ss += '"';
            break;
          case 'a':
            ss += '\a';
            break;
          case 'v':
            ss += '\v';
            break;
          case '?':
            ss += '\?';
            break;
          case 'f':
            ss += '\f';
            break;
          case 'b':
            ss += '\b';
            break;
          default:
            return err(fmt::format("invalid escape sequence: \\{}", c));
        }
        escape = false;
      }
    }

    return ok(std::move(ss));
  }

  native_transient_string escape(native_transient_string const &input)
  {
    native_transient_string ss;
    /* We can expect on relocation, since escaping anything will result in a larger string.
      * I'm not going to guess at the stats, to predict a better allocation, until this shows
      * up in the profiler, though. */
    ss.reserve(input.size());

    for(auto const c : input)
    {
      switch(c)
      {
        case '\n':
          ss += '\\';
          ss += 'n';
          break;
        case '\t':
          ss += '\\';
          ss += 't';
          break;
        case '\r':
          ss += '\\';
          ss += 'r';
          break;
        case '\\':
          ss += '\\';
          ss += '\\';
          break;
        case '"':
          ss += '\\';
          ss += '"';
          break;
        case '\a':
          ss += '\\';
          ss += 'a';
          break;
        case '\v':
          ss += '\\';
          ss += 'v';
          break;
        case '\?':
          ss += '\\';
          ss += '?';
          break;
        case '\f':
          ss += '\\';
          ss += 'f';
          break;
        case '\b':
          ss += '\\';
          ss += 'b';
          break;
        default:
          ss += c;
      }
    }

    return ss;
  }
}
