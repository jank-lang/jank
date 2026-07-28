#include <charconv>

#include <jtl/primitive.hpp>
#include <jtl/string_builder.hpp>
#include <jtl/utf8.hpp>

#include <jank/util/escape.hpp>
#include <jank/util/fmt.hpp>

namespace jank::util
{
  static jtl::result<u16, unescape_error> unescape_unicode(jtl::immutable_string const &input)
  {
    if(input.size() != 4)
    {
      return err(unescape_error{
        util::format("String contains invalid unicode escape sequence '\\u{}'.", input) });
    }

    u16 codepoint{};
    auto const unicode_buffer(input.c_str());
    auto const unicode_end(unicode_buffer + 4);
    auto const result(std::from_chars(unicode_buffer, unicode_end, codepoint, 16));

    if(result.ec != std::errc() || result.ptr != unicode_end)
    {
      return err(unescape_error{
        util::format("String contains invalid unicode escape sequence '\\u{}'.", input) });
    }

    return ok(codepoint);
  }

  /* Converts escape sequences starting with backslash to their mapped character. e.g., \" => " */
  jtl::result<jtl::immutable_string, unescape_error> unescape(jtl::immutable_string const &input)
  {
    auto const size(input.size());
    jtl::string_builder sb{ size };
    bool escape{};
    usize i{};

    while(i < size)
    {
      auto const c(input[i]);

      if(!escape && c == '\\' && (i + 1) < size && input[i + 1] == 'u')
      {
        auto const high(unescape_unicode(input.substr(i + 2, 4)));
        i += 6;

        if(high.is_err())
        {
          return high.expect_err();
        }

        auto const codepoint_high(high.expect_ok());

        if(!jtl::is_surrogate_high(codepoint_high))
        {
          sb(jtl::to_char(codepoint_high, "?"));
          continue;
        }

        if((i + 6) > size || input[i] != '\\' || input[i + 1] != 'u')
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

        auto const codepoint_low(low.expect_ok());

        if(!jtl::is_surrogate_low(codepoint_low))
        {
          sb("?");
          sb(jtl::to_char(codepoint_low, "?"));
          continue;
        }

        sb(jtl::to_char(jtl::combine_surrogate_pair(codepoint_high, codepoint_low)));
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
