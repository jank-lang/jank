#include <jank/jit/parse_ld_script.hpp>

#include <array>
#include <cctype>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace jank::jit
{
  namespace
  {
    struct directive_range
    {
      usize begin{};
      usize end{};
    };

    bool is_identifier_character(char const c) noexcept
    {
      auto const unsigned_c{ static_cast<unsigned char>(c) };
      return std::isalnum(unsigned_c) != 0 || c == '_';
    }

    bool is_separator(char const c) noexcept
    {
      auto const unsigned_c{ static_cast<unsigned char>(c) };
      return std::isspace(unsigned_c) != 0 || c == ',';
    }

    bool matches_directive(std::string_view const source,
                           usize const position,
                           std::string_view const directive) noexcept
    {
      if(position + directive.size() > source.size()
         || source.compare(position, directive.size(), directive) != 0)
      {
        return false;
      }

      /* Require word boundaries around the directive name so we don't match a substring of a
       * longer identifier. */
      auto const has_identifier_before{ position > 0
                                        && is_identifier_character(source[position - 1]) };
      auto const directive_end{ position + directive.size() };
      auto const has_identifier_after{ directive_end < source.size()
                                       && is_identifier_character(source[directive_end]) };
      return !has_identifier_before && !has_identifier_after;
    }

    std::string strip_comments(std::string_view const source)
    {
      std::string result;
      result.reserve(source.size());

      bool in_comment{ false };
      for(usize i{}; i < source.size(); ++i)
      {
        if(in_comment)
        {
          if(source[i] == '*' && i + 1 < source.size() && source[i + 1] == '/')
          {
            in_comment = false;
            ++i;
          }
          continue;
        }

        if(source[i] == '/' && i + 1 < source.size() && source[i + 1] == '*')
        {
          in_comment = true;
          ++i;
          continue;
        }

        result.push_back(source[i]);
      }

      return result;
    }

    jtl::option<directive_range> find_input_directive(std::string_view const source)
    {
      /* We only care about the first top-level GROUP(...) or INPUT(...) we find. Real-world
       * ld scripts only ever contain one. I'm not going to write a full robust parser. */
      static std::array<std::string_view, 2> const directives{ "GROUP", "INPUT" };
      usize parenthesis_depth{};

      for(usize i{}; i < source.size(); ++i)
      {
        /* Only look for directives when we're not already nested inside some other
         * parenthesized construct. This way we don't match "INPUT" appearing as an argument
         * to another directive, for example. */
        if(parenthesis_depth == 0)
        {
          for(auto const directive : directives)
          {
            if(!matches_directive(source, i, directive))
            {
              continue;
            }

            auto open_paren{ i + directive.size() };
            while(open_paren < source.size() && is_separator(source[open_paren]))
            {
              ++open_paren;
            }
            if(open_paren >= source.size() || source[open_paren] != '(')
            {
              continue;
            }

            /* Track nested parentheses so we find the matching close paren for the
             * directive itself, not an inner one. */
            auto depth{ 1u };
            for(auto close_paren{ open_paren + 1 }; close_paren < source.size(); ++close_paren)
            {
              if(source[close_paren] == '(')
              {
                ++depth;
              }
              else if(source[close_paren] == ')')
              {
                --depth;
                if(depth == 0)
                {
                  return directive_range{ open_paren + 1, close_paren };
                }
              }
            }

            /* Unbalanced parentheses; treat this as an unparsable script. */
            return none;
          }
        }

        if(source[i] == '(')
        {
          ++parenthesis_depth;
        }
        else if(source[i] == ')' && parenthesis_depth > 0)
        {
          --parenthesis_depth;
        }
      }

      return none;
    }

    /* Reads a single token from `source[position, end)`, advancing `position` past it. Handles
     * both quoted tokens and bare tokens delimited by whitespace/commas/parens.
     * Returns an empty string at `end`. */
    std::string read_token(std::string_view const source, usize &position, usize const end)
    {
      if(position >= end)
      {
        return {};
      }

      if(source[position] == '"' || source[position] == '\'')
      {
        auto const quote{ source[position++] };
        std::string token;
        while(position < end && source[position] != quote)
        {
          if(source[position] == '\\' && position + 1 < end)
          {
            token.push_back(source[position + 1]);
            position += 2;
          }
          else
          {
            token.push_back(source[position++]);
          }
        }
        if(position < end)
        {
          ++position;
        }
        return token;
      }

      auto const begin{ position };
      while(position < end && !is_separator(source[position]) && source[position] != '('
            && source[position] != ')')
      {
        ++position;
      }
      return std::string{ source.substr(begin, position - begin) };
    }

    jtl::option<std::string> read_file(jtl::immutable_string const &path)
    {
      std::ifstream file{ path.c_str(), std::ios::binary };
      if(!file)
      {
        return none;
      }

      std::string contents{ std::istreambuf_iterator<char>{ file },
                            std::istreambuf_iterator<char>{} };
      if(file.bad())
      {
        return none;
      }
      return contents;
    }
  }

  bool is_elf_file(jtl::immutable_string const &path)
  {
    std::array<unsigned char, 4> magic{};
    std::ifstream file{ path.c_str(), std::ios::binary };
    if(!file.read(reinterpret_cast<char *>(magic.data()), magic.size()))
    {
      /* Files shorter than 4 bytes, or otherwise unreadable, can't be a real object file. */
      return false;
    }

    return magic == std::array<unsigned char, 4>{ 0x7f, 'E', 'L', 'F' };
  }

  /* Attempts to parse `path` as a GNU ld linker script and extract the real shared library it
   * points at. glibc, on many Linux systems, ships some "shared libraries" as text scripts like:
   *
   *   GROUP ( /path/to/libm.so.6 AS_NEEDED ( /path/to/libmvec.so.1 ) )
   *
   * Here, libm.so.6 is the library we actually want to load. libmvec.so.1 is only pulled in
   * if something in the binary actually needs symbols from it. We don't do the
   * "does anything need this" analysis ourselves, so we prefer the first library that's
   * not wrapped in AS_NEEDED. If every path is wrapped in AS_NEEDED, we fall back to
   * the very first one we saw. That's more likely to be useful than loading nothing at all. */
  jtl::option<jtl::immutable_string> parse_ld_script(jtl::immutable_string const &path)
  {
    auto const contents{ read_file(path) };
    if(contents.is_none())
    {
      return none;
    }

    auto const script{ strip_comments(contents.unwrap()) };
    auto const directive{ find_input_directive(script) };
    if(directive.is_none())
    {
      return none;
    }

    auto const range{ directive.unwrap() };
    usize position{ range.begin };
    /* How many nested AS_NEEDED(...) wrappers we're currently inside of, while scanning the
     * directive's argument list. */
    usize as_needed_depth{};
    bool found_path{};
    bool found_required_path{};
    std::string first_path;
    std::string first_required_path;

    while(position < range.end)
    {
      if(is_separator(script[position]))
      {
        ++position;
        continue;
      }

      if(script[position] == '(')
      {
        ++position;
        continue;
      }

      if(script[position] == ')')
      {
        if(as_needed_depth > 0)
        {
          --as_needed_depth;
        }
        ++position;
        continue;
      }

      auto token{ read_token(script, position, range.end) };
      if(token.empty())
      {
        continue;
      }

      if(token == "AS_NEEDED")
      {
        while(position < range.end && is_separator(script[position]))
        {
          ++position;
        }
        if(position < range.end && script[position] == '(')
        {
          ++as_needed_depth;
          ++position;
        }
        continue;
      }

      if(!found_path)
      {
        first_path = token;
        found_path = true;
      }
      if(as_needed_depth == 0 && !found_required_path)
      {
        first_required_path = token;
        found_required_path = true;
      }
    }

    if(found_required_path)
    {
      return jtl::immutable_string{ first_required_path };
    }
    if(found_path)
    {
      return jtl::immutable_string{ first_path };
    }
    return none;
  }
}
