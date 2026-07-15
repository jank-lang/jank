#include <charconv>

#include <iostream>

#include <jtl/primitive.hpp>
#include <jtl/string_builder.hpp>
#include <jtl/utf8.hpp>

#include <jank/util/escape.hpp>
#include <jank/util/fmt.hpp>

namespace jank::util
{
  static jtl::result<u16, unescape_error> unescape_unicode(jtl::immutable_string const &input)
  {
    u16 codepoint{};
    auto const unicode_buffer(input.c_str());
    auto const result(std::from_chars(unicode_buffer, unicode_buffer + 4, codepoint, 16));

    if(result.ec != std::errc())
    {
      return err(unescape_error{
        util::format("String contains invalid unicode escape sequence '\\u{}'.", unicode_buffer) });
    }

    return ok(codepoint);
  }

  /* Converts escape sequences starting with backslash to their mapped character. e.g., \" => " */
  jtl::result<jtl::immutable_string, unescape_error> unescape(jtl::immutable_string const &input)
  {
    jtl::string_builder sb{ input.size() };
    bool escape{};
    usize i{};

    while(i < input.size())
    {
      auto const c(input[i]);

      if(c == '\\' && input[i + 1] == 'u')
      {
        auto const high(unescape_unicode(input.substr(i + 2, 4)));
        i += 6;

        if(high.is_err())
        {
          return high.expect_err();
        }

        auto const codepoint_high(high.expect_ok());

        if(!jtl::is_surrogate_pairs(codepoint_high))
        {
          sb(jtl::to_char(codepoint_high));
          continue;
        }

        if(input[i] != '\\' || input[i + 1] != 'u')
        {
          sb('?');
          continue;
        }

        auto const low(unescape_unicode(input.substr(i + 2, 4)));
        i += 6;

        if(low.is_err())
        {
          return low.expect_err();
        }

        sb(jtl::to_char(jtl::combine_surrogate_pairs(codepoint_high, low.expect_ok())));
      }
      else if(!escape)
      {
        if(c == '\\')
        {
          escape = true;
        }
        else
        {
          sb(c);
        }
        ++i;
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
            return err(
              unescape_error{ util::format("String contains invalid escape sequence '\\{}'.", c) });
        }
        escape = false;
        ++i;
      }
    }

    return ok(sb.release());
  }

  /* Converts special characters to their escape sequences. e.g., " => \" */
  jtl::immutable_string escape(jtl::immutable_string const &input)
  {
    /* We can expect on relocation, since escaping anything will result in a larger string.
     * I'm not going to guess at the stats, to predict a better allocation, until this shows
     * up in the profiler, though. */
    jtl::string_builder sb{ input.size() };

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
