#include <fmt/format.h>

#include <jank/util/escape.hpp>
#include <jank/util/string_builder.hpp>

namespace jank::util
{
  /* Converts escape sequences starting with backslash to their mapped character. e.g., \" => " */
  result<native_persistent_string, unescape_error> unescape(native_persistent_string const &input)
  {
    util::string_builder sb{ input.size() };
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
          sb(c);
        }
      }
      else
      {
        switch(c)
        {
          case 'n':
            sb('\n');
            break;
          case 't':
            sb('\t');
            break;
          case 'r':
            sb('\r');
            break;
          case '\\':
            sb('\\');
            break;
          case '"':
            sb('"');
            break;
          case 'a':
            sb('\a');
            break;
          case 'v':
            sb('\v');
            break;
          case '?':
            sb('?');
            break;
          case 'f':
            sb('\f');
            break;
          case 'b':
            sb('\b');
            break;
          default:
            return err(unescape_error{ fmt::format("Invalid escape sequence '\\{}'", c) });
        }
        escape = false;
      }
    }

    return ok(sb.release());
  }

  /* Converts special characters to their escape sequences. e.g., " => \" */
  native_persistent_string escape(native_persistent_string const &input)
  {
    /* We can expect on relocation, since escaping anything will result in a larger string.
     * I'm not going to guess at the stats, to predict a better allocation, until this shows
     * up in the profiler, though. */
    util::string_builder sb{ input.size() };

    for(auto const c : input)
    {
      switch(c)
      {
        case '\n':
          sb('\\');
          sb('n');
          break;
        case '\t':
          sb('\\');
          sb('t');
          break;
        case '\r':
          sb('\\');
          sb('r');
          break;
        case '\\':
          sb('\\');
          sb('\\');
          break;
        case '"':
          sb('\\');
          sb('"');
          break;
        case '\a':
          sb('\\');
          sb('a');
          break;
        case '\v':
          sb('\\');
          sb('v');
          break;
        case '\f':
          sb('\\');
          sb('f');
          break;
        case '\b':
          sb('\\');
          sb('b');
          break;
        default:
          sb(c);
      }
    }

    return sb.release();
  }
}
