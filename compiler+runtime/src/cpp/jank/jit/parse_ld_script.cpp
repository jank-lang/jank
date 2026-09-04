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
      static std::array<std::string_view, 2> const directives{ "GROUP", "INPUT" };
      std::size_t parenthesis_depth{};

      for(std::size_t i{}; i < source.size(); ++i)
      {
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

  bool is_object_file(jtl::immutable_string const &path)
  {
    std::array<unsigned char, 4> magic{};
    std::ifstream file{ path.c_str(), std::ios::binary };
    if(!file.read(reinterpret_cast<char *>(magic.data()), magic.size()))
    {
      return false;
    }

    if(magic == std::array<unsigned char, 4>{ 0x7f, 'E', 'L', 'F' }
       || magic == std::array<unsigned char, 4>{ 0xfe, 0xed, 0xfa, 0xce }
       || magic == std::array<unsigned char, 4>{ 0xce, 0xfa, 0xed, 0xfe }
       || magic == std::array<unsigned char, 4>{ 0xfe, 0xed, 0xfa, 0xcf }
       || magic == std::array<unsigned char, 4>{ 0xcf, 0xfa, 0xed, 0xfe }
       || magic == std::array<unsigned char, 4>{ 0xca, 0xfe, 0xba, 0xbe }
       || magic == std::array<unsigned char, 4>{ 0xbe, 0xba, 0xfe, 0xca }
       || magic == std::array<unsigned char, 4>{ 0xca, 0xfe, 0xba, 0xbf }
       || magic == std::array<unsigned char, 4>{ 0xbf, 0xba, 0xfe, 0xca })
    {
      return true;
    }

    return magic[0] == 'M' && magic[1] == 'Z';
  }

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
