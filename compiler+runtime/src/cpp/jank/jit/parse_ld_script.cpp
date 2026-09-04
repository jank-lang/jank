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
    /* Half-open [begin, end) byte range, within the (comment-stripped) script text, spanning
     * the contents inside the outermost parentheses of a GROUP(...) or INPUT(...) directive. */
    struct directive_range
    {
      std::size_t begin;
      std::size_t end;
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
                           std::size_t const position,
                           std::string_view const directive) noexcept
    {
      if(position + directive.size() > source.size()
         || source.compare(position, directive.size(), directive) != 0)
      {
        return false;
      }

      /* Require word boundaries around the directive name so we don't match a substring of a
       * longer identifier (e.g. some hypothetical `MY_GROUP` symbol). */
      auto const has_identifier_before{ position > 0
                                        && is_identifier_character(source[position - 1]) };
      auto const directive_end{ position + directive.size() };
      auto const has_identifier_after{ directive_end < source.size()
                                       && is_identifier_character(source[directive_end]) };
      return !has_identifier_before && !has_identifier_after;
    }

    /* Strips C-style block comments, which ld scripts (including the "GNU ld script" header
     * comment glibc adds) may contain, before we try to find directives. We don't need to
     * handle ld's line comments (# ...) since glibc's generated scripts don't use them, and
     * skipping that keeps this simpler. */
    std::string strip_comments(std::string_view const source)
    {
      std::string result;
      result.reserve(source.size());

      bool in_comment{ false };
      for(std::size_t i{}; i < source.size(); ++i)
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
       * ld scripts (e.g. glibc's libm.so) only ever contain one, and this keeps us from having
       * to build a full ld script grammar for directives we don't need (OUTPUT_FORMAT,
       * SEARCH_DIR, etc). */
      static std::array<std::string_view, 2> const directives{ "GROUP", "INPUT" };
      std::size_t parenthesis_depth{};

      for(std::size_t i{}; i < source.size(); ++i)
      {
        /* Only look for directives when we're not already nested inside some other
         * parenthesized construct, so we don't, say, match "INPUT" appearing as an argument
         * to another directive. */
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

            /* Track nested parentheses (e.g. from AS_NEEDED(...) inside GROUP(...)) so we find
             * the matching close paren for the directive itself, not an inner one. */
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
     * both quoted tokens (ld scripts may quote paths containing special characters) and bare
     * tokens delimited by whitespace/commas/parens. Returns an empty string at `end` or when
     * the caller calls this pointed at a delimiter (which it filters out before calling). */
    std::string
    read_token(std::string_view const source, std::size_t &position, std::size_t const end)
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

  /* Distinguishes a real object/shared-library file from a textual GNU ld linker script (the
   * form glibc ships some "shared libraries" as, e.g. libm.so, in order to pull in a GROUP of
   * several actual .so files). LLVM's JIT dynamic loader can only load real ELF/Mach-O/PE
   * binaries, so we peek at the file's leading bytes to detect one before handing it off.
   *
   * We check for every format jank might plausibly encounter across its supported platforms
   * (Linux, macOS, and Windows), even though only one is relevant on any given build, since
   * this is cheap and keeps the check self-contained and easy to extend. */
  bool is_object_file(jtl::immutable_string const &path)
  {
    std::array<unsigned char, 4> magic{};
    std::ifstream file{ path.c_str(), std::ios::binary };
    if(!file.read(reinterpret_cast<char *>(magic.data()), magic.size()))
    {
      /* Files shorter than 4 bytes, or otherwise unreadable, can't be a real object file. Ld
       * scripts are always longer than this, so it's safe to report false here rather than
       * treating this as "unknown". */
      return false;
    }

    if(magic == std::array<unsigned char, 4>{ 0x7f, 'E', 'L', 'F' } /* ELF (Linux). */
       || magic == std::array<unsigned char, 4>{ 0xfe, 0xed, 0xfa, 0xce }
       /* Mach-O 32-bit, big-endian. */
       || magic == std::array<unsigned char, 4>{ 0xce, 0xfa, 0xed, 0xfe }
       /* Mach-O 32-bit, little-endian. */
       || magic == std::array<unsigned char, 4>{ 0xfe, 0xed, 0xfa, 0xcf }
       /* Mach-O 64-bit, big-endian. */
       || magic == std::array<unsigned char, 4>{ 0xcf, 0xfa, 0xed, 0xfe }
       /* Mach-O 64-bit, little-endian (macOS). */
       || magic == std::array<unsigned char, 4>{ 0xca, 0xfe, 0xba, 0xbe }
       /* Mach-O fat/universal binary, big-endian. */
       || magic == std::array<unsigned char, 4>{ 0xbe, 0xba, 0xfe, 0xca }
       /* Mach-O fat/universal binary, little-endian. */
       || magic == std::array<unsigned char, 4>{ 0xca, 0xfe, 0xba, 0xbf }
       /* Mach-O fat/universal binary, 64-bit, big-endian. */
       || magic
         == std::array<unsigned char, 4>{
           0xbf,
           0xba,
           0xfe,
           0xca } /* Mach-O fat/universal binary, 64-bit, little-endian. */)
    {
      return true;
    }

    /* PE/COFF (Windows DLLs) start with the "MZ" DOS header magic; we don't need to look any
     * further into the PE header, since only the first two bytes are needed to disambiguate
     * this from a text-based ld script. */
    return magic[0] == 'M' && magic[1] == 'Z';
  }

  /* Attempts to parse `path` as a GNU ld linker script and extract the real shared library it
   * points at. glibc, on many Linux systems (notably Nix), ships some "shared libraries" (e.g.
   * libm.so) as text scripts like:
   *
   *   GROUP ( /path/to/libm.so.6 AS_NEEDED ( /path/to/libmvec.so.1 ) )
   *
   * Here, libm.so.6 is the library we actually want to load; libmvec.so.1 is only pulled in
   * if something in the binary actually needs symbols from it (that's what AS_NEEDED means to
   * the real `ld`). We don't do the "does anything need this" analysis ourselves, so we prefer
   * the first library that's *not* wrapped in AS_NEEDED, since that's the one the script
   * considers unconditionally required. If every path is wrapped in AS_NEEDED, we fall back to
   * the very first one we saw, on the assumption that it's more likely to be useful than
   * loading nothing at all. */
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
    std::size_t position{ range.begin };
    /* How many nested AS_NEEDED(...) wrappers we're currently inside of, while scanning the
     * directive's argument list. */
    std::size_t as_needed_depth{};
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
